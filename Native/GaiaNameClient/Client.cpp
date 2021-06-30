#include "Client.hpp"

#include <thread>
#include <chrono>

namespace Gaia::NameService
{
    /// Get current timestamp, in the format of the count of seconds since the epoch.
    long Client::GetTimestamp()
    {
        return static_cast<long>(
                std::chrono::duration_cast<std::chrono::seconds>(
                        (std::chrono::system_clock::now() - TimestampEpoch)).count());
    }

    /// Construct and connect to the Redis server on the given address.
    Client::Client(unsigned int port, const std::string &ip)
    {
        auto new_epoch_time = std::tm{.tm_year=2018};
        TimestampEpoch = std::chrono::system_clock::from_time_t(std::mktime(&new_epoch_time));

        // Configure the connection and connect to the Redis server.
        sw::redis::ConnectionOptions options;
        options.port = static_cast<int>(port);
        options.host = ip;
        options.socket_timeout = std::chrono::milliseconds(100);

        Connection = std::make_unique<sw::redis::Redis>(options);

        // Start the background updater thread.
        UpdaterFlag = true;
        UpdaterToken = std::async(std::launch::async, [this](){
           while (this->UpdaterFlag)
           {
               // Update the timestamp of names.
               std::shared_lock lock(this->NamesMutex);
               for (const auto& name : this->Names)
               {
                    this->Connection->zadd("gaia/names", name, static_cast<double>(GetTimestamp()));
               }
               lock.unlock();
               // Update interval time is 1.5s.
               std::this_thread::sleep_for(std::chrono::milliseconds(1500));
           }
        });
    }

    /// Get all registered names.
    std::unordered_set<std::string> Client::GetNames()
    {
        std::unordered_set<std::string> names;
        Connection->zrange("gaia/names", GetTimestamp() - 3, GetTimestamp() + 1,
                           std::inserter(names, names.end()));
        return names;
    }

    /// Register a name and get the corresponding token.
    std::unique_ptr<Token> Client::HoldName(const std::string &name)
    {
        RegisterName(name);
        return std::make_unique<Token>(this, name);
    }

    /// Activate a name.
    void Client::RegisterName(const std::string &name)
    {
        std::unique_lock lock(NamesMutex);
        Names.insert(name);
        Connection->zadd("gaia/names", name, static_cast<double>(GetTimestamp()));
    }

    /// Deactivate a name.
    void Client::UnregisterName(const std::string &name)
    {
        std::unique_lock lock(NamesMutex);
        Names.erase(name);
        Connection->zrem("gaia/names", name);
    }

    /// Stop the background updater thread.
    Client::~Client()
    {
        // Stop the background updater thread.
        if (UpdaterFlag)
        {
            UpdaterFlag = false;
            if (UpdaterToken.valid())
            {
                UpdaterToken.get();
            }
        }
    }

    /// Query whether a name is valid or not.
    bool Client::HasName(const std::string &name)
    {
        auto names = GetNames();
        auto finder = names.find(name);
        return finder != names.end();
    }
}
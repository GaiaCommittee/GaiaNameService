#include "NameClient.hpp"

#include <thread>
#include <chrono>

namespace Gaia::NameService
{
    /// Get current timestamp, in the format of the count of seconds since the epoch.
    long NameClient::GetTimestamp()
    {
        return static_cast<long>(
                std::chrono::duration_cast<std::chrono::seconds>(
                        (std::chrono::system_clock::now() - TimestampEpoch)).count());
    }

    /// Construct and connect to the Redis server on the given address.
    NameClient::NameClient(unsigned int port, const std::string &ip)
    {
        auto new_epoch_time = std::tm{.tm_year=2018};
        TimestampEpoch = std::chrono::system_clock::from_time_t(std::mktime(&new_epoch_time));

        // Configure the connection and connect to the Redis server.
        sw::redis::ConnectionOptions options;
        options.port = static_cast<int>(port);
        options.host = ip;
        options.socket_timeout = std::chrono::milliseconds(100);

        Connection = std::make_unique<sw::redis::Redis>(options);
    }

    /// Get all registered names.
    std::unordered_set<std::string> NameClient::GetNames()
    {
        std::unordered_set<std::string> names;
        Connection->zrange("gaia/names", GetTimestamp() - 3, GetTimestamp() + 1,
                           std::inserter(names, names.end()));
        return names;
    }

    /// Register a name and get the corresponding token.
    std::unique_ptr<NameToken> NameClient::HoldName(const std::string &name)
    {
        return std::unique_ptr<NameToken>(new NameToken(this, name));
    }

    /// Activate a name.
    void NameClient::RegisterName(const std::string &name)
    {
        Connection->zadd("gaia/names", name, static_cast<double>(GetTimestamp()));
    }

    /// Deactivate a name.
    void NameClient::UnregisterName(const std::string &name)
    {
        Connection->zrem("gaia/names", name);
    }

    /// Query whether a name is valid or not.
    bool NameClient::HasName(const std::string &name)
    {
        auto names = GetNames();
        auto finder = names.find(name);
        return finder != names.end();
    }

    /// Update the timestamp of a name to keep it valid.
    void NameClient::UpdateName(const std::string &name)
    {
        this->Connection->zadd("gaia/names", name, static_cast<double>(GetTimestamp()));
    }
}
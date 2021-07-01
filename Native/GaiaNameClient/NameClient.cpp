#include "NameClient.hpp"

#include <thread>
#include <chrono>

namespace Gaia::NameService
{
    /// Construct and connect to the Redis server on the given address.
    NameClient::NameClient(unsigned int port, const std::string &ip)
    {
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
        unsigned long long cursor = 0;

        std::list<std::string> results;
        do
        {
            cursor = Connection->scan(0, "gaia.names/*", results.end());
        }while (cursor != 0);

        std::unordered_set<std::string> names;

        // Remove prefix "gaia.names/"
        for (const auto& result : results)
        {
            names.insert(result.substr(11));
        }

        return names;
    }

    /// Register a name and get the corresponding token.
    std::unique_ptr<NameToken> NameClient::HoldName(const std::string &name)
    {
        return std::unique_ptr<NameToken>(new NameToken(this, name));
    }

    /// Activate a name.
    void NameClient::RegisterName(const std::string &name, const std::string& address)
    {
        Connection->set("gaia.names/" + name, address, std::chrono::seconds(3));
    }

    /// Deactivate a name.
    void NameClient::UnregisterName(const std::string &name)
    {
        Connection->del("gaia.names/" + name);
    }

    /// Query whether a name is valid or not.
    bool NameClient::HasName(const std::string &name)
    {
        return Connection->exists(name);
    }

    /// Update the timestamp of a name to keep it valid.
    void NameClient::UpdateName(const std::string &name)
    {
        Connection->expire("gaia.names/" + name, std::chrono::seconds(3));
    }

    /// Get the address text of the given name.
    std::string NameClient::GetAddress(const std::string &name)
    {
        return Connection->get(name).value_or("");
    }

    /// Set the address text of the given name.
    void NameClient::SetAddress(const std::string &name, const std::string &address)
    {
        if (Connection->exists(name))
        {
            Connection->set(name, address);
        }
    }
}
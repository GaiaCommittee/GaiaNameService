#include "NameClient.hpp"

#include <thread>
#include <chrono>
#include <utility>

namespace Gaia::NameService
{
    /// Construct and connect to the Redis server on the given address.
    NameClient::NameClient(unsigned int port, const std::string &ip) :
        NameClient(std::make_shared<sw::redis::Redis>("tcp://" + ip + ":" + std::to_string(port)))
    {}

    /// Reuse the connection to a Redis server.
    NameClient::NameClient(std::shared_ptr<sw::redis::Redis> connection) : Connection(std::move(connection))
    {}

    /// Get all registered names.
    std::unordered_set<std::string> NameClient::GetNames()
    {
        unsigned long long cursor = 0;

        std::list<std::string> results;
        do
        {
            cursor = Connection->scan(0, "names/*", results.end());
        }while (cursor != 0);

        std::unordered_set<std::string> names;

        // Remove prefix "names/"
        for (const auto& result : results)
        {
            names.insert(result.substr(6));
        }

        return names;
    }

    /// Register a name and get the corresponding token.
    NameToken NameClient::HoldName(const std::string &name)
    {
        return NameToken(this, name);
    }

    /// Activate a name.
    void NameClient::RegisterName(const std::string &name, const std::string& address)
    {
        Connection->set("names/" + name, address, std::chrono::seconds(3));
    }

    /// Deactivate a name.
    void NameClient::UnregisterName(const std::string &name)
    {
        Connection->del("names/" + name);
    }

    /// Query whether a name is valid or not.
    bool NameClient::HasName(const std::string &name)
    {
        return Connection->exists(name);
    }

    /// Update the timestamp of a name to keep it valid.
    void NameClient::UpdateName(const std::string &name)
    {
        Connection->expire("names/" + name, std::chrono::seconds(3));
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
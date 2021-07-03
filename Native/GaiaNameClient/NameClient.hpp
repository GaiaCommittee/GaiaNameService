#pragma once

#include <string>
#include <unordered_set>
#include <memory>
#include <future>
#include <atomic>
#include <shared_mutex>
#include <chrono>
#include <sw/redis++/redis++.h>

#include "NameToken.hpp"

namespace Gaia::NameService
{
    /**
     * @brief NameClient for name resolving service.
     * @details
     *  This client instance represents a connection to a Redis server.-
     */
    class NameClient
    {
    private:
        friend class NameToken;

    protected:
        /// Connection to Redis server, default address is '127.0.0.1:6379'
        std::shared_ptr<sw::redis::Redis> Connection;

    private:
        /// Activate a name.
        void RegisterName(const std::string& name, const std::string& address = "");
        /// Deactivate a name.
        void UnregisterName(const std::string& name);
        /// Update the timestamp of a name to keep it valid.
        void UpdateName(const std::string& name);

    public:
        /**
         * @brief Construct and try to connect to the Redis server on the given address.
         * @param port The port of the redis server.
         * @param ip The ip of the redis server.
         */
        explicit NameClient(unsigned int port = 6379, const std::string& ip = "127.0.0.1");
        /// Reuse the connection to a Redis server.
        explicit NameClient(std::shared_ptr<sw::redis::Redis> connection);

        /**
         * @brief Query all registered names.
         * @return Set of valid names which have not been expired yet.
         * @attention This is a time consuming function.
         */
        std::unordered_set<std::string> GetNames();

        /**
         * @brief Query whether a name is valid or not.
         * @retval true The given name is valid.
         * @retval false The given name does not exist.
         * @attention This is a time consuming function.
         */
        bool HasName(const std::string& name);

        /**
         * @brief Register a name and get the corresponding token.
         * @param name The name to take up.
         * @return The corresponding token to the given name.
         */
        NameToken HoldName(const std::string& name);

        /**
         * @brief Get the address text of the given name.
         * @param name The name to query.
         * @return The address text of given name, maybe empty.
         */
        std::string GetAddress(const std::string& name);

        /**
         * @brief Set the address text of the given name.
         * @param name The name to set.
         * @details This function will not update the timestamp of the given name,
         *          nor create this name if it does not exist.
         */
        void SetAddress(const std::string& name, const std::string& address);
    };
}

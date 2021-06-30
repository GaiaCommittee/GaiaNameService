#pragma once

#include <string>
#include <unordered_set>
#include <memory>
#include <future>
#include <atomic>
#include <shared_mutex>
#include <chrono>
#include <sw/redis++/redis++.h>

#include "Token.hpp"

namespace Gaia::NameService
{
    /**
     * @brief Client for name resolving service.
     * @details
     *  This client instance represents a connection to a Redis server.-
     */
    class Client
    {
    private:
        friend class Token;

        /// The starting time point of the timestamp.
        std::chrono::system_clock::time_point TimestampEpoch;

        /// Get current timestamp, in the format of the count of seconds since the epoch.
        long GetTimestamp();

    protected:
        /// Connection to Redis server, default address is '127.0.0.1:6379'
        std::unique_ptr<sw::redis::Redis> Connection;

    private:
        /// Token for the background updater thread.
        std::future<void> UpdaterToken {};
        /// Life flag for the background updater thread.
        std::atomic<bool> UpdaterFlag {false};

        /// Mutex for names set.
        std::shared_mutex NamesMutex;
        /// Valid names.
        std::unordered_set<std::string> Names;

        /// Activate a name.
        void RegisterName(const std::string& name);
        /// Deactivate a name.
        void UnregisterName(const std::string& name);

    public:
        /**
         * @brief Construct and try to connect to the Redis server on the given address.
         * @param port The port of the redis server.
         * @param ip The ip of the redis server.
         */
        explicit Client(unsigned int port = 6379, const std::string& ip = "127.0.0.1");
        /// Stop the background updater thread.
        virtual ~Client();

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
        std::unique_ptr<Token> HoldName(const std::string& name);
    };
}

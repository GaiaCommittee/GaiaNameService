#pragma once

#include <future>
#include <atomic>
#include <string>
#include <memory>

namespace Gaia::NameService
{
    class Client;

    /**
     * @brief Token for a registered name.
     * @details
     *  A token represents a registered name, and also will automatically notify the client to
     *  remove the name when be destructed.
     *  A token should only be gotten from the name service client,
     *  for there are many background jobs to do in order to register a name.
     */
    class Token
    {
        friend class Client;

    protected:
        /// Pointer to the host name service client.
        Client* Host;
        /// Token for the background updater thread.
        std::future<void> UpdaterToken {};
        /// Life flag for the background updater thread.
        std::atomic<bool> UpdaterFlag {false};

        /// Construct and bind the name.
        Token(Client* host, std::string name) noexcept;

    public:
        /// Destruct and notify the host client to unregister the bound name.
        ~Token();
        /// Corresponding name.
        const std::string Name;

        /// Update the timestamp of this name to keep it valid.
        void Update();

        /// Start the background updater thread.
        void StartBackgroundUpdater();
        /// Stop the background updater thread.
        void StopBackgroundUpdater();
        /// Check whether the background updater is running or not.
        bool IsBackgroundUpdaterRunning();
    };
}
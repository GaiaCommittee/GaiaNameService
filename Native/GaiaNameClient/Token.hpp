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

    public:
        /// Construct and bind the name.
        explicit Token(Client* host, std::string  name);
        /// Destruct and notify the host client to unregister the bound name.
        ~Token();
        /// Corresponding name.
        const std::string Name;
    };
}
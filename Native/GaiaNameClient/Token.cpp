#include "Token.hpp"

#include <utility>
#include "Client.hpp"

namespace Gaia::NameService
{
    /// Construct and bind the name.
    Token::Token(Client* host, std::string name) :
        Host(host), Name(std::move(name))
    {}

    /// Destruct and notify the host client to unregister the bound name.
    Token::~Token()
    {
        if (Host)
        {
            Host->UnregisterName(Name);
        }
    }
}
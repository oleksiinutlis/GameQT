#include <iostream>
#include <map>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;


class IClientSession
{
public:
    virtual void sendMessage( std::string message ) = 0;
    virtual ~IClientSession() = default;
};

class IGame
{
protected:
    virtual ~IGame() = default;

public:
    virtual void handlePlayerMessage( IClientSession&, boost::asio::streambuf& message ) = 0;
};

class IPlayerInterface
{
protected:
    virtual ~IPlayerInterface() = default;

public:
    virtual void handleServerMessage( boost::asio::streambuf& message ) = 0;
};
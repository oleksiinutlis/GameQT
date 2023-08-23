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
    virtual void onPlayerMessage( IClientSession&, boost::asio::streambuf& message ) = 0;
};


#include <iostream>
#include <map>
#include <boost/asio.hpp>

#pragma once

#define GAME_STARTED_CMD    "GameStarted"
#define UPDATE_SCENE_CMD    "UpdateScene"

using namespace boost::asio;
using ip::tcp;

class IClientSession
{
public:
    virtual void sendMessage( std::string message ) = 0;
    virtual void sendMessage( std::shared_ptr<boost::asio::streambuf> wrStreambuf ) = 0;
    virtual ~IClientSession() = default;
};

class IGame
{
protected:
    virtual ~IGame() = default;

public:
    virtual void handlePlayerMessage( IClientSession&, boost::asio::streambuf& message ) = 0;
};

class IClientPlayer
{
protected:
    virtual ~IClientPlayer() = default;

public:
    virtual void handleServerMessage( const std::string& command, boost::asio::streambuf& message ) = 0;
    virtual const std::string& playerName() const = 0;
};
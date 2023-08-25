#include <iostream>
#include <chrono>
#include <thread>

#include "Client.h"
#include "Server.h"
#include "Game.h"


int main()
{
    Game game;
    
    std::thread( [&game]
    {
        TcpServer server(game,1234);
        server.execute();
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread( []
    {
        Client client;
        client.execute( "127.0.0.1", 1234, "StartGame;001;800;600;" );
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Client client2;
    client2.execute( "127.0.0.1", 1234, "StartGame;001;1000;800;" );

    return 0;
}

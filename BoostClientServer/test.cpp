#include <iostream>
#include <chrono>
#include <thread>

#include "Client.h"
#include "Server.h"
#include "Game.h"
#include "ClientPlayer.h"

ClientPlayer player1{"player1"};
ClientPlayer player2{"player2"};


int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::istream::sync_with_stdio(false);
    
    io_context serverIoContext;

    Game game(serverIoContext);
    
    std::thread( [&game,&serverIoContext]
    {
        TcpServer server(serverIoContext,game,1234);
        server.execute();
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::thread( []
    {
        Client client(player1);
        player1.setTcpClient(&client);
        client.execute( "127.0.0.1", 1234, "StartGame;001;800;600;" );
    }).detach();


    std::thread( []
    {
        Client client2(player2);
        player2.setTcpClient(&client2);
        client2.execute( "127.0.0.1", 1234, "StartGame;001;1000;800;" );
    }).detach();


    std::this_thread::sleep_for(std::chrono::milliseconds(100000000));

    return 0;
}

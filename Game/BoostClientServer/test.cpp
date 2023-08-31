#include <iostream>
#include <chrono>
#include <thread>

#include "Client.h"
#include "Server.h"
#include "Game.h"
#include "ClientPlayer.h"

int main()
{
    // server
    std::thread( []
    {
        io_context serverIoContext;
        Game game(serverIoContext);
        
        TcpServer server(serverIoContext,game,1234);
        server.execute();
    }).detach();

    //std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // clients
    {
        io_context  ioContext;
        
        ClientPlayer player1{"player1"};
        ClientPlayer player2{"player2"};

        Client client1( ioContext, player1 );
        player1.setTcpClient(&client1);
        client1.execute( "127.0.0.1", 1235, "StartGame;001;800;600;" );
        
        Client client2( ioContext, player2 );
        player2.setTcpClient(&client2);
        client2.execute( "127.0.0.1", 1234, "StartGame;001;1000;800;" );
        
        ioContext.run();
    }

    return 0;
}

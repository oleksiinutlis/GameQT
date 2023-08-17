#include <iostream>
#include <chrono>
#include <thread>

#include "Client.h"
#include "Server.h"


int main()
{
    std::thread( []
    {
        TcpServer server(12345);
        server.execute();
    }).detach();

    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::thread( []
    {
        Client client;
        client.execute( "127.0.0.1", 12345, "StartGame;001;800;600;" );
    }).detach();

    Client client2;
    client2.execute( "127.0.0.1", 12345, "StartGame;001;1000;800;" );

    return 0;
}

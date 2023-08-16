#include <iostream>
#include <chrono>
#include <thread>

#include "Client.h"
#include "Server.h"


int main()
{
    std::thread( []
    {
        boost::asio::io_service io_service;
        Server server(io_service, 12345);
        io_service.run();
    }).detach();
    
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
//    std::thread( []
//    {
//        Client client;
//        client.execute( "127.0.0.1", 12345, "StartGame;001;800;600;" );
//    }).detach();

//    Client client2;
//    client2.execute( "127.0.0.1", 12345, "StartGame;001;1000;800;" );

    try {
        io_context ioContext;
        tcp::socket socket(ioContext);

        // Connect to the server
        socket.connect(tcp::endpoint(ip::address::from_string("127.0.0.1"), 12345));

        // Send a message to the server
        std::string message = "Hello from client!;\n";
        write(socket, buffer(message));

        // Receive the response from the server
        char data[1024];
        size_t len = socket.read_some(buffer(data));
        std::cout << "Received from server: " << data << std::endl;

        len = socket.read_some(buffer(data));
        std::cout << "(2) Received from server: " << data << std::endl;

        // Close the connection
        socket.close();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

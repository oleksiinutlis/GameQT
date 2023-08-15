#include <iostream>
#include <boost/asio.hpp>


using namespace boost::asio;
using ip::tcp;

class Client
{
    io_context ioContext;
    //boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work;
    tcp::socket socket;
    boost::asio::streambuf streambuf;
    
public:
    Client() :
        ioContext(),
        //m_work( boost::asio::make_work_guard(ioContext) ),
        socket(ioContext)
    {}
    
    // Close the connection
    //socket.close();
    

    void execute( std::string addr, int port, std::string greeting )
    {
        socket = tcp::socket(ioContext);

        // Connect to the server
        socket.connect( tcp::endpoint(ip::address::from_string("127.0.0.1"), 12345) );

        // Send a message to the server
        write( socket, buffer(greeting+"\n") );
        
        readResponse();
        readResponse();
//
//        ioContext.run();

        
        
        socket = tcp::socket(ioContext);
        auto endpoint = tcp::endpoint(ip::address::from_string( addr.c_str()), port);

        socket.async_connect(endpoint, [this,greeting=greeting] (const boost::system::error_code& error)
        {
            if ( error )
            {
                std::cout << "Connection error: " << error.message() << std::endl;
            }
            else
            {
                std::cout << "Connected to the server!" << std::endl;
                async_write( socket, boost::asio::buffer( greeting+"\n" ), [this] ( const boost::system::error_code& error, std::size_t bytes_transferred )
                {
                    if ( error )
                    {
                        std::cout << "Client write error: " << error.message() << std::endl;
                    }
                    else
                    {
                        readResponse();
                    }
                });
            }
        });
        
        ioContext.run();
    }
    
    void readResponse()
    {
        // Receive the response from the server
        char data[1024];
        size_t len = socket.read_some(buffer(data,1024));
        std::cout << "Received: " << std::string( data, len ) << std::endl;
        readResponse();
        return;

        boost::asio::async_read_until( socket, streambuf, '\n',
          [this]( const boost::system::error_code& error_code, std::size_t bytes_transferred )
        {
            if ( error_code )
            {
                std::cout << "Client read error: " << error_code.message() << std::endl;
            }
            else
            {
                {
                    std::istream response( &streambuf );

                    std::string command;
                    std::getline( response, command, ';' );

                    std::cout << "response command: " << command << std::endl;
                }
                
                streambuf.consume(bytes_transferred);
                readResponse();
            }
        });
    }
};

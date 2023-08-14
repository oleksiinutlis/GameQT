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
        
        post( ioContext, [this] { readResponse(); } );

        ioContext.run();
    }
    
    void readResponse()
    {
        // Receive the response from the server
        boost::asio::async_read_until( socket, streambuf, '\n',
          [this]( const boost::system::error_code& error_code, std::size_t bytes_transferred )
        {
            if ( ! error_code )
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

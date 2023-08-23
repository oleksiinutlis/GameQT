#include <iostream>
#include <boost/asio.hpp>


using namespace boost::asio;
using ip::tcp;

class Client
{
    io_context  m_ioContext;
    tcp::socket m_socket;
    
    boost::asio::streambuf m_wrStreambuf;
    boost::asio::streambuf m_streambuf;

public:
    Client() :
        m_ioContext(),
        m_socket(m_ioContext)
    {}
    
    ~Client()
    {
        std::cout << "!!!! ~Client(): " << std::endl;
    }
    
    void execute( std::string addr, int port, std::string greeting )
    {
        m_socket = tcp::socket(m_ioContext);
        auto endpoint = tcp::endpoint(ip::address::from_string( addr.c_str()), port);

        m_socket.async_connect(endpoint, [this,greeting=greeting] (const boost::system::error_code& error)
        {
            if ( error )
            {
                std::cout << "Connection error: " << error.message() << std::endl;
            }
            else
            {
                std::shared_ptr<boost::asio::streambuf> wrStreambuf = std::make_shared<boost::asio::streambuf>();
                std::ostream os(&(*wrStreambuf));
                os << greeting+"\n";

                std::cout << "Connected to the server!" << std::endl;
                async_write( m_socket, *wrStreambuf,
                    [this,wrStreambuf] ( const boost::system::error_code& error, std::size_t bytes_transferred )
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

        m_ioContext.run();
    }
    
    void readResponse()
    {
        // Receive the response from the server

        boost::asio::async_read_until( m_socket, m_streambuf, '\n',
          [this]( const boost::system::error_code& error_code, std::size_t bytes_transferred )
        {
            if ( error_code )
            {
                std::cout << "Client read error: " << error_code.message() << std::endl;
            }
            else
            {
                {
                    std::istream response( &m_streambuf );

                    std::string command;
                    std::getline( response, command, ';' );

                    std::cout << "response command: " << command << std::endl;
                }

                m_streambuf.consume(bytes_transferred);
                readResponse();
            }
        });
    }
};

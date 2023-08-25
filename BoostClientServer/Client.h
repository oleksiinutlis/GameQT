#include <iostream>
#include <boost/asio.hpp>
#include "Interfaces.h"

using namespace boost::asio;
using ip::tcp;

class Client
{
    io_context  m_ioContext;
    tcp::socket m_socket;
    IClientPlayer* m_player;
    boost::asio::streambuf m_wrStreambuf;
    boost::asio::streambuf m_streambuf;

public:
    Client( IClientPlayer& player ) :
        m_ioContext(),
        m_socket(m_ioContext),
        m_player(&player)
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
                
                sendMessageToServer( wrStreambuf );
            }
        });

        m_ioContext.run();
    }
    
    void sendMessageToServer( std::shared_ptr<boost::asio::streambuf> streambuf )
    {
        std::cout << "Connected to the server!" << std::endl;
        async_write( m_socket, *streambuf,
            [this,streambuf] ( const boost::system::error_code& error, std::size_t bytes_transferred )
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

                    std::cout << "#CLIENT: RECIEVED FROM SERVER: " << command << std::endl;
                    m_player->handleServerMessage(m_streambuf);

                }

                m_streambuf.consume(bytes_transferred);
                readResponse();
            }
        });
    }
};

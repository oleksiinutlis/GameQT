#include <iostream>
#include <map>
#include <boost/asio.hpp>
#include "Interfaces.h"

using namespace boost::asio;
using ip::tcp;

class ClientSession : public std::enable_shared_from_this<ClientSession>, public IClientSession
{
    IGame&                  m_game;
    tcp::socket             m_socket;
    boost::asio::streambuf  m_streambuf;

public:
    ClientSession( IGame& game, tcp::socket&& socket)
      : m_game(game),
        m_socket(std::move(socket))
    {}

    ~ClientSession() { std::cout << "!!!! ~ClientSession()" << std::endl; }

    virtual void sendMessage( std::string command ) override
    {
        auto self(shared_from_this());

        std::shared_ptr<boost::asio::streambuf> wrStreambuf = std::make_shared<boost::asio::streambuf>();
        std::ostream os(&(*wrStreambuf));
        os << command;

        async_write( m_socket, *wrStreambuf,
            [this,self,wrStreambuf] ( const boost::system::error_code& ec, std::size_t bytes_transferred  )
            {
                if ( ec )
                {
                    std::cout << "!!!! ClientSession::sendMessage error: " << ec.message() << std::endl;
                    exit(-1);
                }
            });
    }

    void readMessage()
    {
        auto self(shared_from_this());

        async_read_until( m_socket, m_streambuf, '\n',
            [this,self] ( const boost::system::error_code& ec, std::size_t bytes_transferred )
            {
                if ( ec )
                {
                    std::cout << "!!!! ClientSession::readMessage error: " << ec.message() << std::endl;
                    exit(-1);
                }
                else
                {
                    // handle message
                    m_game.handlePlayerMessage( *this, m_streambuf );
                    m_streambuf.consume(bytes_transferred);

                    // read next message
                    readMessage();
                }
        });
    }
};


class TcpServer
{
    IGame&          m_game;
    
    io_context      m_ioContext;
    tcp::acceptor   m_acceptor;
    tcp::socket     m_socket;
    
    std::vector<std::shared_ptr<ClientSession>> m_sessions;

public:
    TcpServer( IGame& game, int port ) :
        m_game(game),
        m_ioContext(),
        m_acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port) ),
        m_socket(m_ioContext)
    {
        accept();
    }

    void execute()
    {
        post( m_ioContext, [this] { accept(); } );
        m_ioContext.run();
    }
    
    void accept()
    {
        m_acceptor.async_accept(m_socket, [this](boost::system::error_code ec) {
            if (!ec)
            {
                std::make_shared<ClientSession>( m_game, std::move(m_socket) )->readMessage();
            }

            accept();
        });
    }
};

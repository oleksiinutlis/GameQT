#include <iostream>
#include <map>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class IClientSession
{
public:
    virtual void sendMessage( const std::string& message ) = 0;
};

class IGame
{
public:
    virtual void handleMessage( const std::string& command, std::istream& request, IClientSession& client ) = 0;
};

class ClientSession: public IClientSession
{
    IGame&                  m_game;
    tcp::socket             m_socket;
    boost::asio::streambuf  m_streambuf;

public:
    ClientSession( IGame& game, io_context& ioContext ) : m_game(game), m_socket( ioContext )
    {
    }
    
    tcp::socket& socket() { return m_socket; }
    
    virtual void sendMessage( const std::string& command ) override
    {
        async_write( m_socket, buffer( command+"\n"),
            [] ( const boost::system::error_code& error_code, std::size_t bytes_transferred  )
            {
            });
    }

    void readClientRequest()
    {
        async_read_until( m_socket, m_streambuf, '\n',
            [this] ( const boost::system::error_code& ec, std::size_t bytes_transferred )
            {
                if ( ec )
                {
                    std::cout << "!!!! readClientRequest error: " << ec.message() << std::endl;
                    exit(-1);
                }
                else
                {
                    std::cout << "Received: " << (const char*)m_streambuf.data().data() << std::endl;

//todo
//                std::istream request( &m_streambuf );
//
//                std::string command;
//                std::getline( request, command, ';' );
//
//                std::cout << "command: " << command << std::endl;
//
//                    m_game.handleMessage( command, request, *this );
                    
                    sendMessage( "WaitingSecondPlayer;" );
                }
        });
    }
};

class Server
{
    IGame&          m_game;
    io_context      ioContext;
    tcp::acceptor   acceptor;
    
    std::vector<ClientSession*> m_sessions;

public:
    Server( IGame& game, int port ) :
        m_game(game),
        acceptor( ioContext, tcp::endpoint(tcp::v4(), port) )
    {
    }

    void start()
    {
        post( ioContext, [this] { accept(); } );
    }
    
    void execute()
    {
        ioContext.run();
    }
    
    void accept()
    {
        auto* clientSession = new ClientSession( m_game, ioContext );
        m_sessions.push_back( clientSession );
        
        acceptor.async_accept( clientSession->socket(), [ this, clientSession ] ( boost::system::error_code ec )
        {
            if ( ec )
            {
                std::cout << "!!!! async_accept error: " << ec.message() << std::endl;
                exit(-1);
            }
            std::cout << "Client connected: " << clientSession->socket().remote_endpoint() << std::endl;
            clientSession->readClientRequest();

            accept();
        });
    }
};

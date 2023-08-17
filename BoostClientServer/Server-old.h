#include <iostream>
#include <map>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class IClientSession
{
public:
    virtual void sendMessage( std::string message ) = 0;
    virtual ~IClientSession() = default;
};

class ClientSession : public std::enable_shared_from_this<ClientSession> //: public IClientSession
{
    tcp::socket             m_socket;
    boost::asio::streambuf  m_streambuf;

public:
    ClientSession(tcp::socket&& socket) : m_socket(std::move(socket)) {}

    ~ClientSession() { std::cout << "!!!! ~ClientSession()" << std::endl; }

    virtual void sendMessage( std::string command )
    {
        auto self(shared_from_this());

        boost::asio::streambuf streambuf;
        std::ostream os(&streambuf);
        os << command+"\n";
        
        async_write( m_socket, streambuf,
            [this,self] ( const boost::system::error_code& ec, std::size_t bytes_transferred  )
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
                    std::cout << "Received: " << std::string( (const char*)m_streambuf.data().data(), m_streambuf.size() ) << std::endl;
                    m_streambuf.consume(bytes_transferred);
                    sendMessage( "WaitingSecondPlayer;" );
                    readMessage();
                }
        });
    }
};


class TcpServer
{
    io_context      m_ioContext;
    tcp::acceptor   acceptor;
    tcp::socket     m_socket;
    
    std::vector<std::shared_ptr<ClientSession>> m_sessions;

public:
    TcpServer( int port ) :
        m_ioContext(),
        acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port) ),
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
        acceptor.async_accept(m_socket, [this](boost::system::error_code ec) {
            if (!ec)
            {
                std::make_shared<ClientSession>(std::move(m_socket))->readMessage();
            }

            accept();
        });
    }
};

#include <iostream>
#include <map>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

//class IClientSession
//{
//public:
//    virtual void sendMessage( std::string message ) = 0;
//    virtual ~IClientSession() = default;
//};

//class ClientSession //: public IClientSession
//{
//    tcp::socket             m_socket;
//    boost::asio::streambuf  m_streambuf;
//
//public:
//    ClientSession( tcp::socket&& socket ) : m_socket( std::move(socket) )
//    {
//    }
//
//    ~ClientSession()
//    {
//        std::cout << "!!!! ~ClientSession()" << std::endl;
//    }
//
//    tcp::socket& socket() { return m_socket; }
//
//    virtual void sendMessage( std::string command )
//    {
//        async_write( m_socket, buffer( command+"\n" ),
//            [] ( const boost::system::error_code& ec, std::size_t bytes_transferred  )
//            {
//                if ( ec )
//                {
//                    std::cout << "!!!! ClientSession::sendMessage error: " << ec.message() << std::endl;
//                    exit(-1);
//                }
//            });
//    }
//
//    void readMessage()
//    {
//        async_read_until( m_socket, m_streambuf, '\n',
//            [this] ( const boost::system::error_code& ec, std::size_t bytes_transferred )
//            {
//                if ( ec )
//                {
//                    std::cout << "!!!! ClientSession::readMessage error: " << ec.message() << std::endl;
//                    exit(-1);
//                }
//                else
//                {
//                    std::cout << "Received: " << std::string( (const char*)m_streambuf.data().data(), m_streambuf.size() ) << std::endl;
//                    sendMessage( "WaitingSecondPlayer;" );
//                }
//        });
//    }
//};

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
    tcp::socket socket_;
    boost::asio::streambuf  m_streambuf;
//    enum { max_length = 1024 };
//    char data_[max_length];

public:
    explicit ClientSession(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    ~ClientSession()
    {
        std::cout << "~Session";
    }

    void start()
    {
        doRead();
    }

private:
    void doRead()
    {
        auto self(shared_from_this());
        async_read_until( socket_, m_streambuf, '\n',
        //socket_.async_read_some(boost::asio::buffer(data_, max_length-1),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec)
                                    {
                                        std::cout << "Received: " << std::string( (const char*)m_streambuf.data().data(), m_streambuf.size() ) << std::endl;
                                        doWrite(length);
                                    }
                                });

//        socket_.async_read_some( boost::asio::buffer(data_, max_length-1),
//                                [this, self](boost::system::error_code ec, std::size_t length) {
//                                    if (!ec)
//                                    {
//                                        std::cout << "Received from client: " << m_streambuf.data().data() << std::endl;
//                                        doWrite(length);
//                                    }
//                                });
    }

    void doWrite(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, m_streambuf, //boost::asio::buffer(data_, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec)
                                     {
                                         doRead();
                                     }
                                 });
    }
};


class TcpServer
{
    tcp::acceptor   acceptor;
    tcp::socket     m_socket;
    
//    std::vector<ClientSession*> m_sessions;

public:
    TcpServer( boost::asio::io_service&  ioContext, int port ) :
        acceptor( ioContext, tcp::endpoint(tcp::v4(), port) ),
        m_socket(ioContext)
    {
        accept();
    }

//    void execute()
//    {
//        post( ioContext, [this] { accept(); } );
//        ioContext.run();
//    }
    
    void accept()
    {
        acceptor.async_accept(m_socket, [this](boost::system::error_code ec) {
            if (!ec)
            {
                std::make_shared<ClientSession>(std::move(m_socket))->start();
            }

            accept();
        });

//        acceptor.async_accept( m_socket, [this] ( boost::system::error_code ec )
//        {
//            if ( ec )
//            {
//                std::cout << "!!!! async_accept error: " << ec.message() << std::endl;
//                exit(-1);
//            }
//            //std::cout << "Client connected: " << clientSession->socket().remote_endpoint() << std::endl;
//
//            auto* clientSession = new ClientSession( std::move(m_socket) );
//            m_sessions.push_back( clientSession );
//
//            clientSession->readMessage();
//
//            accept();
//        });
    }
};

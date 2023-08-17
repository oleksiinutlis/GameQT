#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    ~Session()
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
        socket_.async_read_some(boost::asio::buffer(data_, max_length-1),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec)
                                    {
                                        data_[length] = 0;
                                        std::cout << "Received from client: " << data_ << std::endl;
                                        doWrite(length);
                                    }
                                });
    }

    void doWrite(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                                     if (!ec)
                                     {
                                         doRead();
                                     }
                                 });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class Server
{
public:
    Server(boost::asio::io_service& io_service, short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service)
    {
        doAccept();
    }

private:
    void doAccept()
    {
        acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
            if (!ec)
            {
                std::make_shared<Session>(std::move(socket_))->start();
            }

            doAccept();
        });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

//int main()
//{
//    try
//    {
//        boost::asio::io_service io_service;
//        Server server(io_service, 1234);
//        io_service.run();
//    }
//    catch (std::exception& e)
//    {
//        std::cerr << "Exception: " << e.what() << std::endl;
//    }
//
//    return 0;
//}

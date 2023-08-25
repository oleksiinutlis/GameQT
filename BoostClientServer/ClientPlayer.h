#include <iostream>
#include <boost/asio.hpp>
#include "Interfaces.h"

class Client;

class ClientPlayer : public IClientPlayer
{
    bool m_isLeftPlayer = false;
    
    Client* m_tcpClient;
    
public:
    void setTcpClient( Client* tcpClient ) { m_tcpClient = tcpClient; }
    
    void sendBallMessage( double x, double y )
    {
        std::shared_ptr<boost::asio::streambuf> wrStreambuf = std::make_shared<boost::asio::streambuf>();
        std::ostream os(&(*wrStreambuf));
        os << "Ball;" << x << ";" << y << ";\n";

        m_tcpClient->sendMessageToServer(wrStreambuf);
    }
    
protected:
    
    virtual void handleServerMessage( boost::asio::streambuf& message ) override
    {
        LOG("Client: Recieved from server: " << std::string((const char*)message.data().data(), message.size()) << std::endl);
        std::istringstream input;
        input.str(std::string((const char*)message.data().data(), message.size()));

        std::string command;
        std::getline(input, command, ';');
        
        if (command == "GameStarted")
        {
            std::string direction;
            std::getline(input, direction, ';');
            
            if ( command == "left" )
            {
                m_isLeftPlayer = true;
            }
            else {
                m_isLeftPlayer = false;
            }
            //TODO
        }
        else if (command == "Ball")
        {
            std::string number;

            std::getline(input, number, ';');
            double x = std::stod(number);

            std::getline(input, number, ';');
            double y = std::stod(number);
            
            LOG( "Ball: " << this << "  :" << x << " " << y );

            //TODO : QT->CircleWidget.setX etc...
        }
        else if (command == "Score")
        {
            std::getline(input, command, ';');
            int leftScore = std::stoi(command);

            std::getline(input, command, ';');
            int rightScore = std::stoi(command);

            // TODO : Qt -> set score
            // TODO : Qt -> set x and y to the beginning location
        }
    }
};


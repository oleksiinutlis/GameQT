#include <iostream>
#include <boost/asio.hpp>
#include "Interfaces.h"

class Client;

class ClientPlayer : public IClientPlayer
{
    std::string m_playerName;
    
    bool m_isLeftPlayer = false;
    
    Client* m_tcpClient = nullptr;
    
public:
    ClientPlayer( std::string playerName ) : m_playerName(playerName) {}
    
    void setTcpClient( Client* tcpClient ) { m_tcpClient = tcpClient; }
    
    void sendBallMessage( double x, double y )
    {
        std::shared_ptr<boost::asio::streambuf> wrStreambuf = std::make_shared<boost::asio::streambuf>();
        std::ostream os(&(*wrStreambuf));
        os << "Ball;" << x << ";" << y << ";\n";

        m_tcpClient->sendMessageToServer(wrStreambuf);
    }
    
protected:
    
    int couner = 0;
    
    virtual void handleServerMessage( const std::string& command, boost::asio::streambuf& message ) override
    {
        LOG("Client: Recieved from server: " << m_playerName << ": " << std::string((const char*)message.data().data(), message.size()) << std::endl);
        std::istringstream input;
        input.str(std::string((const char*)message.data().data(), message.size()));

        if (command == "WaitingSecondPlayer")
        {
        }
        else if (command == "GameStarted")
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
            
            LOG( "Ball: " << m_playerName << "  :" << x << " " << y );

            //TODO : QT->CircleWidget.setX etc...
        }
        else if (command == "Score")
        {
            std::string score;
            std::getline(input, score, ';');
            int leftScore = std::stoi(score);

            std::getline(input, score, ';');
            int rightScore = std::stoi(score);

            // TODO : Qt -> set score
            // TODO : Qt -> set x and y to the beginning location
        }
    }
};


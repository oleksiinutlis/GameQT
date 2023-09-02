#include <iostream>
#include <boost/asio.hpp>
#include "Interfaces.h"

class TcpClient;

class ClientPlayer : public IClientPlayer
{
    std::string m_playerName;
    
    bool m_isLeftPlayer = false;
    
    TcpClient* m_tcpClient = nullptr;
    
public:
    ClientPlayer( std::string playerName ) : m_playerName(playerName) {}
    
    const std::string& playerName() const override { return m_playerName; }

    void setTcpClient( TcpClient* tcpClient ) { m_tcpClient = tcpClient; }
    
    void sendBallMessage( double x, double y )
    {
        std::shared_ptr<boost::asio::streambuf> wrStreambuf = std::make_shared<boost::asio::streambuf>();
        std::ostream os(&(*wrStreambuf));
        os << "Ball;" << x << ";" << y << ";\n";

        m_tcpClient->sendMessageToServer(wrStreambuf);
    }
    
protected:
    
    int counter = 0;
    
    virtual void handleServerMessage( const std::string& command, boost::asio::streambuf& message ) override
    {
        LOG("Client: Recieved from server: " << m_playerName << ": " << command << " " << std::string((const char*)message.data().data(), message.size()-1) );
        std::istringstream input;
        input.str(std::string((const char*)message.data().data(), message.size()));

        if (command == "WaitingSecondPlayer")
        {
        }
        else if (command == GAME_STARTED_CMD)
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
        else if (command == UPDATE_SCENE_CMD)
        {
            std::string number;

            std::getline(input, number, ';');
            double x = std::stod(number);

            std::getline(input, number, ';');
            double y = std::stod(number);
            
            LOG( "Ball: " << ++counter << ": " << m_playerName << "  :" << x << " " << y << "\n" );

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

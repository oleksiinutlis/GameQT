#include <iostream>
#include <chrono>
#include <thread>

#include "Client.h"
#include "Server.h"

//------------------------------------------------------------------
// Client->Server: StartGame, MatchId, width, haight ( GameId random sequesnce)
// Server->Client: WaitingGame
//------------------------------------------------------------------
// Server->Client: GameStarded, direction [left|right]
// Server->Client: Ball, x,y,dx,dy
// Client->Server: ToolPosition, x, y
// Server->Client: Score, number1, number2
//------------------------------------------------------------------

struct Match
{
    std::string m_matchId;
    
    IClientSession* m_leftPlayer = nullptr;
    IClientSession* m_rightPlayer = nullptr;
    
    int m_width;
    int m_height;
};

class Game : public IGame
{
    std::map<std::string,Match> m_matches;
    
    std::map< IClientSession*, std::pair<Match*,int> > m_players;

    virtual std::string handleMessage( const std::string& command, std::istream& request, IClientSession& client ) override
    {
        if ( command == "StartGame" )
        {
            std::string matchId;
            if ( getline( request, matchId, ';' ).eof() )
            {
                std::cout << "Bad request; is ignored:" << std::endl;
                return "";
            }
            
            std::string widthStr;
            if ( getline( request, widthStr, ';' ).eof() )
            {
                std::cout << "Bad request; is ignored:" << std::endl;
                return "";
            }
            int width = stoi(widthStr);

            std::string heightStr;
            if ( getline( request, heightStr, ';' ).eof() )
            {
                std::cout << "Bad request; is ignored:" << std::endl;
                return "";
            }
            int height = stoi(heightStr);


            if ( auto matchIt = m_matches.find( matchId ); matchIt == m_matches.end() )
            {
                m_matches[matchId] = Match{matchId,&client,nullptr,width,height};
                return "WaitingSecondPlayer;";
            }
            else
            {
                matchIt->second.m_rightPlayer = &client;
                matchIt->second.m_width = std::min( matchIt->second.m_width, width );
                matchIt->second.m_height = std::min( matchIt->second.m_width, height );
                matchIt->second.m_leftPlayer->sendMessage( "GameStarted;left;" + std::to_string(matchIt->second.m_width) + ";" + std::to_string(matchIt->second.m_height) + ";" );
                return "GameStarted;right;" + std::to_string(matchIt->second.m_width) + ";" + std::to_string(matchIt->second.m_height) + ";" ;
            }
            
        }
        
    }
};



int main()
{
    Game game;
    Server server{game,12345};
    server.start();

    std::thread( [&server]
    {
        server.execute();
    }).detach();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::thread( [&server]
    {
        Client client;
        client.execute( "127.0.0.1", 12345, "StartGame;001;800;600;" );
    }).detach();

    Client client2;
    client2.execute( "127.0.0.1", 12345, "StartGame;001;1000;800;" );

    return 0;
}

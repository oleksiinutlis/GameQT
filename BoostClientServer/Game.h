#include <iostream>
#include <map>
#include <list>
#include <optional>

#include "Log.h"

//------------------------------------------------------------------
// Client->Server: StartGame, MatchId, width, height ( GameId random sequesnce)
// Server->Client: WaitingGame
//------------------------------------------------------------------
// Server->Client: GameStarded, direction [left|right]
// Server->Client: Ball, x,y,dx,dy
// Client->Server: ToolPosition, x, y
// Server->Client: Score, number1, number2
//------------------------------------------------------------------

class Match;

struct Player
{
    Match&          m_match;
    IClientSession* m_session;
    
    Player( Match& match, IClientSession* session ) : m_match(match), m_session(session) {}
};

class Match
{
public:
    const std::string       m_matchId;

    std::optional<Player>   m_player1;
    std::optional<Player>   m_player2;

public:
    Match( const std::string& matchId ) : m_matchId(matchId) {}
};

class Game: public IGame
{
    std::map<IClientSession*,std::shared_ptr<Match>> m_clientMap;
    std::list<Match> m_matchList;
    
public:
    virtual void onPlayerMessage( IClientSession& client, boost::asio::streambuf& message ) override
    {
        LOG( "Received: " << std::string( (const char*)message.data().data(), message.size() ) );

        std::istringstream input;
        input.str( std::string( (const char*)message.data().data(), message.size() ) );

        std::string command;
        std::getline( input, command, ';');
        
        if ( command == "StartGame" )
        {
            // Get 'MatchId'
            std::string matchId;
            std::getline( input, matchId, ';');

            auto matchIt = std::find_if( m_matchList.begin(), m_matchList.end(), [&matchId] ( const auto& match ) {
                return match.m_matchId == matchId;
            });
            
            // Match is created (we have received 'StartGame' message from 2-d player)
            if ( matchIt != m_matchList.end() )
            {
                if ( matchIt->m_player1 && matchIt->m_player2 )
                {
                    LOG_ERR( "MatchIdTaken" );
                    client.sendMessage( "MatchIdTaken;\n" );
                    return;
                }
                if ( ! matchIt->m_player1 || matchIt->m_player2 )
                {
                    LOG_ERR( "MatchIdInternalError" );
                    client.sendMessage( "MatchIdInternalError;\n" );
                    return;
                }
                
                LOG( "message from 2-d player" );
                matchIt->m_player2.emplace( *matchIt, &client );
                matchIt->m_player2->m_session->sendMessage( "GameStarded;right;\n" );
                matchIt->m_player1->m_session->sendMessage( "GameStarded;left;\n" );
                return;
            }
            
            // we have received 'StartGame' message from 1-st player
            LOG( "message from 1-d player" );
            
            if ( auto it = m_clientMap.find(&client); it != m_clientMap.end() )
            {
                LOG_ERR( "OldClientSessionIsNotRemoved" );
                client.sendMessage( "OldClientSessionIsNotRemoved;\n" );
                return;
            }
            
            m_matchList.push_front( Match{matchId} );
            m_matchList.front().m_player1.emplace( *matchIt, &client );
            client.sendMessage( "WaitingSecondPlayer;\n" );
        }
    }

};

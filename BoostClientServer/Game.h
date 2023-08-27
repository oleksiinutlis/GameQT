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
    
    int             m_width;
    int             m_height;

    Player( Match& match, IClientSession* session, int width, int height ) : m_match(match), m_session(session), m_width(width), m_height(height) {}
};

class Match
{
    io_context&             m_serverIoContext;
    
public:
    const std::string       m_matchId;

    std::optional<Player>   m_player1;
    std::optional<Player>   m_player2;

private:
    boost::asio::high_resolution_timer m_timer;
    
public:
    Match( io_context& serverIoContext, const std::string& matchId )
    : m_serverIoContext(serverIoContext),
      m_matchId(matchId),
      m_timer( m_serverIoContext )
    {}
    
public:
    double m_width;
    double m_height;
    
    double m_xBall;
    double m_yBall;
    double m_dx;
    double m_dy;
    
    double m_x1Player;
    double m_y1Player;
    
    double m_x2Player;
    double m_y2Player;
    
    double m_ballRadius;
    double m_playerRadius;
    
    void init( int width, int height )
    {
        m_width = width;
        m_height = height;
        
        m_ballRadius = (5 * 1800) / (m_width+m_height);
        m_playerRadius = (15 * 1800) / (m_width+m_height);
        
        m_xBall = m_width/2.0;
        m_yBall = m_height/2.0;
        m_dx = 3;
        m_dy = 1;
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTimestamp;
    
    void start()
    {
        m_lastTimestamp = std::chrono::high_resolution_clock::now();

        m_timer.expires_after( std::chrono::milliseconds( 3000 ));
        m_timer.async_wait([this](const boost::system::error_code& ec )
        {
            if ( ec )
            {
                LOG( "Timer error:" << ec.message() );
                exit(1);
            }
            tick();
        });
    }
    
    int counter = 0;
    int startCounter = 0;

    void tick()
    {
        if ( ++counter < 10 )
        {
            tick();
            return;
        }
        else if ( counter == 10 )
        {
            m_lastTimestamp = std::chrono::high_resolution_clock::now();
            tick();
            return;
        }
        
        if (counter>10+100)
        {
            exit(0);
        }
        
        m_timer.expires_after( std::chrono::milliseconds( 30 ));
        m_timer.async_wait([this](const boost::system::error_code& ec )
        {
            if ( ec )
            {
                LOG( "Timer error:" << ec.message() );
                exit(1);
            }
            
            auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_lastTimestamp);
            m_lastTimestamp = std::chrono::high_resolution_clock::now();

            LOG( "duration: " << durationMs.count() );
            m_xBall = m_xBall + m_dx*(durationMs.count()/30000.0);
            m_yBall = m_yBall + m_dy*(durationMs.count()/30000.0);
            
            std::shared_ptr<boost::asio::streambuf> wrStreambuf1 = std::make_shared<boost::asio::streambuf>();
            std::ostream os1(&(*wrStreambuf1));
            os1 << "Ball;" << int(m_xBall) << ";" << int(m_yBall) << ";\n";

            m_player1->m_session->sendMessage( wrStreambuf1 );

            std::shared_ptr<boost::asio::streambuf> wrStreambuf2 = std::make_shared<boost::asio::streambuf>();
            std::ostream os2(&(*wrStreambuf2));
            //os1 << "Ball;" << int(m_xBall) << ";" << int(m_yBall) << ";\n";

            m_player2->m_session->sendMessage( wrStreambuf2 );

            tick();
        });
    }
};

class Game: public IGame
{
    io_context&             m_serverIoContext;

    std::map<IClientSession*,std::shared_ptr<Match>> m_clientMap;
    std::list<Match> m_matchList;
    
public:
    
    Game( io_context& serverIoContext ) : m_serverIoContext( serverIoContext ) {}
    
    virtual void handlePlayerMessage( IClientSession& client, boost::asio::streambuf& message ) override
    {
        LOG( "SERVER: Recieved from client: " << std::string( (const char*)message.data().data(), message.size() ) << std::endl);

        std::istringstream input;
        input.str( std::string( (const char*)message.data().data(), message.size() ) );

        std::string command;
        std::getline( input, command, ';');
        
        if ( command == "StartGame" )
        {
            // Get 'MatchId'
            std::string matchId;
            std::getline( input, matchId, ';');

            std::string widthStr;
            std::getline( input, widthStr, ';');
            int width = std::stoi(widthStr);

            std::string heightStr;
            std::getline( input, heightStr, ';');
            int height = std::stoi(heightStr);

            LOG( "m_matchList.size(): " << m_matchList.size() );
            {
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
                    int minWidth = std::min( width, matchIt->m_player1->m_width );
                    int minHeight = std::min( height, matchIt->m_player1->m_height );
                    matchIt->m_player1->m_width = minWidth;
                    matchIt->m_player1->m_height = minHeight;
                    
                    matchIt->m_player1->m_match.init( minWidth, minHeight );
                    
                    matchIt->m_player2.emplace( *matchIt, &client, minWidth, minHeight );
                    
                    std::shared_ptr<boost::asio::streambuf> wrStreambuf1 = std::make_shared<boost::asio::streambuf>();
                    std::ostream os1(&(*wrStreambuf1));
                    os1 << "GameStarted;right;" << minWidth << ";" << minHeight << ";\n";

                    matchIt->m_player2->m_session->sendMessage( wrStreambuf1 );

                    std::shared_ptr<boost::asio::streambuf> wrStreambuf2 = std::make_shared<boost::asio::streambuf>();
                    std::ostream os2(&(*wrStreambuf2));
                    os2 << "GameStarted;left;" << minWidth << ";" << minHeight << ";\n";

                    matchIt->m_player1->m_session->sendMessage( wrStreambuf2 );

                    matchIt->m_player1->m_match.start();
                    return;
                }
            }
            
            // we have received 'StartGame' message from 1-st player
            LOG( "message from 1-d player" );
            
            if ( auto it = m_clientMap.find(&client); it != m_clientMap.end() )
            {
                LOG_ERR( "OldClientSessionIsNotRemoved" );
                client.sendMessage( "OldClientSessionIsNotRemoved;\n" );
                return;
            }
            
            m_matchList.emplace_front( m_serverIoContext, matchId );
            auto& front = m_matchList.front();
            front.m_player1.emplace( front, &client, width, height );
            client.sendMessage( "WaitingSecondPlayer;\n" );
        }
    }

};

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

struct Player: public IClientSessionUserData
{
    Match&          m_match;
    IClientSession* m_session;
    
    int             m_width;
    int             m_height;

    Player( Match& match, IClientSession* session, int width, int height ) : m_match(match), m_session(session), m_width(width), m_height(height) {
        
    }
};

class Match
{
    io_context&             m_serverIoContext;
    
public:
    const std::string       m_matchId;

    std::shared_ptr<Player>   m_player1;
    std::shared_ptr<Player>   m_player2;

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
    
    double m_ballRadius = 15;
    double m_playerRadius = 50;
    
    bool   m_isIntersected = false;
    
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

        m_x1Player = 2*m_playerRadius;
        m_y1Player = m_height/2;
        
        m_x2Player = m_width - 2*m_playerRadius;
        m_y2Player = m_height/2;
    }
    
    void onClientPositionChanged( Player* player, int x, int y )
    {
        if ( player == m_player1.get() )
        {
            m_x1Player = x;
            m_y1Player = y;
        }
        else if ( player == m_player2.get() )
        {
            m_x2Player = x;
            m_y2Player = y;
        }
        else
        {
            LOG_ERR( "Unknown player pointer!" );
            exit(0);
        }
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
    
    int startCounter = 0;
    const int skipNumber = 1;

    void tick()
    {
        m_timer.expires_after( std::chrono::milliseconds( 30 ));
        m_timer.async_wait([this](const boost::system::error_code& ec )
        {
            if ( ++startCounter < skipNumber )
            {
                tick();
                return;
            }
            else if ( startCounter == skipNumber )
            {
                m_lastTimestamp = std::chrono::high_resolution_clock::now();
                tick();
                return;
            }
//            else if ( startCounter > skipNumber+1000 )
//            {
//                exit(0);
//            }
            
            if ( ec )
            {
                LOG( "Timer error:" << ec.message() );
                exit(1);
            }
            
            auto durationMs = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_lastTimestamp);
            m_lastTimestamp = std::chrono::high_resolution_clock::now();

            calculateScene( durationMs.count()/30000.0 );
            tick();
        });
    }
    
    void calculateScene( double deltaTime )
    {
        auto x1PlayerCenter = m_x1Player + m_playerRadius;
        auto y1PlayerCenter = m_y1Player + m_playerRadius;

        auto x2PlayerCenter = m_x2Player + m_playerRadius;
        auto y2PlayerCenter = m_y2Player + m_playerRadius;

        auto xBallCenter = m_xBall + m_ballRadius;
        auto yBallCenter = m_yBall + m_ballRadius;

        auto realX = xBallCenter;
        auto realY = yBallCenter;

        auto realX2 = x1PlayerCenter;
        auto realY2 = y1PlayerCenter;

        auto ellipseRadius = m_playerRadius;
        auto radius = m_ballRadius;

        auto& dx = m_dx;
        auto& dy = m_dy;

        auto& x = m_xBall;
        auto& y = m_yBall;


        if ((realX - realX2) * (realX - realX2) + (realY - realY2) * (realY - realY2) > (radius + ellipseRadius) * (radius + ellipseRadius))
        {
            if (x + dx > m_width - radius || x + dx < 0) {
                dx = -dx;
            }
            if (y + dy > m_height - radius || y + dy < 0) {
                dy = -dy;
            }

            if ( m_isIntersected )
            {
                static int counter = 0;
                std::cout << "--" << counter++ << "\n";

                m_isIntersected = false;
            }
        }
        else if ( !m_isIntersected )
        {
            // intersected

            // rotate 180
            double rDx = -dx;
            double rDy = -dy;

            // get 2-d axis
            double x2 = realX - realX2;
            double y2 = realY - realY2;

            // calculate cos(fi) and sin(Fi)
            double cosFi = (rDx * x2 + rDy * y2) / sqrt((rDx * rDx + rDy * rDy) * (x2 * x2 + y2 * y2));
            double sinFi = sqrt(1 - cosFi * cosFi);

            // do rotation
            double dxRotated = cosFi * rDx - sinFi * rDy;
            double dyRotated = sinFi * rDx + cosFi * rDy;

            // test direction
            double testCosFi = (dxRotated * dyRotated + x2 * y2) / sqrt((dxRotated * dxRotated + dyRotated * dyRotated) * (x2 * x2 + y2 * y2));

            if (testCosFi < cosFi)
            {
                dx = cosFi * dxRotated - sinFi * dyRotated;
                dy = sinFi * dxRotated + cosFi * dyRotated;
            }
            else
            {
                dxRotated = cosFi * rDx + sinFi * rDy;
                dyRotated = -sinFi * rDx + cosFi * rDy;
                dx = cosFi * dxRotated + sinFi * dyRotated;
                dy = -sinFi * dxRotated + cosFi * dyRotated;
            }

            std::cout << "isIntersected" << "\n";
            m_isIntersected = true;
        }

        m_xBall = m_xBall + m_dx*deltaTime;
        m_yBall = m_yBall + m_dy*deltaTime;

        
        sendUpdateScene();
    }
    
    void sendUpdateScene()
    {
        {
            std::shared_ptr<boost::asio::streambuf> wrStreambuf1 = std::make_shared<boost::asio::streambuf>();
            std::ostream os1(&(*wrStreambuf1));
            os1 << UPDATE_SCENE_CMD ";"
                << int(m_xBall) << ";" << int(m_yBall) << ";"
                << int(m_x1Player) << ";" << int(m_y1Player) << ";"
                << int(m_x2Player) << ";" << int(m_y2Player) << ";"
                << int(m_ballRadius) << ";" << int(m_playerRadius) << ";\n";

            m_player1->m_session->sendMessage( wrStreambuf1 );
        }

        {
            std::shared_ptr<boost::asio::streambuf> wrStreambuf2 = std::make_shared<boost::asio::streambuf>();
            std::ostream os2(&(*wrStreambuf2));
            os2 << UPDATE_SCENE_CMD ";"
                << int(m_xBall) << ";" << int(m_yBall) << ";"
                << int(m_x1Player) << ";" << int(m_y1Player) << ";"
                << int(m_x2Player) << ";" << int(m_y2Player) << ";"
                << int(m_ballRadius) << ";" << int(m_playerRadius) << ";\n";

            m_player2->m_session->sendMessage( wrStreambuf2 );
        }
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
        
        if ( command == START_GAME_CMD )
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
                    
                    matchIt->m_player2 = std::make_shared<Player>( *matchIt, &client, minWidth, minHeight );
                    
                    std::shared_ptr<boost::asio::streambuf> wrStreambuf1 = std::make_shared<boost::asio::streambuf>();
                    std::ostream os1(&(*wrStreambuf1));
                    os1 << GAME_STARTED_CMD ";right;" << minWidth << ";" << minHeight << ";\n";

                    matchIt->m_player2->m_session->sendMessage( wrStreambuf1 );

                    std::shared_ptr<boost::asio::streambuf> wrStreambuf2 = std::make_shared<boost::asio::streambuf>();
                    std::ostream os2(&(*wrStreambuf2));
                    os2 << GAME_STARTED_CMD ";left;" << minWidth << ";" << minHeight << ";\n";

                    matchIt->m_player1->m_session->sendMessage( wrStreambuf2 );
                    auto base = std::dynamic_pointer_cast<IClientSessionUserData>( matchIt->m_player1 );
                    client.setUserInfoPtr( std::weak_ptr<IClientSessionUserData>( base ) );

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
            front.m_player1 = std::make_shared<Player>( front, &client, width, height );
            client.sendMessage( "WaitingSecondPlayer;\n" );
            
            auto base = std::dynamic_pointer_cast<IClientSessionUserData>( front.m_player1 );
            client.setUserInfoPtr( std::weak_ptr<IClientSessionUserData>( base ) );
        }
        else if ( command == CLIENT_POSITION_CMD )
        {
            // Get mouse 'x'
            std::string xStr;
            std::getline( input, xStr, ';');
            int mouseX = std::stoi(xStr);

            // Get mouse 'y'
            std::string yStr;
            std::getline( input, yStr, ';');
            int mouseY = std::stoi(yStr);
            
            if ( auto ptr = client.getUserInfoPtr().lock(); ptr )
            {
                auto player = std::dynamic_pointer_cast<Player>( ptr );
                player->m_match.onClientPositionChanged( player.get(), mouseX, mouseY );
            }
        }
    }
};

#pragma once

#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QTimer>
#include <QMouseEvent>

#include <iostream>

#include "BoostClientServer/Interfaces.h"

#include "Scene.h"

class QtClientPlayer : public IClientPlayer
{
    Scene& m_scene;
    bool        m_isLeftPlayer;
    double      m_xRatio;
    double      m_yRatio;

    std::string m_playerName = "QtClientPlayer";

public:
    QtClientPlayer(Scene& scene) : m_scene(scene) {}

    virtual void handleServerMessage(const std::string& command, boost::asio::streambuf& message) override
    {
        LOG("QtClientPlayer: Recieved from server: " << m_playerName << ": " << command << " " << std::string((const char*)message.data().data(), message.size() - 1));

        std::istringstream input;
        input.str(std::string((const char*)message.data().data(), message.size()));

        if (command == "WaitingSecondPlayer")
        {
        }
        else if (command == GAME_STARTED_CMD)
        {
            std::string direction;
            std::getline(input, direction, ';');

            if (command == "left")
            {
                m_isLeftPlayer = true;
            }
            else {
                m_isLeftPlayer = false;
            }
            //TODO

            std::string widthStr;
            std::getline(input, widthStr, ';');
            double width = std::stod(widthStr);

            std::string heightStr;
            std::getline(input, heightStr, ';');
            double height = std::stod(heightStr);

            m_scene.setSceneSize(width, height);
        }
        else if (command == UPDATE_SCENE_CMD)
        {
            std::string number;

            std::getline(input, number, ';');
            double x = std::stod(number);

            std::getline(input, number, ';');
            double y = std::stod(number);

            //            << int(m_x1Player) << ";" << int(m_y1Player) << ";"
            //            << int(m_x2Player) << ";" << int(m_y2Player) << ";"
            //            << int(m_ballRadius) << ";" << int(m_playerRadius) << ";\n"

            std::getline(input, number, ';');
            double x1Player = std::stod(number);

            std::getline(input, number, ';');
            double y1Player = std::stod(number);

            std::getline(input, number, ';');
            double x2Player = std::stod(number);

            std::getline(input, number, ';');
            double y2Player = std::stod(number);

            std::getline(input, number, ';');
            double ballRadius = std::stod(number);

            std::getline(input, number, ';');
            double playerRadius = std::stod(number);

            //LOG( UPDATE_SCENE_CMD ": " << m_playerName << "  :" << x << " " << y << "\n" );

            m_scene.draw(x, y, x1Player, y1Player, x2Player, y2Player, ballRadius, playerRadius);
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

    virtual const std::string& playerName() const override { return m_playerName; }
};
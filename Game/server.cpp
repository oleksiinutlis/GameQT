#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QTimer>
#include <QMouseEvent>

#include <iostream>

#include "BoostClientServer/Server.h"
#include "BoostClientServer/TcpClient.h"
#include "BoostClientServer/Game.h"
#include "BoostClientServer/ClientPlayer.h"

#include "Scene.h"
#include "QtClientPlayer.h"
#include "CircleWidget.h"


int main(int argc, char* argv[])
{
    io_context serverIoContext;
    Game game(serverIoContext);

    TcpServer server(serverIoContext, game, 1234);
    server.execute();
}


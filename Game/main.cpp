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
    QApplication a(argc, argv);
    MainWindow w;

    Scene scene(w);
    scene.init();

    // server
    std::thread([]
        {
            io_context serverIoContext;
            Game game(serverIoContext);

            TcpServer server(serverIoContext, game, 1234);
            server.execute();
        }).detach();

        //std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // clients
        std::thread([]
            {
                io_context  ioContext;

                ClientPlayer player1{ "player1" };

                TcpClient client1(ioContext, player1);
                player1.setTcpClient(&client1);
                client1.execute("127.0.0.1", 1235, "StartGame;001;800;600;");

                ioContext.run();
            }).detach();

            std::thread([&scene]
                {
                    QtClientPlayer player2{ scene };

                    io_context  ioContext2;
                    TcpClient client2(ioContext2, player2);
                    //player2.setTcpClient(&client2);
                    client2.execute("127.0.0.1", 1234, "StartGame;001;1000;800;");

                    ioContext2.run();
                }).detach();

                //    w.resize(1200, 600);
                //    CircleWidget circleWidget;
                //    w.centralWidget()->setLayout(new QVBoxLayout);
                //    w.centralWidget()->layout()->addWidget(&circleWidget);
                //
                //    w.show();

                return a.exec();
}
#pragma once

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QTimer>
#include <QMouseEvent>

#include "mainwindow.h"

class Scene : public QWidget
{
    Q_OBJECT
    
    MainWindow& m_mainWindow;
    
    QPoint m_ballPos;
    QPoint m_1playerPos;
    QPoint m_2playerPos;

    int m_radius = 15;
    int m_playerRadius = 50;

signals:
    void updateSignal();

public:
    Scene( MainWindow& mainWindow, QWidget *parent = nullptr ) : QWidget(parent), m_mainWindow(mainWindow)
    {
        connect( this, &Scene::updateSignal, this, QOverload<>::of(&Scene::update));
    }

    void init()
    {
        m_mainWindow.resize(1200, 600);
        
        m_mainWindow.centralWidget()->setLayout(new QVBoxLayout);
        m_mainWindow.centralWidget()->layout()->addWidget(this);
        
        m_mainWindow.show();
    }
    
    QSize setSceneSize( double width, double height )
    {
        QMetaObject::invokeMethod( this, [=,this] { m_mainWindow.resize( width, height ); }, Qt::QueuedConnection);
    }
    
    void draw( double x, double y, double xPlayer1, double yPlayer1, double xPlayer2, double yPlayer2, double ballRadius, double playerRadius )
    {
        m_ballPos.setX( x );
        m_ballPos.setY( y );
        m_1playerPos.setX( xPlayer1 );
        m_1playerPos.setY( yPlayer1 );
        m_2playerPos.setX( xPlayer2 );
        m_2playerPos.setY( yPlayer2 );
        m_radius = ballRadius;
        m_playerRadius = playerRadius;

        emit updateSignal();
    }
    
protected:
    
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.setBrush(Qt::lightGray);
        painter.drawRect(0, 0, width(), height());

        painter.setBrush(Qt::red);
        painter.drawEllipse( m_1playerPos.x(), m_1playerPos.y(), m_playerRadius*2, m_playerRadius*2 );

        painter.setBrush(Qt::red);
        painter.drawEllipse( m_2playerPos.x(), m_2playerPos.y(), m_playerRadius*2, m_playerRadius*2 );

        painter.setBrush(Qt::blue);
        painter.drawEllipse( m_ballPos.x(), m_ballPos.y(), m_radius*2, m_radius*2);
    }

    void mousePressEvent(QMouseEvent *event) override {
//        calculateScene();
//        if (event->button() == Qt::LeftButton) {
//            isMousePressed = true;
//        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
//        calculateScene();
//        if (isMousePressed) {
//            ellipsePos.setX( event->pos().x() - ellipseRadius );
//            ellipsePos.setY( event->pos().y() - ellipseRadius );
//            calculateScene();
//        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
//        calculateScene();
//        if (event->button() == Qt::LeftButton) {
//            isMousePressed = false;
//        }
    }
};

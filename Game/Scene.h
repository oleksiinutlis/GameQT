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
    QPoint m_ellipsePos;

    int m_radius = 15;
    int m_ellipseRadius = 50;

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
    
    void updateScene( QPoint ballPos )
    {
        m_ballPos = ballPos;
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
        painter.drawEllipse( m_ellipsePos.x(), m_ellipsePos.y(), m_ellipseRadius*2, m_ellipseRadius*2 );

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

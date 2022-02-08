#include "mainwindow.h"
#include "ui_mainwindow.h"
static bool g_bDbg = false;
void MainWindow::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == m_nConnectTimer)
    {
        if(g_bDbg) qDebug() << "client status:" << m_pClient->state();
        if(m_bIsConnected == false)
        {
            qDebug() << "try to connect server...";
            m_pClient->open(QUrl("ws://10.10.42.70:8081"));

            //连接结果
            connect(m_pClient,&QWebSocket::connected,[this](){
                qDebug () << "IP:" << m_pClient->localAddress().toString();
                qDebug () << "PORT:" << m_pClient->localPort();
                qDebug()<<"connected";
                m_bIsConnected = true;
            });
            connect(m_pClient,&QWebSocket::disconnected,[this](){
                qDebug()<<"disconnected";
                m_bIsConnected = false;
            });
            //发送数据
            m_pClient->sendTextMessage("hello!!");
            //接收数据
            connect(m_pClient,&QWebSocket::textMessageReceived,[this](const QString &msg){
                qDebug() << msg;
            });
        }
        else
        {
            return;
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_bIsConnected = false;
    m_nConnectTimer = -1;

    m_pClient=new QWebSocket;

    m_pClient->setParent(this);

    m_nConnectTimer = startTimer(2000);

}

MainWindow::~MainWindow()
{
    if(m_pClient->state() == QAbstractSocket::SocketState::ConnectedState)
    {
        m_pClient->close();
    }

    delete ui;
}




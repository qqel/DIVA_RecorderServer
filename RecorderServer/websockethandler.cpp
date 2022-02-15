#include "websockethandler.h"
static bool g_bWebsocketDbg = false;

void WebsocketHandler::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == m_nConnectTimer )
    {
        if(g_bWebsocketDbg) qDebug() << "client status:" << m_pClient->state();
        if(m_bIsConnected == false)
        {
            if(g_bWebsocketDbg) qDebug() << "try to connect server...";
            m_pClient->open(QUrl("ws://10.10.42.70:8081"));
        }
        else
        {
            return;
        }
    }
}

WebsocketHandler::WebsocketHandler(QObject *parent) : QObject(parent)
{
    qDebug() << __func__;

    m_bIsConnected = false;

    m_nConnectTimer = -1;

    m_pClient = new QWebSocket;

    initWebSocket();

    m_pClient->setParent(this);

    m_nConnectTimer = startTimer(2000);

}

WebsocketHandler::~WebsocketHandler()
{
    qDebug() << __func__;

    m_pClient->close();
}

void WebsocketHandler::initWebSocket()
{
    qDebug() << __func__;

    connect(m_pClient,&QWebSocket::connected,[this](){
        qDebug () << "IP:" << m_pClient->localAddress().toString();
        qDebug () << "PORT:" << m_pClient->localPort();
        qDebug() << "connected";
        m_bIsConnected = true;

        m_pClient->sendTextMessage("connected!!");
    });
    connect(m_pClient,&QWebSocket::disconnected,[this](){
        qDebug()<<"disconnected";
        m_bIsConnected = false;
    });

    connect(m_pClient,&QWebSocket::textMessageReceived,[this](const QString &msg){
        qDebug() << msg;
    });
}

#include "websockethandler.h"
static bool g_bWebsocketDbg = true;

void WebsocketHandler::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == m_nConnectTimer )
    {
        if(m_bIsConnected == false)
        {
            if(g_bWebsocketDbg) qDebug() << "try to connect server...";
            m_pClient->open(QUrl("ws://10.10.42.70:8081"));
        }
        else
        {
            if(this->m_bLoginFlag == false)
            {
                QJsonObject jsonSend;

                jsonSend["FUNCTION"] = "SDVOEWEB_SYS_SET_LOGIN";
                jsonSend["ACCOUNT"] = "root";
                jsonSend["PASSWORD"] = "root";

                QJsonDocument doc(jsonSend);
                setMessage(doc.toJson());
            }
            else
            {

            }
            return;
        }
    }
}

void WebsocketHandler::onWebsocketReceiveMessage(const QString &msg)
{
    if(g_bWebsocketDbg) qDebug() << __func__;

    qDebug() << msg;
}

WebsocketHandler::WebsocketHandler(QObject *parent) : QObject(parent)
{
    qDebug() << __func__;

    m_bIsConnected = false;
    m_bLoginFlag = false;
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
        qDebug () << "connected";
        m_bIsConnected = true;

        m_pClient->sendTextMessage("connected!!");
    });
    connect(m_pClient,&QWebSocket::disconnected,[this](){
        qDebug()<<"disconnected";
        m_bIsConnected = false;
    });

    connect(m_pClient,&QWebSocket::textMessageReceived,this,&WebsocketHandler::onWebsocketReceiveMessage);
}

void WebsocketHandler::setMessage(QString msg)
{
    m_pClient->sendTextMessage(msg);
}



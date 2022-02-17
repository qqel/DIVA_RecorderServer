#include "websockethandler.h"
static bool g_bWebsocketDbg = false;

void WebsocketHandler::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == m_nConnectTimer )
    {
        if(m_bIsConnected == false)
        {
            if(m_strServerURL != "") {
                if(g_bWebsocketDbg) qDebug() << "[FINN] try to connect server...";
                m_pClient->open(QUrl(m_strServerURL));
            }
        }
        else
        {
            if(this->m_bIsLogin == false)
            {
                QJsonObject jsonSend;

                jsonSend["FUNCTION"] = "SDVOERTC_";
                jsonSend["ACCOUNT"]  = "root";
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
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    QJsonObject obj = doc.object();

    QString strFunction = obj["FUNCTION"].toString();

    if( strFunction == "SDVOEWEB_SYS_SET_LOGIN" )
    {
        m_nSessid = obj["SESSION"].toInt();

        this->m_bIsLogin = true;

        // send information message
        QJsonObject jsonSendObj;
        jsonSendObj["FUNCTION"]      = "SDVOE_REC_SET_INFO";
        jsonSendObj["RECORDER_NAME"] = "";
        jsonSendObj["CAPTURE_NAME"]  = "SC0710 PCI";
        QJsonDocument doc(jsonSendObj);
        setMessage(doc.toJson());
    }
}

WebsocketHandler::WebsocketHandler(QObject *parent) : QObject(parent)
{
    qDebug() << __func__;

    m_bIsConnected = false;
    m_bIsLogin = false;
    m_nConnectTimer = -1;
    m_strServerURL = "ws://10.10.42.70:8081";

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



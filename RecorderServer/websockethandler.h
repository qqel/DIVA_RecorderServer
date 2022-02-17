#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTimerEvent>

class WebsocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit WebsocketHandler(QObject *parent = nullptr);
    ~WebsocketHandler();
    void        setServerProperty(QString strServerURL);
    int         m_nSessid;
    int         m_nConnectTimer;
    bool        m_bIsConnected;   
    QString     m_strServerURL;
    QWebSocket  *m_pClient;
    bool        m_bIsLogin;
    void        initWebSocket();    
    void        setMessage(QString msg);
signals:

private slots:
    void timerEvent( QTimerEvent *event );
    void onWebsocketReceiveMessage(const QString &msg);

};

#endif // WEBSOCKETHANDLER_H

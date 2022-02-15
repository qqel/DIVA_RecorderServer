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
    /* WebSocket */
    int         m_nConnectTimer;
    bool        m_bIsConnected;   
    QWebSocket  *m_pClient;
    bool        m_bLoginFlag;
    void        initWebSocket();    
    void        setMessage(QString msg);
signals:

private slots:
    void timerEvent( QTimerEvent *event );
    void onWebsocketReceiveMessage(const QString &msg);

};

#endif // WEBSOCKETHANDLER_H

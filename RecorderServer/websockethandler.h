#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H
#include <QWebSocket>
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
    void        initWebSocket();
signals:

private slots:
    void timerEvent( QTimerEvent *event );

};

#endif // WEBSOCKETHANDLER_H

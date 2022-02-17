#ifndef QCAPHANDLER_H
#define QCAPHANDLER_H

#include <QObject>
#include <Windows.h>
#include "QCAP.H"

class WebRTCHandler{
public:
    WebRTCHandler();
    ~WebRTCHandler();
public:
    PVOID   m_pChatRoomDev;
    PVOID   m_pChatter;
    PVOID   m_pSender;
    BOOL    m_bIsUsed;
    QString m_strChatterName;
    ULONG   m_nPort;
    ULONG   m_nChatter_id;
    void    setInit(QString ip, ULONG port, QString name);
    void    setChatter(ULONG peer_id);
    void    enumUser();
};

class QcapHandler : public QObject
{
    Q_OBJECT
public:
    explicit QcapHandler(QObject *parent = nullptr);
    ~QcapHandler();
    PVOID			m_pDevice;
    ULONG			m_nDeviceVideoWidth;
    ULONG			m_nDeviceVideoHeight;
    BOOL			m_bDeviceVideoIsInterleaved;
    double			m_dDeviceVideoFrameRate;
    ULONG			m_nDeviceAudioChannel;
    ULONG			m_nDeviceAudioBitsPerSample;
    ULONG			m_nDeviceAudioSampleFrequency;

    QList<WebRTCHandler*>  m_listWebRTC;
    void            addNewChatter(QString ip, ULONG port, QString name);
    int             getChatRoomPort();
    void            setChatter(ULONG peer_id, ULONG port);
    QList<WebRTCHandler*> getWebrtcList();

signals:

};

#endif // QCAPHANDLER_H

#ifndef QCAPHANDLER_H
#define QCAPHANDLER_H

#include <QObject>
#include <Windows.h>
#include "QCAP.H"

class WebRTC{
public:
    WebRTC();
    ~WebRTC();
public:
    PVOID   m_pChatter;
    PVOID   m_pSender;
    ULONG   m_nIsStart;
    QString m_strChatterName;
    ULONG   m_nChatter_id;
    void    setInit(QString ip, ULONG port, QString name);
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

    QList<WebRTC*>  m_listWebRTC;
    void            addNewChatter(QString ip, ULONG port, QString name);

signals:

};

#endif // QCAPHANDLER_H

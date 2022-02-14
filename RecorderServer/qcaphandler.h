#ifndef QCAPHANDLER_H
#define QCAPHANDLER_H

#include <QObject>
#include <Windows.h>
#include "QCAP.H"

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


    PVOID			m_pChatter;
    PVOID			m_pSender;
    PVOID			m_pReceiver;
    ULONG			m_nStartChatState;
    CHAR			m_chatter_username[MAX_PATH];
    ULONG           m_chatter_id;

signals:

};

#endif // QCAPHANDLER_H

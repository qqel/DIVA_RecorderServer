#ifndef QCAPHANDLER_H
#define QCAPHANDLER_H

#include "libqcap/qcapdevice.h"
#include "libqcap/qcapstream.h"


class QcapHandler : public QcapBase
{
    Q_OBJECT
public:
    explicit QcapHandler();
    ~QcapHandler();
    void autoCreateDevice();
    // QCAP DEVICE
    QcapDevice *getQcapDevice(uint32_t previewCH);
    void newQcapDevice(uint32_t previewCH);
    void deleteQcapDevice(uint32_t previewCH);
    void refreshQcapDevicePreviewChannel();
    void setQcapDeviceStartStreamRtspServer(uint32_t previewCH, QString account, QString password, uint32_t port, uint32_t httpPort, uint32_t encoderType, uint32_t encoderFormat, uint32_t recordMode, uint32_t complexity, uint32_t bitrateKbps, uint32_t gop);
    void setQcapDeviceStartStreamWebrtcServer(uint32_t previewCH, QString ip, uint32_t port, QString name, uint32_t encoderType, uint32_t encoderFormat, uint32_t recordMode, uint32_t complexity, uint32_t bitrateKbps, uint32_t gop);
    void setQcapDeviceStartStreamWebrtcChatter(uint32_t previewCH, QString strPeerID);
private:
    QList<QcapDevice *> m_pQcapDeviceList;
};

#endif // QCAPHANDLER_H

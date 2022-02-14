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
    void autoCreateDevicePGM();
    /* QCAP_DEVICE */
    QcapDevice *getQcapDevice(uint32_t previewCH);
    void newQcapDevice(uint32_t previewCH);
    void deleteQcapDevice(uint32_t previewCH);
    void refreshQcapDevicePreviewChannel();

    /* QCAP PGM */
    void setQcapPgm(uint32_t previewCH, ULONG width, ULONG height, double framerate);
    // STREAM
    void setQcapEncoderStartStreamWebrtcServer(uint32_t previewCH, QString ip, uint32_t port, QString name, uint32_t encoderType, uint32_t encoderFormat, uint32_t recordMode, uint32_t complexity, uint32_t bitrateKbps, uint32_t gop);
    void setQcapEncoderStartStreamWebrtcChatter(uint32_t previewCH, ULONG nPeerID);
    void enumStreamWebrtcChatter();
private:
    QList<QcapDevice *>  m_pQcapDeviceList;
    QList<QcapPgm *> m_pQcapEncoderList;
};

#endif // QCAPHANDLER_H

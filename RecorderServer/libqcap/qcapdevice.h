#ifndef QCAPDEVICE_H
#define QCAPDEVICE_H

#include <QAudioOutput>
#include "qcapstream.h"
#include "qcapwebstream.h"
#include "qcapbase.h"

struct qcap_device_av_input_config {
    ULONG video;
    ULONG audio;
};
typedef qcap_device_av_input_config qcap_device_av_input_config_t;

struct qcap_device_mirror {
    ULONG ver;
    ULONG hor;
};
typedef qcap_device_mirror qcap_device_mirror_t;

class QcapDevice : public QcapBase
{
    Q_OBJECT
public:
    explicit QcapDevice(uint32_t previewCH);
    ~QcapDevice();

    void paramInit();
    void paramUninit();

    static QMultiMap<QString, int> enumDevice();

    uint32_t PreviewCH();
    void setPreviewCH(uint32_t previewCH);
    uint32_t DeviceNum();

    void setDevice(QString deviceName, uint32_t deviceNum);
    void runDevice();
    void stopDevice();

    qcap_format_t* Format();
    qcap_data_t* VideoData();
    qcap_data_t* AudioData();
    qcap_resize_frame_buffer_t* ResizeBuffer();

    qcap_device_av_input_config_t InputConfig() const;

    ULONG VideoInputType() const;
    void setVideoInputType(const ULONG &input);

    ULONG AudioInputType() const;
    void setAudioInputType(const ULONG &input);

    // AUDIO VOLUME
    ULONG AudioVolume() const;
    void setAudioVolume(const ULONG value);

    // PROPERTIES
    ULONG TransferPipe() const;
    void setTransferPipe(const ULONG value);

    ULONG ColorRange() const;
    void setColorRange(const ULONG value);

    ULONG EdidMode() const;
    void setEdidMode(const ULONG value);

    ULONG Resolution() const;
    void setResolution(const ULONG value);

    ULONG SdiMapping() const;
    void setSdiMapping(const ULONG value);

    ULONG HdrToneMapping() const;
    void setHdrToneMapping(const ULONG value);

    ULONG OSD() const;
    void setOSD(const ULONG value);

    qcap_device_mirror_t Mirror() const;
    void setMirror(const qcap_device_mirror_t &mirror);

    ULONG SdiEqMode() const;
    void setSdiEqMode(const ULONG value);

    ULONG CropAspectRatio() const;
    void setCropAspectRatio(const ULONG value);

    // AMP
    ULONG Brightness() const;
    void setBrightness(const ULONG &value);

    ULONG Contrast() const;
    void setContrast(const ULONG &value);

    ULONG Hue() const;
    void setHue(const ULONG &value);

    ULONG Saturation() const;
    void setSaturation(const ULONG &value);

    ULONG Sharpness() const;
    void setSharpness(const ULONG &value);

    // STREAM
    QList<QcapStream *> QcapStreamServerList();
    void startStreamRtspServer(qcap_encode_property_t *property, CHAR *account, CHAR *password, ULONG port, ULONG httpPort);
    void startStreamHlsServer(qcap_encode_property_t *property, CHAR *rootFolderPath, CHAR *subFolderPath);
    void startStreamWebrtcServer(qcap_encode_property_t *property, CHAR *ip, ULONG port, CHAR *name);
    void startStreamWebrtcChatter(QString strPeerID);
    void stopStreamServer();

signals:
    void deviceVideoPreviewCB(qcap_format_t *format, qcap_data_t *data);

private:
    void deviceInit(QString deviceName, uint32_t deviceNum);
    void deviceUninit();

    // AUDIO OUTPUT
    QAudioOutput* m_pAudioOutput = nullptr;
    QIODevice* m_pAudioIODevice;

    QString m_strDeviceName;
    int m_nPreviewCH;
    uint32_t m_nDeviceNum;

    PVOID m_pDevice = NULL;

    qcap_format_t *m_pQcapFormat;
    qcap_data_t *m_pQcapVideoData;
    qcap_data_t *m_pQcapAudioData;
    qcap_resize_frame_buffer_t *m_pResizeBuffer;

    // STREAM
    QList<QcapStream*> m_pQcapStreamServerList;
    // WEB RTC
    QList<QcapWebstream*> m_pQcapWebstreamList;


};

#endif // QCAPDEVICE_H

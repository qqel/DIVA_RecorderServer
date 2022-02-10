#ifndef QCAPSTREAM_H
#define QCAPSTREAM_H

#include "qcapbase.h"

class QcapStream : public QcapBase
{
    Q_OBJECT
public:
    explicit QcapStream();
    ~QcapStream();

    void startRtspServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *account, CHAR *password, ULONG port, ULONG httpPort);
    void startRtspWebPortalServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *url, CHAR *account, CHAR *password);
    void startHlsServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *rootFolderPath, CHAR *subFolderPath);
    void startRtmpServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *account, CHAR *password, ULONG port, ULONG httpPort);
    void startRtmpWebPortalServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *url, CHAR *account, CHAR *password);
    void startTsOverSrtServer(qcap_format_t *format, qcap_encode_property_t *property, ULONG port, CHAR *key);
    void startTsOverSrtWebPortalServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *url, CHAR *key, CHAR *account, CHAR *password);
    void startNdiServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *NdiName);
    void startNdiHxServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *NdiName, CHAR *key);
    void startWebrtcServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *ip, ULONG port, CHAR *name);
    void stopServer();

    // RAW
    void setVideoFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime = 0.0);
    void setAudioFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime = 0.0);
    // STREAM
    void setVideoStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, BOOL bIsKeyFrame, double dSampleTime = 0.0);
    void setAudioStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, double dSampleTime = 0.0);

private:
    void startServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *key = NULL);

    PVOID m_pServer = nullptr;
    PVOID m_pChatter = nullptr;
    bool m_bSvrFlag = false;
};

#endif // QCAPSTREAM_H

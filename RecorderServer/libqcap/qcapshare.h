#ifndef QCAPSHARE_H
#define QCAPSHARE_H

#include "qcapbase.h"

struct qcap_share_callback {
    PF_VIDEO_SHARE_RECORD_CALLBACK pVideoStreamCB = nullptr;
    PF_AUDIO_SHARE_RECORD_CALLBACK pAudioStreamCB = nullptr;
    PF_VIDEO_DECODER_SHARE_RECORD_CALLBACK pVideoFrameCB = nullptr;
    PF_AUDIO_DECODER_SHARE_RECORD_CALLBACK pAudioFrameCB = nullptr;
    PF_VIDEO_SHARE_RECORD_MEDIA_TIMER_CALLBACK pVideoMediaTimerCB = nullptr;
    PF_AUDIO_SHARE_RECORD_MEDIA_TIMER_CALLBACK pAudioMediaTimerCB = nullptr;
    PVOID pUserData = nullptr;
};
typedef qcap_share_callback qcap_share_callback_t;

class QcapShare : public QcapBase
{
    Q_OBJECT
public:
    explicit QcapShare();
    ~QcapShare();

    void start(qcap_format_t *format, qcap_encode_property_t *property, qcap_share_callback_t *callback, ULONG flag, CHAR *fileName = nullptr);
    void stop();

    // RAW
    void setVideoFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime = 0.0, bool bCrop = false, ULONG crop_x = 0, ULONG crop_y = 0, ULONG crop_w = 0, ULONG crop_h = 0);
    void setAudioFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime = 0.0);
    // STREAM
    void setVideoStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, BOOL bIsKeyFrame, double dSampleTime = 0.0);
    void setAudioStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, double dSampleTime = 0.0);

    // NO SIGNAL FRAME BUFFER
    void InitNoSignalBuffer(QString filepath);
    void setNoSignalBuffer();

    // OSD
    void setOSDTextW(ULONG iOsdNum, int x, int y, int w, int h, QString text);

private:
    ULONG m_RecNum;
    bool m_bRecordFlag = false;

#if defined (Q_OS_LINUX)

    void resetVideoFramePool(qcap_format_t *format);
    void resetAudioFramePool(qcap_format_t *format, ULONG frameSize);

    PVOID m_pFramePoolVideo = nullptr;
    PVOID m_pFramePoolAudio = nullptr;
    qcap_format_t m_FramePoolFormat;
    ULONG m_nFramePoolAudioFrameSize;

#endif

    // NO SIGNAL FRAME BUFFER
    qcap_format_t m_pNoSignalFormat;
    BYTE *m_pNoSignalBuffer = nullptr;
    ULONG m_nNoSignalBufferLen = 0;
};

#endif // QCAPSHARE_H

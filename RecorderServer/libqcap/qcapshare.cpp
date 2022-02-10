#include "qcapshare.h"

inline ULONG getValidRecNum()
{
    ULONG num = 0;

    BOOL valid;
    do {

        QCAP_GET_SHARE_RECORD_STATUS(num++, &valid);
    } while (valid);

    num--;

    return num > 63 ? 63 : num;
}

QcapShare::QcapShare()
{
#if ENABLE_MOVE_TO_THREAD
    qcap_move_to_thread(this);
#endif
}

QcapShare::~QcapShare()
{
    stop();
}

void QcapShare::start(qcap_format_t *format, qcap_encode_property_t *property, qcap_share_callback_t *callback, ULONG flag, CHAR *fileName)
{
    stop();

    ULONG nProfile = adjustRecordProfile(format->nVideoWidth, format->nVideoHeight);
    ULONG nLevel = adjustRecordLevel(format->nVideoWidth, format->nVideoHeight);
    double dFrameRate = adjustFrameRate(format->dVideoFrameRate);

    m_RecNum = getValidRecNum();

    UINT iGpuNum = 0;

#if defined (Q_OS_WIN32)
    if(property->nEncoderType == QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK)
        iGpuNum = QCAP_QUERY_ENCODER_TYPE_CAP(1, QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK, QCAP_ENCODER_FORMAT_H264, NULL) == QCAP_RS_ERROR_NON_SUPPORT ? 0 : 1;
#endif

    QCAP_SET_VIDEO_SHARE_RECORD_PROPERTY_EX(
                m_RecNum, iGpuNum, property->nEncoderType, property->nVideoEncoderFormat, format->nColorspaceType,
                format->nVideoWidth, format->nVideoHeight, dFrameRate, nProfile, nLevel, QCAP_RECORD_ENTROPY_CABAC,
                property->nComplexity, property->nRecordMode, 8000, property->nBitrateKbps * 1024, property->nGop,
                0, FALSE, 0, 0, 0, TRUE, FALSE, FALSE, 0, 0, 0, 0, 0, 0, 0
                );

    QCAP_SET_AUDIO_SHARE_RECORD_PROPERTY(
                m_RecNum, QCAP_ENCODER_TYPE_SOFTWARE, property->nAudioEncoderFormat,
                format->nAudioChannels, format->nAudioBitsPerSample, format->nAudioSampleFrequency
                );

    QCAP_REGISTER_VIDEO_SHARE_RECORD_CALLBACK(m_RecNum, callback->pVideoStreamCB, callback->pUserData);

    QCAP_REGISTER_AUDIO_SHARE_RECORD_CALLBACK(m_RecNum, callback->pAudioStreamCB, callback->pUserData);

    QCAP_REGISTER_VIDEO_DECODER_SHARE_RECORD_CALLBACK(m_RecNum, callback->pVideoFrameCB, callback->pUserData);

    QCAP_REGISTER_AUDIO_DECODER_SHARE_RECORD_CALLBACK(m_RecNum, callback->pAudioFrameCB, callback->pUserData);

#if defined (Q_OS_WIN32)

    QCAP_REGISTER_VIDEO_SHARE_RECORD_MEDIA_TIMER_CALLBACK(m_RecNum, callback->pVideoMediaTimerCB, callback->pUserData);

    QCAP_REGISTER_AUDIO_SHARE_RECORD_MEDIA_TIMER_CALLBACK(m_RecNum, callback->pAudioMediaTimerCB, callback->pUserData);

#elif defined (Q_OS_LINUX)

    memset((void *)&m_FramePoolFormat, 0, sizeof (qcap_format_t));

    resetVideoFramePool(format);
    resetAudioFramePool(format, 4096);

#endif

    qDebug() << QCAP_START_SHARE_RECORD(m_RecNum, fileName, flag, 0.0, 0.0, property->dSegment);

    m_bRecordFlag = true;
}

void QcapShare::stop()
{
    if (m_bRecordFlag) {

        m_bRecordFlag = false;

        QCAP_STOP_SHARE_RECORD(m_RecNum);

#if defined (Q_OS_LINUX)

        if (m_pFramePoolVideo) {

            QCAP_DESTROY_FRAME_POOL(m_pFramePoolVideo);

            m_pFramePoolVideo = nullptr;
        }

        if (m_pFramePoolAudio) {

            QCAP_DESTROY_FRAME_POOL(m_pFramePoolAudio);

            m_pFramePoolAudio = nullptr;
        }

#endif
    }
}

void QcapShare::setVideoFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime, bool bCrop, ULONG crop_x, ULONG crop_y, ULONG crop_w, ULONG crop_h)
{
    if (m_bRecordFlag && nFrameBufferLen > 0) {

#if defined (Q_OS_WIN32)

        BYTE *buffer = pFrameBuffer;
        ULONG bufferSize = nFrameBufferLen;

#elif defined (Q_OS_LINUX)

        BYTE *buffer = nullptr;
        ULONG bufferSize = 0;

        if (nFrameBufferLen == 0xFFFECAFE) { // RCBUFFER

            buffer = pFrameBuffer;
            bufferSize = nFrameBufferLen;
        }
        else { //RAW

            resetVideoFramePool(format);

            QCAP_SET_VIDEO_SHARE_RECORD_CUSTOM_PROPERTY(m_RecNum, QCAP_SRPROP_SOURCE_FRAME_RATE,
                                                        reinterpret_cast<BYTE*>(&format->dVideoFrameRate), sizeof (double));

            QCAP_GET_FRAME_BUFFER(m_pFramePoolVideo, &buffer, &bufferSize);

            PVOID pRCBuffer = QCAP_BUFFER_GET_RCBUFFER(buffer, bufferSize);
            qcap_av_frame_t *pAVFrame = reinterpret_cast<qcap_av_frame_t *>(QCAP_RCBUFFER_LOCK_DATA(pRCBuffer));

            if (format->nColorspaceType == QCAP_COLORSPACE_TYPE_YUY2) {

                memcpy(pAVFrame->pData[0], pFrameBuffer, nFrameBufferLen);
            }
            else {

                int pos = 0;
                memcpy(pAVFrame->pData[0], pFrameBuffer, format->nVideoWidth * format->nVideoHeight);
                pos += format->nVideoWidth * format->nVideoHeight;
                memcpy(pAVFrame->pData[1], pFrameBuffer + pos, format->nVideoWidth * format->nVideoHeight / 4);
                pos += format->nVideoWidth * format->nVideoHeight / 4;
                memcpy(pAVFrame->pData[2], pFrameBuffer + pos, format->nVideoWidth * format->nVideoHeight / 4);
            }

            QCAP_RCBUFFER_UNLOCK_DATA(pRCBuffer);
        }

#endif

        if (bCrop) {
            QCAP_SET_VIDEO_SHARE_RECORD_UNCOMPRESSION_BUFFER_EX(
                        m_RecNum, format->nColorspaceType, format->nVideoWidth, format->nVideoHeight,
                        buffer, bufferSize, crop_x, crop_y, crop_w, crop_h, QCAP_SCALE_STYLE_STRETCH, FALSE, dSampleTime
                        );
        }
        else {

            QCAP_SET_VIDEO_SHARE_RECORD_UNCOMPRESSION_BUFFER(
                        m_RecNum, format->nColorspaceType, format->nVideoWidth, format->nVideoHeight,
                        buffer, bufferSize, dSampleTime
                        );
        }
    }
}

void QcapShare::setAudioFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime)
{
    if (m_bRecordFlag && nFrameBufferLen > 0) {

#if defined (Q_OS_WIN32)

        QCAP_SET_AUDIO_SHARE_RECORD_UNCOMPRESSION_BUFFER_EX(
                    m_RecNum, format->nAudioChannels, format->nAudioBitsPerSample, format->nAudioSampleFrequency,
                    pFrameBuffer, nFrameBufferLen, dSampleTime
                    );

#elif defined (Q_OS_LINUX)

        BYTE *buffer = nullptr;
        ULONG bufferSize = 0;

        if (nFrameBufferLen == 0xFFFECAFE) { // RCBUFFER

            buffer = pFrameBuffer;
            bufferSize = nFrameBufferLen;
        }
        else { // RAW

            resetAudioFramePool(format, nFrameBufferLen);

            QCAP_GET_FRAME_BUFFER(m_pFramePoolAudio, &buffer, &bufferSize);

            PVOID pRCBuffer = QCAP_BUFFER_GET_RCBUFFER(buffer, bufferSize);
            qcap_av_frame_t *pAVFrame = reinterpret_cast<qcap_av_frame_t *>(QCAP_RCBUFFER_LOCK_DATA(pRCBuffer));

            memcpy(pAVFrame->pData[0], pFrameBuffer, nFrameBufferLen);

            QCAP_RCBUFFER_UNLOCK_DATA(pRCBuffer);
        }

        QCAP_SET_AUDIO_SHARE_RECORD_UNCOMPRESSION_BUFFER(m_RecNum, buffer, bufferSize, dSampleTime);

#endif
    }
}

void QcapShare::setVideoStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, BOOL bIsKeyFrame, double dSampleTime)
{
    if (m_bRecordFlag && nStreamBufferLen > 0)
        QCAP_SET_VIDEO_SHARE_RECORD_COMPRESSION_BUFFER(
                    m_RecNum, pStreamBuffer, nStreamBufferLen, bIsKeyFrame, dSampleTime
                    );
}

void QcapShare::setAudioStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, double dSampleTime)
{
    if (m_bRecordFlag && nStreamBufferLen > 0)
        QCAP_SET_AUDIO_SHARE_RECORD_COMPRESSION_BUFFER(
                    m_RecNum, pStreamBuffer, nStreamBufferLen, dSampleTime
                    );
}

void QcapShare::InitNoSignalBuffer(QString filepath)
{
    QImage Img(filepath);
    m_pNoSignalFormat.nVideoWidth = Img.width();
    m_pNoSignalFormat.nVideoHeight = Img.height();

    m_nNoSignalBufferLen = Img.width() * Img.height() * 4;
    m_pNoSignalBuffer = reinterpret_cast<BYTE *>(malloc(m_nNoSignalBufferLen));

    QByteArray pszFilePath = filepath.toLocal8Bit();

    QCAP_LOAD_PICTURE_BUFFER(pszFilePath.data(), &m_pNoSignalFormat.nColorspaceType, m_pNoSignalBuffer, &m_nNoSignalBufferLen,
                             &m_pNoSignalFormat.nVideoWidth, &m_pNoSignalFormat.nVideoHeight, &m_pNoSignalFormat.nVideoPitch);
}

void QcapShare::setNoSignalBuffer()
{
    if (m_nNoSignalBufferLen > 0)
        setVideoFrameBuffer(&m_pNoSignalFormat, m_pNoSignalBuffer, m_nNoSignalBufferLen);
}

inline WSTRING qToWstring(const QString &str)
{
#if defined (Q_OS_WIN32)

    return reinterpret_cast<WSTRING>(SysAllocString(std::wstring((const wchar_t*)str.utf16()).c_str()));

#else

    Q_UNUSED(str);

    return (WSTRING)"";

#endif
}

void QcapShare::setOSDTextW(ULONG iOsdNum, int x, int y, int w, int h, QString text)
{
#if defined (Q_OS_WIN32)

    QCAP_SET_OSD_SHARE_RECORD_TEXT_EX_W(m_RecNum, iOsdNum, x, y, w, h,
                                        text.isEmpty() ? NULL : qToWstring(text),
                                        qToWstring("Arial"), QCAP_FONT_STYLE_REGULAR,
                                        36, 0xFFFFFFFF, 0x00000000, 0xFF000000, 4, 0xFF, 0, 0,
                                        QCAP_STRING_ALIGNMENT_STYLE_CENTER);

#elif defined (Q_OS_LINUX)

    QCAP_SET_OSD_SHARE_RECORD_TEXT_EX(m_RecNum, iOsdNum, x, y, w, h,
                                        text.isEmpty() ? NULL : text.toLocal8Bit().data(),
                                        QString("Arial").toLocal8Bit().data(), QCAP_FONT_STYLE_REGULAR,
                                        36, 0xFFFFFFFF, 0x00000000, 0xFF000000, 4, 0xFF, 0, 0,
                                        QCAP_STRING_ALIGNMENT_STYLE_CENTER);

#endif
}

#if defined (Q_OS_LINUX)

void QcapShare::resetVideoFramePool(qcap_format_t *format)
{
    if (format->nColorspaceType != m_FramePoolFormat.nColorspaceType ||
            format->nVideoWidth != m_FramePoolFormat.nVideoWidth ||
            format->nVideoHeight != m_FramePoolFormat.nVideoHeight) {

        if (m_pFramePoolVideo) {

            QCAP_DESTROY_FRAME_POOL(m_pFramePoolVideo);

            m_pFramePoolVideo = nullptr;
        }

        QCAP_CREATE_VIDEO_FRAME_POOL(format->nColorspaceType, format->nVideoWidth, format->nVideoHeight, 5, &m_pFramePoolVideo);

        m_FramePoolFormat.nColorspaceType = format->nColorspaceType;
        m_FramePoolFormat.nVideoWidth = format->nVideoWidth;
        m_FramePoolFormat.nVideoHeight = format->nVideoHeight;
    }
}

void QcapShare::resetAudioFramePool(qcap_format_t *format, ULONG frameSize)
{
    if (format->nAudioChannels != m_FramePoolFormat.nAudioChannels ||
            format->nAudioBitsPerSample != m_FramePoolFormat.nAudioBitsPerSample ||
            format->nAudioSampleFrequency != m_FramePoolFormat.nAudioSampleFrequency ||
            frameSize != m_nFramePoolAudioFrameSize) {

        if (m_pFramePoolAudio) {

            QCAP_DESTROY_FRAME_POOL(m_pFramePoolAudio);

            m_pFramePoolAudio = nullptr;
        }

        QCAP_CREATE_AUDIO_FRAME_POOL(format->nAudioChannels, format->nAudioBitsPerSample, format->nAudioSampleFrequency,
                                     frameSize / format->nAudioChannels / (format->nAudioBitsPerSample / 8), 5, &m_pFramePoolAudio);

        m_FramePoolFormat.nAudioChannels = format->nAudioChannels;
        m_FramePoolFormat.nAudioBitsPerSample = format->nAudioBitsPerSample;
        m_FramePoolFormat.nAudioSampleFrequency = format->nAudioSampleFrequency;
        m_nFramePoolAudioFrameSize = frameSize;
    }
}

#endif

#include "qcapwebstream.h"

QRETURN ON_VIDEO_ENCODER_SHARE_RECORD_CALLBACK(UINT iRecNum,
                                               double dSampleTime,
                                               BYTE * pStreamBuffer,
                                               ULONG nStreamBufferLen,
                                               BOOL bIsKeyFrame,
                                               PVOID pUserData)
{
    Q_UNUSED(iRecNum);

    QcapWebstream* pQcapWebstream = reinterpret_cast<QcapWebstream*>(pUserData);

    if (nStreamBufferLen > 0) {

        qcap_format_t *format = pQcapWebstream->Format();

        ULONG nBufferLen = nStreamBufferLen;
        BYTE *pBuffer = pStreamBuffer;

        // STREAM
        QList<QcapStream *> streamList = pQcapWebstream->QcapStreamServerList();
        if (!streamList.isEmpty())
            foreach (QcapStream *stream, streamList)
                stream->setVideoStreamBuffer(pStreamBuffer, nStreamBufferLen, bIsKeyFrame, dSampleTime);

    }

    return QCAP_RT_SKIP_DISPLAY;
}

QRETURN ON_AUDIO_ENCODER_SHARE_RECORD_CALLBACK(UINT iRecNum,
                                               double dSampleTime,
                                               BYTE * pStreamBuffer,
                                               ULONG nStreamBufferLen,
                                               PVOID pUserData)
{
    Q_UNUSED(iRecNum);

    QcapWebstream* pQcapWebstream = reinterpret_cast<QcapWebstream*>(pUserData);

    if (nStreamBufferLen > 0) {

        qcap_format_t *format = pQcapWebstream->Format();

        // STREAM
        QList<QcapStream *> streamList = pQcapWebstream->QcapStreamServerList();
        if (!streamList.isEmpty())
            foreach (QcapStream *stream, streamList)
                stream->setAudioStreamBuffer(pStreamBuffer, nStreamBufferLen, dSampleTime);
    }

    return QCAP_RT_SKIP_DISPLAY;
}

QcapWebstream::QcapWebstream(uint32_t previewCH, ULONG width, ULONG height, double framerate) : m_nPreviewCH(previewCH)
{
#if ENABLE_MOVE_TO_THREAD
    qcap_move_to_thread(this);
#endif

    // INIT PARAM
    m_Format = new qcap_format_t();
    m_Property = new qcap_encode_property_t();
    m_Callback = new qcap_share_callback_t();

    // INIT Preview RESIZE BUFFER
    m_pResizeBuffer = new qcap_resize_frame_buffer_t();
    qcap_resize_frame_buffer_t *resizeBuffer = ResizeBuffer();
    resizeBuffer->nWidth = 416;
    resizeBuffer->nHeight = 240;
    resizeBuffer->nPitch = getVideoPitch(QCAP_COLORSPACE_TYPE_YV12, resizeBuffer->nWidth);

    resizeBuffer->nBufferLen = resizeBuffer->nPitch*resizeBuffer->nHeight
            *(resizeBuffer->nWidth == resizeBuffer->nPitch ? (double)3 / 2 : 1);

    if (resizeBuffer->pBuffer)
        delete resizeBuffer->pBuffer;

    resizeBuffer->pBuffer = new BYTE[resizeBuffer->nBufferLen];

    // INIT Preview
    m_pQcapShareEncoder = new QcapShare();
    startPreview(width, height, framerate);

    // INIT RECORD
    m_pQcapShareRecord = new QcapShare();
}

QcapWebstream::~QcapWebstream()
{
    stopRecord();
    delete m_pQcapShareRecord;

    stopPreview();
    delete m_pQcapShareEncoder;

    delete m_Format;
    delete m_Property;
    delete m_Callback;

    if (m_pResizeBuffer->pBuffer)
        delete m_pResizeBuffer->pBuffer;

    delete m_pResizeBuffer;
}

qcap_format_t* QcapWebstream::Format()
{
    return m_Format;
}

qcap_resize_frame_buffer_t* QcapWebstream::ResizeBuffer()
{
    return m_pResizeBuffer;
}

void QcapWebstream::emitOtherScreenVideoPreview(unsigned char *frameBuffer, int frameBufferSize, int width, int height, int bytesPerLine, int colorFormat)
{
    std::shared_ptr<QVideoFrame> frame
            = std::make_shared<QVideoFrame>(
                QVideoFrame(frameBufferSize, QSize(width, height), bytesPerLine, static_cast<QVideoFrame::PixelFormat>(colorFormat)));

    if (frame->map(QAbstractVideoBuffer::WriteOnly)) {

        memcpy(frame->bits(), frameBuffer, static_cast<size_t>(frameBufferSize));

        frame->unmap();

        emit otherScreenVideoPreview(frame);
    }
}

void QcapWebstream::startPreview(ULONG width, ULONG height, double framerate)
{
    stopPreview();

    m_nPreviewFrameNum = 0;

    m_Format->nColorspaceType = QCAP_COLORSPACE_TYPE_YV12;
    m_Format->pixelFormat = getPixelFormat(QCAP_COLORSPACE_TYPE_YV12);
    m_Format->nVideoWidth = width;
    m_Format->nVideoHeight = height;
    m_Format->nVideoPitch = getVideoPitch(QCAP_COLORSPACE_TYPE_YV12, width);
    m_Format->dVideoFrameRate = framerate;
    m_Format->nAudioChannels = 2;
    m_Format->nAudioBitsPerSample = 16;
    m_Format->nAudioSampleFrequency = 48000;
    m_Callback->pVideoStreamCB = ON_VIDEO_ENCODER_SHARE_RECORD_CALLBACK;
    m_Callback->pUserData = reinterpret_cast<PVOID>(this);

    ULONG flag = static_cast<ULONG>(
                QCAP_RECORD_FLAG_DISPLAY |
                QCAP_RECORD_FLAG_VIDEO_ONLY
                );

    m_Property->nEncoderType = QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK;

    m_pQcapShareEncoder->start(m_Format, m_Property, m_Callback, flag);
}

void QcapWebstream::stopPreview()
{
    m_pQcapShareEncoder->stop();
}

void QcapWebstream::InitNoSignalBuffer(QString filepath)
{
    m_pQcapShareEncoder->InitNoSignalBuffer(filepath);
}

void QcapWebstream::setNoSignalBuffer()
{
    m_pQcapShareEncoder->setNoSignalBuffer();
}

void QcapWebstream::receivVideoData(qcap_format_t *format, qcap_data_t *data)
{

    bool bCrop = false;
    ULONG nWidth = format->nVideoWidth;
    ULONG nHeight = format->nVideoHeight;
    ULONG nCropX = 0;
    ULONG nCropY = 0;
    ULONG nCropW = 0;
    ULONG nCropH = 0;

    m_pQcapShareEncoder->setVideoFrameBuffer(format, data->buffer, data->bufferLen, data->sampleTime,
                                         bCrop, nCropX, nCropY, nCropW, nCropH);
}

void QcapWebstream::receivAudioData(qcap_format_t *format, qcap_data_t *data)
{
    m_pQcapShareEncoder->setAudioFrameBuffer(format, data->buffer, data->bufferLen, data->sampleTime);
}

QcapShare* QcapWebstream::QcapShareRecord()
{
    return m_pQcapShareRecord;
}

void QcapWebstream::startRecord(qcap_encode_property_t *property)
{
    stopRecord();

    qcap_share_callback_t callback;

    QByteArray pszFileName = property->filePathName.toLocal8Bit();

    m_pQcapShareRecord->start(m_Format, property, &callback, QCAP_RECORD_FLAG_FULL | QCAP_RECORD_FLAG_IMMEDIATE, pszFileName.data());
}

void QcapWebstream::stopRecord()
{
    m_pQcapShareRecord->stop();
}

QList<QcapStream *> QcapWebstream::QcapStreamServerList()
{
    return m_pQcapStreamServerList;
}

void QcapWebstream::startStreamWebrtcServer(qcap_encode_property_t *property, CHAR *ip, ULONG port, CHAR *name)
{
    QcapStream *stream = new QcapStream();
    stream->startWebrtcServer(m_Format, property, ip, port, name);

    m_pQcapStreamServerList.append(stream);
}

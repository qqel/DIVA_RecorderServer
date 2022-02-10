#include "qcappreview.h"
#include "controller.h"
QRETURN ON_VIDEO_DECODER_SHARE_RECORD_CALLBACK(UINT iRecNum,
                                               double dSampleTime,
                                               BYTE * pFrameBuffer,
                                               ULONG nFrameBufferLen,
                                               PVOID pUserData)
{
    Q_UNUSED(iRecNum);

    QcapPreview* pQcapPreview = reinterpret_cast<QcapPreview*>(pUserData);

    if (nFrameBufferLen > 0) {

        qcap_format_t *format = pQcapPreview->Format();

#if defined (Q_OS_WIN32)

        ULONG nBufferLen = nFrameBufferLen;
        BYTE *pBuffer = pFrameBuffer;

#elif defined (Q_OS_LINUX)

        ULONG nBufferLen = format->nVideoWidth * format->nVideoHeight * 3 / 2;
        BYTE *pBuffer = new BYTE[nBufferLen];

        PVOID pRCBuffer = QCAP_BUFFER_GET_RCBUFFER(pFrameBuffer, nFrameBufferLen);
        qcap_av_frame_t *pAVFrame = reinterpret_cast<qcap_av_frame_t *>(QCAP_RCBUFFER_LOCK_DATA(pRCBuffer));

        int pos = 0;

        memcpy(pBuffer, pAVFrame->pData[0], format->nVideoWidth * format->nVideoHeight);
        pos += format->nVideoWidth * format->nVideoHeight;
        memcpy(pBuffer + pos, pAVFrame->pData[2], format->nVideoWidth * format->nVideoHeight / 4);
        pos += format->nVideoWidth * format->nVideoHeight / 4;
        memcpy(pBuffer + pos, pAVFrame->pData[1], format->nVideoWidth * format->nVideoHeight / 4);

#endif

        // EMIT Preview RESIZE PREVIEW
        if (pQcapPreview->m_nCount % (int)pQcapPreview->Format()->dVideoFrameRate == 0) {

            qcap_resize_frame_buffer_t *resizeBuffer = pQcapPreview->ResizeBuffer();
            pQcapPreview->resizeFrameBuffer(resizeBuffer, format, pBuffer);
            pQcapPreview->emitVideoPreview(resizeBuffer->pBuffer, resizeBuffer->nBufferLen, resizeBuffer->nWidth,
                                       resizeBuffer->nHeight, resizeBuffer->nPitch, format->pixelFormat);
        }
        pQcapPreview->m_nCount;

#if defined (Q_OS_LINUX)

        QCAP_RCBUFFER_UNLOCK_DATA(pRCBuffer);
        delete [] pBuffer;

#endif

        // SNAPSHOT
        qcap_snapshot_info_t *snapshot = pQcapPreview->SnapshotInfo();
        QString filePathName = pQcapPreview->snapshot(format, snapshot, pFrameBuffer, nFrameBufferLen);

        if (snapshot->flag && !filePathName.isEmpty())
            emit pQcapPreview->callSnapshotDoneCallback(filePathName);
    }

    return QCAP_RT_SKIP_DISPLAY;
}

QRETURN ON_AUDIO_DECODER_SHARE_RECORD_CALLBACK(UINT iRecNum,
                                               double dSampleTime,
                                               BYTE * pFrameBuffer,
                                               ULONG nFrameBufferLen,
                                               PVOID pUserData)
{
    Q_UNUSED(iRecNum);

    QcapPreview* pQcapPreview = reinterpret_cast<QcapPreview*>(pUserData);

    if (nFrameBufferLen > 0) {

        qcap_format_t *format = pQcapPreview->Format();

        // RECORD
        QcapShare *record = pQcapPreview->QcapShareRecord();
        record->setAudioFrameBuffer(format, pFrameBuffer, nFrameBufferLen, dSampleTime);
    }

    return QCAP_RT_SKIP_DISPLAY;
}

QcapPreview::QcapPreview(uint32_t previewCH, ULONG width, ULONG height, double framerate) : m_nPreviewCH(previewCH)
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
    resizeBuffer->nWidth = PREVIEW_W;
    resizeBuffer->nHeight = PREVIEW_H;
    resizeBuffer->nPitch = getVideoPitch(QCAP_COLORSPACE_TYPE_YV12, resizeBuffer->nWidth);

    resizeBuffer->nBufferLen = resizeBuffer->nPitch*resizeBuffer->nHeight
            *(resizeBuffer->nWidth == resizeBuffer->nPitch ? (double)3 / 2 : 1);

    if (resizeBuffer->pBuffer)
        delete resizeBuffer->pBuffer;

    resizeBuffer->pBuffer = new BYTE[resizeBuffer->nBufferLen];

    // INIT Preview
    m_pQcapSharePreview = new QcapShare();
    startPreview(width, height, framerate);

    // INIT RECORD
    m_pQcapShareRecord = new QcapShare();
}

QcapPreview::~QcapPreview()
{
    stopRecord();
    delete m_pQcapShareRecord;

    stopPreview();
    delete m_pQcapSharePreview;

    delete m_Format;
    delete m_Property;
    delete m_Callback;

    if (m_pResizeBuffer->pBuffer)
        delete m_pResizeBuffer->pBuffer;

    delete m_pResizeBuffer;
}

qcap_format_t* QcapPreview::Format()
{
    return m_Format;
}

qcap_resize_frame_buffer_t* QcapPreview::ResizeBuffer()
{
    return m_pResizeBuffer;
}

void QcapPreview::emitOtherScreenVideoPreview(unsigned char *frameBuffer, int frameBufferSize, int width, int height, int bytesPerLine, int colorFormat)
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

void QcapPreview::startPreview(ULONG width, ULONG height, double framerate)
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

    m_Callback->pVideoFrameCB = ON_VIDEO_DECODER_SHARE_RECORD_CALLBACK;
    m_Callback->pUserData = reinterpret_cast<PVOID>(this);

    ULONG flag = static_cast<ULONG>(
                QCAP_RECORD_FLAG_DISPLAY |
                QCAP_RECORD_FLAG_VIDEO_ONLY
                );

    m_Property->nEncoderType = QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK;

    m_pQcapSharePreview->start(m_Format, m_Property, m_Callback, flag);
}

void QcapPreview::stopPreview()
{
    m_pQcapSharePreview->stop();
}

void QcapPreview::InitNoSignalBuffer(QString filepath)
{
    m_pQcapSharePreview->InitNoSignalBuffer(filepath);
}

void QcapPreview::setNoSignalBuffer()
{
    m_pQcapSharePreview->setNoSignalBuffer();
}

void QcapPreview::receivVideoData(qcap_format_t *format, qcap_data_t *data)
{

    bool bCrop = true;
    ULONG nWidth = format->nVideoWidth;
    ULONG nHeight = format->nVideoHeight;
    ULONG nCropX = (nWidth/3) * (m_nPreviewCH % 3);
    ULONG nCropY = (nHeight/3) * (m_nPreviewCH / 3);
    ULONG nCropW = nWidth / 3;
    ULONG nCropH = nHeight / 3;

    nCropW = nCropW - nCropW % 2;
    nCropH = nCropH - nCropH % 2;

    m_pQcapSharePreview->setVideoFrameBuffer(format, data->buffer, data->bufferLen, data->sampleTime,
                                         bCrop, nCropX, nCropY, nCropW, nCropH);
}

void QcapPreview::receivAudioData(qcap_format_t *format, qcap_data_t *data)
{
    m_pQcapSharePreview->setAudioFrameBuffer(format, data->buffer, data->bufferLen, data->sampleTime);
}

QcapShare* QcapPreview::QcapShareRecord()
{
    return m_pQcapShareRecord;
}

void QcapPreview::startRecord(qcap_encode_property_t *property)
{
    stopRecord();

    qcap_share_callback_t callback;

    QByteArray pszFileName = property->filePathName.toLocal8Bit();

    m_pQcapShareRecord->start(m_Format, property, &callback, QCAP_RECORD_FLAG_FULL | QCAP_RECORD_FLAG_IMMEDIATE, pszFileName.data());
}

void QcapPreview::stopRecord()
{
    m_pQcapShareRecord->stop();
}

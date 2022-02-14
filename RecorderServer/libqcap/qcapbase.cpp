#include "qcapbase.h"

void copySourceFrameBuffer(qcap_format_t *srcFormat, qcap_data_t *srcData, qcap_format_t *dstFormat, qcap_data_t *dstData)
{
    memcpy(dstFormat, srcFormat, sizeof (qcap_format_t));

    dstFormat->nColorspaceType = QCAP_COLORSPACE_TYPE_YV12;

    if (dstData->bufferLen != (dstFormat->nVideoWidth * dstFormat->nVideoHeight * 3 / 2)) {

        dstData->bufferLen = dstFormat->nVideoWidth * dstFormat->nVideoHeight * 3 / 2;

        if (dstData->buffer)
            free(dstData->buffer);

        dstData->buffer = reinterpret_cast<BYTE *>(malloc(dstData->bufferLen));
    }

    switch (srcFormat->nColorspaceType) {

    case QCAP_COLORSPACE_TYPE_YV12:

        memcpy(dstData->buffer, srcData->buffer, dstData->bufferLen);

        break;
    case QCAP_COLORSPACE_TYPE_YUY2:

        QCAP_COLORSPACE_YUY2_TO_YV12(srcData->buffer, srcFormat->nVideoWidth, srcFormat->nVideoHeight, 0,
                                     dstData->buffer, dstFormat->nVideoWidth, dstFormat->nVideoHeight, 0);

        break;
    case QCAP_COLORSPACE_TYPE_ABGR32:
#if defined (Q_OS_WIN32)
        QCAP_COLORSPACE_ABGR32_TO_YV12(srcData->buffer, srcFormat->nVideoWidth, srcFormat->nVideoHeight, 0,
                                       dstData->buffer, dstFormat->nVideoWidth, dstFormat->nVideoHeight, 0);
#endif
        break;
    case QCAP_COLORSPACE_TYPE_BGR24:
#if defined (Q_OS_WIN32)
        QCAP_COLORSPACE_BGR24_TO_YV12(srcData->buffer, srcFormat->nVideoWidth, srcFormat->nVideoHeight, 0,
                                      dstData->buffer, dstFormat->nVideoWidth, dstFormat->nVideoHeight, 0);
#endif
        break;
    }
}

QcapBase::QcapBase()
{
#if ENABLE_MOVE_TO_THREAD
    m_pThread = new QThread();
#endif
}

QcapBase::~QcapBase()
{
    m_QcapSnapshotInfo.flag = false;

#if ENABLE_MOVE_TO_THREAD

    if (m_pThread->isRunning()) {

#if defined (Q_OS_WIN)
        CoUninitialize();
#endif
        m_pThread->quit();
    }

    m_pThread->deleteLater();

#endif
}

void QcapBase::qcap_move_to_thread(QObject *obj)
{
#if ENABLE_MOVE_TO_THREAD

#if defined (Q_OS_WIN)
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

    obj->moveToThread(m_pThread);
    m_pThread->start();
#else
    Q_UNUSED(obj);
#endif
}

const qcap_version_t QcapBase::version()
{
    qcap_version_t version;

    QCAP_GET_VERSION(&version.major, &version.minor);

    return version;
}

void QcapBase::setSystemConfig(const qcap_system_config_t &config)
{

#if defined (Q_OS_WIN32)
    QCAP_SET_SYSTEM_DEBUG_LEVEL(0x00000001);
#endif

    QCAP_SET_SYSTEM_CONFIGURATION(
                config.bEnableMultipleUsersAccess,
                config.bEnableVideoPreviewDevice,
                config.bEnableAudioPreviewDevice,
                config.bEnableVideoHardwareMainEncoderDevice,
                config.bEnableVideoHardwareSubEncoderDevice,
                config.nAutoInputDetectionTimeout,
                config.bEnableSCF,
                config.pszDB3,
                config.bEnableAsyncBackgroundSnapshot,
                config.bEnableEnhancedVideoRenderer,
                config.bEnableSystemTimeCallback,
                config.bEnableFileRepairFunction,
                config.bEnableNewRTSPLibrary,
                config.pszWebServerRootFolderPath,
                config.pszWebServerIP,
                config.nSystemColorRangeType,
                config.bEnableVideoMixingRendererBugPatch,
                config.nEnableCustomVideoRenderer,
                config.bEnableGraphicMemoryForVideoEncoder,
                config.bEnableSingleGraphCaptureMode,
                config.nSignalDetectionDuration,
                config.bEnableNewChromaKeyLibrary,
                config.bEnableNewSnapshotLibrary,
                config.bEnableAppleOnFlyMP4Editing,
                config.bEnableNewSoftwareEncoderLibrary);
}

inline QRESULT getQueryEncoderTypeResult(const uint32_t &num, const uint32_t &type)
{
    return QCAP_QUERY_ENCODER_TYPE_CAP(num, type, QCAP_ENCODER_FORMAT_H264, NULL);
}

const QList<uint32_t> QcapBase::queryEncoderType()
{
    QList<uint32_t> list;

    QRESULT rs = getQueryEncoderTypeResult(0, QCAP_ENCODER_TYPE_SOFTWARE);

    if (rs == QCAP_RS_SUCCESSFUL) {

        list.append(QCAP_ENCODER_TYPE_SOFTWARE);
    }

    rs = getQueryEncoderTypeResult(0, QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK);
    if (rs == QCAP_RS_SUCCESSFUL) {

        list.append(QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK);
    }

    rs = getQueryEncoderTypeResult(0, QCAP_ENCODER_TYPE_AMD_VCE);
    if (rs == QCAP_RS_SUCCESSFUL) {

        list.append(QCAP_ENCODER_TYPE_AMD_VCE);
    }

    uint32_t nvencCount = 0;
    rs = getQueryEncoderTypeResult(nvencCount++, QCAP_ENCODER_TYPE_NVIDIA_NVENC);
    while (rs == QCAP_RS_SUCCESSFUL) {

        list.append(QCAP_ENCODER_TYPE_NVIDIA_NVENC);
        rs = getQueryEncoderTypeResult(nvencCount++, QCAP_ENCODER_TYPE_NVIDIA_NVENC);
    }

    return list;
}

QVideoFrame::PixelFormat QcapBase::getPixelFormat(const ULONG &colorspace)
{
    QVideoFrame::PixelFormat pixelFormat = QVideoFrame::Format_Invalid;

    switch (colorspace)
    {
    case QCAP_COLORSPACE_TYPE_NV12:
        pixelFormat = QVideoFrame::Format_NV12;
        break;

    case QCAP_COLORSPACE_TYPE_I420:
        pixelFormat = QVideoFrame::Format_YUV420P;
        break;

    case QCAP_COLORSPACE_TYPE_YV12:
        pixelFormat = QVideoFrame::Format_YV12;
        break;

    case QCAP_COLORSPACE_TYPE_YUY2:
        pixelFormat = QVideoFrame::Format_YUYV;
        break;

    case QCAP_COLORSPACE_TYPE_P010:
        pixelFormat = QVideoFrame::Format_Invalid;
        break;

    case QCAP_COLORSPACE_TYPE_RGB24:
        pixelFormat = QVideoFrame::Format_RGB24;
        break;

    case QCAP_COLORSPACE_TYPE_BGR24:
        pixelFormat = QVideoFrame::Format_BGR24;
        break;

    case QCAP_COLORSPACE_TYPE_ARGB32:
        pixelFormat = QVideoFrame::Format_ARGB32;
        break;

    case QCAP_COLORSPACE_TYPE_ABGR32:
        pixelFormat = QVideoFrame::Format_ABGR32;
        break;
    }

    return pixelFormat;
}

ULONG QcapBase::getVideoPitch(const ULONG &colorspace, const ULONG &width)
{
    ULONG pitch = 0;

    switch (colorspace)
    {
    case QCAP_COLORSPACE_TYPE_YV12:
    case QCAP_COLORSPACE_TYPE_NV12:
    case QCAP_COLORSPACE_TYPE_I420:
        pitch = width;
        break;

    case QCAP_COLORSPACE_TYPE_YUY2:
    case QCAP_COLORSPACE_TYPE_P010:
        pitch = width*2;
        break;

    case QCAP_COLORSPACE_TYPE_RGB24:
    case QCAP_COLORSPACE_TYPE_BGR24:
        pitch = width*3;
        break;

    case QCAP_COLORSPACE_TYPE_ARGB32:
    case QCAP_COLORSPACE_TYPE_ABGR32:
        pitch = width*4;
        break;
    }

    return pitch;
}

void QcapBase::setQcapData(qcap_data_t *data, double dSampleTime, BYTE *pBuffer, ULONG nBufferLen, BOOL bIsKeyFrame)
{
    if (data->bufferLen != nBufferLen) {

        data->bufferLen = nBufferLen;

        if (data->buffer != nullptr)
            delete data->buffer;

        data->buffer = new BYTE[data->bufferLen];
    }

    data->sampleTime = dSampleTime;
    data->isKeyFrame = bIsKeyFrame;

    memcpy(data->buffer, pBuffer, nBufferLen);
}

double QcapBase::adjustFrameRate(double dFrameRate)
{
    double dAdjustedFrameRate = dFrameRate;

    switch (static_cast<int>(dFrameRate * 100.0))
    {
    case 2397:
        dAdjustedFrameRate = 24.0 / 1.001;
        break;

    case 2997:
        dAdjustedFrameRate = 30.0 / 1.001;
        break;

    case 5994:
        dAdjustedFrameRate = 60.0 / 1.001;
        break;
    }

    return dAdjustedFrameRate;
}

unsigned long QcapBase::adjustRecordProfile(const int &width, const int &height)
{
    unsigned long nAdjustedRecordProfile = QCAP_RECORD_PROFILE_BASELINE;

    if (width >= 1280 || height >= 720) {

        nAdjustedRecordProfile = QCAP_RECORD_PROFILE_MAIN;
    }
    if (width >= 3840 || height >= 2160) {

        nAdjustedRecordProfile = QCAP_RECORD_PROFILE_HIGH;
    }

    return nAdjustedRecordProfile;
}

unsigned long QcapBase::adjustRecordLevel(const int &width, const int &height)
{
    unsigned long nAdjustedRecordLevel = QCAP_RECORD_LEVEL_22;

    if (width > 352 || height > 288) {

        nAdjustedRecordLevel = QCAP_RECORD_LEVEL_32;
    }
    if (width > 1280 || height > 720) {

        nAdjustedRecordLevel = QCAP_RECORD_LEVEL_42;
    }
    if (width > 1920 || height > 1080) {

        nAdjustedRecordLevel = QCAP_RECORD_LEVEL_52;
    }
    if (width > 3840 || height > 2160) {

        nAdjustedRecordLevel = QCAP_RECORD_LEVEL_62;
    }

    return nAdjustedRecordLevel;
}

void QcapBase::emitVideoPreview(unsigned char *frameBuffer, int frameBufferSize, int width, int height, int bytesPerLine, int colorFormat)
{
    std::shared_ptr<QVideoFrame> frame
            = std::make_shared<QVideoFrame>(
                QVideoFrame(frameBufferSize, QSize(width, height), bytesPerLine, static_cast<QVideoFrame::PixelFormat>(colorFormat)));

    if (frame->map(QAbstractVideoBuffer::WriteOnly)) {

        memcpy(frame->bits(), frameBuffer, static_cast<size_t>(frameBufferSize));

        frame->unmap();

        emit videoPreview(frame);
    }
}

void QcapBase::resizeFrameBuffer(qcap_resize_frame_buffer_t *resizeBuffer, qcap_format_t *format, BYTE *pBuffer)
{
    if (resizeBuffer->pBuffer != NULL) {

        ULONG colorspace = format->nColorspaceType == (ULONG)QCAP_COLORSPACE_TYPE_I420 ?
                    (ULONG)QCAP_COLORSPACE_TYPE_YV12 : format->nColorspaceType;

        QCAP_RESIZE_VIDEO_BUFFER(colorspace, pBuffer, format->nVideoWidth, format->nVideoHeight, format->nVideoPitch,
                                 resizeBuffer->pBuffer, resizeBuffer->nWidth, resizeBuffer->nHeight, resizeBuffer->nPitch);
    }
    else {

        qDebug() << __func__ << QString(" Fail !!");
    }
}

const qcap_audio_volume_db_t QcapBase::audioBufferVolumeDB(qcap_format_t *format, BYTE *pBuffer, ULONG nBufferLen)
{
    qcap_audio_volume_db_t dB;

    double dB_L = 0.0, dB_R = 0.0;

#if defined (Q_OS_WIN32)

    QCAP_GET_AUDIO_BUFFER_VOLUME_DB(pBuffer, nBufferLen, format->nAudioChannels,
                                    format->nAudioBitsPerSample, format->nAudioSampleFrequency, 0, &dB_L);

    QCAP_GET_AUDIO_BUFFER_VOLUME_DB(pBuffer, nBufferLen, format->nAudioChannels,
                                    format->nAudioBitsPerSample, format->nAudioSampleFrequency, 1, &dB_R);

#elif defined (Q_OS_LINUX)

    if (format->nAudioChannels == 2 && format->nAudioBitsPerSample == 16) {

        PVOID pRCBuffer = QCAP_BUFFER_GET_RCBUFFER(pBuffer, nBufferLen);
        qcap_av_frame_t *pAVFrame = reinterpret_cast<qcap_av_frame_t *>(QCAP_RCBUFFER_LOCK_DATA(pRCBuffer));

        SHORT *pos = reinterpret_cast<SHORT *>(pAVFrame->pData[0]);
        ULONG samples = pAVFrame->nSamples;

        for (ULONG i = 0; i < samples; i++) {

            dB_L += abs(*pos++);
            dB_R += abs(*pos++);
        }

        dB_L /= samples;
        dB_R /= samples;

        dB_L = 20 * log10(dB_L / 32768.0);
        dB_R = 20 * log10(dB_R / 32768.0);

        QCAP_RCBUFFER_UNLOCK_DATA(pRCBuffer);
    }

#endif

    dB.dB_L = static_cast<int>(qBound(0, (int)dB_L + 90, 100));
    dB.dB_R = static_cast<int>(qBound(0, (int)dB_R + 90, 100));

    emit audioVolumeDB(dB);

    return dB;
}

qcap_snapshot_info_t* QcapBase::SnapshotInfo()
{
    return &m_QcapSnapshotInfo;
}

QString QcapBase::snapshot(qcap_format_t *format, qcap_snapshot_info_t *snapshot, BYTE *buffer, ULONG bufferSize)
{
    QString filePathName;

    if (snapshot->flag) {

        if (snapshot->nCount % snapshot->nSkipNum == 0) {

            if (static_cast<ULONG>(snapshot->nCount / snapshot->nSkipNum) < snapshot->nContinuousNum) {

                QDate dt_now = QDate::currentDate();
                QTime tm_now = QTime::currentTime();
                QString currentTime = dt_now.toString("yyyy/MM/dd/") + tm_now.toString("hh/mm/ss/zzz");

                QStringList list = { "%Y", "%M", "%D", "%h", "%m", "%s", "%i" };

                filePathName = snapshot->filePathName;

                for (int i = 0; i < list.count(); i++)
                    filePathName = filePathName.replace(list.at(i), currentTime.split("/")[i]);

                if (snapshot->bIsBmp) {

                    filePathName = filePathName + ".bmp";

#if defined (Q_OS_WIN32)

                    Q_UNUSED(bufferSize);

                    QCAP_SNAPSHOT_BUFFER_TO_BMP_EX(filePathName.toLocal8Bit().data(), format->nColorspaceType, buffer, format->nVideoWidth,
                                                   format->nVideoHeight, format->nVideoPitch, 0, 0, 0, 0, 0, 0, FALSE);

#elif defined (Q_OS_LINUX)

                    QCAP_SNAPSHOT_BUFFER_TO_BMP_EX(filePathName.toLocal8Bit().data(), format->nColorspaceType, buffer, format->nVideoWidth,
                                                   format->nVideoHeight, bufferSize, 0, 0, 0, 0, 0, 0, FALSE);

#endif
                }
                else {

                    filePathName = filePathName + ".jpg";

#if defined (Q_OS_WIN32)

                    Q_UNUSED(bufferSize);

                    QCAP_SNAPSHOT_BUFFER_TO_JPG_EX(filePathName.toLocal8Bit().data(), format->nColorspaceType, buffer, format->nVideoWidth,
                                                   format->nVideoHeight, format->nVideoPitch, 0, 0, 0, 0, 0, 0, 100, FALSE);

#elif defined (Q_OS_LINUX)

                    QCAP_SNAPSHOT_BUFFER_TO_JPG_EX(filePathName.toLocal8Bit().data(), format->nColorspaceType, buffer, format->nVideoWidth,
                                                   format->nVideoHeight, bufferSize, 0, 0, 0, 0, 0, 0, 100, FALSE);

#endif
                }
            }
            else {

                snapshot->flag = false;
            }
        }

        snapshot->nCount++;
    }

    return filePathName;
}

void QcapBase::startSnapshot(qcap_snapshot_info_t *snapshot)
{
    memcpy((void*)&m_QcapSnapshotInfo, (void*)snapshot, sizeof(qcap_snapshot_info_t));

    m_QcapSnapshotInfo.filePathName = QString(snapshot->filePathName);

    m_QcapSnapshotInfo.flag = true;
}

void QcapBase::stopSnapshot()
{
    m_QcapSnapshotInfo.flag = false;
}

QImage QcapBase::ConvertToQImage(qcap_format_t *format, BYTE *buffer)
{
    QImage img(format->nVideoWidth, format->nVideoHeight, QImage::Format_BGR888);

    ULONG nBufferLen = format->nVideoWidth * format->nVideoHeight * 3;

    BYTE *pBuffer = nullptr;
    if (format->nColorspaceType == QCAP_COLORSPACE_TYPE_BGR24) {

        img.fromData(buffer, nBufferLen);
    }
    else {

        pBuffer = reinterpret_cast<BYTE *>(malloc(nBufferLen));

        switch (format->nColorspaceType) {

        case QCAP_COLORSPACE_TYPE_YV12:

            QCAP_COLORSPACE_YV12_TO_BGR24(buffer, format->nVideoWidth, format->nVideoHeight, 0,
                                          pBuffer, format->nVideoWidth, format->nVideoHeight, 0);

            break;
        case QCAP_COLORSPACE_TYPE_YUY2:

            QCAP_COLORSPACE_YUY2_TO_BGR24(buffer, format->nVideoWidth, format->nVideoHeight, 0,
                                          pBuffer, format->nVideoWidth, format->nVideoHeight, 0);

            break;
        }

        for (ulong y = 0; y < format->nVideoHeight; y++)
            memcpy(img.scanLine(y), pBuffer + (y * format->nVideoWidth * 3), img.bytesPerLine());

        free(pBuffer);
    }

    return img;
}

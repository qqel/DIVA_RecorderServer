#include "qcapdevice.h"

inline QString ProductName(PVOID pDevice)
{
    BYTE serialNum[MAX_PATH];

    memset(serialNum, 0, MAX_PATH);

    QCAP_GET_DEVICE_CUSTOM_PROPERTY_EX(pDevice, 28, serialNum, MAX_PATH);

    return QString(QByteArray((char *)serialNum, MAX_PATH));
}

QRETURN onDeviceFormatChangedCB(PVOID  pDevice,
                                ULONG  nVideoInput,
                                ULONG  nAudioInput,
                                ULONG  nVideoWidth,
                                ULONG  nVideoHeight,
                                BOOL   bVideoIsInterleaved,
                                double dVideoFrameRate,
                                ULONG  nAudioChannels,
                                ULONG  nAudioBitsPerSample,
                                ULONG  nAudioSampleFrequency,
                                PVOID  pUserData)
{
    QcapDevice *pQcapDevice = reinterpret_cast<QcapDevice *>(pUserData);

    if (nVideoWidth > 0 && nVideoHeight > 0) {

        ULONG colorspaceType = 0xFFFFFFFF;
        qcap_format_t *format = pQcapDevice->Format();

        QCAP_GET_VIDEO_CURRENT_INPUT_FORMAT(
                    pDevice, &colorspaceType, &format->nVideoWidth, &format->nVideoHeight,
                    &format->bVideoIsInterleaved, &format->dVideoFrameRate);

        if (colorspaceType == QCAP_COLORSPACE_TYPE_MJPG)
            colorspaceType = QCAP_COLORSPACE_TYPE_YV12;

        format->nColorspaceType = colorspaceType;

        format->pixelFormat = pQcapDevice->getPixelFormat(colorspaceType);

        // INPUT TYPE
        format->nVideoInput = nVideoInput;
        format->nAudioInput = nAudioInput;

        // VIDEO FORMAT
        format->nVideoWidth = nVideoWidth;
        format->nVideoHeight = nVideoHeight;

        format->nVideoPitch = pQcapDevice->getVideoPitch(colorspaceType, nVideoWidth);

        format->bVideoIsInterleaved = bVideoIsInterleaved;

        format->dVideoFrameRate = dVideoFrameRate;

        // AUDIO FORMAT
        format->nAudioChannels = nAudioChannels;
        format->nAudioBitsPerSample = nAudioBitsPerSample;
        format->nAudioSampleFrequency = nAudioSampleFrequency;

        pQcapDevice->VideoData()->pFormat = format;
        pQcapDevice->AudioData()->pFormat = format;

        // INIT PREVIEW RESIZE BUFFER
        qcap_resize_frame_buffer_t *resizeBuffer = pQcapDevice->ResizeBuffer();
        resizeBuffer->nWidth = 416;
        resizeBuffer->nHeight = 234;
        resizeBuffer->nPitch = pQcapDevice->getVideoPitch(colorspaceType, resizeBuffer->nWidth);

        resizeBuffer->nBufferLen = resizeBuffer->nPitch*resizeBuffer->nHeight
                *(resizeBuffer->nWidth == resizeBuffer->nPitch ? (double)3/2 : 1);

        if (resizeBuffer->pBuffer)
            delete resizeBuffer->pBuffer;

        resizeBuffer->pBuffer = new BYTE[resizeBuffer->nBufferLen];
    }

    return QCAP_RT_OK;
}

QRETURN onDeviceNoSignalDetectedCB(PVOID pDevice,
                                   ULONG nVideoInput,
                                   ULONG nAudioInput,
                                   PVOID pUserData)
{
    Q_UNUSED(nVideoInput);
    Q_UNUSED(nAudioInput);

    QcapDevice *pQcapDevice = reinterpret_cast<QcapDevice *>(pUserData);

    return QCAP_RT_OK;
}

QRETURN onDeviceSignalRemovedCB(PVOID pDevice,
                                ULONG nVideoInput,
                                ULONG nAudioInput,
                                PVOID pUserData)
{
    Q_UNUSED(pDevice);
    Q_UNUSED(nVideoInput);
    Q_UNUSED(nAudioInput);

    return QCAP_RT_OK;
}

QRETURN onDeviceVideoPreviewCB(PVOID  pDevice,
                               double dSampleTime,
                               BYTE * pFrameBuffer,
                               ULONG  nFrameBufferLen,
                               PVOID  pUserData)
{
    Q_UNUSED(pDevice)

    QcapDevice *pQcapDevice = reinterpret_cast<QcapDevice *>(pUserData);

    if (nFrameBufferLen > 0) {

        qcap_format_t *format = pQcapDevice->Format();

        // EMIT VIDEO PREVIEW
        qcap_resize_frame_buffer_t *resizeBuffer = pQcapDevice->ResizeBuffer();
        pQcapDevice->resizeFrameBuffer(resizeBuffer, format, pFrameBuffer);
        pQcapDevice->emitVideoPreview(resizeBuffer->pBuffer, resizeBuffer->nBufferLen, resizeBuffer->nWidth,
                                      resizeBuffer->nHeight, resizeBuffer->nPitch, format->pixelFormat);

        // EMIT VIDEO BUFFER
        qcap_data_t *data = pQcapDevice->VideoData();
        pQcapDevice->setQcapData(data, dSampleTime, pFrameBuffer, nFrameBufferLen);
        emit pQcapDevice->deviceVideoPreviewCB(format, data);

        // SNAPSHOT
        qcap_snapshot_info_t *snapshot = pQcapDevice->SnapshotInfo();
        pQcapDevice->snapshot(format, snapshot, pFrameBuffer);

        // EMIT VIDEO STREAM
        QList<QcapStream *> streamList = pQcapDevice->QcapStreamServerList();
        if (!streamList.isEmpty())
            foreach (QcapStream *stream, streamList)
                stream->setVideoFrameBuffer(pQcapDevice->Format(), pFrameBuffer, nFrameBufferLen, dSampleTime);
    }

    return QCAP_RT_OK;
}

QRETURN onDeviceAudioPreviewCB(PVOID  pDevice,
                               double dSampleTime,
                               BYTE * pFrameBuffer,
                               ULONG  nFrameBufferLen,
                               PVOID  pUserData)
{
    Q_UNUSED(pDevice);

    QcapDevice *pQcapDevice = reinterpret_cast<QcapDevice *>(pUserData);

    if (nFrameBufferLen > 0) {

        qcap_format_t *format = pQcapDevice->Format();
        // STREAM
        QList<QcapStream *> streamList = pQcapDevice->QcapStreamServerList();
        if (!streamList.isEmpty())
            foreach (QcapStream *stream, streamList)
                stream->setAudioFrameBuffer(format, pFrameBuffer, nFrameBufferLen, dSampleTime);

    }

    return QCAP_RT_OK;
}

QcapDevice::QcapDevice(uint32_t previewCH) : m_nPreviewCH(previewCH)
{
#if ENABLE_MOVE_TO_THREAD
    qcap_move_to_thread(this);
#endif
    paramInit();
}

QcapDevice::~QcapDevice()
{
    stopStreamServer();

    deviceUninit();

    paramUninit();
}

void QcapDevice::paramInit()
{
    m_pQcapFormat = new qcap_format_t();
    m_pQcapVideoData = new qcap_data_t();
    m_pQcapAudioData = new qcap_data_t();
    m_pResizeBuffer = new qcap_resize_frame_buffer_t();
}

void QcapDevice::paramUninit()
{
    delete m_pQcapFormat;
    delete m_pQcapVideoData;
    delete m_pQcapAudioData;

    if (m_pResizeBuffer->pBuffer)
        delete m_pResizeBuffer->pBuffer;

    delete m_pResizeBuffer;
}


QMultiMap<QString, int> QcapDevice::enumDevice()
{
    qDebug() << __func__;

    QMultiMap<QString, int> map;

    ULONGLONG *list = nullptr;
    ULONG size = 0;
    int ch = 0;

    QCAP_DEVICE_ENUMERATION(&list, &size,
                            nullptr, nullptr, nullptr,
                            nullptr, nullptr, nullptr,
                            QCAP_ENUM_TYPE_SERIAL_NUMBER);

    if (size > 0)
        map.insert(QString::number(list[0], 16).right(8).toUpper(), ch++);

    for (int i = 1; i < static_cast<int>(size); i++) {

        if (list[i] != list[i-1])
            ch = 0;

        map.insert(QString::number(list[i], 16).right(8).toUpper(), ch++);
    }

    return map;
}

uint32_t QcapDevice::PreviewCH()
{
    return m_nPreviewCH;
}

void QcapDevice::setPreviewCH(uint32_t previewCH)
{
    m_nPreviewCH = previewCH;
}

uint32_t QcapDevice::DeviceNum()
{
    return m_nDeviceNum;
}

void QcapDevice::setDevice(QString deviceName, uint32_t deviceNum)
{    
    deviceInit(deviceName, deviceNum);
}

void QcapDevice::runDevice()
{
    QtConcurrent::run([&]() { if (m_pDevice != NULL) QCAP_RUN(m_pDevice); });
}

void QcapDevice::stopDevice()
{
    QtConcurrent::run([&]() { if (m_pDevice != NULL) QCAP_STOP(m_pDevice); });
}

void QcapDevice::deviceInit(QString deviceName, uint32_t deviceNum)
{

    deviceUninit();

    m_strDeviceName = deviceName;

    m_nDeviceNum = deviceNum;

    QByteArray devName = deviceName.toLocal8Bit();

    PVOID pUserData = reinterpret_cast<PVOID>(this);

    QRESULT RS = QCAP_CREATE(devName.data(), deviceNum, NULL, &m_pDevice);

    qDebug() << "RS" << RS;

    static ulong create_count = 0;

    if (RS == QCAP_RS_SUCCESSFUL) {

        create_count = 0;

        QCAP_REGISTER_FORMAT_CHANGED_CALLBACK(m_pDevice, onDeviceFormatChangedCB, pUserData);

        QCAP_REGISTER_NO_SIGNAL_DETECTED_CALLBACK(m_pDevice, onDeviceNoSignalDetectedCB, pUserData);

        QCAP_REGISTER_SIGNAL_REMOVED_CALLBACK(m_pDevice, onDeviceSignalRemovedCB, pUserData);

        QCAP_REGISTER_VIDEO_PREVIEW_CALLBACK(m_pDevice, onDeviceVideoPreviewCB, pUserData);

        QCAP_REGISTER_AUDIO_PREVIEW_CALLBACK(m_pDevice, onDeviceAudioPreviewCB, pUserData);

        qDebug() << QCAP_RUN(m_pDevice);

    }
    else {

        if (create_count <= 30)
            deviceInit(deviceName, deviceNum);

        create_count++;
    }
}

void QcapDevice::deviceUninit()
{
    if (m_pDevice) {

        QCAP_STOP(m_pDevice);

        QCAP_DESTROY(m_pDevice);

        m_pDevice = NULL;
    }
}

qcap_format_t* QcapDevice::Format()
{
    return m_pQcapFormat;
}

qcap_data_t* QcapDevice::VideoData()
{
    return m_pQcapVideoData;
}

qcap_data_t* QcapDevice::AudioData()
{
    return m_pQcapAudioData;
}

qcap_resize_frame_buffer_t* QcapDevice::ResizeBuffer()
{
    return m_pResizeBuffer;
}

qcap_device_av_input_config_t QcapDevice::InputConfig() const
{
    qcap_device_av_input_config_t config;

    QCAP_GET_VIDEO_INPUT_CONFIG(m_pDevice, &config.video);

    QCAP_GET_AUDIO_INPUT_CONFIG(m_pDevice, &config.audio);

    return config;
}

ULONG QcapDevice::VideoInputType() const
{
    ULONG input;

    QCAP_GET_VIDEO_INPUT(m_pDevice, &input);

    return input;
}

void QcapDevice::setVideoInputType(const ULONG &input)
{
    QCAP_SET_VIDEO_INPUT(m_pDevice, input);
}

ULONG QcapDevice::AudioInputType() const
{
    ULONG input;

    if (m_strDeviceName == QString("UB3300 USB")) {

        QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 255, &input);
    }
    else {

        QCAP_GET_AUDIO_INPUT(m_pDevice, &input);
    }

    return input;
}

void QcapDevice::setAudioInputType(const ULONG &input)
{
    if (m_strDeviceName == QString("UB3300 USB")) {

        QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 255, input);
    }
    else {

        QCAP_SET_AUDIO_INPUT(m_pDevice, input);
    }
}

ULONG QcapDevice::AudioVolume() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 251, &value);

    value = (int)(value * 100 / 155);

    return value;
}

void QcapDevice::setAudioVolume(const ULONG value)
{
    ULONG volume = value;

    volume = (int)(volume * 155 / 100);

    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 251, volume);
}

ULONG QcapDevice::TransferPipe() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 10, &value);

    return value == 1 ? 0 : 1;
}

void QcapDevice::setTransferPipe(const ULONG value)
{
    // 0 : BULK, 1 : ISO
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 10, value == 1 ? 0 : 1);

    Sleep(1000);

    deviceInit(m_strDeviceName, m_nDeviceNum);
}

ULONG QcapDevice::ColorRange() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 231, &value);

    return value;
}

void QcapDevice::setColorRange(const ULONG value)
{
    // 0 : PASS, 1 : SHRINK, 2 : EXPAND
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 231, value);
}

ULONG QcapDevice::EdidMode() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 968, &value);

    return value;
}

void QcapDevice::setEdidMode(const ULONG value)
{
    // 0 : DEFAULT, 1 : PASS THROUGH MONITOR, 2 : MERGE
    // LOOD THROUGH ONLY, OTHER USING PROPERTY 978
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 968, value);
}

ULONG QcapDevice::Resolution() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 238, &value);

    return value == 1 ? 0 : 1;
}

void QcapDevice::setResolution(const ULONG value)
{
    // 0 : NORMAL, 1 : FIX MODE
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 238, value == 1 ? 0 : 1);

    Sleep(1000);
}

ULONG QcapDevice::SdiMapping() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 613, &value);

    return value;
}

void QcapDevice::setSdiMapping(const ULONG value)
{
    // 0 : 2SI, 1 : QSD
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 613, value);
}

ULONG QcapDevice::HdrToneMapping() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 722, &value);

    return value == 1 ? 0 : 1;
}

void QcapDevice::setHdrToneMapping(const ULONG value)
{
    // 0 : DISABLE, 1 : ENABLE
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 722, value == 1 ? 0 : 1);
}

ULONG QcapDevice::OSD() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 914, &value);

    return value;
}

void QcapDevice::setOSD(const ULONG value)
{
    // 0 : DISABLE, 1 : ENABLE
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 914, value);
}

qcap_device_mirror_t QcapDevice::Mirror() const
{
    qcap_device_mirror_t mirror;

    mirror.ver = 0;
    mirror.hor = 0;

    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 244, &mirror.ver);

    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 245, &mirror.hor);

    return mirror;
}

void QcapDevice::setMirror(const qcap_device_mirror_t &mirror)
{
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 244, mirror.ver);

    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 245, mirror.hor);
}

ULONG QcapDevice::SdiEqMode() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 240, &value);

    return value == 1 ? 0 : 1;
}

void QcapDevice::setSdiEqMode(const ULONG value)
{
    // 0 : BYPASS, 1 : RX EQ
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 240, value == 1 ? 0 : 1);
}

ULONG QcapDevice::CropAspectRatio() const
{
    ULONG value = 0;
    QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 275, &value);

    return (value == 0 || value == 1) ? value : 0;
}

void QcapDevice::setCropAspectRatio(const ULONG value)
{
    // 0 : BYPASS, 1 : 4:3 (BOSER)
    QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_pDevice, 275, value);
}

ULONG QcapDevice::Brightness() const
{
    ULONG value;

    QCAP_GET_VIDEO_BRIGHTNESS(m_pDevice, &value);

    return value;
}

void QcapDevice::setBrightness(const ULONG &value)
{
    QtConcurrent::run([&, value]() {

        QCAP_SET_VIDEO_BRIGHTNESS(m_pDevice, value);
    });
}

ULONG QcapDevice::Contrast() const
{
    ULONG value;

    QCAP_GET_VIDEO_CONTRAST(m_pDevice, &value);

    return value;
}

void QcapDevice::setContrast(const ULONG &value)
{
    QtConcurrent::run([&, value]() {

        QCAP_SET_VIDEO_CONTRAST(m_pDevice, value);
    });
}

ULONG QcapDevice::Hue() const
{
    ULONG value;

    QCAP_GET_VIDEO_HUE(m_pDevice, &value);

    return value;
}

void QcapDevice::setHue(const ULONG &value)
{
    QtConcurrent::run([&, value]() {

        QCAP_SET_VIDEO_HUE(m_pDevice, value);
    });
}

ULONG QcapDevice::Saturation() const
{
    ULONG value;

    QCAP_GET_VIDEO_SATURATION(m_pDevice, &value);

    return value;
}

void QcapDevice::setSaturation(const ULONG &value)
{
    QtConcurrent::run([&, value]() {

        QCAP_SET_VIDEO_SATURATION(m_pDevice, value);
    });
}

ULONG QcapDevice::Sharpness() const
{
    ULONG value;

    QCAP_GET_VIDEO_SHARPNESS(m_pDevice, &value);

    return value;
}

void QcapDevice::setSharpness(const ULONG &value)
{
    QtConcurrent::run([&, value]() {

        QCAP_SET_VIDEO_SHARPNESS(m_pDevice, value);
    });
}

QList<QcapStream *> QcapDevice::QcapStreamServerList()
{
    return m_pQcapStreamServerList;
}

void QcapDevice::startStreamRtspServer(qcap_encode_property_t *property, CHAR *account, CHAR *password, ULONG port, ULONG httpPort)
{
    QcapStream *stream = new QcapStream();
    stream->startRtspServer(m_pQcapFormat, property, account, password, port, httpPort);

    m_pQcapStreamServerList.append(stream);
}

void QcapDevice::startStreamHlsServer(qcap_encode_property_t *property, CHAR *rootFolderPath, CHAR *subFolderPath)
{
    QcapStream *stream = new QcapStream();
    stream->startHlsServer(m_pQcapFormat, property, rootFolderPath, subFolderPath);

    m_pQcapStreamServerList.append(stream);
}

void QcapDevice::startStreamWebrtcServer(qcap_encode_property_t *property, CHAR *ip, ULONG port, CHAR *name)
{
    QcapStream *stream = new QcapStream();
    stream->startWebrtcServer(m_pQcapFormat, property, ip, port, name);

    m_pQcapStreamServerList.append(stream);
}

void QcapDevice::startStreamWebrtcChatter(QString strPeerID)
{

}

void QcapDevice::stopStreamServer()
{
    while (m_pQcapStreamServerList.length() > 0) {

        QcapStream* stream = m_pQcapStreamServerList.front();

        m_pQcapStreamServerList.pop_front();

        delete stream;
    }
    m_pQcapStreamServerList.clear();
}


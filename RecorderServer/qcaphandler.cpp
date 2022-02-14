#include "qcaphandler.h"

inline void setEncodeProperty(qcap_encode_property_t *property,
                              uint32_t encoderType,
                              uint32_t encoderFormat,
                              uint32_t recordMode,
                              uint32_t complexity,
                              uint32_t bitrateKbps,
                              uint32_t gop,
                              double_t segment = 0.0)
{
    property->nEncoderType = encoderType;
    property->nVideoEncoderFormat = encoderFormat;
    property->nRecordMode = recordMode;
    property->nComplexity = complexity;
    property->nBitrateKbps = bitrateKbps;
    property->nGop = gop;
    property->dSegment = segment;
}

QcapHandler::QcapHandler()
{
#if ENABLE_MOVE_TO_THREAD
    qcap_move_to_thread(this);
#endif
    // INIT QCAP SYSTEM CONFIG
    qcap_system_config_t config;
    config.nAutoInputDetectionTimeout = 1000;
    config.bEnableAsyncBackgroundSnapshot = TRUE;
    setSystemConfig(config);
}

QcapHandler::~QcapHandler()
{
    foreach (QcapDevice *pQcapDevice, m_pQcapDeviceList)
    {
        qDebug() << "delete pQcapDevice";
        delete pQcapDevice;
    }

    foreach (QcapPgm *pQcapEncoder, m_pQcapEncoderList)
    {
        qDebug() << "delete pQcapDevice";
        delete pQcapEncoder;
    }
}

void QcapHandler::autoCreateDevicePGM()
{
    QtConcurrent::run([&]() {

        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        QMultiMap<QString, int> multiMap = QcapDevice::enumDevice();

        foreach (QString key, multiMap.uniqueKeys())
            foreach (int value, multiMap.values(key)){
                qDebug() << key << value << false << multiMap.size();

                QcapDevice *pQcapDevice = new QcapDevice(value);
                m_pQcapDeviceList.append(pQcapDevice);

                getQcapDevice(value)->setDevice("SC0710 PCI", 0);

                setQcapPgm(0,1920,1080,60);
            }
    });

}

QcapDevice *QcapHandler::getQcapDevice(uint32_t previewCH)
{
    return m_pQcapDeviceList.length() > static_cast<int>(previewCH) ?
                m_pQcapDeviceList.at(previewCH) : nullptr;
}

void QcapHandler::newQcapDevice(uint32_t previewCH)
{
    QcapDevice *pQcapDevice = new QcapDevice(previewCH);

    m_pQcapDeviceList.append(pQcapDevice);

}

void QcapHandler::deleteQcapDevice(uint32_t previewCH)
{
    delete m_pQcapDeviceList.at(previewCH);

    m_pQcapDeviceList.removeAt(previewCH);
}

void QcapHandler::refreshQcapDevicePreviewChannel()
{
    for (int ch = 0; ch < m_pQcapDeviceList.length(); ch++)
        m_pQcapDeviceList.at(ch)->setPreviewCH(ch);
}

void QcapHandler::setQcapPgm(uint32_t previewCH, ULONG width, ULONG height, double framerate)
{
    QcapPgm *pQcapPgm = new QcapPgm(previewCH, width, height, framerate);

    pQcapPgm->startPGM(width, height, framerate);

    m_pQcapEncoderList.append(pQcapPgm);

}

void QcapHandler::setQcapEncoderStartStreamWebrtcServer(uint32_t previewCH, QString ip, uint32_t port, QString name, uint32_t encoderType, uint32_t encoderFormat, uint32_t recordMode, uint32_t complexity, uint32_t bitrateKbps, uint32_t gop)
{
    qDebug() << __func__;
    if (m_pQcapEncoderList.at(previewCH)) {

        QByteArray pszIp = ip.toLocal8Bit();
        QByteArray pszName = name.toLocal8Bit();

        qcap_encode_property_t *pProperty = new qcap_encode_property_t();
        setEncodeProperty(pProperty, encoderType, encoderFormat,
                          recordMode, complexity, bitrateKbps, gop);

#if defined (Q_OS_LINUX)

        pProperty->nAudioEncoderFormat = QCAP_ENCODER_FORMAT_AAC_ADTS;

#endif

        m_pQcapEncoderList.at(previewCH)->startStreamWebrtcServer(pProperty, pszIp.data(), port, pszName.data());
    }
    else {

        qDebug() << __func__ << "Invalid m_pQcapDevice!!";
    }
}

void QcapHandler::setQcapEncoderStartStreamWebrtcChatter(uint32_t previewCH, ULONG nPeerID)
{

}

void QcapHandler::enumStreamWebrtcChatter()
{
    qDebug() << __func__;

    m_pQcapEncoderList.at(0)->enumStreamWebrtcChatter();
}


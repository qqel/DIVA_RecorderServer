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
}

void QcapHandler::autoCreateDevice()
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

void QcapHandler::setQcapDeviceStartStreamRtspServer(uint32_t previewCH,
                                                     QString account,
                                                     QString password,
                                                     uint32_t port,
                                                     uint32_t httpPort,
                                                     uint32_t encoderType,
                                                     uint32_t encoderFormat,
                                                     uint32_t recordMode,
                                                     uint32_t complexity,
                                                     uint32_t bitrateKbps,
                                                     uint32_t gop)
{
    if (m_pQcapDeviceList.at(previewCH)) {

        QByteArray pszAccount = account.toLocal8Bit();
        QByteArray pszPassword = password.toLocal8Bit();

        qcap_encode_property_t *pProperty = new qcap_encode_property_t();
        setEncodeProperty(pProperty, encoderType, encoderFormat,
                          recordMode, complexity, bitrateKbps, gop);

#if defined (Q_OS_LINUX)

        pProperty->nAudioEncoderFormat = QCAP_ENCODER_FORMAT_AAC_ADTS;

#endif

        m_pQcapDeviceList.at(previewCH)->startStreamRtspServer(pProperty, pszAccount.data(), pszPassword.data(),
                                                               port, httpPort);
    }
    else {

        qDebug() << __func__ << "Invalid m_pQcapDevice!!";
    }
}

void QcapHandler::setQcapDeviceStartStreamWebrtcServer(uint32_t previewCH,
                                                       QString ip,
                                                       uint32_t port,
                                                       QString name,
                                                       uint32_t encoderType,
                                                       uint32_t encoderFormat,
                                                       uint32_t recordMode,
                                                       uint32_t complexity,
                                                       uint32_t bitrateKbps,
                                                       uint32_t gop)
{
    if (m_pQcapDeviceList.at(previewCH)) {

        QByteArray pszIp = ip.toLocal8Bit();
        QByteArray pszName = name.toLocal8Bit();

        qcap_encode_property_t *pProperty = new qcap_encode_property_t();
        setEncodeProperty(pProperty, encoderType, encoderFormat,
                          recordMode, complexity, bitrateKbps, gop);

#if defined (Q_OS_LINUX)

        pProperty->nAudioEncoderFormat = QCAP_ENCODER_FORMAT_AAC_ADTS;

#endif

        m_pQcapDeviceList.at(previewCH)->startStreamWebrtcServer(pProperty, pszIp.data(), port, pszName.data());
    }
    else {

        qDebug() << __func__ << "Invalid m_pQcapDevice!!";
    }
}

void QcapHandler::setQcapDeviceStartStreamWebrtcChatter(uint32_t previewCH, QString strPeerID)
{

}

#include "qcapstream.h"

inline ULONG getValidSvrNum()
{
    ULONG num = 0;

    BOOL valid;
    do { QCAP_GET_BROADCAST_SERVER_STATUS(num++, &valid); } while (valid);
    num--;

    return num > 63 ? 63 : num;
}

QcapStream::QcapStream()
{
#if ENABLE_MOVE_TO_THREAD
    qcap_move_to_thread(this);
#endif
}

QcapStream::~QcapStream()
{
    stopServer();
}

void QcapStream::startRtspServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *account, CHAR *password, ULONG port, ULONG httpPort)
{
    stopServer();

    QCAP_CREATE_BROADCAST_RTSP_SERVER(getValidSvrNum(), 1, &m_pServer, account, password, port, httpPort);

    startServer(format, property);
}

void QcapStream::startRtspWebPortalServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *url, CHAR *account, CHAR *password)
{
#if defined (Q_OS_WIN32)
    stopServer();

    QCAP_CREATE_BROADCAST_RTSP_WEB_PORTAL_SERVER(getValidSvrNum(), url, &m_pServer, account, password);

    startServer(format, property);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(url);
    Q_UNUSED(account);
    Q_UNUSED(password);
#endif
}

void QcapStream::startHlsServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *rootFolderPath, CHAR *subFolderPath)
{
    stopServer();

    QCAP_CREATE_BROADCAST_HLS_SERVER(getValidSvrNum(), 1, &m_pServer, rootFolderPath, subFolderPath, 500, FALSE, 10);

    startServer(format, property);
}

void QcapStream::startRtmpServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *account, CHAR *password, ULONG port, ULONG httpPort)
{
#if defined (Q_OS_WIN32)
    stopServer();

    QCAP_CREATE_BROADCAST_RTMP_SERVER(getValidSvrNum(), 1, &m_pServer, account, password, port, httpPort);

    startServer(format, property);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(account);
    Q_UNUSED(password);
    Q_UNUSED(port);
    Q_UNUSED(httpPort);
#endif
}

void QcapStream::startRtmpWebPortalServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *url, CHAR *account, CHAR *password)
{
    stopServer();

    QCAP_CREATE_BROADCAST_RTMP_WEB_PORTAL_SERVER(getValidSvrNum(), url, &m_pServer, account, password);

    startServer(format, property);
}

void QcapStream::startTsOverSrtServer(qcap_format_t *format, qcap_encode_property_t *property, ULONG port, CHAR *key)
{
#if defined (Q_OS_WIN32)
    stopServer();

    QCAP_CREATE_BROADCAST_TS_OVER_SRT_SERVER(getValidSvrNum(), &m_pServer, port);

    startServer(format, property, key);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(port);
    Q_UNUSED(key);
#endif
}

void QcapStream::startTsOverSrtWebPortalServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *url, CHAR *key, CHAR *account, CHAR *password)
{
#if defined (Q_OS_WIN32)
    stopServer();

    QCAP_CREATE_BROADCAST_TS_OVER_SRT_WEB_PORTAL_SERVER(getValidSvrNum(), url, &m_pServer, account, password);

    startServer(format, property, key);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(url);
    Q_UNUSED(key);
    Q_UNUSED(account);
    Q_UNUSED(password);
#endif
}

void QcapStream::startNdiServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *NdiName)
{
#if defined (Q_OS_WIN32)
    stopServer();

    QCAP_CREATE_BROADCAST_NDI_SERVER(getValidSvrNum(), NdiName, NULL, &m_pServer);

    startServer(format, property);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(NdiName);
#endif
}

void QcapStream::startNdiHxServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *NdiName, CHAR *key)
{
#if defined (Q_OS_WIN32)
    stopServer();

    QCAP_CREATE_BROADCAST_NDI_HX_SERVER(getValidSvrNum(), NdiName, NULL, &m_pServer);

    startServer(format, property, key);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(NdiName);
    Q_UNUSED(key);
#endif
}

void QcapStream::startWebrtcServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *ip, ULONG port, CHAR *name)
{
#if defined (Q_OS_WIN32)
    stopServer();

    ULONG m_chatter_id;

    QCAP_CREATE_WEBRTC_CHATTER( ip, port, name, &m_pChatter, &m_chatter_id  );

    QCAP_CREATE_WEBRTC_SENDER( m_pChatter, 0, 1, &m_pServer );

    startServer(format, property);
#else
    Q_UNUSED(format);
    Q_UNUSED(property);
    Q_UNUSED(NdiName);
    Q_UNUSED(key);
#endif
}

void QcapStream::startWebrtcChat(ULONG strPeerID)
{
    if(m_pChatter)
    {
        QRESULT RT;

        RT = QCAP_START_WEBRTC_CHAT( m_pChatter, strPeerID );

        if( RT == QCAP_RS_SUCCESSFUL)
        {
            m_bIsConnected = true;
        }
        else{
            QCAP_STOP_WEBRTC_CHAT(m_pChatter);

            m_bIsConnected = false;
        }
    }
}

QList<ULONG> QcapStream::enumWebrtcChatter()
{
    QList<ULONG> listPeerID;

    ULONG nUserCount = 0, nPeerID = 0;

    QString PeerColumn;

    CHAR *PeerName;

    listPeerID.clear();

    QRESULT ret = QCAP_ENUM_WEBRTC_USER_IN_CHATROOM( m_pChatter, &nPeerID, &PeerName, FALSE );

    qDebug("QCAP_ENUM_WEBRTC_USER_IN_CHATROOM ret = %u, peerID = %u, peerName = %s\n", ret, nPeerID, PeerName );

    PeerColumn = QString::number(nPeerID) + ", " + QString(PeerName);

    qDebug("HGHH %s", PeerColumn.toLatin1().data());

    if ( ret != QCAP_RS_SUCCESSFUL ) {

        qDebug("Faild to QCAP_ENUM_WEBRTC_USER_IN_CHATROOM \n");

        return listPeerID;
    }

    listPeerID.append(nPeerID);

    nUserCount ++;

    while ( QCAP_RS_SUCCESSFUL == QCAP_ENUM_WEBRTC_USER_IN_CHATROOM( m_pChatter, &nPeerID, &PeerName, TRUE )) {

        qDebug("QCAP_ENUM_WEBRTC_USER_IN_CHATROOM ret = %u, peerID = %u, peerName = %s\n", ret, nPeerID, PeerName );

        sprintf( PeerColumn.toLatin1().data(), "%u, %s", nPeerID, PeerName );

        listPeerID.append(nPeerID);

        nUserCount ++;
    }
}

bool QcapStream::getWebrtcIsConnected()
{
    return m_bIsConnected;
}

void QcapStream::startServer(qcap_format_t *format, qcap_encode_property_t *property, CHAR *key)
{
    ULONG nProfile = adjustRecordProfile(format->nVideoWidth, format->nVideoHeight);
    ULONG nLevel = adjustRecordLevel(format->nVideoWidth, format->nVideoHeight);
    double dFrameRate = adjustFrameRate(format->dVideoFrameRate);

    UINT iGpuNum = 0;

#if defined (Q_OS_WIN32)
    if(property->nEncoderType == QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK)
        iGpuNum = QCAP_QUERY_ENCODER_TYPE_CAP(1, QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK, QCAP_ENCODER_FORMAT_H264, NULL) == QCAP_RS_ERROR_NON_SUPPORT ? 0 : 1;
#endif

    QCAP_SET_VIDEO_BROADCAST_SERVER_PROPERTY_EX(
                m_pServer, 0, iGpuNum, property->nEncoderType, property->nVideoEncoderFormat, format->nColorspaceType,
                format->nVideoWidth, format->nVideoHeight, dFrameRate, nProfile, nLevel, QCAP_RECORD_ENTROPY_CABAC,
                property->nComplexity, property->nRecordMode, 8000, property->nBitrateKbps*1024, property->nGop,
                0, FALSE, 0, 0, 0, TRUE, FALSE, FALSE, 0, 0, 0, 0, 0, 0, 0, NULL, FALSE, FALSE, QCAP_BROADCAST_FLAG_FULL | QCAP_BROADCAST_FLAG_IMMEDIATE
                );

    QCAP_SET_AUDIO_BROADCAST_SERVER_PROPERTY(
                m_pServer, 0, QCAP_ENCODER_TYPE_SOFTWARE, property->nAudioEncoderFormat,
                format->nAudioChannels, format->nAudioBitsPerSample, format->nAudioSampleFrequency
                );

    QCAP_START_BROADCAST_SERVER(m_pServer, key);

    m_bSvrFlag = true;
}

void QcapStream::stopServer()
{
    if (m_bSvrFlag) {

        m_bSvrFlag = false;

        QCAP_STOP_BROADCAST_SERVER(m_pServer);

        QCAP_DESTROY_BROADCAST_SERVER(m_pServer);

        m_pServer = nullptr;

        if(m_pChatter) {

            QCAP_DESTROY_WEBRTC_CHATTER(m_pChatter);

            m_pChatter = nullptr;
        }
    }
}

void QcapStream::setVideoFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime)
{
    if (m_bSvrFlag)
        QCAP_SET_VIDEO_BROADCAST_SERVER_UNCOMPRESSION_BUFFER(
                    m_pServer, 0, format->nColorspaceType, format->nVideoWidth, format->nVideoHeight,
                    pFrameBuffer, nFrameBufferLen, dSampleTime
                    );
}

void QcapStream::setAudioFrameBuffer(qcap_format_t *format, BYTE *pFrameBuffer, ULONG nFrameBufferLen, double dSampleTime)
{
#if defined (Q_OS_WIN32)

    if (m_bSvrFlag)
        QCAP_SET_AUDIO_BROADCAST_SERVER_UNCOMPRESSION_BUFFER_EX(
                    m_pServer, 0, format->nAudioChannels, format->nAudioBitsPerSample, format->nAudioSampleFrequency,
                    pFrameBuffer, nFrameBufferLen, NULL, 0, dSampleTime
                    );

#elif defined (Q_OS_LINUX)

    Q_UNUSED(format);

    if (m_bSvrFlag)
        QCAP_SET_AUDIO_BROADCAST_SERVER_UNCOMPRESSION_BUFFER(m_pServer, 0, pFrameBuffer, nFrameBufferLen, dSampleTime);

#endif
}

void QcapStream::setVideoStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, BOOL bIsKeyFrame, double dSampleTime)
{
    if (m_bSvrFlag)
        QCAP_SET_VIDEO_BROADCAST_SERVER_COMPRESSION_BUFFER(
                    m_pServer, 0, pStreamBuffer, nStreamBufferLen, bIsKeyFrame, dSampleTime
                    );
}

void QcapStream::setAudioStreamBuffer(BYTE *pStreamBuffer, ULONG nStreamBufferLen, double dSampleTime)
{
    if (m_bSvrFlag)
        QCAP_SET_AUDIO_BROADCAST_SERVER_COMPRESSION_BUFFER(
                    m_pServer, 0, pStreamBuffer, nStreamBufferLen, dSampleTime
                    );
}

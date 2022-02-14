#include "qcaphandler.h"

#define SHARE_ENCODER 0
#define ENCODER_WIDTH 1920
#define ENCODER_HEIGHT 1080
#define ENCODER_FPS 60

QRETURN on_format_changed_callback( PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, ULONG nVideoWidth, ULONG nVideoHeight, BOOL bVideoIsInterleaved, double dVideoFrameRate, ULONG nAudioChannels, ULONG nAudioBitsPerSample, ULONG nAudioSampleFrequency, PVOID pUserData )
{
    QcapHandler* m_pMainDialog =  (QcapHandler * ) pUserData;

    m_pMainDialog->m_nDeviceVideoWidth = nVideoWidth;

    m_pMainDialog->m_nDeviceVideoHeight = nVideoHeight;

    m_pMainDialog->m_bDeviceVideoIsInterleaved = bVideoIsInterleaved;

    m_pMainDialog->m_dDeviceVideoFrameRate = dVideoFrameRate;

    m_pMainDialog->m_nDeviceAudioChannel = nAudioChannels;

    m_pMainDialog->m_nDeviceAudioBitsPerSample = nAudioBitsPerSample;

    m_pMainDialog->m_nDeviceAudioSampleFrequency = nAudioSampleFrequency;

    qDebug( "on_format_changed_callback( %d, %d, %d, %2.3f, %d, %d, %d )", nVideoWidth, nVideoHeight, bVideoIsInterleaved, dVideoFrameRate, nAudioChannels, nAudioBitsPerSample, nAudioSampleFrequency );

    return QCAP_RT_OK;
}

QRETURN on_no_signal_detected_callback( PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, PVOID pUserData )
{
    QcapHandler* m_pMainDialog =  (QcapHandler * ) pUserData;

    m_pMainDialog->m_nDeviceVideoWidth = 0;

    m_pMainDialog->m_nDeviceVideoHeight = 0;

    m_pMainDialog->m_bDeviceVideoIsInterleaved = FALSE;

    m_pMainDialog->m_dDeviceVideoFrameRate = 0.0;

    m_pMainDialog->m_nDeviceAudioChannel = 0;

    m_pMainDialog->m_nDeviceAudioBitsPerSample = 0;

    m_pMainDialog->m_nDeviceAudioSampleFrequency = 0;

    qDebug( "on_no_signal_detected_callback()" );

    return QCAP_RT_OK;
}

QRETURN on_video_preview_callback( PVOID pDevice, double dSampleTime, BYTE * pFrameBuffer, ULONG nFrameBufferLen, PVOID pUserData )
{
    QcapHandler* m_pMainDialog =  (QcapHandler * ) pUserData;

    if(nFrameBufferLen != 0)
    {
        QCAP_SET_VIDEO_SHARE_RECORD_UNCOMPRESSION_BUFFER(SHARE_ENCODER,QCAP_COLORSPACE_TYPE_YUY2, 1920, 1080, pFrameBuffer, nFrameBufferLen, dSampleTime);
    }

    return QCAP_RT_OK;
}

QRETURN on_audio_preview_callback( PVOID pDevice, double dSampleTime, BYTE * pFrameBuffer, ULONG nFrameBufferLen, PVOID pUserData )
{
    QcapHandler* m_pMainDialog =  (QcapHandler * ) pUserData;

    if(nFrameBufferLen != 0)
    {
        QCAP_SET_AUDIO_SHARE_RECORD_UNCOMPRESSION_BUFFER(SHARE_ENCODER, pFrameBuffer, nFrameBufferLen, dSampleTime);
    }

    return QCAP_RT_OK;
}

QRETURN on_video_sharerecord_callback(UINT iRecNum /*IN*/, double dSampleTime /*IN*/, BYTE * pStreamBuffer /*IN*/, ULONG nStreamBufferLen /*IN*/, BOOL bIsKeyFrame /*IN*/, PVOID pUserData /*IN*/ )
{
    QcapHandler* m_pMainDialog =  (QcapHandler * ) pUserData;

    static int count = 0;

    if( m_pMainDialog->m_nStartChatState >= 0x00000001 ) {

        char buffer[64];

        sprintf( buffer, "%s -  %d", m_pMainDialog->m_chatter_username, count);

        count ++ ;

        QCAP_SET_OSD_TEXT( m_pMainDialog->m_pDevice, 0, 0, 0, 0, 0, buffer, (char*)"Arial", QCAP_FONT_STYLE_BOLD, 72, 0xFFFFFFFF, 0xFFFF0000, 0xFF, 0, 0 );

        QCAP_SET_VIDEO_BROADCAST_SERVER_COMPRESSION_BUFFER( m_pMainDialog->m_pSender,
                                                            0,
                                                            pStreamBuffer,
                                                            nStreamBufferLen,
                                                            bIsKeyFrame,
                                                            dSampleTime);
    }

    return QCAP_RT_OK;
}

QRETURN on_audio_sharerecord_callback(UINT iRecNum /*IN*/, double dSampleTime /*IN*/, BYTE * pStreamBuffer /*IN*/, ULONG nStreamBufferLen /*IN*/, PVOID pUserData /*IN*/ )
{
    QcapHandler* m_pMainDialog =  (QcapHandler * ) pUserData;

    if( m_pMainDialog->m_nStartChatState >= 0x00000001 ) {

        QCAP_SET_AUDIO_BROADCAST_SERVER_UNCOMPRESSION_BUFFER( m_pMainDialog->m_pSender,
                                                              0,
                                                              pStreamBuffer,
                                                              nStreamBufferLen );
    }

    return QCAP_RT_OK;
}
QcapHandler::QcapHandler(QObject *parent) : QObject(parent)
{
    QCAP_CREATE( (char*) "SC0710 PCI", 0, NULL, &m_pDevice, TRUE, FALSE );

    QCAP_REGISTER_FORMAT_CHANGED_CALLBACK( m_pDevice, on_format_changed_callback, this );

    QCAP_REGISTER_NO_SIGNAL_DETECTED_CALLBACK( m_pDevice, on_no_signal_detected_callback, this );

    QCAP_REGISTER_VIDEO_PREVIEW_CALLBACK( m_pDevice, on_video_preview_callback, this );

    QCAP_REGISTER_AUDIO_PREVIEW_CALLBACK( m_pDevice, on_audio_preview_callback, this );

    QCAP_RUN( m_pDevice );

    QCAP_SET_AUDIO_VOLUME(m_pDevice , 0);

    QCAP_SET_VIDEO_SHARE_RECORD_PROPERTY(SHARE_ENCODER,
                                         QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK,
                                         QCAP_ENCODER_FORMAT_H264,
                                         QCAP_COLORSPACE_TYPE_YUY2,
                                         ENCODER_WIDTH,ENCODER_HEIGHT,ENCODER_FPS,QCAP_RECORD_MODE_CBR,8000,8*1000*1000,30,0,0);
    QCAP_SET_AUDIO_SHARE_RECORD_PROPERTY(SHARE_ENCODER,
                                         QCAP_ENCODER_TYPE_SOFTWARE,
                                         QCAP_ENCODER_FORMAT_AAC,
                                         2,16,48000);

    QCAP_REGISTER_VIDEO_SHARE_RECORD_CALLBACK(SHARE_ENCODER, on_video_sharerecord_callback, this);

    QCAP_REGISTER_AUDIO_SHARE_RECORD_CALLBACK(SHARE_ENCODER, on_audio_sharerecord_callback, this);

    QCAP_START_SHARE_RECORD(SHARE_ENCODER, (char*)"", QCAP_RECORD_FLAG_ENCODE);
}

QcapHandler::~QcapHandler()
{
    QCAP_STOP_SHARE_RECORD(SHARE_ENCODER);

    m_nStartChatState = 0x00000000;

    if( m_pChatter ) {

        QCAP_DESTROY_WEBRTC_CHATTER( m_pChatter );

        m_pChatter= nullptr;
    }

    if( m_pSender ) {

        QCAP_STOP_BROADCAST_SERVER( m_pSender );

        QCAP_DESTROY_BROADCAST_SERVER( m_pSender );

        m_pSender = nullptr;
    }

    if( m_pReceiver ) {

        QCAP_STOP_BROADCAST_CLIENT( m_pReceiver );

        QCAP_DESTROY_BROADCAST_CLIENT( m_pReceiver );

        m_pReceiver = nullptr;
    }

    if(m_pDevice)
    {
        QCAP_STOP(m_pDevice);

        QCAP_DESTROY(m_pDevice);

        m_pDevice = nullptr;
    }
}

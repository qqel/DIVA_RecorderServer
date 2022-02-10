#ifndef QCAPBASE_H
#define QCAPBASE_H

#include <QObject>
#include <QVideoSurfaceFormat>
#include <QThread>
#include <QtConcurrent>
#include <QMetaType>
#include <QQueue>
#include <QDebug>
#if defined (Q_OS_WIN)
#include <Windows.h>
#include <comdef.h>
#include "QCAP.H"
#elif defined (Q_OS_LINUX)
#include "qcap.h"
#include "qcap.windef.h"
#include "qcap.linux.h"
#endif


#define ENABLE_MOVE_TO_THREAD 1

// LENOVO
#define ENABLE_LENOVO (CUSTOMER == 9)

struct qcap_version {
    ULONG major;
    ULONG minor;
};
typedef struct qcap_version qcap_version_t;

struct qcap_system_config {
    BOOL   bEnableMultipleUsersAccess = TRUE;
    BOOL   bEnableVideoPreviewDevice = TRUE;
    BOOL   bEnableAudioPreviewDevice = TRUE;
    BOOL   bEnableVideoHardwareMainEncoderDevice = TRUE;
    BOOL   bEnableVideoHardwareSubEncoderDevice = TRUE;
    ULONG  nAutoInputDetectionTimeout = 3000;
    BOOL   bEnableSCF = FALSE;
    CHAR * pszDB3 = NULL;
    BOOL   bEnableAsyncBackgroundSnapshot = FALSE;
    BOOL   bEnableEnhancedVideoRenderer = TRUE;
    BOOL   bEnableSystemTimeCallback = FALSE;
    BOOL   bEnableFileRepairFunction = TRUE;
    BOOL   bEnableNewRTSPLibrary = TRUE;
    CHAR * pszWebServerRootFolderPath = NULL;
    CHAR * pszWebServerIP = NULL;
    ULONG  nSystemColorRangeType = QCAP_COLORRANGE_TYPE_FULL;
    BOOL   bEnableVideoMixingRendererBugPatch = TRUE;
    ULONG  nEnableCustomVideoRenderer = 0x00000000;
    BOOL   bEnableGraphicMemoryForVideoEncoder = FALSE;
    BOOL   bEnableSingleGraphCaptureMode = FALSE;
    ULONG  nSignalDetectionDuration = 200;
    BOOL   bEnableNewChromaKeyLibrary = FALSE;
    BOOL   bEnableNewSnapshotLibrary = TRUE;
    BOOL   bEnableAppleOnFlyMP4Editing = FALSE;
    BOOL   bEnableNewSoftwareEncoderLibrary = FALSE;
};
typedef struct qcap_system_config qcap_system_config_t;

struct qcap_format {
    ULONG  nColorspaceType = 0xFFFFFFFF;
    QVideoFrame::PixelFormat pixelFormat = QVideoFrame::Format_Invalid;
    ULONG  nVideoInput = 0;
    ULONG  nAudioInput = 0;
    ULONG  nVideoWidth = 0;
    ULONG  nVideoHeight = 0;
    ULONG  nVideoPitch = 0;
    BOOL   bVideoIsInterleaved = FALSE;
    double dVideoFrameRate = 0.0;
    ULONG  nAudioChannels = 0;
    ULONG  nAudioBitsPerSample = 0;
    ULONG  nAudioSampleFrequency = 0;
};
typedef struct qcap_format qcap_format_t;

struct qcap_data {
    qcap_format_t* pFormat = NULL;
    double         sampleTime;
    BYTE *         buffer = nullptr;
    ULONG          bufferLen = 0;
    BOOL           isKeyFrame;
    bool           bCrop = false;
    ULONG          crop_x = 0;
    ULONG          crop_y = 0;
    ULONG          crop_w = 0;
    ULONG          crop_h = 0;
    bool           bFace5kps = false;
    float *        face5kpsFeature;
    QString        face5kpsName;
    QList<float *> pFace5kpsFeatureList;
    QList<QString> pFace5kpsNameList;
    QList<ulong>   cropXList;
    QList<ulong>   cropYList;
    QList<ulong>   cropWList;
    QList<ulong>   cropHList;
};
typedef struct qcap_data qcap_data_t;

struct qcap_resize_frame_buffer {
    BYTE * pBuffer = NULL;
    ULONG  nBufferLen = 0;
    ULONG  nWidth = 0;
    ULONG  nHeight = 0;
    ULONG  nPitch = 0;
};
typedef qcap_resize_frame_buffer qcap_resize_frame_buffer_t;

struct qcap_audio_volume_db {
    int dB_L;
    int dB_R;
};
typedef struct qcap_audio_volume_db qcap_audio_volume_db_t;

struct qcap_encode_property {
    QString filePathName;
    ULONG   nEncoderType = QCAP_ENCODER_TYPE_SOFTWARE;
    ULONG   nVideoEncoderFormat = QCAP_ENCODER_FORMAT_H264;
    ULONG   nAudioEncoderFormat = QCAP_ENCODER_FORMAT_AAC;
    ULONG   nComplexity = QCAP_RECORD_COMPLEXITY_0;
    ULONG   nRecordMode = QCAP_RECORD_MODE_CBR;
    ULONG   nBitrateKbps = 3000;
    ULONG   nGop = 30;
    double  dSegment = 0.0;
};
typedef qcap_encode_property qcap_encode_property_t;

struct qcap_snapshot_info {
     QString filePathName;
     bool    bIsBmp = false;
     ULONG   nContinuousNum = 0;
     ULONG   nSkipNum = 0;
     ULONG   nCount = 0;
     bool    flag = false;
};
typedef qcap_snapshot_info qcap_snapshot_info_t;

void copySourceFrameBuffer(qcap_format_t *srcFormat, qcap_data_t *srcData, qcap_format_t *dstFormat, qcap_data_t *dstData);

class QcapBase : public QObject
{
    Q_OBJECT
public:
    explicit QcapBase();
    ~QcapBase();

    // MOVE TO THREAD
    void qcap_move_to_thread(QObject *obj);

    static const qcap_version_t version();
    static void setSystemConfig(const qcap_system_config_t &config);
    static const QList<uint32_t> queryEncoderType();

    // QCAP FORMAT
    QVideoFrame::PixelFormat getPixelFormat(const ULONG &colorspace);
    ULONG getVideoPitch(const ULONG &colorspace, const ULONG &width);

    // QCAP DATA
    void setQcapData(qcap_data_t *data, double dSampleTime, BYTE *pBuffer, ULONG nBufferLen, BOOL bIsKeyFrame = FALSE);

    // QCAP ENCODER PROPERTY
    double adjustFrameRate(double dFrameRate);
    unsigned long adjustRecordProfile(const int &width, const int &height);
    unsigned long adjustRecordLevel(const int &width, const int &height);

    void emitVideoPreview(unsigned char *frameBuffer, int frameBufferSize, int width, int height, int bytesPerLine, int colorFormat);

    // RESIZE FRAME BUFFER
    void resizeFrameBuffer(qcap_resize_frame_buffer_t *resizeBuffer, qcap_format_t *format, BYTE *pBuffer);

    // CALCULATE AUDIO VOLUME BUFFER
    const qcap_audio_volume_db_t audioBufferVolumeDB(qcap_format_t *format, BYTE *pBuffer, ULONG nBufferLen);

    // SNAPSHOT
    qcap_snapshot_info_t* SnapshotInfo();
    QString snapshot(qcap_format_t *format, qcap_snapshot_info_t *snapshot, BYTE *buffer, ULONG bufferSize = 0);
    void startSnapshot(qcap_snapshot_info_t *snapshot);
    void stopSnapshot();

    // HELPER FUNCTION
    QImage ConvertToQImage(qcap_format_t *format, BYTE *buffer);

signals:
    void videoPreview(std::shared_ptr<QVideoFrame>frame);
    void audioVolumeDB(qcap_audio_volume_db_t db);

    void callSnapshotDoneCallback(QString filename);

private:
#if ENABLE_MOVE_TO_THREAD
    QThread *m_pThread;
#endif

    qcap_snapshot_info_t m_QcapSnapshotInfo;
};

#endif // QCAPBASE_H

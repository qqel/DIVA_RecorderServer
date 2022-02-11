#ifndef QCAPPREVIEW_H
#define QCAPPREVIEW_H

#include <QAudioOutput>
#include "qcapshare.h"
#include "qcapstream.h"

class QcapPgm : public QcapBase
{
    Q_OBJECT
public:
    explicit QcapPgm(uint32_t previewCH, ULONG width, ULONG height, double framerate);
    ~QcapPgm();

    qcap_format_t* Format();
    qcap_resize_frame_buffer_t* ResizeBuffer();
    ULONG m_nPreviewFrameNum;
    bool isOtherScreenVideoPreview;
    bool isHighPerformance;
    int m_nCount=0;

    void emitOtherScreenVideoPreview(unsigned char *frameBuffer, int frameBufferSize, int width, int height, int bytesPerLine, int colorFormat);

    void startPGM(ULONG width, ULONG height, double framerate);
    void stopEncoder();

    // NO SIGNAL FRAME BUFFER
    void InitNoSignalBuffer(QString filepath);
    void setNoSignalBuffer();

    // RECORD
    QcapShare* QcapShareRecord();
    void startRecord(qcap_encode_property_t *property);
    void stopRecord();

    // STREAM
    QList<QcapStream *> QcapStreamServerList();
    void startStreamWebrtcServer(qcap_encode_property_t *property, CHAR *ip, ULONG port, CHAR *name);
    void startStreamWebrtcChat(ULONG strPeerID);
    QList<ULONG> enumStreamWebrtcChatter();

signals:
    void callFrameCountSignal();
    void otherScreenVideoPreview(std::shared_ptr<QVideoFrame>frame);

public slots:
    void receivVideoData(qcap_format_t *format, qcap_data_t *data);
    void receivAudioData(qcap_format_t *format, qcap_data_t *data);

private:
    QcapShare *m_pQcapShareEncoder;
    qcap_format_t *m_Format;
    qcap_encode_property_t *m_Property;
    qcap_share_callback_t *m_Callback;
    qcap_resize_frame_buffer_t *m_pResizeBuffer;
    uint32_t m_nPreviewCH;

    // RECORD
    QcapShare *m_pQcapShareRecord;
    // STREAM
    QList<QcapStream*> m_pQcapStreamServerList;

};
#endif // QCAPPREVIEW_H

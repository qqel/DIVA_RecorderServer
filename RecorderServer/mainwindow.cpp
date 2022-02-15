#include "mainwindow.h"
#include "ui_mainwindow.h"
static bool g_bMainDbg = false;

void MainWindow::timerEvent( QTimerEvent *event )
{
    if( event->timerId() == m_nQcapTimer )
    {

    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEdit_peerId->setText("1");

    m_nQcapTimer = -1;

    m_pQcapHandler = new QcapHandler();

    m_pWebsocketHandler = new WebsocketHandler();

    m_nQcapTimer = startTimer(2000);

}

MainWindow::~MainWindow()
{
    qDebug() << __func__;

    delete m_pWebsocketHandler;

    delete m_pQcapHandler;

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    // start chat
    qDebug() << __func__;
    ULONG peer_id = ui->lineEdit_peerId->text().toLong();
    m_pQcapHandler->setChatter(peer_id);
}

void MainWindow::on_pushButton_2_clicked()
{
    // enum
    qDebug() << "user total:" << m_pQcapHandler->m_listWebRTC.count();
    m_pQcapHandler->m_listWebRTC.at(0)->enumUser();
}

void MainWindow::on_pushButton_3_clicked()
{
    //start server

    m_pQcapHandler->addNewChatter("127.0.0.1", 8888, "test_client");

//    m_pQcapHandler->setQcapEncoderStartStreamWebrtcServer(0,
//                                                          "127.0.0.1",
//                                                          8888,
//                                                          "client0",
//                                                          QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK,
//                                                          QCAP_ENCODER_FORMAT_H264,
//                                                          QCAP_RECORD_MODE_CBR,
//                                                          0,
//                                                          8*1000*1000,
//                                                          30);
}

void MainWindow::on_pushButton_4_clicked()
{
    // TEST
    QJsonObject jsonSend;

    jsonSend["FUNCTION"] = "SDVOEWEB_SYS_SET_LOGIN";
    jsonSend["ACCOUNT"] = "root";
    jsonSend["PASSWORD"] = "root";

    QJsonDocument doc(jsonSend);
    m_pWebsocketHandler->setMessage(doc.toJson());
}

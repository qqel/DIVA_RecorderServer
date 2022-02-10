#include "mainwindow.h"
#include "ui_mainwindow.h"
static bool g_bDbg = false;

void MainWindow::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == m_nConnectTimer )
    {
        if(g_bDbg) qDebug() << "client status:" << m_pClient->state();
        if(m_bIsConnected == false)
        {
            qDebug() << "try to connect server...";
            m_pClient->open(QUrl("ws://10.10.42.70:8081"));
        }
        else
        {
            return;
        }
    }
    else if( event->timerId() == m_nQcapTimer )
    {
        QcapDevice *pDevice = m_pQcapHandler->getQcapDevice(0);
        if(pDevice != nullptr)
        {
            if(pDevice->Format()->nVideoWidth != 0)
            {
//                m_pQcapHandler->setQcapDeviceStartStreamRtspServer(0,
//                                                                   "root",
//                                                                   "root",
//                                                                   5566,
//                                                                   5567,
//                                                                   QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK,
//                                                                   QCAP_ENCODER_FORMAT_H264,
//                                                                   0,
//                                                                   QCAP_RECORD_MODE_CBR,
//                                                                   8*1000*1000,
//                                                                   30);
//                m_pQcapHandler->setQcapDeviceStartStreamWebrtcServer(0,
//                                                                     "127.0.0.1",
//                                                                     8888,
//                                                                     "client0",
//                                                                     QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK,
//                                                                     QCAP_ENCODER_FORMAT_H264,
//                                                                     QCAP_RECORD_MODE_CBR,
//                                                                     0,
//                                                                     8*1000*1000,
//                                                                     30);
                killTimer(m_nQcapTimer);
            }
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_bIsConnected = false;

    m_nConnectTimer = -1;

    m_nQcapTimer = -1;

    m_pQcapHandler = new QcapHandler();

    m_pQcapHandler->autoCreateDevice();

    //m_pQcapHandler->
    m_pClient = new QWebSocket;

    initWebSocket();

    m_pClient->setParent(this);

    m_nConnectTimer = startTimer(2000);

    m_nQcapTimer = startTimer(2000);

}

MainWindow::~MainWindow()
{
    m_pClient->close();

    delete m_pQcapHandler;

    delete ui;
}

void MainWindow::initWebSocket()
{
    connect(m_pClient,&QWebSocket::connected,[this](){
        qDebug () << "IP:" << m_pClient->localAddress().toString();
        qDebug () << "PORT:" << m_pClient->localPort();
        qDebug()<<"connected";
        m_bIsConnected = true;

        m_pClient->sendTextMessage("connected!!");
    });
    connect(m_pClient,&QWebSocket::disconnected,[this](){
        qDebug()<<"disconnected";
        m_bIsConnected = false;
    });

    connect(m_pClient,&QWebSocket::textMessageReceived,[this](const QString &msg){
        qDebug() << msg;
    });
}

void MainWindow::on_pushButton_clicked()
{

}

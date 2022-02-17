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

    ui->lineEdit_peerName->setText("rec0");

    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)//我們使用IPv4地址
            qDebug() << address.toString();
    }

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

void MainWindow::on_pushButton_Preview_clicked()
{
    // step1. WEB send button signal to CTRL to REC
    // setp2. REC send port to CTRL to WEB
    ui->lineEdit_port->setText(QString::number(m_pQcapHandler->getChatRoomPort()));
}

void MainWindow::on_pushButton_clicked()
{
    // step3. WEB send peer_id and port to CTRL to REC
    // step4. REC start cahtter

    ULONG peer_id = ui->lineEdit_peerId->text().toLong();
    m_pQcapHandler->setChatter(peer_id, ui->lineEdit_port->text().toUInt());
}

void MainWindow::on_pushButton_2_clicked()
{
    // enum
    m_pQcapHandler->getWebrtcList();
}

void MainWindow::on_pushButton_3_clicked()
{
    //start server

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



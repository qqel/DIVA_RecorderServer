#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkInterface>
#include "websockethandler.h"
#include "qcaphandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /* WebSocket */
    WebsocketHandler *m_pWebsocketHandler;

    /* QCAP */
    int         m_nQcapTimer;
    QcapHandler *m_pQcapHandler;


private slots:
    void timerEvent( QTimerEvent *event );

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_Preview_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

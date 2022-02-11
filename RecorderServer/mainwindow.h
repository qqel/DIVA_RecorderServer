#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
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
    int         m_nConnectTimer;
    bool        m_bIsConnected;
    QWebSocket  *m_pClient;
    void        initWebSocket();

    /* QCAP */
    int         m_nQcapTimer;
    QcapHandler *m_pQcapHandler;

    PVOID pServer = NULL;
    PVOID pChatter = NULL;

private slots:
    void timerEvent( QTimerEvent *event );

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

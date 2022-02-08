#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int         m_nConnectTimer;
    bool        m_bIsConnected;
    QWebSocket *m_pClient;


private slots:
    void timerEvent( QTimerEvent *event );

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

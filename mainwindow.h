#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "serialcommunicator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void displayMessage(const QString &message);
    void updateSystemData(const system_data_to_pc &packet);
    void tryConnectionPeriodically();
    void handleSendClicked();

private:
    Ui::MainWindow *ui;
    SerialCommunicator *m_serialComm;
    int tryToConnectToCorrectSerialPort();
    bool m_isConnected;
    QTimer *m_connectionTimer;
    void printDebug(const QString &msg);
};
#endif // MAINWINDOW_H

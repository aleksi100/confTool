#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    void updateSystemData(const system_data_to_pc *packet);


private slots:
    void displayMessage(const QString &message);

private:
    Ui::MainWindow *ui;
    SerialCommunicator *m_serialComm;
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_serialComm = new SerialCommunicator(this);
    connect(m_serialComm, &SerialCommunicator::messageReceived,
            this, &MainWindow::displayMessage);

    // Open serial port (adjust port name as needed)
    if (!m_serialComm->openSerialPort("COM5")) {  // For Windows
        ui->debugTextEdit->append("Failed to open serial port");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayMessage(const QString &message)
{
    ui->debugTextEdit->append(message);
}

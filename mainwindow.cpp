#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QSerialPortInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_serialComm = new SerialCommunicator(this);
    connect(m_serialComm, &SerialCommunicator::messageReceived,
            this, &MainWindow::displayMessage);
    connect(m_serialComm, &SerialCommunicator::systemDataReceived,
            this, &MainWindow::updateSystemData); // Uusi yhteys signaaliin

    // if (!m_serialComm->openSerialPort("COM5")) {
    //     ui->debugTextEdit->append("Failed to open serial port");
    // }
    // Alusta ajastin
    m_connectionTimer = new QTimer(this);
    connect(m_connectionTimer, &QTimer::timeout, this, &MainWindow::tryConnectionPeriodically);
    m_connectionTimer->start(1000); // Aloita ajastin, joka laukaisee sekunnin välein

    // Yritä heti alussa yhdistää
    if (tryToConnectToCorrectSerialPort() == 1) {
        m_isConnected = true;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayMessage(const QString &message)
{
    printDebug(message);
}

void MainWindow::updateSystemData(const system_data_to_pc &packet)
{
    QString text;

    // Basic Info
    text += QString("Version: %1\n").arg(packet.data.version);
    text += QString("Current Kauha: %1\n").arg(packet.data.current_kauha);
    text += QString("Korkeus: %1\n").arg(packet.data.korkeus);
    text += QString("Korkeus ilman kaatoa: %1\n").arg(packet.data.korkeus_ilman_kaatoa);
    text += QString("Kaato: %1\n").arg(packet.data.kaato);
    text += QString("Tila: %1\n").arg(packet.data.tila);

    QScrollBar *system_info_scrollBar = ui->system_info_textBrowser->verticalScrollBar();
    int scrollPosition = system_info_scrollBar->value();
    ui->system_info_textBrowser->setText(text);
    system_info_scrollBar->setValue(scrollPosition);

    text = "";
    // Puomit
    for (int i = 0; i < 3; i++) {
        text += QString("Puomi %1:\n").arg(i);
        text += QString("  Pituus: %1\n").arg(packet.data.puomit[i].pituus);
        text += QString("  Korjaus: %1\n").arg(packet.data.puomit[i].korjaus);
    }
    text += "\n";

    // Kauhat
    for (int i = 0; i < 5; i++) {
        text += QString("Kauha %1:\n").arg(i);
        text += QString("  Name: %1\n").arg(QString(packet.data.kauhat[i].disp_name));
        text += QString("  Pituus: %1\n").arg(packet.data.kauhat[i].pituus);
        text += QString("  Korjaus: %1\n").arg(packet.data.kauhat[i].korjaus);
    }
    QScrollBar *puomisto_info_scrollBar = ui->puomisto_info_textBrowser->verticalScrollBar();
    scrollPosition = puomisto_info_scrollBar->value();
    ui->puomisto_info_textBrowser->setText(text);
    puomisto_info_scrollBar->setValue(scrollPosition);

    text = "";
    for (int i = 0; i < 4; i++) {
        text += QString("Anturi %1:\n").arg(i);
        text += QString("  Last Kulma: %1\n").arg(packet.data.kulma_anturit[i].last_kulma);
        text += QString("  Position: %1\n").arg(packet.data.kulma_anturit[i].position);
        text += QString("  Last Update: %1\n").arg(packet.data.kulma_anturit[i].last_update);
    }
    text += "\n";
    QScrollBar *anturi_info_scrollBar = ui->anturi_info_textBrowser->verticalScrollBar();
    scrollPosition = anturi_info_scrollBar->value();
    ui->anturi_info_textBrowser->setText(text);
    anturi_info_scrollBar->setValue(scrollPosition);
}

void MainWindow::tryConnectionPeriodically()
{
    if (!m_isConnected) {
        if (tryToConnectToCorrectSerialPort() == 1) {
            m_isConnected = true;
            m_connectionTimer->stop(); // Lopeta ajastin, kun yhteys onnistuu
            printDebug("Connection established, timer stopped.");
        }
    }
}

int MainWindow::tryToConnectToCorrectSerialPort()
{
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        qDebug() << "Checking port:" << info.portName()
        << "VID:" << QString::number(info.vendorIdentifier(), 16).toUpper()
        << "PID:" << QString::number(info.productIdentifier(), 16).toUpper()
        << "Description:" << info.description();

        if (info.vendorIdentifier() == 1155 &&
            info.productIdentifier() == 0x1333) {

            if (m_serialComm->openSerialPort(info.portName())) {
                printDebug("Auto-connected to " + info.portName() +
                                          " (Description: " + info.description() + ")");
                return 1; // Onnistui
            } else {

                printDebug("Failed to connect to " + info.portName());
            }
        }
    }
    ui->debugTextEdit->append("No suitable serial port found with VID=0x1155, PID=0x1333");
    return 0; // Epäonnistui
}
void MainWindow::printDebug(const QString &msg){
    QScrollBar *debug_scrollBar = ui->debugTextEdit->verticalScrollBar();
    int scrollPosition = debug_scrollBar->value();
    ui->debugTextEdit->append(msg);
    debug_scrollBar->setValue(scrollPosition);

}

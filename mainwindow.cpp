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

    connect(ui->sendCmdBtn, &QPushButton::clicked, this, &MainWindow::handleSendClicked);
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

void MainWindow::updateSystemData(const system_data_t &packet)
{
    system_data = packet;
    QString text;

    // Basic Info
    text += QString("Version: %1\n").arg(packet.version);
    text += QString("Current Kauha: %1\n").arg(packet.current_kauha);
    text += QString("Korkeus: %1\n").arg(packet.korkeus);
    text += QString("Korkeus ilman kaatoa: %1\n").arg(packet.korkeus_ilman_kaatoa);
    text += QString("Kaato: %1\n").arg(packet.kaato);
    text += QString("Tila: %1\n").arg(packet.tila);

    QScrollBar *system_info_scrollBar = ui->system_info_textBrowser->verticalScrollBar();
    int scrollPosition = system_info_scrollBar->value();
    ui->system_info_textBrowser->setText(text);
    system_info_scrollBar->setValue(scrollPosition);

    text = "";
    // Puomit
    for (int i = 0; i < 3; i++) {
        text += QString("Puomi %1:\n").arg(i);
        text += QString("  Pituus: %1\n").arg(packet.puomit[i].pituus);
        text += QString("  Korjaus: %1\n").arg(packet.puomit[i].korjaus);
    }
    text += "\n";

    // Kauhat
    for (int i = 0; i < 5; i++) {
        text += QString("Kauha %1:\n").arg(i);
        text += QString("  Name: %1\n").arg(QString(packet.kauhat[i].disp_name));
        text += QString("  Pituus: %1\n").arg(packet.kauhat[i].pituus);
        text += QString("  Korjaus: %1\n").arg(packet.kauhat[i].korjaus);
    }
    QScrollBar *puomisto_info_scrollBar = ui->puomisto_info_textBrowser->verticalScrollBar();
    scrollPosition = puomisto_info_scrollBar->value();
    ui->puomisto_info_textBrowser->setText(text);
    puomisto_info_scrollBar->setValue(scrollPosition);

    text = "";
    for (int i = 0; i < 4; i++) {
        text += QString("Anturi %1:\n").arg(i);
        text += QString("  Last Kulma: %1\n").arg(packet.kulma_anturit[i].last_kulma);
        text += QString("  Position: %1\n").arg(packet.kulma_anturit[i].position);
        text += QString("  Last Update: %1\n").arg(packet.kulma_anturit[i].last_update);
    }
    text += "\n";
    QScrollBar *anturi_info_scrollBar = ui->anturi_info_textBrowser->verticalScrollBar();
    scrollPosition = anturi_info_scrollBar->value();
    ui->anturi_info_textBrowser->setText(text);
    anturi_info_scrollBar->setValue(scrollPosition);
}

void MainWindow::tryConnectionPeriodically()
{
    qDebug() << m_serialComm->isSerialPortOpen();
    if (!m_serialComm->isSerialPortOpen()) {
        if (tryToConnectToCorrectSerialPort() == 1) {
            //m_connectionTimer->stop(); // Lopeta ajastin, kun yhteys onnistuu
            printDebug("Connection established");
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

    if(!ui->autoScroll_radioButton->isChecked()){
        QScrollBar *debug_scrollBar = ui->debugTextEdit->verticalScrollBar();
        int scrollPosition = debug_scrollBar->value();
        ui->debugTextEdit->append(msg);
        debug_scrollBar->setValue(scrollPosition);

    }else{
        ui->debugTextEdit->append(msg);
    }

}

void MainWindow::handleSendClicked(){
    system_data_t m_system_data;
    memcpy(&m_system_data, &system_data, sizeof(system_data_t));
    QString msg = ui->sendCmdLineEdit->text();
    QStringList msg_osat = msg.split(" ");
    QString root_var = msg_osat[0].split(".")[0];
    QString var = msg_osat[0].split(".")[1];
    QString operand = msg_osat[1];
    QByteArray value = msg_osat[2].toUtf8();
    if(root_var == "kauha1"){
        if(var == "nimi"){
            strncpy(m_system_data.kauhat[0].disp_name, value.constData(), sizeof(system_data.kauhat[0].disp_name));
        }
    }


    qDebug() << "kauhan nimi " << m_system_data.kauhat[0].disp_name;
    if(m_serialComm->sendPacketToSerial(m_system_data)){
        qDebug() << "error viesti ei lähtenyut";
            // Käsittele error jos jaksat
    }


}

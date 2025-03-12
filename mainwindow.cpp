#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>

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

void MainWindow::updateSystemData(const system_data_to_pc *packet)
{
    QString text;

    // Basic Info
    text += QString("Version: %1\n").arg(packet->data.version);
    text += QString("Current Kauha: %1\n").arg(packet->data.current_kauha);
    text += QString("Korkeus: %1\n").arg(packet->data.korkeus);
    text += QString("Korkeus ilman kaatoa: %1\n").arg(packet->data.korkeus_ilman_kaatoa);
    text += QString("Kaato: %1\n").arg(packet->data.kaato);
    text += QString("Tila: %1\n").arg(packet->data.tila);

    // Tallenna vieritysposition
    QScrollBar *system_info_scrollBar = ui->system_info_textBrowser->verticalScrollBar();
    int scrollPosition = system_info_scrollBar->value();
    ui->system_info_textBrowser->setText(text);
    system_info_scrollBar->setValue(scrollPosition);

    text = "";
    // Puomit
    for (int i = 0; i < 3; i++) {
        text += QString("Puomi %1:\n").arg(i);
        text += QString("  Pituus: %1\n").arg(packet->data.puomit[i].pituus);
        text += QString("  Korjaus: %1\n").arg(packet->data.puomit[i].korjaus);
    }
    text += "\n";

    // Kauhat
    for (int i = 0; i < 5; i++) {
        text += QString("Kauha %1:\n").arg(i);
        text += QString("  Name: %1\n").arg(QString(packet->data.kauhat[i].disp_name));
        text += QString("  Pituus: %1\n").arg(packet->data.kauhat[i].pituus);
        text += QString("  Korjaus: %1\n").arg(packet->data.kauhat[i].korjaus);
    }
    // Tallenna vieritysposition
    QScrollBar *puomisto_info_scrollBar = ui->puomisto_info_textBrowser->verticalScrollBar();
    scrollPosition = puomisto_info_scrollBar->value();
    ui->puomisto_info_textBrowser->setText(text);
    puomisto_info_scrollBar->setValue(scrollPosition);



    text = "";
    for (int i = 0; i < 4; i++) {
        text += QString("Anturi %1:\n").arg(i);
        text += QString("  Last Kulma: %1\n").arg(packet->data.kulma_anturit[i].last_kulma);
        text += QString("  Position: %1\n").arg(packet->data.kulma_anturit[i].position);
        text += QString("  Last Update: %1\n").arg(packet->data.kulma_anturit[i].last_update);
    }
    text += "\n";
    // Tallenna vierityspo
    QScrollBar *anturi_info_scrollBar = ui->anturi_info_textBrowser->verticalScrollBar();
    scrollPosition = anturi_info_scrollBar->value();
    ui->anturi_info_textBrowser->setText(text);
    anturi_info_scrollBar->setValue(scrollPosition);


}

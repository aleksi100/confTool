#include "serialcommunicator.h"
#include "mainwindow.h"
#include <QDebug>

SerialCommunicator::SerialCommunicator(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunicator::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialCommunicator::handleError);
}

SerialCommunicator::~SerialCommunicator()
{
    closeSerialPort();
}

bool SerialCommunicator::openSerialPort(const QString &portName)
{
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Serial port opened successfully";
        return true;
    }
    qDebug() << "Failed to open serial port:" << m_serialPort->errorString();
    return false;
}

void SerialCommunicator::closeSerialPort()
{
    if (m_serialPort->isOpen())
        m_serialPort->close();
}

void SerialCommunicator::handleReadyRead()
{
    m_buffer.append(m_serialPort->readAll());
    qDebug() << "Received buffer size:" << m_buffer.size();

    while (m_buffer.size() >= sizeof(system_data_to_pc)) {
        int startIdx = m_buffer.indexOf(char(0xAA));
        if (startIdx == -1) {
            m_buffer.clear();
            return;
        }

        if (startIdx > 0) {
            m_buffer = m_buffer.mid(startIdx);
        }

        system_data_to_pc *sysMsg = reinterpret_cast<system_data_to_pc*>(m_buffer.data());
        if (sysMsg->start == 0xAA && sysMsg->end == 0xBB && sysMsg->id == ID_SYSTEM_DATA_PACKET) {
            processPacket(m_buffer.left(sizeof(system_data_to_pc)));
            m_buffer = m_buffer.mid(sizeof(system_data_to_pc));
            continue;
        }
        m_buffer = m_buffer.mid(1); // Ohita virheellinen paketti
    }
}

void SerialCommunicator::processPacket(const QByteArray &data)
{
    if (data.size() >= sizeof(system_data_to_pc)) {
        system_data_to_pc *sysMsg = reinterpret_cast<system_data_to_pc*>(const_cast<char*>(data.data()));
        if (sysMsg->id == ID_SYSTEM_DATA_PACKET) {
            MainWindow *mainWin = qobject_cast<MainWindow*>(parent());
            if (mainWin) {
                mainWin->updateSystemData(sysMsg); // Päivitä GUI suoraan
            }
        }
    }
    if (data.size() >= sizeof(can_msg_to_pc)) {
        can_msg_to_pc *msg = reinterpret_cast<can_msg_to_pc*>(const_cast<char*>(data.data()));
        if (msg->id == ID_J1939_MSG) {
            QString message = QString("J1939 Message - ID: %1, PGN: %2, Priority: %3, Source: %4, Data: ")
            .arg(msg->id)
                .arg(msg->frame.pgn)
                .arg(msg->frame.priority)
                .arg(msg->frame.sourceAddr);
            for (int i = 0; i < 8; i++) {
                message += QString("%1 ").arg(msg->frame.data[i], 2, 16, QChar('0'));
            }
            emit messageReceived(message);
        }
    }
}

void SerialCommunicator::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        qDebug() << "Serial port error:" << m_serialPort->errorString();
        emit messageReceived("Serial port error: " + m_serialPort->errorString());
    }
}

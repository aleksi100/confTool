#include "serialcommunicator.h"
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
int SerialCommunicator::getPacketSize(uint8_t packetId) const
{
    switch (packetId) {
    case ID_SYSTEM_DATA_PACKET:
        return sizeof(system_data_t);
    case ID_J1939_MSG:
        return sizeof(can_msg_to_pc);
    case ID_ERROR_MSG:
        return sizeof(msg_packet_t);
    case ID_INFO_MSG:
        return sizeof(msg_packet_t);
    default:
        return -1; // Tuntematon paketti
    }
}
void SerialCommunicator::handleReadyRead()
{
    m_buffer.append(m_serialPort->readAll());
    qDebug() << "Received buffer size:" << m_buffer.size();

    while (m_buffer.size() > 0) {
        // Etsi aloitusmerkki
        int startIdx = m_buffer.indexOf(char(0xAA));
        if (startIdx == -1) {
            m_buffer.clear(); // Ei aloitusmerkkiä, tyhjennä puskuri
            return;
        }

        if (startIdx > 0) {
            m_buffer = m_buffer.mid(startIdx); // Poista roska alusta
        }

        // Varmista, että puskurissa on ainakin minimimäärä dataa (start + id)
        if (m_buffer.size() < 2) {
            return; // Odota lisää dataa
        }

        // Lue paketin id (oletetaan, että se on heti start-merkin jälkeen)
        uint8_t packetId = static_cast<uint8_t>(m_buffer[1]);
        int packetSize = getPacketSize(packetId);

        if (packetSize == -1) {
            // Tuntematon pakettityyppi, siirry seuraavaan tavuun
            m_buffer = m_buffer.mid(1);
            continue;
        }

        // Varmista, että koko paketti on puskurissa
        if (m_buffer.size() < packetSize) {
            return; // Odota lisää dataa
        }

        // Tarkista paketin rakenne
        const uint8_t *data = (uint8_t*)m_buffer.constData();
        if (data[0] == 0xAA && data[packetSize - 1] == 0xBB) {
            // Kelvollinen paketti, käsittele se
            processPacket(m_buffer.left(packetSize));
            m_buffer = m_buffer.mid(packetSize); // Poista käsitelty paketti
        } else {
            // Virheellinen paketti, siirry seuraavaan tavuun
            m_buffer = m_buffer.mid(1);
        }
    }
}

void SerialCommunicator::processPacket(const QByteArray &data)
{
    uint8_t packetId = static_cast<uint8_t>(data[1]);
    if (packetId == ID_SYSTEM_DATA_PACKET && data.size() >= sizeof(system_data_t)) {
        const system_data_t *sysMsg = reinterpret_cast<const system_data_t*>(data.constData());
        processSystemDataPacket(*sysMsg);
    } else if (packetId == ID_J1939_MSG && data.size() >= sizeof(can_msg_to_pc)) {
        const can_msg_to_pc *msg = reinterpret_cast<const can_msg_to_pc*>(data.constData());
        processCanMsgPacket(*msg);
    }else if (packetId == ID_ERROR_MSG && data.size() >= sizeof(msg_packet_t)) {
        const msg_packet_t *msg = reinterpret_cast<const msg_packet_t*>(data.constData());
        processErrorPacket(*msg);
    }else if (packetId == ID_INFO_MSG && data.size() >= sizeof(msg_packet_t)) {
        const msg_packet_t *msg = reinterpret_cast<const msg_packet_t*>(data.constData());
        processInfoPacket(*msg);
    }
}


void SerialCommunicator::processSystemDataPacket(const system_data_t &packet)
{
    emit systemDataReceived(packet); // Emitoi signaali GUI-päivitystä varten
}

void SerialCommunicator::processCanMsgPacket(const can_msg_to_pc &msg)
{
    QString message = QString("J1939 Message - ID: %1, PGN: %2, Priority: %3, Source: %4, Data: ")
    .arg(msg.id)
        .arg(msg.frame.pgn)
        .arg(msg.frame.priority)
        .arg(msg.frame.sourceAddr);
    for (int i = 0; i < 8; i++) {
        message += QString("%1 ").arg(msg.frame.data[i], 2, 16, QChar('0'));
    }
    emit messageReceived(message);
}
void SerialCommunicator::processInfoPacket(const msg_packet_t &msg)
{
    QString message = QString("stm32 -> Info: %1").arg(msg.msg);
    emit messageReceived(message);
}
void SerialCommunicator::processErrorPacket(const msg_packet_t &msg)
{
    QString message = QString("stm32 -> Error: %1").arg(msg.msg);
    emit messageReceived(message);
}

void SerialCommunicator::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        qDebug() << "Serial port error:" << m_serialPort->errorString();
        emit messageReceived("Serial port error: " + m_serialPort->errorString());
        closeSerialPort();

    }
}
bool SerialCommunicator::isSerialPortOpen(){
    return m_serialPort->isOpen();
}
bool SerialCommunicator::sendPacketToSerial(system_data_t &packet){
    int bytes_send = m_serialPort->write(reinterpret_cast<const char *>(&packet), sizeof(packet));
    if(bytes_send != sizeof(packet)){
        return true;
    }
    return false;
}


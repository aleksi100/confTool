#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <QObject>
#include <QSerialPort>
#include "types.h"

class SerialCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit SerialCommunicator(QObject *parent = nullptr);
    ~SerialCommunicator();

    bool openSerialPort(const QString &portName);
    bool isSerialPortOpen();
    void closeSerialPort();
    bool sendPacketToSerial(system_data_t &packet);

signals:
    void messageReceived(const QString &message);
    void systemDataReceived(const system_data_t &packet); // Uusi signaali

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_buffer;

    void processPacket(const QByteArray &data);
    void processSystemDataPacket(const system_data_t &packet);
    void processCanMsgPacket(const can_msg_to_pc &msg);
    void processErrorPacket(const msg_packet_t &msg);
    void processInfoPacket(const msg_packet_t &msg);
    int getPacketSize(uint8_t packetId) const;

};

#endif // SERIALCOMMUNICATOR_H

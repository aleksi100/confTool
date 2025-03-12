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
    void closeSerialPort();

signals:
    void messageReceived(const QString &message);
    void systemDataReceived(const system_data_to_pc &packet); // Uusi signaali

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_buffer;

    void processPacket(const QByteArray &data);
    void processSystemDataPacket(const system_data_to_pc &packet);
    void processCanMsgPacket(const can_msg_to_pc &msg);
    int getPacketSize(uint8_t packetId) const;
};

#endif // SERIALCOMMUNICATOR_H

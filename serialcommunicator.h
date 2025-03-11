#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <QObject>
#include <QSerialPort>
#pragma pack(1)
typedef struct {
    uint32_t pgn;          // Parameter Group Number
    uint8_t data[8];       // J1939 data (max 8 tavua)
    uint8_t priority;      // Prioriteetti
    uint32_t sourceAddr;   // Lähettäjän osoite
} J1939_Message_t;


typedef struct {
    uint8_t start;
    uint8_t id;
    J1939_Message_t frame;
    uint8_t end;
} can_msg_to_pc;
#pragma pack()

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

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_buffer;

    void processPacket(const QByteArray &data);
};

#endif // SERIALCOMMUNICATOR_H

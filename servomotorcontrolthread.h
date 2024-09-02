#ifndef SERVOMOTORCONTROLTHREAD_H
#define SERVOMOTORCONTROLTHREAD_H

#include <QThread>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QSerialPort>

#include <QDebug>

class ServoMotorControlThread : public QThread
{
    Q_OBJECT

public:
    ServoMotorControlThread(const QString &portName, int baudRate, int dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QObject *parent = nullptr);
    ~ServoMotorControlThread() override;

    void run() override;
    void stop();
    void writeRegister(int address, quint16 value, int serverAddress);
    void readRegister(int startAddress, int numberOfEntries, int serverAddress);

signals:
    void dataRead(const QModbusDataUnit &unit);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();

private:
    bool connectToDevice();
    void disconnectDevice();

    QModbusRtuSerialMaster *modbusDevice;
    QString portName;
    int baudRate;
    int dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    bool running;
};

#endif // SERVOMOTORCONTROLTHREAD_H

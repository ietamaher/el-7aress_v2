ifndef MODBUSCONTROLDEVICE_H
#define MODBUSCONTROLDEVICE_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QModbusDataUnit>
#include <QQueue>
#include <QPair>
#include <QTimer>
#include <QSerialPort>

class ModbusControl : public QObject
{
    Q_OBJECT

public:
    explicit ModbusControl(QObject *parent = nullptr);
    ~ModbusControl();

    bool connectToDevice(const QString &portName, int baudRate, int dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits);
    void disconnectDevice();

    void readRegister(int startAddress, int numberOfEntries, int serverAddress, bool highPriority = false);
    void writeRegister(int address, quint16 value, int serverAddress);

signals:
    void dataRead(const QModbusDataUnit &unit);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();
    void onTimeout();



private:
    QModbusRtuSerialMaster *modbusDevice;
    QTimer *timer;
    bool isBusy;
    QQueue<QPair<int, QModbusDataUnit>> requestQueue; // Queue to hold pending requests

    void processNextRequest();
};

#endif // MODBUSCONTROLDEVICE_H

#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QSerialPort>

class SerialPortManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    QSerialPort* configureSerialPort(const QString &portName, int baudRate = QSerialPort::Baud9600,
                                     QSerialPort::DataBits dataBits = QSerialPort::Data8,
                                     QSerialPort::Parity parity = QSerialPort::NoParity,
                                     QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                                     QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl);

private:
    QSerialPort* serialPort;
};


#endif // SERIALPORTMANAGER_H

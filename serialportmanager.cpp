#include "serialportmanager.h"
#include "qdebug.h"

SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent), serialPort(nullptr)
{
}

SerialPortManager::~SerialPortManager()
{
    // Check if the serial port is open
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();  // Close the serial port
    }

    // Clean up the serial port object
    delete serialPort;
    serialPort = nullptr;
}

QSerialPort* SerialPortManager::configureSerialPort(const QString &portName, int baudRate,
                                                    QSerialPort::DataBits dataBits,
                                                    QSerialPort::Parity parity,
                                                    QSerialPort::StopBits stopBits,
                                                    QSerialPort::FlowControl flowControl)
{
    serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setParity(parity);
    serialPort->setStopBits(stopBits);
    serialPort->setFlowControl(flowControl);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open port" << portName;
        delete serialPort;
        return nullptr;
    }

    return serialPort;
}


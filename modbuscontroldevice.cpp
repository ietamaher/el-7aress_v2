#include "modbuscontroldevice.h"
#include "qvariant.h"
#include <QDebug>

ModbusControl::ModbusControl(QObject *parent)
    : QObject(parent), modbusDevice(new QModbusRtuSerialMaster(this)), timer(new QTimer(this)), isBusy(false)
{
    connect(modbusDevice, &QModbusClient::errorOccurred, this, [this](QModbusDevice::Error error) {
        emit errorOccurred(modbusDevice->errorString());
    });

    connect(timer, &QTimer::timeout, this, &ModbusControl::onTimeout);
    timer->start(5000); // Adjust the interval (in milliseconds) as needed
}

ModbusControl::~ModbusControl()
{
    if (modbusDevice->state() != QModbusDevice::UnconnectedState) {
        modbusDevice->disconnectDevice();
    }
}

bool ModbusControl::connectToDevice(const QString &portName, int baudRate, int dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, portName);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudRate);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);
    modbusDevice->setTimeout(1000);
    modbusDevice->setNumberOfRetries(3);

    if (!modbusDevice->connectDevice()) {
        emit errorOccurred("Connection failed: " + modbusDevice->errorString());
        return false;
    }

    qDebug() << "Connected to Modbus device.";
    return true;
}

void ModbusControl::disconnectDevice()
{
    if (modbusDevice->state() != QModbusDevice::UnconnectedState) {
        modbusDevice->disconnectDevice();
    }
}

void ModbusControl::readRegister(int startAddress, int numberOfEntries, int serverAddress, bool highPriority)
{
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    if (isBusy) {
        if (highPriority) {
            // Insert the manual request at the front of the queue
            requestQueue.prepend(qMakePair(serverAddress, readUnit));
        } else {
            // Insert the periodic request at the back of the queue
            requestQueue.enqueue(qMakePair(serverAddress, readUnit));
        }
    } else {
        isBusy = true;
        auto *reply = modbusDevice->sendReadRequest(readUnit, serverAddress);
        if (reply) {
            connect(reply, &QModbusReply::finished, this, &ModbusControl::onReplyFinished);
        } else {
            emit errorOccurred("Read error: " + modbusDevice->errorString());
            isBusy = false;
            processNextRequest();
        }
    }
}

void ModbusControl::writeRegister(int address, quint16 value, int serverAddress)
{
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, address, 1);
    writeUnit.setValue(0, value);

    if (isBusy) {
        // Manual write requests should take precedence, so we add them to the front of the queue
        requestQueue.prepend(qMakePair(serverAddress, writeUnit));
    } else {
        // Temporarily stop the timer to prioritize the write operation
        timer->stop();

        isBusy = true;
        auto *reply = modbusDevice->sendWriteRequest(writeUnit, serverAddress);
        if (reply) {
            connect(reply, &QModbusReply::finished, this, &ModbusControl::onReplyFinished);
        } else {
            emit errorOccurred("Write error: " + modbusDevice->errorString());
            isBusy = false;
            processNextRequest();

        }timer->start(500); // Restart the timer after processing
    }
}

void ModbusControl::onReplyFinished()
{
    auto *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) {
        emit errorOccurred("Reply cast failed.");
    } else if (reply->error() == QModbusDevice::NoError) {
        emit dataRead(reply->result());
    } else {
        emit errorOccurred(reply->errorString());
    }

    reply->deleteLater();
    isBusy = false;
    processNextRequest();
}

void ModbusControl::processNextRequest()
{
    if (!requestQueue.isEmpty()) {
        auto nextRequest = requestQueue.dequeue();
        readRegister(nextRequest.first, nextRequest.second.valueCount(), nextRequest.first);
    }
}

void ModbusControl::onTimeout()
{
    // Periodic reads should have low priority, so they go to the back of the queue
    readRegister(1, 5, 1); // Example address and length, adjust as needed
}

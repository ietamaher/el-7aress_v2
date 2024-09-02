#include "servomotorcontrolthread.h"
#include "qvariant.h"
#include <QDebug>
#include <QThread>

ServoMotorControlThread::ServoMotorControlThread(const QString &portName, int baudRate, int dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QObject *parent)
    : QThread(parent), modbusDevice(new QModbusRtuSerialMaster()), portName(portName), baudRate(baudRate), dataBits(dataBits), parity(parity), stopBits(stopBits), running(false)
{
    connect(modbusDevice, &QModbusRtuSerialMaster::errorOccurred, [this](QModbusDevice::Error error) {
        emit errorOccurred(modbusDevice->errorString());
    });
}

ServoMotorControlThread::~ServoMotorControlThread()
{
    stop();
    wait();
    delete modbusDevice;
}

void ServoMotorControlThread::run()
{

    if (!connectToDevice()) {
        emit errorOccurred("Failed to connect to device");
        return;
    }

    running = true;
    while (running) {
        // Perform cyclic tasks like reading specific registers at intervals
        QThread::msleep(10); // Example delay, adjust as needed
    }

    disconnectDevice();
}

void ServoMotorControlThread::stop()
{
    running = false;
}

void ServoMotorControlThread::writeRegister(int address, quint16 value, int serverAddress)
{
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, address, 1);
    writeUnit.setValue(0, value);

    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, serverAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, &ServoMotorControlThread::onReplyFinished);
        } else {
            delete reply;
        }
    } else {
        emit errorOccurred("Write error: " + modbusDevice->errorString());
    }
}

void ServoMotorControlThread::readRegister(int startAddress, int numberOfEntries, int serverAddress)
{
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, startAddress, numberOfEntries);

    auto *reply = modbusDevice->sendReadRequest(readUnit, serverAddress);
    if (!reply) {
        qDebug() << "Failed to send read request:" << modbusDevice->errorString();
        emit errorOccurred("Read error: " + modbusDevice->errorString());
        return;
    }

    bool connectionSuccess = connect(reply, &QModbusReply::finished, this, &ServoMotorControlThread::onReplyFinished);
    qDebug() << "Connection success:" << connectionSuccess;

    if (!connectionSuccess) {
        qDebug() << "Failed to connect reply signal";
        reply->deleteLater();
        return;
    }

    qDebug() << "Sending read request...";
}

void ServoMotorControlThread::onReplyFinished()
{
    auto *reply = qobject_cast<QModbusReply *>(sender());
    if (!reply) {
        qDebug() << "onReplyFinished: Invalid reply object";
        return;
    }

    if (reply->error() == QModbusDevice::NoError) {
        emit dataRead(reply->result());
        qDebug() << "onReplyFinished: Data read successfully";
    } else {
        emit errorOccurred(reply->errorString());
        qDebug() << "onReplyFinished: Error occurred -" << reply->errorString();
    }

    // Check the state of the modbusDevice
    qDebug() << "Modbus device state after reply:" << modbusDevice->state();
    reply->deleteLater();
    qDebug() << "onReplyFinished: Reply object deleted";
}

bool ServoMotorControlThread::connectToDevice()
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

    qDebug() << "Connected to Modbus device";
    return true;
}

void ServoMotorControlThread::disconnectDevice()
{
    modbusDevice->disconnectDevice();
}

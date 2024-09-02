#include "plccontrolthread.h"
#include <QDebug>

PLCControlThread::PLCControlThread(const char *device, int baud, int slave_id, QObject *parent)
    : QThread(parent), ctx(nullptr) {

    ctx = modbus_new_rtu(device, baud, 'N', 8, 1);
    if (ctx == nullptr) {
        qCritical() << "Failed to create the Modbus RTU context.";
        return;
    }

    if (modbus_set_slave(ctx, slave_id) == -1) {
        qCritical() << "Failed to set slave ID:" << modbus_strerror(errno);
        modbus_free(ctx);
        ctx = nullptr;
        return;
    }

    if (modbus_connect(ctx) == -1) {
        qCritical() << "Failed to connect to Modbus device:" << modbus_strerror(errno);
        modbus_free(ctx);
        ctx = nullptr;
        return;
    }
    qDebug() << "PLCControlThread  is initialized.";
}

PLCControlThread::~PLCControlThread() {
    if (ctx) {
        modbus_close(ctx);
        modbus_free(ctx);
    }
}

void PLCControlThread::run() {
    commandTimer = new QTimer();
    commandTimer->moveToThread(this);

    connect(commandTimer, &QTimer::timeout, this, [this]() {
        readData(0, 6);
        //QThread.msleep(5);
        readInputBits(0,13);
    });

    commandTimer->start(50);
    qDebug() << "PLCControlThread  is running.";
    exec();
}

void PLCControlThread::readData(int address, int numRegisters) {

    if (!ctx) {
        qCritical() << "Modbus context is not initialized.";
        return;
    }

    QVector<uint16_t> data(numRegisters);  // Allocate space for the read data

    modbusMutex.lock();  // Lock the mutex before accessing Modbus
    int rc = modbus_read_registers(ctx, address, numRegisters, data.data());
    modbusMutex.unlock();  // Unlock the mutex after accessing Modbus

    if (rc == -1) {
        qCritical() << "Read error:" << modbus_strerror(errno);
    } else {
        emit registerRead(data);  // Emit signal with the read data for GUI update
    }
}

void PLCControlThread::writeData(int address, const QVector<uint16_t> &values) {
    if (!ctx) {
        qCritical() << "Modbus context is not initialized.";
        return;
    }

    modbusMutex.lock();  // Lock the mutex before accessing Modbus
    int rc = modbus_write_registers(ctx, address, values.size(), values.data());
    modbusMutex.unlock();  // Unlock the mutex after accessing Modbus

    if (rc == -1) {
        qCritical() << "Write error:" << modbus_strerror(errno);
    }
}

void PLCControlThread::writeBits(int address, const QVector<uint8_t> &bits) {
    if (!ctx) {
        qCritical() << "Modbus context is not initialized.";
        return;
    }

    modbusMutex.lock();  // Lock the mutex before accessing Modbus
    int rc = modbus_write_bits(ctx, address, bits.size(), bits.data());
    modbusMutex.unlock();  // Unlock the mutex after accessing Modbus

    if (rc == -1) {
        qCritical() << "Write bits error:" << modbus_strerror(errno);
    }
}

void PLCControlThread::readInputBits(int address, int numBits) {
//qDebug() << "PLCControlThread  readInputBits.0";
    if (!ctx) {
        qCritical() << "Modbus context is not initialized.";
        return;
    }
//qDebug() << "PLCControlThread  readInputBits.";
    QVector<uint8_t> bits(numBits);  // Allocate space for the read bits

    modbusMutex.lock();  // Lock the mutex before accessing Modbus
    int rc = modbus_read_input_bits(ctx, address, numBits, bits.data());
    modbusMutex.unlock();  // Unlock the mutex after accessing Modbus

    if (rc == -1) {
        qCritical() << "Read input bits error:" << modbus_strerror(errno);
    } else {
        emit bitsRead(bits);  // Emit signal with the read bits for GUI update
    }
}


#include "servodrivercontrolthread.h"

ServoDriverControlThread::ServoDriverControlThread(const QString &id, const char *device, int baud, int slave_id, QObject *parent)
    : QThread(parent), identifier(id), ctx(nullptr), logctxFlag(true), logFlag(true) {

    ctx = modbus_new_rtu(device, baud, 'N', 8, 1);
    if (ctx == nullptr) {
        QTimer::singleShot(0, this, [this]() {
            emit logMessage(QString("[%1] Failed to create the Modbus RTU context.").arg(identifier));
        });        return;
    }

    if (modbus_set_slave(ctx, slave_id) == -1) {
        QTimer::singleShot(0, this, [this]() {
            emit logMessage(QString("[%1] Failed to set slave ID: %2").arg(identifier, modbus_strerror(errno)));
        });
        modbus_free(ctx);
        ctx = nullptr;
        return;
    }

    if (modbus_connect(ctx) == -1) {
        QTimer::singleShot(0, this, [this]() {
            emit logMessage(QString("[%1] Failed to connect to Modbus device: %2").arg(identifier, modbus_strerror(errno)));
        });
        modbus_free(ctx);
        ctx = nullptr;
        return;
    }
    QTimer::singleShot(0, this, [this]() {
        emit logMessage(QString("[%1] Modbus connection established successfully.").arg(identifier));
    });
}

ServoDriverControlThread::~ServoDriverControlThread() {
    if (ctx) {
        modbus_close(ctx);
        modbus_free(ctx);
        emit logMessage(QString("[%1] Modbus connection closed.").arg(identifier));
    }
}

void ServoDriverControlThread::run() {
    if (!ctx) {
        // Context initialization failed; exit the thread
        return;
    }
    commandTimer = new QTimer();
    commandTimer->moveToThread(this);

    connect(commandTimer, &QTimer::timeout, this, [this]() {
        readData(196, 30);
    });

    commandTimer->start(50);  // Set the interval to send commands (e.g., every 50 ms)

    emit logMessage(QString("[%1] ServoDriverControlThread started.").arg(identifier));
    exec();
}

void ServoDriverControlThread::readData(int address, int numRegisters) {
    if (!ctx) {
        // Context initialization failed; exit the thread
        return;
    }
    QVector<uint16_t> data(numRegisters);

    modbusMutex.lock();
    int rc = modbus_read_registers(ctx, address, numRegisters, data.data());
    modbusMutex.unlock();

    if (rc == -1) {
        logError(QString("Read/Write error: %1").arg(modbus_strerror(errno)));
    } else {
        emit dataRead(data);
        logFlag = true;  // Reset the log flag upon successful data read
    }

}

void ServoDriverControlThread::writeData(int address, const QVector<uint16_t> &values) {
    if (!ctx) {
        // Context initialization failed; exit the thread
        return;
    }
    modbusMutex.lock();
    int rc = modbus_write_registers(ctx, address, values.size(), values.data());
    modbusMutex.unlock();

    if (rc == -1) {
        logError(QString("Read/Write error: %1").arg(modbus_strerror(errno)));
    } else {
        logFlag = true;  // Reset the log flag upon successful data write
    }

}

void ServoDriverControlThread::logError(const QString &message) {
    if (logFlag) {
        emit logMessage(QString("[%1] %2").arg(identifier, message));
        logFlag = false;  // Disable further logs until the condition changes
    }
}

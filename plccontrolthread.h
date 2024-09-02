#ifndef PLCCONTROLTHREAD_H
#define PLCCONTROLTHREAD_H

#include <QThread>
#include <QTimer>
#include <QVector>
#include <modbus/modbus.h>
#include <QMutex>

class PLCControlThread : public QThread {
    Q_OBJECT

public:
    PLCControlThread(const char *device, int baud, int slave_id, QObject *parent = nullptr);
    ~PLCControlThread();

    void readData(int address, int numRegisters);
    void writeData(int address, const QVector<uint16_t> &values);
    void writeBits(int address, const QVector<uint8_t> &bits);
    void readInputBits(int address, int numBits);

protected:
    void run() override;

private:
    modbus_t *ctx;
    QMutex modbusMutex;
    QTimer *commandTimer;

signals:
    void registerRead(const QVector<uint16_t> &data);
    void bitsRead(const QVector<uint8_t> &bits);
};

#endif // PLCCONTROLTHREAD_H


#ifndef SERVODRIVERCONTROLTHREAD_H
#define SERVODRIVERCONTROLTHREAD_H

#include <QThread>
#include <QTimer>
#include <QVector>
#include <QMutex>
#include <modbus/modbus.h>

class ServoDriverControlThread : public QThread {
    Q_OBJECT

public:
    ServoDriverControlThread(const QString &id, const char *device, int baud, int slave_id, QObject *parent = nullptr);
    ~ServoDriverControlThread();

    void readData(int address, int numRegisters);
    void writeData(int address, const QVector<uint16_t> &values);
    void logError(const QString &message);
signals:
    void logMessage(const QString &message);
    void dataRead(const QVector<uint16_t> &data);

protected:
    void run() override;

private:


    QString identifier;  // Instance identifier
    modbus_t *ctx;
    QTimer *commandTimer;
    QMutex modbusMutex;

    bool logctxFlag;
    bool logFlag;
};

#endif // SERVODRIVERCONTROLTHREAD_H



// lrfcontrolthread.h
#ifndef LRFCONTROLTHREAD_H
#define LRFCONTROLTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>

class LRFControlThread : public QThread {
    Q_OBJECT

public:
    explicit LRFControlThread(QObject *parent = nullptr);
    ~LRFControlThread();

    void setSerialPort(QSerialPort *serial);

    // LRF Command Methods
    void sendSelfCheck();
    void sendSingleRanging();
    void sendContinuousRanging();
    void stopRanging();

protected:
    void run() override;

private:
    QSerialPort *lrfSerial;
    QMutex mutex;
    QWaitCondition condition;
    bool abort;

    // Helper methods
    quint8 calculateChecksum(const QByteArray &command);
    QByteArray buildCommand(const QByteArray &commandTemplate);
    QByteArray sendCommand(const QByteArray &command);

signals:
    void commandSent();
    void responseReceived(const QByteArray &response);
    void logError(const QString &message);

};

#endif // LRFCONTROLTHREAD_H



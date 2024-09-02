#ifndef GPSSNIFFTHREAD_H
#define GPSSNIFFTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>

class GPSSniffThread : public QThread {
    Q_OBJECT

public:
    explicit GPSSniffThread(QObject *parent = nullptr);
    ~GPSSniffThread();

    void setSerialPort(QSerialPort *serial);

protected:
    void run() override;

private:
    QSerialPort *serialPort;
    QMutex mutex;
    QWaitCondition condition;
    bool abort;

    void readNMEA();

signals:
    void nmeaDataReceived(const QString &nmeaData);
};

#endif // GPSSNIFFTHREAD_H


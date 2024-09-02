#ifndef RADARSNIFFTHREAD_H
#define RADARSNIFFTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>

class RadarSniffThread : public QThread {
    Q_OBJECT

public:
    explicit RadarSniffThread(QObject *parent = nullptr);
    ~RadarSniffThread();

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

#endif // RADARSNIFFTHREAD_H

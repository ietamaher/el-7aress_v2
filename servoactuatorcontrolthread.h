#ifndef SERVOACTUATORCONTROLTHREAD_H
#define SERVOACTUATORCONTROLTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>

class ServoActuatorControlThread : public QThread {
    Q_OBJECT

public:
    explicit ServoActuatorControlThread(QObject *parent = nullptr);
    ~ServoActuatorControlThread();

    void setSerialPort(QSerialPort *serial);

    // Servo Command Methods
    void moveToPosition(int position);
    void checkStatus();
    void checkAlarms();

protected:
    void run() override;

private:
    QSerialPort *servoSerial;
    QMutex mutex;
    QWaitCondition condition;
    bool abort;

    QString sendCommand(const QString &command);

signals:
    void commandSent();
    void responseReceived(const QString &response);
};

#endif // SERVOACTUATORCONTROLTHREAD_H


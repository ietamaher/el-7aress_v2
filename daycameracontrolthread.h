#ifndef DAYCAMERACONTROLTHREAD_H
#define DAYCAMERACONTROLTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include "viscacontrol.h"

class DayCameraControlThread : public QThread
{
    Q_OBJECT

public:
    explicit DayCameraControlThread(QObject *parent = nullptr);
    ~DayCameraControlThread();

    void setSerialPort(QSerialPort *serial);
    void sendCommand(const QByteArray &command);
    void sendPanLeft();
    void sendPanRight();
    void logError(const QString &message);

public slots:


signals:
    void errorOccurred(const QString &message);
    void commandSent();
    void logMessage(const QString &message);

protected:
    void run() override;


private:
    QString identifier;  // Instance identifier

    QSerialPort *cameraSerial;
    QByteArray currentCommand;
    QMutex mutex;
    QWaitCondition condition;
    bool abort;
    QTimer *commandTimer; // Timer for sending commands
    ViscaControl viscaControl;
    bool logFlag;
};

#endif // DAYCAMERACONTROLTHREAD_H


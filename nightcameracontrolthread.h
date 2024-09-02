#ifndef NIGHTCAMERACONTROLTHREAD_H
#define NIGHTCAMERACONTROLTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>
#include "viscacontrol.h"

// Include Boson SDK headers
extern "C" {
#include "boson/EnumTypes.h"
#include "boson/UART_Connector.h"
#include "boson/Client_API.h"
}

class NightCameraControlThread : public QThread
{
    Q_OBJECT
public:
    explicit NightCameraControlThread(QObject *parent = nullptr);
    ~NightCameraControlThread();

    void setSerialPort(QSerialPort *serial);

    // VISCA Commands
    void sendPanLeft();
    void sendPanRight();

    // Boson SDK Commands
    bool initializeBosonCamera();
    void getCameraInfo();
    void getSoftwareVersion();
    void getCameraSerialNumber();

signals:
    void errorOccurred(const QString &message);
    void commandSent();
    void bosonCameraInfoReceived(const QString &info);
    void logError(const QString &message);

protected:
    void run() override;

private:
    QSerialPort *cameraSerial;
    QByteArray currentCommand;
    QMutex mutex;
    QWaitCondition condition;
    bool abort;
    ViscaControl viscaControl;

    // Boson SDK-specific variables
    int32_t dev; // Device port number
    int32_t baud; // Baud rate
};

#endif // NIGHTCAMERACONTROLTHREAD_H


#ifndef LENSNIGHTCAMERACONTROLTHREAD_H
#define LENSNIGHTCAMERACONTROLTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>

class LensNightCameraControlThread : public QThread {
    Q_OBJECT

public:
    explicit LensNightCameraControlThread(QObject *parent = nullptr);
    ~LensNightCameraControlThread();

    void setSerialPort(QSerialPort *serial);

    // Lens Command Methods
    void moveToWFOV();
    void moveToNFOV();
    void moveToIntermediateFOV(int percentage);
    void moveToFocalLength(int efl);
    void moveToInfinityFocus();
    void moveFocusNear(int amount);
    void moveFocusFar(int amount);
    void getFocusPosition();
    void getLensTemperature();
    void resetController();
    void homeAxis(int axis);
    void turnOnTemperatureCompensation();
    void turnOffTemperatureCompensation();
    void turnOnRangeCompensation();
    void turnOffRangeCompensation();

protected:
    void run() override;

private:
    QSerialPort *lensSerial;
    QMutex mutex;
    QWaitCondition condition;
    bool abort;

    QString sendCommand(const QString &command);

signals:
    void commandSent();
    void responseReceived(const QString &response);
};

#endif // LENSNIGHTCAMERACONTROLTHREAD_H

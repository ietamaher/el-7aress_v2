#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include "dataprocessor.h"
#include "daycameracontrolthread.h"
#include "nightcameracontrolthread.h"
#include "lensnightcameracontrolthread.h"

#include "servodrivercontrolthread.h"
#include "plccontrolthread.h"
#include "camerathread.h"
#include "camerawidget.h"
#include "lrfcontrolthread.h"
#include "servoactuatorcontrolthread.h"
#include "gpssniffthread.h"
#include "radarsniffthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addCameraWidgetToLayout(CameraWidget *cameraWidget);
    void appendLogMessage(const QString &message);
private slots:
    void handleLRFResponse(const QByteArray &response);
    void handleServoActuatorResponse(const QString &response);


    void on_panLeftButton_clicked();
    void on_panRightButton_clicked();
    void handleError(const QString &message);
    void handleCommandSent();
    void onTimeout(); // Slot for periodic reading
    void handleDataRead(const QVector<uint16_t> &data);
    void updateDataLabel(const MonitorDataModel &data);
    //void handleWriteOperation();

    void on_panLeftButton_2_clicked();

private:
    DataProcessor *dataProcessor;

    ServoDriverControlThread *azServoDriverControlThread;
    ServoDriverControlThread *elServoDriverControlThread;
    PLCControlThread *monitorPlcControlThread;
    PLCControlThread *turretPlcControlThread;

    LRFControlThread *lrfControlThread;
    ServoActuatorControlThread *servoActuatorControlThread;

    CameraThread *cameraThread;
    CameraWidget *cameraWidget;

    GPSSniffThread *gpsSniffThread;
    RadarSniffThread *radarSniffThread;

    QSerialPort *gpsSerial;
    QSerialPort *radarSerial;
    QSerialPort *serialPort;
    QSerialPort *dayCameraSerial, *nightCameraSerial, *lensNightCameraSerial, *lrfSerial, *servoActuatorSerial;

    DayCameraControlThread* dayCameraThread;
    NightCameraControlThread* nightCameraThread;
    LensNightCameraControlThread *lensNightCameraThread;

    Ui::MainWindow *ui;

    QTimer* readTimer;

    void setupSerialPort();
};
#endif // MAINWINDOW_H


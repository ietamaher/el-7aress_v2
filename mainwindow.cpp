#include "mainwindow.h"
#include "serialportmanager.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,  lrfControlThread(new LRFControlThread(this))
    , servoActuatorControlThread(new ServoActuatorControlThread(this))
    , gpsSniffThread(new GPSSniffThread(this))
    , radarSniffThread(new RadarSniffThread(this))

    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dataProcessor = new DataProcessor(monitorPlcControlThread, turretPlcControlThread, this);

    SerialPortManager *portManager = new SerialPortManager(this);

    // Configure serial ports for different devices
    dayCameraSerial = portManager->configureSerialPort("/dev/pts/82",921600,QSerialPort::Data8);
    nightCameraSerial = portManager->configureSerialPort("/dev/pts/10",QSerialPort::Baud9600,QSerialPort::Data8);
    lensNightCameraSerial = portManager->configureSerialPort("/dev/pts/10",QSerialPort::Baud9600,QSerialPort::Data8);

    lrfSerial = portManager->configureSerialPort("/dev/pts/12",QSerialPort::Baud9600,QSerialPort::Data8);
    servoActuatorSerial = portManager->configureSerialPort("/dev/pts/14",QSerialPort::Baud9600,QSerialPort::Data8);
    gpsSerial = portManager->configureSerialPort("/dev/pts/14",QSerialPort::Baud9600,QSerialPort::Data8);
    radarSerial = portManager->configureSerialPort("/dev/pts/14",QSerialPort::Baud9600,QSerialPort::Data8);

    dayCameraThread = new DayCameraControlThread(this);
    dayCameraThread->setSerialPort(dayCameraSerial);

    nightCameraThread = new NightCameraControlThread(this);
    nightCameraThread->setSerialPort(nightCameraSerial);

    lensNightCameraThread = new LensNightCameraControlThread(this);
    lensNightCameraThread->setSerialPort(lensNightCameraSerial);

    azServoDriverControlThread = new ServoDriverControlThread("azServo", "/dev/pts/2", 115200, 1, this);
    elServoDriverControlThread = new ServoDriverControlThread("elServo", "/dev/pts/7", 115200, 1, this);

    monitorPlcControlThread = new PLCControlThread("/dev/pts/25", 115200, 1, this);
    turretPlcControlThread = new PLCControlThread("/dev/pts/75", 115200, 1, this);

    // Connect signals from PLC threads to data processor methods
    connect(monitorPlcControlThread, &PLCControlThread::bitsRead, dataProcessor, &DataProcessor::processBitsMonitorPlcData);
    connect(monitorPlcControlThread, &PLCControlThread::registerRead, dataProcessor, &DataProcessor::processRegistersMonitorPlcData);
    connect(turretPlcControlThread, &PLCControlThread::bitsRead, dataProcessor, &DataProcessor::processBitsTurretPlcData);
    connect(turretPlcControlThread, &PLCControlThread::registerRead, dataProcessor, &DataProcessor::processRegistersTurretPlcData);

    connect(azServoDriverControlThread, &ServoDriverControlThread::dataRead, dataProcessor, &DataProcessor::processAzimuthServoData);
    connect(elServoDriverControlThread, &ServoDriverControlThread::dataRead, dataProcessor, &DataProcessor::processElevationServoData);

    // Connect the logMessage signal to update a QTextEdit or QLabel in the UI
    connect(azServoDriverControlThread, &ServoDriverControlThread::logMessage, this, &MainWindow::appendLogMessage);
    // Connect the logMessage signal to update a QTextEdit or QLabel in the UI
    connect(elServoDriverControlThread, &ServoDriverControlThread::logMessage, this, &MainWindow::appendLogMessage);
    connect(dayCameraThread, &DayCameraControlThread::logMessage, this, &MainWindow::appendLogMessage);

    // Connect processed data signals to update the GUI
   // connect(dataProcessor, &DataProcessor::dataProcessed, this, &MainWindow::updateDataLabel);
    connect(dataProcessor, &DataProcessor::dataUpdated, this, &MainWindow::updateDataLabel);

    //monitorPlcControlThread->start();
    //turretPlcControlThread->start();

    // Connect signals to slots

    connect(dayCameraThread, &DayCameraControlThread::errorOccurred, this, &MainWindow::handleError);
    connect(dayCameraThread, &DayCameraControlThread::commandSent, this, &MainWindow::handleCommandSent);

    connect(nightCameraThread, &NightCameraControlThread::errorOccurred, this, &MainWindow::handleError);
    connect(nightCameraThread, &NightCameraControlThread::commandSent, this, &MainWindow::handleCommandSent);

    //connect(azServoDriverControlThread, &ServoDriverControlThread::errorOccurred, this, &MainWindow::handleError);
    //connect(azServoDriverControlThread, &ServoDriverControlThread::commandSent, this, &MainWindow::handleCommandSent);

    connect(lensNightCameraThread, &LensNightCameraControlThread::responseReceived, dataProcessor, &DataProcessor::handleLensResponse);

    azServoDriverControlThread->start();
    elServoDriverControlThread->start();
    dayCameraThread->start();
    nightCameraThread->start();
    lensNightCameraThread->start();

    // Example: Move to Wide FOV at startup
    lensNightCameraThread->moveToWFOV();
    // Set the serial port in the LRF control thread
    lrfControlThread->setSerialPort(lrfSerial);

    // Connect signals
    connect(lrfControlThread, &LRFControlThread::responseReceived, this, &MainWindow::handleLRFResponse);

    // Start the LRF control thread
    lrfControlThread->start();

    // Example: Send a self-check command at startup
    lrfControlThread->sendSelfCheck();

    servoActuatorControlThread->setSerialPort(servoActuatorSerial);

    connect(servoActuatorControlThread, &ServoActuatorControlThread::responseReceived, this, &MainWindow::handleServoActuatorResponse);

    servoActuatorControlThread->start();
    // Example: Move to a specific position at startup
    servoActuatorControlThread->moveToPosition(5000);

    gpsSniffThread->setSerialPort(gpsSerial);

    // Set the Radar serial port in the Radar sniff thread
    radarSniffThread->setSerialPort(radarSerial);
    //dayCameraThread->sendPanLeft();
    // Connect signals
    connect(gpsSniffThread, &GPSSniffThread::nmeaDataReceived, dataProcessor, &DataProcessor::handleGPSNMEAData);
    connect(radarSniffThread, &RadarSniffThread::nmeaDataReceived, dataProcessor, &DataProcessor::handleRadarNMEAData);

    // Start the sniff threads
    gpsSniffThread->start();
    radarSniffThread->start();


    cameraThread = new CameraThread(this);
    connect(cameraThread, &CameraThread::cameraWidgetReady, this, &MainWindow::addCameraWidgetToLayout);
    //connect(this, &MainWindow::primaryObjectPositionUpdated, cameraThread, &CameraThread::handlePrimaryObjectPositionUpdate);
    //connect(this, SIGNAL(primaryObjectPositionUpdated(QPoint)), cameraThread, SLOT(handlePrimaryObjectPositionUpdate(QPoint)));
    //connect(this, &MainWindow::primaryObjectPositionUpdated, cameraThread, &CameraThread::handlePrimaryObjectPositionUpdate);
    //connect(this, &MainWindow::settingParameters, cameraThread, &CameraThread::handleSettingParameters);
    //connect(this, &MainWindow::displayingMotorParameters, cameraThread, &CameraThread::handleMotorParameters);
    //connect(this, &MainWindow::displayingPlcParameters, cameraThread, &CameraThread::handlePlcParameters);

    // Inside MainWindow constructor or initialization function
    //connect(this, &MainWindow::detectionToggled, cameraWidget, &CameraWidget::toggleDetection);
    //connect(this, &MainWindow::trackingToggled, cameraWidget, &CameraWidget::toggleTracking);
    cameraThread->start(); // Start the thread
}

MainWindow::~MainWindow()
{
    if (dayCameraThread) {
        dayCameraThread->quit();
        dayCameraThread->wait();
        dayCameraSerial->close();
        delete dayCameraSerial;
    }

    if (nightCameraThread) {
        nightCameraThread->quit();
        nightCameraThread->wait();
        nightCameraSerial->close();
        delete nightCameraThread;
    }
    if (azServoDriverControlThread) {
        azServoDriverControlThread->quit();
        azServoDriverControlThread->wait();
        delete azServoDriverControlThread;
    }
    if (elServoDriverControlThread) {
        elServoDriverControlThread->quit();
        elServoDriverControlThread->wait();
        delete elServoDriverControlThread;
    }
    if (monitorPlcControlThread) {
        monitorPlcControlThread->quit();
        monitorPlcControlThread->wait();
        delete monitorPlcControlThread;
    }

    if (lrfControlThread) {
        lrfControlThread->quit();
        lrfControlThread->wait();
    }
    if (servoActuatorControlThread) {
        servoActuatorControlThread->quit();
        servoActuatorControlThread->wait();
    }
    if (gpsSniffThread) {
        gpsSniffThread->quit();
        gpsSniffThread->wait();
    }
    if (radarSniffThread) {
        radarSniffThread->quit();
        radarSniffThread->wait();
    }
    delete ui;
}
void MainWindow::addCameraWidgetToLayout(CameraWidget *cameraWidget) {

    QVBoxLayout *layout = new QVBoxLayout(ui->widget); // Use the central widget's layout
    layout->addWidget(cameraWidget);
}

void MainWindow::onTimeout()
{
        qDebug() << "ModbusWorker is not initialized.";
}

void MainWindow::handleError(const QString &message)
{
    qDebug() << "Error: " << message;
    // Handle error (e.g., display message to user)
}

void MainWindow::handleCommandSent()
{
    qDebug() << "Command sent successfully";
    // Handle successful command (e.g., update UI)
}

void MainWindow::handleLRFResponse(const QByteArray &response) {
    // Handle the response from the LRF
    qDebug() << "LRF Response:" << response.toHex();
}
void MainWindow::handleServoActuatorResponse(const QString &response) {
    // Handle the response from the Servo Actuator
    qDebug() << "Servo Response:" << response;
}

void MainWindow::appendLogMessage(const QString &message) {
    ui->textEdit->append(message);  // Assume you have a QTextEdit named textEdit in your UI
}



void MainWindow::on_panLeftButton_clicked()
{
    // Writing to 3 registers starting from address 10
    //QVector<uint16_t> valuesToWrite = {100, 200, 300};
    //elServoDriverControlThread->writeData(10, valuesToWrite);

    QVector<uint8_t> valuesToWrite = {255,255};
    monitorPlcControlThread->writeBits(0, valuesToWrite);
}

void MainWindow::on_panRightButton_clicked()
{
    // set SPEED
    // Starting address (Dec): 1152
    // Speed: 10000 [Hz]
    QVector<uint16_t> valuesToWrite = {10000};
    azServoDriverControlThread->writeData(1152, valuesToWrite);

    //this->msleep(30);

    // Set Direction
    // Starting address (Dec): 124
    // Direction: 10000 [Hz]
    // # Write data: (0100 0000 0000 0000) = 16384 (START signal ON) FWD
    // # Write data: (1000 0000 0000 0000) = 16384 (START signal ON) REV

    valuesToWrite = {16384};
    azServoDriverControlThread->writeData(124, valuesToWrite);

    //START signal ON
    // Starting address (Dec): 124
    // # Write data: (0000 0000 0000 1000) = 8 (STOP signal ON)
    valuesToWrite = {8};
    azServoDriverControlThread->writeData(124, valuesToWrite);

    //STOP
    // Starting address (Dec): 124
    // # Write data: (0000 0000 0000 0000) = 32 (STOP signal ON)
    valuesToWrite = {32};
    azServoDriverControlThread->writeData(124, valuesToWrite);

    //All bits OFF
    // Starting address (Dec): 124
    // # Write data: (0000 0000 0000 0000) = 0 (All bits OFF)
    valuesToWrite = {0};
    azServoDriverControlThread->writeData(124, valuesToWrite);
}

void MainWindow::handleDataRead(const QVector<uint16_t> &data) {
    // Convert the data to a string and display it in the label
    QStringList dataList;
    for (uint16_t value : data) {
        dataList << QString::number(value);
    }

    ui->dataLabel->setText("Data: " + dataList.join(", "));

    // Optionally, log the data to the console for debugging
    qDebug() << "Data received:" << dataList.join(", ");
}
void MainWindow::updateDataLabel(const MonitorDataModel &data) {
    qDebug() << "GUI Data received:" << data.gun_state;

    // Update the GUI based on the DataModel data
    QString displayText;
    //displayText += QString::number(data.authorize_sw, 'f', 2) + " ";
    //displayText += QString::number(data.ammunition_ready, 'f', 2) + " ";
    displayText += QString(data.gun_state ? "Low" : "OK") + " ";


    ui->dataLabel->setText(displayText);
}

void MainWindow::on_panLeftButton_2_clicked()
{
    QVector<uint8_t> valuesToWrite = {0,0};
    monitorPlcControlThread->writeBits(0, valuesToWrite);
}


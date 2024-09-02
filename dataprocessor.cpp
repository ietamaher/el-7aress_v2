#include "dataprocessor.h"
#include <QDebug>
#include <QElapsedTimer>

//DataProcessor::DataProcessor(QObject *parent) : QObject(parent) {}
DataProcessor::DataProcessor(PLCControlThread *monitorPlc, PLCControlThread *turretPlc, QObject *parent)
    : QObject(parent), monitorPlcThread(monitorPlc), turretPlcThread(turretPlc) {}


void DataProcessor::processBitsMonitorPlcData(const QVector<uint8_t> &bits) {
    QElapsedTimer timer;
    timer.start();

    QMutexLocker locker(&mutex);

    if (bits.size() >= 9) {
        monitorDataModel.gun_state = bits[0];
        monitorDataModel.fire_mode_state_input = bits[1];
        monitorDataModel.load_ammunition_state = bits[2];
        monitorDataModel.station_state = bits[3];
        monitorDataModel.speed_plus_sw = bits[4];
        monitorDataModel.speed_minus_sw = bits[5];
        monitorDataModel.home_sw = bits[6];
        monitorDataModel.stabilization_sw = bits[7];
        monitorDataModel.authorize_sw = bits[8];

        emit dataUpdated(monitorDataModel);
        qint64 elapsedTime = timer.nsecsElapsed();  // Time in nanoseconds
        qDebug() << "ProcessBits took" << elapsedTime << "ns";
    }
}

void DataProcessor::processRegistersMonitorPlcData(const QVector<uint16_t> &data) {
    //qDebug() << "processRegisters took" <<  data ;
    QElapsedTimer timer;
    timer.start();

    QMutexLocker locker(&mutex);
    //qDebug() << "processRegistersMonitorPlcData 1"  ;

    if (data.size() >= 1) {
        monitorDataModel.panel_temperature = data[0];  // updating panel_temperature
    }
    //qDebug() << "processRegistersMonitorPlcData 2"  ;
    emit dataUpdated(monitorDataModel);

    qint64 elapsedTime = timer.nsecsElapsed();  // Time in nanoseconds
    qDebug() << "processRegisters took" << elapsedTime << "ns";
}

// Method to write outputs to the Monitor PLC
void  DataProcessor::updateSMonitorOutputs() {
    QVector<uint8_t> outputBits(2);
    outputBits[0] = monitorDataModel.gun_led_state;
    outputBits[1] = monitorDataModel.safety_station_led;

    if (monitorPlcThread) {
        monitorPlcThread->writeBits(0, outputBits);
    } else {
        qCritical() << "Monitor PLC thread is not initialized!";
    }
}

void DataProcessor::processBitsTurretPlcData(const QVector<uint8_t> &bits) {
    QMutexLocker locker(&mutex);

    if (bits.size() >= 3) {
        turretDataModel.max_tilt_sensor = bits[0];
        turretDataModel.min_tilt_sensor = bits[1];
        turretDataModel.low_ammunition_sensor = bits[2];
    }
    //emit dataUpdated(*turretDataModel);

}

void DataProcessor::processRegistersTurretPlcData(const QVector<uint16_t> &data) {
    QMutexLocker locker(&mutex);

    if (data.size() >= 2) {
        turretDataModel.eo_temperature = data[0];
        turretDataModel.eo_pressure = data[1];
    }
    //emit dataUpdated(*turretDataModel);

}

// Method to write outputs to the station PLC
void DataProcessor::updateTurretOutputs() {
    QVector<uint16_t> outputRegisters(6);
    outputRegisters[0] = turretDataModel.azimuth_pulse;
    outputRegisters[1] = turretDataModel.azimuth_direction;
    outputRegisters[2] = turretDataModel.elevation_pulse;
    outputRegisters[3] = turretDataModel.elevation_direction;
    outputRegisters[4] = turretDataModel.solenoid;
    outputRegisters[5] = monitorDataModel.fire_mode_state_input;

    if (turretPlcThread) {
        turretPlcThread->writeData(0, outputRegisters);
    } else {
        qCritical() << "Turret PLC thread is not initialized!";
    }
}

void DataProcessor::processAzimuthServoData(const QVector<uint16_t> &data) {
    QMutexLocker locker(&mutex);

    if (data.size() >= 50) {
        dataModel.azimuth_servo_angle = (data[6] << 16) | data[7];
        dataModel.azimuth_servo_speed = (data[8] << 16) | data[9];
        dataModel.azimuth_servo_torque = (data[16] << 16) | data[17];
        dataModel.azimuth_servo_motor_temp = (data[46] << 16) | data[47];
        dataModel.azimuth_servo_driver_temp = (data[48] << 16) | data[49];

    }
}

void DataProcessor::processElevationServoData(const QVector<uint16_t> &data) {
    QMutexLocker locker(&mutex);

    if (data.size() >= 50) {
        dataModel.elevation_servo_angle = (data[6] << 16) | data[7];
        dataModel.elevation_servo_speed = (data[8] << 16) | data[9];
        dataModel.elevation_servo_torque = (data[016] << 16) | data[17];
        dataModel.elevation_servo_motor_temp = (data[46] << 16) | data[47];
        dataModel.elevation_servo_driver_temp = (data[48] << 16) | data[49];

    }
}

void DataProcessor::processActuatorData() {
    QMutexLocker locker(&mutex);

    // Example: Process actuator data
    //dataModel.actuatorPosition = dataModel.plcRegisters[2] / 100.0;

    //emit dataProcessed(dataModel);
}

void DataProcessor::processJoystickData() {
    QMutexLocker locker(&mutex);

    // Example: Update joystick data and its effect on servo positions
    //dataModel.joystickX = dataModel.guiCommand * 0.1;  // Example calculation
    //dataModel.joystickY = dataModel.guiCommand * 0.2;  // Example calculation

    //emit dataProcessed(dataModel);
}

void DataProcessor::processGuiData() {
    QMutexLocker locker(&mutex);

    // Example: Process GUI data and update actuators
    //if (dataModel.guiCommand == 1) {
    //    dataModel.actuatorPosition += 1.0;
    //} else if (dataModel.guiCommand == 2) {
    //    dataModel.actuatorPosition -= 1.0;
    //}

    //emit dataProcessed(dataModel);
}

void DataProcessor::handleGPSNMEAData(const QString &nmeaData)
{
    // Handle the response from the GPS
    qDebug() << "GPS Frame:" << nmeaData;
}

void DataProcessor::handleRadarNMEAData(const QString &nmeaData)
{
    // Handle the response from the Radar
    qDebug() << "Radar Frame:" << nmeaData;
}

void DataProcessor::handleLensResponse(const QString &response)
{
    // Handle the response from the Lens Controller
    qDebug() << "Lens Response:" << response;
}
/*
 *
Given your application architecture, the additional methods should be placed in a way that respects the responsibilities of each class. Here’s how you can organize the methods:

1. Data Validation and Filtering
Location: ProcessData Class
Reason: The ProcessData class is responsible for updating and processing the DataModel values. Data validation and filtering should occur immediately after data is received from the threads and before any further processing or decision-making occurs.
2. State Management
Location: ProcessData Class
Reason: State management involves checking the current conditions and updating the system's state. Since the ProcessData class already handles updates to DataModel, it is well-positioned to manage the system's state based on the processed data.
3. Logging and Event Tracking
Location: Could be in either ProcessData or a dedicated Logger class.
Reason: If logging is directly related to data changes (e.g., logging alarms or state transitions), it can be in ProcessData. However, for a more modular design, you could create a dedicated Logger class that ProcessData calls when significant events occur.
4. Error Handling and Recovery
Location: Thread Classes (plc1modbusthread, stationmodbusthread, joystickthread)
Reason: Error handling and recovery are specific to the communication and operations performed by each thread. Each thread should monitor its operations for errors (e.g., communication failures) and attempt to recover or report the error to the ProcessData class.
5. User Interface Updates
Location: MainWindow Class
Reason: The MainWindow class should handle updating the UI in response to changes in DataModel. It can connect to signals from ProcessData to trigger UI updates.
6. Periodic Maintenance Checks
Location: ProcessData Class or MainWindow Class
Reason: If maintenance checks are tied to data processing or state management, they belong in ProcessData. If they are more about user interaction or system-wide checks, they could be in MainWindow.
7. Communication Watchdog
Location: Thread Classes (plc1modbusthread, stationmodbusthread, joystickthread)
Reason: The watchdog should monitor the communication health within each thread. Each thread can have its own watchdog mechanism to ensure it maintains a healthy connection with its respective device.
8. Security Checks
Location: ProcessData Class or MainWindow Class
Reason: If security checks are related to authorizing data changes, they belong in ProcessData. If they are related to UI actions (e.g., preventing unauthorized access to certain screens or functions), they belong in MainWindow.
Implementation Outline
ProcessData Class
cpp
Copier le code
class ProcessData : public QObject {
    Q_OBJECT

public:
    ProcessData(QObject *parent = nullptr);

    void updateDataModel();  // Method to update DataModel values
    void validateAndFilterData();  // Validate and filter incoming data
    void manageSystemState();  // Manage the system's state based on DataModel
    void processAlarms();  // Check and handle alarm conditions
    void performMaintenanceChecks();  // Schedule and perform periodic checks

private:
    DataModel *dataModel;
};
MainWindow Class
cpp
Copier le code
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void updateUI();  // Update the UI based on DataModel changes

private:
    ProcessData *processData;
};
Thread Classes (plc1modbusthread, stationmodbusthread, joystickthread)
cpp
Copier le code
class PLC1ModbusThread : public QThread {
    Q_OBJECT

public:
    PLC1ModbusThread(QObject *parent = nullptr);

    void run() override;
    void handleCommunicationErrors();  // Handle communication errors
    void startCommunicationWatchdog();  // Start a communication watchdog

private:
    modbus_t *ctx;
};
Workflow
Threads (plc1modbusthread, stationmodbusthread, joystickthread) read data from their respective devices, handle any communication errors, and emit signals with the data.

ProcessData receives the data from the threads, validates and filters it, updates the DataModel, checks for alarms, manages the system state, and performs periodic maintenance checks.

MainWindow listens to changes in ProcessData (or directly to DataModel updates) and updates the UI accordingly. It may also handle user interactions that trigger security checks or other UI-specific actions.

Logger (if implemented as a separate class) can be called from ProcessData to log events, state changes, and alarms.
*/
void DataProcessor::updateDataModel()
{

}

void DataProcessor::validateAndFilterData()
{

}

void DataProcessor::manageSystemState()
{

}

void DataProcessor::processAlarms()
{

}

void DataProcessor::performMaintenanceChecks()
{

}

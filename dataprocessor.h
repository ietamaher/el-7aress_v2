// dataprocessor.h
#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QMutex>
#include "datamodel.h"
#include "plccontrolthread.h"

class DataProcessor : public QObject {
    Q_OBJECT

public:
    explicit DataProcessor(PLCControlThread *monitorPlc, PLCControlThread *turretPlc, QObject *parent = nullptr);

public slots:
    void handleGPSNMEAData(const QString &nmeaData);
    void handleRadarNMEAData(const QString &nmeaData);
    void handleLensResponse(const QString &response);


    void processBitsMonitorPlcData(const QVector<uint8_t> &bits);
    void processRegistersMonitorPlcData(const QVector<uint16_t> &data);
    void processBitsTurretPlcData(const QVector<uint8_t> &bits);
    void processRegistersTurretPlcData(const QVector<uint16_t> &data);
    void updateTurretOutputs();
    void updateSMonitorOutputs();
    void processAzimuthServoData(const QVector<uint16_t> &data);
    void processElevationServoData(const QVector<uint16_t> &data);

    void processActuatorData();
    void processJoystickData();
    void processGuiData();

    void updateDataModel();  // Method to update DataModel values
    void validateAndFilterData();  // Validate and filter incoming data
    void manageSystemState();  // Manage the system's state based on DataModel
    void processAlarms();  // Check and handle alarm conditions
    void performMaintenanceChecks();  // Schedule and perform periodic checks

signals:
    void processedDataReady(const DataModel &data);
    void dataProcessed(const DataModel &data);
    void dataUpdated(const MonitorDataModel &data);

private:
    PLCControlThread *monitorPlcThread;
    PLCControlThread *turretPlcThread;
    MonitorDataModel monitorDataModel;
    TurretDataModel turretDataModel;
    DataModel dataModel;

    QMutex mutex;  // Protects access to dataModel
};

#endif // DATAPROCESSOR_H


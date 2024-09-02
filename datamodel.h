// datamodel.h
#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QObject>
#include <QVector>

struct MonitorDataModel {
    // PLC Data
    //QVector<uint16_t> plcRegisters;
    //QVector<uint8_t> plcInputs;
    //QVector<uint8_t> plcOutputs;
    // PLC Data - Inputs
    bool gun_state;
    bool fire_mode_state_input;  // Input from the first PLC
    bool load_ammunition_state;
    bool station_state;
    bool speed_plus_sw;
    bool speed_minus_sw;
    bool home_sw;
    bool stabilization_sw;
    bool authorize_sw;

    // PLC Data - Outputs
    bool gun_led_state;
    bool safety_station_led;

    // Analog Input
    int panel_temperature;
};
struct TurretDataModel {
    // Station PLC - Inputs
    bool max_tilt_sensor;
    bool min_tilt_sensor;
    bool low_ammunition_sensor;
    int eo_temperature;
    int eo_pressure;

    // Station PLC - Outputs
    int azimuth_pulse;
    int azimuth_direction;
    int elevation_pulse;
    int elevation_direction;
    int solenoid;
    int fire_mode_state_output;  // Output for the station PLC
};
struct DataModel {
    // Azimuth Servo Motor Data
    uint32_t azimuth_servo_angle;
    uint32_t azimuth_servo_speed;
    uint32_t azimuth_servo_torque;
    uint32_t azimuth_servo_motor_temp;
    uint32_t azimuth_servo_driver_temp;

    // Elevation Servo Motor Data
    uint32_t elevation_servo_angle;
    uint32_t elevation_servo_speed;
    uint32_t elevation_servo_torque;
    uint32_t elevation_servo_motor_temp;
    uint32_t elevation_servo_driver_temp;

    // Servo Data
    double azimutServoPosition;
    double elevationServoPosition;

    // Actuator Data
    double actuatorPosition;

    // Joystick Data
    double joystickX;
    double joystickY;

    // GUI Data
    int guiCommand;


    int incr_;
    int incr_1;
    bool in1;

    int steps;                  // Total steps taken by the motor
    double azimuth;             // Initial azimuth value
    double elevation;           // Elevation value
    double lrf;                 // Laser Range Finder distance
    QString lrf_rdy;            // LRF ready state

    double fov;                 // Field of View
    int speed_;                 // Speed
    int latestSpeed;
    int lastButtonSpdPlusValue;
    int lastButtonSpdMinusValue;
    int mot1Speed;
    int direction;

    QString gun_ready;          // Gun ready state
    QString gun_charged;        // Gun charged state
    QString gun_armed;          // Gun armed state
    QString ammunition_low;     // Ammunition low state
    QString ammunition_ready;   // Ammunition ready state

    // Initial states and modes
    int last_button_burst_mode_value;
    int last_button_track_value;
    int last_button_detect_value;
    int last_button_stab_value;
    int lastButtonBurstModeValue;
    int last_buttonFireValue;


    QStringList burstModes;    // Burst modes
    int currentBurstModeIndex;  // Current burst mode index
    QString track_state;        // Tracking state
    QString detect_state;       // Detection state
    QString stab_state;         // Stabilization state

};

#endif // DATAMODEL_H


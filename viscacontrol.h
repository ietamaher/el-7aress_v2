#ifndef VISCACONTROL_H
#define VISCACONTROL_H

#include <QSerialPort>
#include <QByteArray>

class ViscaControl {
public:
    ViscaControl();

    QByteArray createPanLeftCommand();
    QByteArray createPanRightCommand();
    QByteArray createTiltUpCommand();
    QByteArray createTiltDownCommand();
    QByteArray createZoomInCommand();
    QByteArray createZoomOutCommand();
    // Other VISCA commands can be added here

//private:


    // Helper functions for command creation can be added here
};

#endif // VISCACONTROL_H


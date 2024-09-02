#include "viscacontrol.h"

ViscaControl::ViscaControl() {
    // Constructor implementation (if needed)
}

/*void ViscaControl::panLeft(QSerialPort *serialPort) {
    QByteArray command = createPanLeftCommand();
    serialPort->write(command);
}

void ViscaControl::panRight(QSerialPort *serialPort) {
    QByteArray command = createPanRightCommand();
    serialPort->write(command);
}

void ViscaControl::tiltUp(QSerialPort *serialPort) {
    QByteArray command = createTiltUpCommand();
    serialPort->write(command);
}

void ViscaControl::tiltDown(QSerialPort *serialPort) {
    QByteArray command = createTiltDownCommand();
    serialPort->write(command);
}

void ViscaControl::zoomIn(QSerialPort *serialPort) {
    QByteArray command = createZoomInCommand();
    serialPort->write(command);
}

void ViscaControl::zoomOut(QSerialPort *serialPort) {
    QByteArray command = createZoomOutCommand();
    serialPort->write(command);
}*/

QByteArray ViscaControl::createPanLeftCommand() {
    QByteArray command;
    command.append(0x80);
    command.append(0x01);
    command.append(0x06);
    command.append(0x01);
    command.append(0x03);
    command.append(0x01);
    command.append(0x01);
    command.append(0x03);
    return command;
}

QByteArray ViscaControl::createPanRightCommand() {
    QByteArray command;
    command.append(0x80);
    command.append(0x01);
    command.append(0x06);
    command.append(0x01);
    command.append(0x03);
    command.append(0x01);
    command.append(0x02);
    command.append(0x03);
    return command;
}

QByteArray ViscaControl::createTiltUpCommand() {
    QByteArray command;
    command.append(0x80);
    command.append(0x01);
    command.append(0x06);
    command.append(0x01);
    command.append(0x03);
    command.append(0x02);
    command.append(0x01);
    command.append(0x03);
    return command;
}

QByteArray ViscaControl::createTiltDownCommand() {
    QByteArray command;
    command.append(0x80);
    command.append(0x01);
    command.append(0x06);
    command.append(0x01);
    command.append(0x03);
    command.append(0x02);
    command.append(0x02);
    command.append(0x03);
    return command;
}

QByteArray ViscaControl::createZoomInCommand() {
    QByteArray command;
    command.append(0x80);
    command.append(0x01);
    command.append(0x04);
    command.append(0x07);
    command.append(0x02);
    command.append(0xFF);
    return command;
}

QByteArray ViscaControl::createZoomOutCommand() {
    QByteArray command;
    command.append(0x80);
    command.append(0x01);
    command.append(0x04);
    command.append(0x07);
    command.append(0x03);
    command.append(0xFF);
    return command;
}

#include "serialcommandqueue.h"

#define COMMAND_TIMEOUT_TIME 1000

SerialCommandQueue::SerialCommandQueue(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
    serial->setSettingsRestoredOnClose(false);

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(handleSerialError(QSerialPort::SerialPortError)));
    connect(serial,SIGNAL(readyRead()),this,SLOT(handleReadData()));

    commandTimeoutTimer = new QTimer(this);
    commandTimeoutTimer->setSingleShot(true);

    connect(commandTimeoutTimer, SIGNAL(timeout()),
            this, SLOT(handleCommandTimeout()));
}


bool SerialCommandQueue::open(QSerialPortInfo info) {
    if(isConnected()) {
        qCritical("Already connected to serial device");
        return false;
    }

    qDebug() << "connecting to " << info.portName();

    serial->setPortName(info.portName());
    serial->setBaudRate(QSerialPort::Baud115200);

    if( !serial->open(QIODevice::ReadWrite) ) {
        qDebug() << "Could not connect to serial device. Error: " << serial->error() << serial->errorString();
        return false;
    }

    serial->clear(QSerialPort::AllDirections);
    serial->clearError();

    return true;
}

void SerialCommandQueue::close() {
    if(!(serial->isOpen())) {
        return;
    }

    serial->close();
}

bool SerialCommandQueue::isConnected() {
    return serial->isOpen();
}

void SerialCommandQueue::queueCommand(QString name,
                                 QByteArray data,
                                 QByteArray expectedRespone) {

    commandQueue.push_back(Command(name, data, expectedRespone));

    // Try to start processing commands.
    processCommandQueue();
}

void SerialCommandQueue::processCommandQueue() {
    // Nothing to do if we don't have any commands...
    if(commandQueue.length() == 0) {
        return;
    }

    // Don't start a new command if one is already in progress
    if(commandTimeoutTimer->isActive()) {
        return;
    }

    if(!isConnected()) {
        qCritical() << "Device disappeared, cannot run command";
        return;
    }

//    qDebug() << "Starting Command:" << commandQueue.front().name;
    responseData.clear();

    if(serial->write(commandQueue.front().commandData) != commandQueue.front().commandData.length()) {
        qCritical() << "Error writing to device";
        return;
    }

    // Start the timer; the command must complete before it fires, or it
    // is considered an error. This is to prevent a misbehaving device from hanging
    // the programmer code.
    commandTimeoutTimer->start(COMMAND_TIMEOUT_TIME);
}

void SerialCommandQueue::handleReadData() {
    if(commandQueue.length() == 0) {
        // TODO: error, we got unexpected data.
        qCritical() << "Got data when we didn't expect it!";
        return;
    }

    if(isConnected()) {
        responseData.append(serial->readAll());
    }

    // TODO: Handle disconnect here?

    if(responseData.length() > commandQueue.front().expectedResponse.length()) {
        // TODO: error, we got unexpected data.
        qCritical() << "Got more data than we expected";
        return;
    }

    if(responseData.length() < commandQueue.front().expectedResponse.length()) {
        qDebug() << "Didn't get enough data yet, so just waiting";
        return;
    }

    // If the command was to read from flash, short-circuit the response data check.
    if(commandQueue.front().name == "readFlash") {
        if(responseData.at(responseData.length()-1) != '\r') {
            qCritical() << "readFlash response didn't end with a \\r";
            return;
        }
    }
    else if(responseData != commandQueue.front().expectedResponse) {
        qCritical() << "Got unexpected data back";
        return;
    }

    // At this point, we've gotten all of the data that we expected.
    commandTimeoutTimer->stop();

//    qDebug() << "Command completed successfully: " << commandQueue.front().name;
    emit(commandFinished(commandQueue.front().name,responseData));


    // If the command was reset, disconnect from the programmer and cancel
    // any further commands
    if(commandQueue.front().name == "reset") {
        qDebug() << "Disconnecting from programmer";

        // TODO: Better way to reset state!
        close();
        commandQueue.clear();
        responseData.clear();
        return;
    }

    commandQueue.pop_front();

    // Start another command, if there is one.
    processCommandQueue();
}

void SerialCommandQueue::handleSerialError(QSerialPort::SerialPortError serialError)
{
    if(serialError == QSerialPort::NoError) {
        // The serial library appears to emit an extraneous SerialPortError
        // when open() is called. Just ignore it.
        return;
    }

    emit(error(serial->errorString()));

    // TODO: Better way to reset state!
    close();
    commandQueue.clear();
    responseData.clear();
}

void SerialCommandQueue::handleCommandTimeout()
{
    qCritical() << "Command timed out, disconnecting from programmer";
    emit(error("Command timed out, disconnecting from programmer"));

    // TODO: Better way to reset state!
    close();
    commandQueue.clear();
    responseData.clear();
}
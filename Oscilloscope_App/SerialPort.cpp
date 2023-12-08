#include "SerialPort.h"

SerialPort::SerialPort(QObject *parent)
    : QObject{parent}
    ,serialPort(nullptr)
{
    for(int i=0;i<CAPTURE_DEPTH;i++)
    {
        data[i] = 0;
    }
    readDataCounter = 0;
}


SerialPort::~SerialPort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        qDebug() << "Port closed";
    }
    delete serialPort;
}

bool SerialPort::IsConnect(QString portName)
{
    serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    if(serialPort->open(QIODevice::ReadWrite))
    {
        connect(serialPort, &QSerialPort::readyRead, this, &SerialPort::ReadData);
    }

    return serialPort->isOpen();
}

void SerialPort::writeData(const QByteArray &data)
{
    const qint64 written = serialPort->write(data);
    if (written != data.size())
    {
        const QString error = tr("Failed to write all data to port %1.\n"
                                 "Error: %2").arg(serialPort->portName(),
                                       serialPort->errorString());
        qDebug() << error;
    }
}

void SerialPort::ReadData()
{
    qint64 bytesRead = serialPort->read((char *)data+(4096*readDataCounter), 4096);
    readDataCounter++;
    if (bytesRead > 0)
    {
        qDebug() << "Read (" << bytesRead << " bytes)";
    }

    if(readDataCounter == 3)
    {
        emit dataReceived();
        readDataCounter = 0;
    }
}

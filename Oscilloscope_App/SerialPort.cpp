#include "SerialPort.h"

SerialPort::SerialPort(QObject *parent)
    : QObject{parent}
    ,serialPort(nullptr)
{
    for(int i=0;i<CAPTURE_DEPTH;i++)
    {
        captured_data[i] = 0;
        data_to_plot.append(0);
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
    if(readDataCounter <= 1)
    {
        serialPort->read((char *)captured_data+(4096*readDataCounter), 4096);
        readDataCounter++;
    }else
    {
        serialPort->read((char *)captured_data+(4096*readDataCounter), 1808);
        for(int i = 0;i<CAPTURE_DEPTH;++i)
        {
            data_to_plot[i] = captured_data[i] * ADC_CONVERT;
        }
        emit dataReceived();
        readDataCounter = 0;
    }
}

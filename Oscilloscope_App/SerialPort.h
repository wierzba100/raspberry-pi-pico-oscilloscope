#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QDebug>
#include <QMessageBox>

#define CAPTURE_DEPTH 1000

class SerialPort : public QObject
{
    Q_OBJECT

public:
    SerialPort(QObject *parent = nullptr);
    ~SerialPort();
    QSerialPort *serialPort;
    quint8 data[CAPTURE_DEPTH];

signals:
    void dataReceived();

public slots:
    void ReadData();
    bool IsConnect(QString portName);
    void writeData(const QByteArray &data);

private:

};

#endif // SERIALPORT_H

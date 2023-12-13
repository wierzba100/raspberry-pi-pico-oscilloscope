#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QDebug>
#include <QMessageBox>

#define CAPTURE_DEPTH 10000
#define ADC_VREF 3.3
#define ADC_RANGE (1 << 8)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

class SerialPort : public QObject
{
    Q_OBJECT

public:
    SerialPort(QObject *parent = nullptr);
    ~SerialPort();
    QSerialPort *serialPort;
    QVector<double>data_to_plot;

signals:
    void dataReceived();

public slots:
    void ReadData();
    bool IsConnect(QString portName);
    void writeData(const QByteArray &data);

private:
    quint8 captured_data[CAPTURE_DEPTH];
    quint8 readDataCounter;

};

#endif // SERIALPORT_H

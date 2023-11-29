#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include "SerialPort.h"
#include "Chart.h"

#define ADC_VREF 3.3
#define ADC_RANGE (1 << 8)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void SendData();

public slots:
    void updateData();

private:
    Ui::MainWindow *ui;
    SerialPort _serial;
    Chart _chart;
    QTimer *timer;
    void loadPorts();
    void on_btnOpenPort_clicked();
    void on_sendBtn_clicked();
    QByteArray prepare_bytes_to_send();
};

#endif // MAINWINDOW_H

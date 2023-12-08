#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QVector>
#include "SetPortWindow.h"
#include "SerialPort.h"

#define ADC_VREF 3.3
#define ADC_RANGE (1 << 8)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))
#define REFRESH_RATE_HZ 10

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
    SetPortWindow *setPortWindow;
    QTimer *timer;
    void SetRefreshRate();
    void on_pushButton_toggled(bool checked);
    void setupDiagram();
    QByteArray prepare_bytes_to_send();
    QVector<double>x_1channel;
    QVector<double>x_2channels;
};

#endif // MAINWINDOW_H

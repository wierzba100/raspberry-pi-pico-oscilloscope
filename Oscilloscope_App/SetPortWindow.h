#ifndef SETPORTWINDOW_H
#define SETPORTWINDOW_H

#include <QDialog>
#include <QSerialPortInfo>
#include "SerialPort.h"

namespace Ui {
class SetPortWindow;
}

class SetPortWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SetPortWindow(SerialPort* serial, QWidget *parent = nullptr);
    ~SetPortWindow();
    bool isAppClosed;

private slots:

private:
    Ui::SetPortWindow *ui;
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    SerialPort* _serial;
    void loadPorts();

};

#endif // SETPORTWINDOW_H

#include "SetPortWindow.h"
#include "qpushbutton.h"
#include "ui_SetPortWindow.h"
#include <QMessageBox>

SetPortWindow::SetPortWindow(SerialPort* serial, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetPortWindow),
    _serial(serial)
{
    ui->setupUi(this);
    loadPorts();
    connect(ui->okButton, &QPushButton::clicked, this, &SetPortWindow::on_okButton_clicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &SetPortWindow::on_cancelButton_clicked);
}

SetPortWindow::~SetPortWindow()
{
    delete ui;
}

void SetPortWindow::loadPorts()
{
    foreach (auto &port, QSerialPortInfo::availablePorts())
    {
        ui->cmbPorts->addItem(port.portName());
    }
}

void SetPortWindow::on_okButton_clicked()
{
    auto isConnected = _serial->IsConnect(ui->cmbPorts->currentText());
    if(!isConnected)
    {
        QMessageBox::critical(this, "Error", "Port connection error");
    }else
    {
        QMessageBox::information(this, "Info", "Port connected");
    }
    close();
}


void SetPortWindow::on_cancelButton_clicked()
{
    QCoreApplication::quit();
}


#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "qforeach.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->btnOpenPort, &QPushButton::clicked, this, &MainWindow::on_btnOpenPort_clicked);
    connect(ui->sendBtn, &QPushButton::clicked, this, &MainWindow::on_sendBtn_clicked);
    connect(&_serial, SIGNAL(dataReceived()), this, SLOT(updateData()));
    loadPorts();
    auto _ChartView = new QChartView(_chart.m_chart);
    _ChartView->setParent(ui->horizontalFrame);
    _ChartView->resize(1000,600);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SendData()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}

void MainWindow::loadPorts()
{
    foreach (auto &port, QSerialPortInfo::availablePorts())
    {
        ui->cmbPorts->addItem(port.portName());
    }
}


void MainWindow::on_btnOpenPort_clicked()
{
    auto isConnected = _serial.IsConnect(ui->cmbPorts->currentText());
    if(!isConnected)
    {
        QMessageBox::critical(this, "Error", "Port connection error");
    }else
    {
        QMessageBox::information(this, "Info", "Port connected");
    }
}


void MainWindow::SendData()
{
    _serial.writeData(prepare_bytes_to_send());
}

void MainWindow::on_sendBtn_clicked()
{
    timer->stop();
    if(ui->spinBoxfrequency->value() != 0)
    {
        timer->start(1.0 / ui->spinBoxfrequency->value() * 1000.0);
    }else
    {
        MainWindow::SendData();
    }
}

QByteArray MainWindow::prepare_bytes_to_send()
{
    QByteArray data;
    data.append(ui->modeCmbBox->currentIndex());

    if(ui->channels_nrCmbBox->currentIndex() == 1)
    {
        data.append(ui->trigger_channelCmbBox->currentIndex());
        data.append(3);
    }else
    {
        data.append(ui->trigger_channelCmbBox->currentIndex());
        data.append(ui->trigger_channelCmbBox->currentIndex()+1);
    }

    data.append(static_cast<quint8>(ui->trigger_levelDoubleSpinBox->value() / ADC_CONVERT));

    data.append(ui->trigger_edgeCmbBox->currentIndex());

    data.append('\n');

    return data;
}

void MainWindow::updateData()
{
    _chart.m_series_1->clear();
    _chart.m_series_2->clear();
    _chart.m_series_trigger->clear();
    if(ui->modeCmbBox->currentIndex() == 1)
    {
        _chart.m_series_trigger->append(0, ui->trigger_levelDoubleSpinBox->value());
        _chart.m_series_trigger->append(CAPTURE_DEPTH-1, ui->trigger_levelDoubleSpinBox->value());
    }
    if(ui->channels_nrCmbBox->currentIndex() == 0)
    {
        if(ui->trigger_channelCmbBox->currentIndex() == 0)
        {
            for(int i=0;i<CAPTURE_DEPTH;i++)
            {
                _chart.m_series_1->append(i, _serial.data[i] * ADC_CONVERT);
            }
        }else
        {
            for(int i=0;i<CAPTURE_DEPTH;i++)
            {
                _chart.m_series_2->append(i, _serial.data[i] * ADC_CONVERT);
            }
        }
    }else
    {
        for(int i=0;i<CAPTURE_DEPTH;i++)
        {
            if(i%2 == 0)
            {
                _chart.m_series_1->append(i/2, _serial.data[i] * ADC_CONVERT);
            }else
            {
                _chart.m_series_2->append(i/2, _serial.data[i] * ADC_CONVERT);
            }
        }
    }
}


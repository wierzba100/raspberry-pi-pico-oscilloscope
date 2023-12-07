#include "MainWindow.h"
#include "./ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    _serial(this)
{
    ui->setupUi(this);
    setPortWindow = new SetPortWindow(&_serial, this);
    setPortWindow->show();
    setPortWindow->exec();
    connect(&_serial, SIGNAL(dataReceived()), this, SLOT(updateData()));
    ui->start_stopButton->setCheckable(true);
    connect(ui->start_stopButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_toggled);
    auto _ChartView = new QChartView(_chart.m_chart);
    _ChartView->setParent(ui->horizontalFrame);
    _ChartView->setRenderHint(QPainter::Antialiasing);
    _ChartView->setRubberBand(QChartView::HorizontalRubberBand);
    _ChartView->resize(1000,600);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SendData()));
    SetRefreshRate();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}

void MainWindow::SendData()
{
    _serial.writeData(prepare_bytes_to_send());
}

void MainWindow::SetRefreshRate()
{
    timer->stop();
    timer->start(1.0 / REFRESH_RATE_HZ * 1000.0);
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

    for(int i=0;i<CAPTURE_DEPTH;i++)
    {
        if(ui->channels_nrCmbBox->currentIndex() == 0)
        {
            if(ui->trigger_channelCmbBox->currentIndex() == 0)
            {
                _chart.m_series_1->append(i, _serial.data[i] * ADC_CONVERT);
            }else
            {
                _chart.m_series_2->append(i, _serial.data[i] * ADC_CONVERT);
            }
        }else
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

void MainWindow::on_pushButton_toggled(bool checked)
{
    if(checked)
    {
        timer->stop();
    }else
    {
        timer->start(1.0 / REFRESH_RATE_HZ * 1000.0);
    }
}


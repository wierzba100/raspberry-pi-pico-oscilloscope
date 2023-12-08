#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    _serial(this)
{
    ui->setupUi(this);
    setPortWindow = new SetPortWindow(&_serial, this);
    setPortWindow->show();
    setPortWindow->exec();
    if(setPortWindow->isAppClosed)
    {
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<CAPTURE_DEPTH;i++)
    {
        x_1channel.append(i);
    }
    x_2channels = x_1channel.mid(0, CAPTURE_DEPTH/2);
    connect(&_serial, SIGNAL(dataReceived()), this, SLOT(updateData()));
    ui->start_stopButton->setCheckable(true);
    connect(ui->start_stopButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_toggled);
    setupDiagram();
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

void MainWindow::setupDiagram()
{
    ui->myPlot->xAxis->setLabel("Samples");
    ui->myPlot->xAxis->setRange(0, CAPTURE_DEPTH);
    ui->myPlot->yAxis->setLabel("Voltage (V)");
    ui->myPlot->yAxis->setRange(0, 3.3);
    QCPItemStraightLine *horizontalLine = new QCPItemStraightLine(ui->myPlot);
    horizontalLine->point1->setCoords(0, 1.65);
    horizontalLine->point2->setCoords(CAPTURE_DEPTH, 1.65);
    horizontalLine->setPen(QPen(Qt::black));
    ui->myPlot->setInteractions(QCP::iRangeDrag | QCP::iSelectPlottables);
    ui->myPlot->axisRect()->setRangeZoomAxes(ui->myPlot->xAxis, NULL);
    ui->myPlot->setSelectionRectMode(QCP::srmZoom);
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
    QCPItemStraightLine *triggerLine = new QCPItemStraightLine(ui->myPlot);
    triggerLine->point1->setCoords(0, ui->trigger_levelDoubleSpinBox->value());
    triggerLine->point2->setCoords(CAPTURE_DEPTH, ui->trigger_levelDoubleSpinBox->value());
    triggerLine->setPen(QPen(Qt::gray));

    ui->myPlot->clearGraphs();

    if(ui->channels_nrCmbBox->currentIndex() == 0)
    {
        QVector<double>vect_data(CAPTURE_DEPTH);
        for(int i=0;i<CAPTURE_DEPTH;i++)
        {
            vect_data[i] = _serial.data[i] * ADC_CONVERT;
        }

        if(ui->trigger_channelCmbBox->currentIndex() == 0)
        {
            ui->myPlot->addGraph();
            ui->myPlot->graph(0)->setPen(QPen(Qt::blue));
            ui->myPlot->graph(0)->setData(x_1channel, vect_data, true);
        }else
        {
            ui->myPlot->addGraph();
            ui->myPlot->graph(0)->setPen(QPen(Qt::red));
            ui->myPlot->graph(0)->setData(x_1channel, vect_data, true);
        }
    }else
    {
        QVector<double>vect_data_ch1;
        QVector<double>vect_data_ch2;
        for(int i=0;i<CAPTURE_DEPTH;i=i+2)
        {
            vect_data_ch1.append(_serial.data[i] * ADC_CONVERT);
            vect_data_ch2.append(_serial.data[i+1] * ADC_CONVERT);
        }
        ui->myPlot->addGraph();
        ui->myPlot->graph(0)->setPen(QPen(Qt::blue));
        ui->myPlot->graph(0)->setData(x_2channels, vect_data_ch1, true);

        ui->myPlot->addGraph();
        ui->myPlot->graph(1)->setPen(QPen(Qt::red));
        ui->myPlot->graph(1)->setData(x_2channels, vect_data_ch2, true);
    }

    ui->myPlot->replot();
}

void MainWindow::on_pushButton_toggled(bool checked)
{
    if(checked)
    {
        timer->stop();
        ui->myPlot->setInteractions(QCP::iRangeZoom);
        ui->myPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    }else
    {
        timer->start(1.0 / REFRESH_RATE_HZ * 1000.0);
        ui->myPlot->setInteractions(QCP::iRangeDrag);
        ui->myPlot->axisRect()->setRangeDrag(Qt::Horizontal);
        ui->myPlot->axisRect()->setRangeZoomAxes(ui->myPlot->xAxis, NULL);
        ui->myPlot->setSelectionRectMode(QCP::srmZoom);
    }
}


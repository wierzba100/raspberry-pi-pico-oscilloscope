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
        x_1channel[0].append(i*ADC_ONE_CONVERSION_TIME);
        x_1channel[1].append(i*ADC_ONE_CONVERSION_TIME/1000);
    }
    for(int i=0;i<CAPTURE_DEPTH/2;i++)
    {
        x_2channels[0].append(i*ADC_ONE_CONVERSION_TIME*2);
        x_2channels[1].append(i*ADC_ONE_CONVERSION_TIME*2/1000);
    }
    connect(&_serial, SIGNAL(dataReceived()), this, SLOT(updateData()));
    ui->start_stopButton->setCheckable(true);
    connect(ui->start_stopButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_toggled);
    connect(ui->zoomOutpushBtn, &QPushButton::clicked, this, &MainWindow::on_zoomOutpushBtn_clicked);
    connect(ui->shiftToLeftBtn, &QPushButton::clicked, this, &MainWindow::on_shiftToLeftBtn_clicked);
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
    ui->myPlot->xAxis->setLabel("Time[us]");
    ui->myPlot->xAxis->setRange(0, CAPTURE_DEPTH*ADC_ONE_CONVERSION_TIME);
    ui->myPlot->yAxis->setLabel("Voltage (V)");
    ui->myPlot->yAxis->setRange(0, 3.3);
    QCPItemStraightLine *horizontalLine = new QCPItemStraightLine(ui->myPlot);
    horizontalLine->point1->setCoords(0, 1.65);
    horizontalLine->point2->setCoords(CAPTURE_DEPTH, 1.65);
    horizontalLine->setPen(QPen(Qt::black));
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
            ui->myPlot->graph(0)->setData(x_1channel[ui->timeCmbBox->currentIndex()], vect_data, true);
        }else
        {
            ui->myPlot->addGraph();
            ui->myPlot->graph(0)->setPen(QPen(Qt::red));
            ui->myPlot->graph(0)->setData(x_1channel[ui->timeCmbBox->currentIndex()], vect_data, true);
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
        ui->myPlot->graph(0)->setData(x_2channels[ui->timeCmbBox->currentIndex()], vect_data_ch1, true);

        ui->myPlot->addGraph();
        ui->myPlot->graph(1)->setPen(QPen(Qt::red));
        ui->myPlot->graph(1)->setData(x_2channels[ui->timeCmbBox->currentIndex()], vect_data_ch2, true);
    }

    ui->myPlot->xAxis->setLabel("Time[" + ui->timeCmbBox->currentText() + "]");
    ui->myPlot->replot();
}

void MainWindow::on_pushButton_toggled(bool checked)
{
    if(checked)
    {
        timer->stop();
        ui->myPlot->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables);
        ui->myPlot->axisRect()->setRangeZoomAxes(ui->myPlot->xAxis, NULL);
        ui->myPlot->setSelectionRectMode(QCP::srmZoom);
    }else
    {
        ui->myPlot->setSelectionRectMode(QCP::srmNone);
        ui->myPlot->setInteractions(QCP::iSelectPlottables);
        timer->start(1.0 / REFRESH_RATE_HZ * 1000.0);
    }
}

void MainWindow::zoomOut()
{
    ui->myPlot->rescaleAxes();
    ui->myPlot->replot();
}

void MainWindow::on_zoomOutpushBtn_clicked()
{
    zoomOut();
}

void MainWindow::shift_to_left()
{
    QCPAxisRect *axisRect = ui->myPlot->axisRect();

    axisRect->setRangeZoom(Qt::Horizontal);
    axisRect->axis(QCPAxis::atBottom)->setRange(0, axisRect->axis(QCPAxis::atBottom)->range().size(), Qt::AlignLeft);

    ui->myPlot->replot();
}

void MainWindow::on_shiftToLeftBtn_clicked()
{
    shift_to_left();
}


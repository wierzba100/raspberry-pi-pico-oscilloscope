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
    if(setPortWindow->isAppClosed)
    {
        exit(EXIT_FAILURE);
    }
    for(int i=0;i<CAPTURE_DEPTH;i++)
    {
        x_1channel.append(i*ADC_ONE_CONVERSION_TIME);
    }
    for(int i=0;i<CAPTURE_DEPTH/2;i++)
    {
        x_2channels.append(i*ADC_ONE_CONVERSION_TIME*2);
    }
    connect(&_serial, SIGNAL(dataReceived()), this, SLOT(updateData()));
    connect(ui->modeCmbBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateWindow()));
    ui->start_stopButton->setCheckable(true);
    connect(ui->start_stopButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_toggled);
    connect(ui->zoomOutpushBtn, &QPushButton::clicked, this, &MainWindow::on_zoomOutpushBtn_clicked);
    connect(ui->shiftToLeftBtn, &QPushButton::clicked, this, &MainWindow::on_shiftToLeftBtn_clicked);
    setupDiagram();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SendData()));
    setFocusPolicy(Qt::StrongFocus);
    SetRefreshRate();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timer;
}

void MainWindow::SendData()
{
    if(ui->ch_1_checkBox->isChecked() || ui->ch_2_checkBox->isChecked())
    {
        _serial.writeData(prepare_bytes_to_send());
    }else
    {
        ui->myPlot->clearGraphs();
        ui->myPlot->replot();
    }
}

void MainWindow::SetRefreshRate()
{
    timer->stop();
    timer->start(1.0 / REFRESH_RATE_HZ * 1000.0);
}

void MainWindow::setupDiagram()
{
    QColor Color = QColor(0, 0, 255);
    Color = Color.lighter(150);
    ui->ch_1_checkBox->setStyleSheet(QString("QCheckBox { background-color: %1; }").arg(Color.name()));
    ui->ch_1_radioButton->setStyleSheet(QString("QRadioButton { background-color: %1; }").arg(Color.name()));
    Color = QColor(255, 0, 0);
    Color = Color.lighter(150);
    ui->ch_2_checkBox->setStyleSheet(QString("QCheckBox { background-color: %1; }").arg(Color.name()));
    ui->ch_2_radioButton->setStyleSheet(QString("QRadioButton { background-color: %1; }").arg(Color.name()));
    ui->myPlot->setNoAntialiasingOnDrag(true);
    ui->myPlot->xAxis->setLabel("Time[us]");
    ui->myPlot->xAxis->setRange(0, CAPTURE_DEPTH*ADC_ONE_CONVERSION_TIME);
    ui->myPlot->yAxis->setLabel("Voltage (V)");
    ui->myPlot->yAxis->setRange(0, 3.4);
    Color = QColor(0, 255, 0);
    Color = Color.lighter(150);
    triggerLine = new QCPItemStraightLine(ui->myPlot);
    triggerLine->setPen(QPen(Color, 2, Qt::DashLine));
    ui->myPlot->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->myPlot->axisRect()->setRangeZoom(Qt::Horizontal);
    ui->myPlot->setSelectionRectMode(QCP::srmZoom);
}


QByteArray MainWindow::prepare_bytes_to_send()
{
    QByteArray data;
    data.append(ui->modeCmbBox->currentIndex());

    if(ui->ch_1_checkBox->isChecked() && ui->ch_2_checkBox->isChecked())
    {
        if(ui->ch_2_radioButton->isChecked())
        {
            data.append(1);
        }else
        {
            data.append((char) 0);
        }
        data.append(3);
        if(ui->modeCmbBox->currentIndex() == 1)
        {
            ui->ch_1_radioButton->setDisabled(0);
            ui->ch_2_radioButton->setDisabled(0);
        }
    }else
    {
        ui->ch_1_radioButton->setDisabled(1);
        ui->ch_2_radioButton->setDisabled(1);
        if(ui->ch_1_checkBox->isChecked())
        {
            data.append((char) 0);
            data.append(1);
        }else
        {
            data.append(1);
            data.append(2);
        }
    }

    data.append(static_cast<quint8>(ui->trigger_levelDoubleSpinBox->value() / ADC_CONVERT));

    data.append(ui->trigger_edgeCmbBox->currentIndex());
    data.append('\n');

    return data;
}

QVector<double>vect_data_ch1(CAPTURE_DEPTH/2);
QVector<double>vect_data_ch2(CAPTURE_DEPTH/2);
void MainWindow::updateData()
{
    ui->myPlot->clearGraphs();

    triggerLine->point1->setCoords(0, ui->trigger_levelDoubleSpinBox->value());
    triggerLine->point2->setCoords(CAPTURE_DEPTH, ui->trigger_levelDoubleSpinBox->value());

    if(ui->ch_1_checkBox->isChecked() && ui->ch_2_checkBox->isChecked())
    {
        quint16 data_index = 0;
        for(int i=0;i<CAPTURE_DEPTH/2;i++)
        {
            vect_data_ch1[i] = _serial.data_to_plot[data_index];
            vect_data_ch2[i] = _serial.data_to_plot[data_index+1];
            data_index = data_index + 2;
        }
        ui->myPlot->addGraph();
        ui->myPlot->graph(0)->setPen(QPen(Qt::blue));
        ui->myPlot->graph(0)->addData(x_2channels, vect_data_ch1, true);

        ui->myPlot->addGraph();
        ui->myPlot->graph(1)->setPen(QPen(Qt::red));
        ui->myPlot->graph(1)->addData(x_2channels, vect_data_ch2, true);
    }else
    {
        ui->myPlot->addGraph();
        if(ui->ch_1_checkBox->isChecked())
        {
            ui->myPlot->graph(0)->setPen(QPen(Qt::blue));
            ui->myPlot->graph(0)->addData(x_1channel, _serial.data_to_plot, true);
        }else
        {
            ui->myPlot->graph(0)->setPen(QPen(Qt::red));
            ui->myPlot->graph(0)->addData(x_1channel, _serial.data_to_plot, true);
        }
    }

    ui->myPlot->xAxis->setLabel("[us]");
    ui->myPlot->replot();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == 32)
    {
        ui->start_stopButton->click();
    }
}

void MainWindow::on_pushButton_toggled(bool checked)
{
    if(checked)
    {
        ui->start_stopButton->setText("Start (Spacebar)");
        timer->stop();
    }else
    {
        ui->start_stopButton->setText("Stop (Spacebar)");
        timer->start(1.0 / REFRESH_RATE_HZ * 1000.0);
    }
}

void MainWindow::zoomOut()
{
    ui->myPlot->xAxis->setRange(0, CAPTURE_DEPTH*ADC_ONE_CONVERSION_TIME);
    ui->myPlot->yAxis->setRange(0, 3.4);
    ui->myPlot->replot();
}

void MainWindow::on_zoomOutpushBtn_clicked()
{
    zoomOut();
}

void MainWindow::shift_to_left()
{
    QCPAxisRect *axisRect = ui->myPlot->axisRect();

    axisRect->axis(QCPAxis::atBottom)->setRange(0, axisRect->axis(QCPAxis::atBottom)->range().size(), Qt::AlignLeft);

    ui->myPlot->replot();
}

void MainWindow::on_shiftToLeftBtn_clicked()
{
    shift_to_left();
}

void MainWindow::updateWindow()
{
    if(ui->modeCmbBox->currentIndex() == 0)
    {
        ui->ch_1_radioButton->setDisabled(1);
        ui->ch_2_radioButton->setDisabled(1);
        ui->trigger_edgeCmbBox->setDisabled(1);
        ui->trigger_levelDoubleSpinBox->setDisabled(1);
    }else
    {
        ui->ch_1_radioButton->setDisabled(0);
        ui->ch_2_radioButton->setDisabled(0);
        ui->trigger_edgeCmbBox->setDisabled(0);
        ui->trigger_levelDoubleSpinBox->setDisabled(0);
    }
}


#include "Chart.h"
#include "SerialPort.h"

Chart::Chart(QWidget *parent)
    : QChartView{parent}
    , m_chart(new QChart)
    , m_series_1(new QLineSeries)
    , m_series_2(new QLineSeries)
    , m_series_trigger(new QLineSeries)
{
    m_chart->addSeries(m_series_1);
    m_chart->addSeries(m_series_2);

    m_series_1->setPen(QPen(Qt::blue, 2));
    m_series_2->setPen(QPen(Qt::red, 2));

    auto axisX = new QValueAxis;
    axisX->setRange(0, CAPTURE_DEPTH-1);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Samples");

    auto axisY = new QValueAxis;
    axisY->setRange(0, 3.3);
    axisY->setTitleText("Voltage (V)");

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series_1->attachAxis(axisX);
    m_series_2->attachAxis(axisX);


    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series_1->attachAxis(axisY);
    m_series_2->attachAxis(axisY);

    //Line Zero
    QLineSeries* horizontalLine = new QLineSeries();
    horizontalLine->append(0, 1.65);
    horizontalLine->append(CAPTURE_DEPTH-1, 1.65);
    horizontalLine->setPen(QPen(Qt::black));  // Kolor linii poziomej

    m_chart->addSeries(horizontalLine);
    horizontalLine->attachAxis(axisX);
    horizontalLine->attachAxis(axisY);

    //Line trigger
    m_chart->addSeries(m_series_trigger);
    m_series_trigger->attachAxis(axisX);
    m_series_trigger->attachAxis(axisY);
    m_series_trigger->setPen(QPen(Qt::gray, 2, Qt::DashLine));

    m_chart->legend()->hide();
    m_chart->setTitle("Data from oscilloscope");

}

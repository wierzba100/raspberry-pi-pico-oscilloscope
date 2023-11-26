#ifndef CHART_H
#define CHART_H

#include <QObject>
#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>

class Chart : public QChartView
{
    Q_OBJECT
public:
    explicit Chart(QWidget *parent = nullptr);
    QChart *m_chart = nullptr;
    QLineSeries *m_series_1 = nullptr;
    QLineSeries *m_series_2 = nullptr;
    QLineSeries *m_series_trigger = nullptr;

signals:

private:

};

#endif // CHART_H

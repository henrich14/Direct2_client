#ifndef AIRCRAFTWIDGET_H
#define AIRCRAFTWIDGET_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class AircraftWidget;
}

class AircraftWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AircraftWidget(QWidget *parent = 0);
    ~AircraftWidget();

signals:
    void metricChanged_signal(const QString &metrics);

private:
    Ui::AircraftWidget *ui;

private slots:
    void metricsChanged(const QString &metrics);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
};

#endif // AIRCRAFTWIDGET_H

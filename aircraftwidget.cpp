#include "aircraftwidget.h"
#include "ui_aircraftwidget.h"

AircraftWidget::AircraftWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AircraftWidget)
{
    ui->setupUi(this);

    ui->comboBox->insertItem(0,"ft");
    ui->comboBox->insertItem(1,"m");

    connect(ui->comboBox, SIGNAL(activated(QString)), this, SLOT(metricsChanged(QString)));
}

AircraftWidget::~AircraftWidget()
{
    delete ui;
}

void AircraftWidget::metricsChanged(const QString &metrics)
{
    emit metricChanged_signal(metrics);
    this->close();
}

void AircraftWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape)
        {
            this->close();
        }
}

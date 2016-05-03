#include "buttonwidget.h"
#include "ui_buttonwidget.h"

ButtonWidget::ButtonWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ButtonWidget)
{
    ui->setupUi(this);

    ui->lineEdit->setEnabled(false);
}

ButtonWidget::~ButtonWidget()
{
    delete ui;
}

void ButtonWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape)
        {
            this->close();
        }
}

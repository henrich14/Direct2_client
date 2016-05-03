#ifndef BUTTONWIDGET_H
#define BUTTONWIDGET_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class ButtonWidget;
}

class ButtonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonWidget(QWidget *parent = 0);
    ~ButtonWidget();

private:
    Ui::ButtonWidget *ui;

protected:
    virtual void keyPressEvent(QKeyEvent *e);
};

#endif // BUTTONWIDGET_H

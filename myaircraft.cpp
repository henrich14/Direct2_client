#include "myaircraft.h"

myAircraft::myAircraft(QGraphicsObject *parent) : QGraphicsObject(parent)
{
}


QRectF myAircraft::boundingRect() const
{
    return QRectF(-5,-5,10,10);
}

void myAircraft::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rec = QRectF(-5,-5,10,10);
    QPen customPen(Qt::red);
    QBrush brush(Qt::blue);

    painter->fillRect(rec,brush);
    painter->setPen(customPen);
}


void myAircraft::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    if(event->button() == Qt::RightButton)
    {
        emit aircraft_clicked();
        //QPointF mousePos = QPointF(mapToParent(event->pos()));
        //qDebug() << mousePos;
        //emit mousePosCoord(mousePos);
    }
    else
    {
       QGraphicsItem::mousePressEvent(event);
    }
}

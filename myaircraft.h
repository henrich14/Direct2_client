#ifndef MYAIRCRAFT_H
#define MYAIRCRAFT_H

#define SCALE_FACTOR 5

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QVariant>
#include <QCursor>
#include <QtGui>
#include <QtCore>
#include <QPointF>

class myAircraft : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit myAircraft(QGraphicsObject *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:
    void aircraft_clicked();
    void mousePosCoord(const QPointF &mousePos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif // MYAIRCRAFT_H

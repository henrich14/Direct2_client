#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPolygon>
#include <QMouseEvent>

#include "waypoint.h"
#include "flightplan.h"
#include "myaircraft.h"
#include "aircraftwidget.h"
#include "buttonwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void drawGrid();
    void drawWP();
    void drawAircraft(const QPointF &ACPos);
    void drawRoute();
    void drawAlternativeRoute(const int &WP_idx);
    Waypoint generateEnrWP(const int &idx);
    QString randString(int len);
    double distance(const double &x1, const double &y1, const double &x2, const double &y2);
    double distance(const QPointF &P1, const QPointF &P2);
    QPointF getACpos(myAircraft &AC);
    QPointF moveAircraftTo(const QPointF &ACPos_current);
    void updateNxtWPIdx(const QPointF &ACPos);
    void stopSimulation();
    QString FP2Message(FlightPlan FP);
    FlightPlan message2FP(const QString &message);
    int checkAlternativeFP(FlightPlan FP);


private slots:
    void readyRead();
    void connected();

    void simulationTimeX2Changed();
    void simulationTimeX05Changed();
    void run();
    void pause();
    void generateFP();
    void submitFPAlternative();
    void accept();
    void reject();

    void timeOut();
    void aircraftClicked();
    void metricsChanged(const QString &metrics);

    void buttonTimeout();

private:
    Ui::MainWindow *ui;

    QTcpSocket *client;
    QTimer *timer;
    QTimer *buttonTimer;

    FlightPlan *flightPlan;
    FlightPlan *alternativeFP;
    QString receivedMessage;
    bool submited;
    int nxtWP_idx;
    bool ftORm;
    int simulationTime;
    int timerConst;
    bool paused;

    QGraphicsScene *scene;
    QGraphicsEllipseItem *ellipse;
    QGraphicsRectItem *rectangle;
    QGraphicsLineItem *line;
    QGraphicsTextItem *text;

    myAircraft *myaircraft;
    AircraftWidget *aircraftWidget;
    ButtonWidget *buttonWidget;

    QPolygonF aircraftPosHistory;
};

#endif // MAINWINDOW_H

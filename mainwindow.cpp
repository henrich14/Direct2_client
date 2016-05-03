#include "mainwindow.h"
#include "ui_mainwindow.h"

#define PI 3.14159265359

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // set the window and scene rect
    setWindowTitle("Direct2 CLIENT");
    //setFixedSize(700,700);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setSceneRect(0, 0, 550, 550);
    ui->graphicsView->setFixedSize(750,650);

    ui->pauseButton->setEnabled(false);
    ui->runButton->setEnabled(false);
    ui->submitAltFPButton->setEnabled(false);
    ui->acceptButton->setEnabled(false);
    ui->rejectButton->setEnabled(false);

    simulationTime = 0;
    timerConst = 1000;
    paused = false;
    submited = false;

    connect(ui->x2chB, SIGNAL(clicked()), this, SLOT(simulationTimeX2Changed()));
    connect(ui->x05chB, SIGNAL(clicked()), this, SLOT(simulationTimeX05Changed()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(ui->pauseButton, SIGNAL(clicked()), this, SLOT(pause()));
    connect(ui->generateFPButton, SIGNAL(clicked()), this, SLOT(generateFP()));
    connect(ui->submitAltFPButton, SIGNAL(clicked()), this, SLOT(submitFPAlternative()));
    connect(ui->acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->rejectButton, SIGNAL(clicked()), this, SLOT(reject()));

    // set seed for random generator for en-route WP generation
    qsrand(QDateTime::currentDateTime().toTime_t());


    ftORm = true; // true:[ft]; false:[m]
    ui->altitudeUnitLabel->setText("Altitude:[ft]");

    // draw basic grid on scene
    drawGrid();

    myaircraft = new myAircraft();
    myaircraft->setPos(0,0);

    buttonWidget = new ButtonWidget();
    buttonTimer = new QTimer(this);
    connect(buttonTimer, SIGNAL(timeout()), this, SLOT(buttonTimeout()));

    aircraftWidget = new AircraftWidget();
    connect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    connect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));


    // client-server communication
    client = new QTcpSocket(this);

    connect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(client, SIGNAL(connected()), this, SLOT(connected()));

    client->connectToHost("localhost", 4200); // connect to port 4200

    // set the timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeOut()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawGrid()
{
    QBrush redBrush(Qt::red);
    QBrush blueBrush(Qt::blue);
    QPen blackpen(Qt::black);
    QPen lightGrayPen(Qt::lightGray);
    blackpen.setWidth(1);

    // draw scene rect
    QLineF TopLine(scene->sceneRect().topLeft(), scene->sceneRect().topRight());
    QLineF RightLine(scene->sceneRect().topRight(), scene->sceneRect().bottomRight());
    QLineF LeftLine(scene->sceneRect().topLeft(), scene->sceneRect().bottomLeft());
    QLineF BottomLine(scene->sceneRect().bottomLeft(), scene->sceneRect().bottomRight());

    scene->addLine(TopLine,blackpen);
    scene->addLine(RightLine,blackpen);
    scene->addLine(LeftLine,blackpen);
    scene->addLine(BottomLine,blackpen);

    for(int i=0; i<11; i++)
    {
        line = scene->addLine(0+i*50, 0, 0+i*50, 550, lightGrayPen);
        line = scene->addLine(0, 0+i*50, 550, 0+i*50, lightGrayPen);
    }
}

void MainWindow::drawWP()
{
    QBrush redBrush(Qt::red);
    QPen blackpen(Qt::black);

    //double scaleFactor = 5;

    for(int i=0; i<12; i++)
    {
        double x = flightPlan->WP(i).x();
        double y = flightPlan->WP(i).y();
        double w = 6;
        double h = 6;
        QString label = flightPlan->WP(i).label();

        rectangle = scene->addRect(x * SCALE_FACTOR - w/2,y * SCALE_FACTOR - h/2, w, h, blackpen, redBrush);
        text = scene->addText(label);
        text->setPos(x * SCALE_FACTOR - w/2,y * SCALE_FACTOR - h/2);
    }
}

void MainWindow::drawAircraft(const QPointF &ACPos)
{
    // draw aircraft on scene; input is scaled for drawing
    myaircraft = new myAircraft();
    myaircraft->setPos(ACPos.x()*SCALE_FACTOR, ACPos.y()*SCALE_FACTOR);
    scene->addItem(myaircraft);

    // draw aircraft label
    double nxtWPAlt = flightPlan->WP(nxtWP_idx).altitude();
    double prevWPAlt = flightPlan->WP(nxtWP_idx-1).altitude();
    double WP2WPdist = distance(QPointF(flightPlan->WP(nxtWP_idx).x(),flightPlan->WP(nxtWP_idx).y()), QPointF(flightPlan->WP(nxtWP_idx-1).x(),flightPlan->WP(nxtWP_idx-1).y()));
    double WP2ACdist = distance(QPointF(flightPlan->WP(nxtWP_idx-1).x(),flightPlan->WP(nxtWP_idx-1).y()), ACPos);
    double ACAltitude = ((nxtWPAlt - prevWPAlt) * (WP2ACdist / WP2WPdist)) + prevWPAlt;

    if(ftORm)
    {
        ACAltitude = ACAltitude * 3.2808399;
    }

    text = scene->addText(QString::number(ACPos.x(),'f',1)+ "; " + QString::number(ACPos.y(),'f',1)+ "; " + QString::number(ACAltitude));
    text->setPos(ACPos.x()*SCALE_FACTOR+10, ACPos.y()*SCALE_FACTOR);

    // saving aircraft flight history
    aircraftPosHistory << QPointF(myaircraft->pos().x() / SCALE_FACTOR, myaircraft->pos().y() / SCALE_FACTOR);
}

void MainWindow::drawRoute()
{
    // behind route
    QPolygonF behindRoute;
    QPen pen(QColor(30,144,255));

    for(int i=0; i<=nxtWP_idx-1; i++)
    {
        if(flightPlan->WP(i).mandatory() == true)
        {
            behindRoute << QPointF(flightPlan->WP(i).x(), flightPlan->WP(i).y());
        }
    }
    behindRoute << getACpos(*myaircraft);

    for(int i=0; i<behindRoute.length()-1; i++)
    {
        line = scene->addLine(behindRoute.at(i).x() * SCALE_FACTOR, behindRoute.at(i).y() * SCALE_FACTOR,
                              behindRoute.at(i+1).x() * SCALE_FACTOR, behindRoute.at(i+1).y() * SCALE_FACTOR, pen);
    }

    // front route
    QPolygonF frontRoute;
    pen.setColor(QColor(0,0,139));

    frontRoute << getACpos(*myaircraft);
    if(nxtWP_idx == 11)
    {
        frontRoute << QPointF(flightPlan->WP(11).x(), flightPlan->WP(11).y());
    }
    else
    {
        for(int i=nxtWP_idx; i<=11; i++)
        {
            if(flightPlan->WP(i).mandatory() == true)
            {
                frontRoute << QPointF(flightPlan->WP(i).x(), flightPlan->WP(i).y());
            }
        }
    }


    for(int i=0; i<frontRoute.length()-1; i++)
    {
        line = scene->addLine(frontRoute.at(i).x() * SCALE_FACTOR, frontRoute.at(i).y() * SCALE_FACTOR,
                              frontRoute.at(i+1).x() * SCALE_FACTOR, frontRoute.at(i+1).y() * SCALE_FACTOR, pen);
    }
}

void MainWindow::drawAlternativeRoute(const int &WP_idx)
{
    // behind route
    QPolygonF behindRoute;
    QPen pen(QColor(30,144,255));
    QPen pen_shared(QColor(255,255,0));
    QPen pen_alternative(QColor(255,0,0));

    for(int i=0; i<=nxtWP_idx-1; i++)
    {
        if(flightPlan->WP(i).mandatory() == true)
        {
            behindRoute << QPointF(flightPlan->WP(i).x(), flightPlan->WP(i).y());
        }
    }
    behindRoute << getACpos(*myaircraft);

    for(int i=0; i<behindRoute.length()-1; i++)
    {
        line = scene->addLine(behindRoute.at(i).x() * SCALE_FACTOR, behindRoute.at(i).y() * SCALE_FACTOR,
                              behindRoute.at(i+1).x() * SCALE_FACTOR, behindRoute.at(i+1).y() * SCALE_FACTOR, pen);
    }

    // front route before WP
    QPolygonF frontRoute;
    pen.setColor(QColor(0,0,139));

    frontRoute << getACpos(*myaircraft);
    if(nxtWP_idx == 11)
    {
        frontRoute << QPointF(flightPlan->WP(11).x(), flightPlan->WP(11).y());
    }
    else
    {
        for(int i=nxtWP_idx; i<=WP_idx-1; i++)
        {
            if(flightPlan->WP(i).mandatory() == true)
            {
                frontRoute << QPointF(flightPlan->WP(i).x(), flightPlan->WP(i).y());
            }
        }
    }

    for(int i=0; i<frontRoute.length()-1; i++)
    {
        line = scene->addLine(frontRoute.at(i).x() * SCALE_FACTOR, frontRoute.at(i).y() * SCALE_FACTOR,
                              frontRoute.at(i+1).x() * SCALE_FACTOR, frontRoute.at(i+1).y() * SCALE_FACTOR, pen);
    }


    // front route in front of WP
    frontRoute.clear();
    if(nxtWP_idx == 11)
    {
        frontRoute << QPointF(flightPlan->WP(11).x(), flightPlan->WP(11).y());
    }
    else
    {
        for(int i=WP_idx+1; i<=11; i++)
        {
            if(flightPlan->WP(i).mandatory() == true)
            {
                frontRoute << QPointF(flightPlan->WP(i).x(), flightPlan->WP(i).y());
            }
        }
    }

    for(int i=0; i<frontRoute.length()-1; i++)
    {
        line = scene->addLine(frontRoute.at(i).x() * SCALE_FACTOR, frontRoute.at(i).y() * SCALE_FACTOR,
                              frontRoute.at(i+1).x() * SCALE_FACTOR, frontRoute.at(i+1).y() * SCALE_FACTOR, pen);
    }

    // route corresponding to change
    frontRoute.clear();
    if(WP_idx == nxtWP_idx)
    {
        // route starts at aircraft position
        frontRoute << getACpos(*myaircraft);
        frontRoute << QPointF(flightPlan->WP(WP_idx).x(), flightPlan->WP(WP_idx).y());
        int k = 1;
        while(flightPlan->WP(WP_idx+k).mandatory() == false)
        {
            k += 1;
        }
        frontRoute << QPointF(flightPlan->WP(WP_idx+k).x(), flightPlan->WP(WP_idx+k).y());
    }
    else
    {
        // route starts at WP position
        int k = 1;
        while(flightPlan->WP(WP_idx-k).mandatory() == false)
        {
            k += 1;
        }
        frontRoute << QPointF(flightPlan->WP(WP_idx-k).x(), flightPlan->WP(WP_idx-k).y());
        frontRoute << QPointF(flightPlan->WP(WP_idx).x(), flightPlan->WP(WP_idx).y());

        k = 1;
        while(flightPlan->WP(WP_idx+k).mandatory() == false)
        {
            k += 1;
        }
        frontRoute << QPointF(flightPlan->WP(WP_idx+k).x(), flightPlan->WP(WP_idx+k).y());
    }

    for(int i=0; i<frontRoute.length()-1; i++)
    {
        line = scene->addLine(frontRoute.at(i).x() * SCALE_FACTOR, frontRoute.at(i).y() * SCALE_FACTOR,
                              frontRoute.at(i+1).x() * SCALE_FACTOR, frontRoute.at(i+1).y() * SCALE_FACTOR, pen_shared);
    }

    // route corresponding to change
    frontRoute.clear();
    if(WP_idx == nxtWP_idx)
    {
        // route starts at aircraft position
        frontRoute << getACpos(*myaircraft);

        int k = 1;
        while(flightPlan->WP(WP_idx+k).mandatory() == false)
        {
            k += 1;
        }
        frontRoute << QPointF(flightPlan->WP(WP_idx+k).x(), flightPlan->WP(WP_idx+k).y());
    }
    else
    {
        // route starts at WP position
        int k = 1;
        while(flightPlan->WP(WP_idx-k).mandatory() == false)
        {
            k += 1;
        }
        frontRoute << QPointF(flightPlan->WP(WP_idx-k).x(), flightPlan->WP(WP_idx-k).y());

        k = 1;
        while(flightPlan->WP(WP_idx+k).mandatory() == false)
        {
            k += 1;
        }
        frontRoute << QPointF(flightPlan->WP(WP_idx+k).x(), flightPlan->WP(WP_idx+k).y());
    }

    for(int i=0; i<frontRoute.length()-1; i++)
    {
        line = scene->addLine(frontRoute.at(i).x() * SCALE_FACTOR, frontRoute.at(i).y() * SCALE_FACTOR,
                              frontRoute.at(i+1).x() * SCALE_FACTOR, frontRoute.at(i+1).y() * SCALE_FACTOR, pen_alternative);
    }
}

Waypoint MainWindow::generateEnrWP(const int &idx)
{
    //double scaleFactor = 5;
    double randNum_y = (0.0 + ((double)rand() / (RAND_MAX + 1))) * (4.0 * SCALE_FACTOR);
    double randNum_alt = (0.0 + ((double)rand() / (RAND_MAX + 1))) * 1000.0;

    double x = idx * 10.0;
    double y = x + (-2 + randNum_y);
    double altitude = 10000 + (-500 + randNum_alt);

    QString label = randString(3);

    Waypoint WP(x, y, altitude, label, true, this);

    return WP;
}

QString MainWindow::randString(int len)
{
    QString str;
    str.resize(len);
    for (int s = 0; s < len ; ++s)
        str[s] = QChar('A' + char(qrand() % ('Z' - 'A')));

    return str;
}

double MainWindow::distance(const double &x1, const double &y1, const double &x2, const double &y2)
{
    // calculate the distance between 2 points in 2D
    double pow_1 = (x1-x2) * (x1-x2);
    double pow_2 = (y1-y2) * (y1-y2);
    double dist = qSqrt(pow_1 + pow_2);

    return dist;
}

double MainWindow::distance(const QPointF &P1, const QPointF &P2)
{
    // calculate the distance between 2 points in 2D
    double x1 = P1.x();
    double x2 = P2.x();
    double y1 = P1.y();
    double y2 = P2.y();

    double pow_1 = (x1-x2) * (x1-x2);
    double pow_2 = (y1-y2) * (y1-y2);
    double dist = qSqrt(pow_1 + pow_2);

    return dist;
}

QPointF MainWindow::getACpos(myAircraft &AC)
{
    // return aircraft position scaled back to real world
    QPointF ACPos = QPointF(AC.pos().x() / SCALE_FACTOR, AC.pos().y() / SCALE_FACTOR);
    return ACPos;
}

QPointF MainWindow::moveAircraftTo(const QPointF &ACPos_current)
{
    // calculate position to move aircraft to



    // vynasobit/vydelit SCALE_FACTORom ale ako vstup a vystup brat original hodnoty

    Waypoint nxtWP = flightPlan->WP(nxtWP_idx);

    double angle = qAtan2(nxtWP.y() - ACPos_current.y(), nxtWP.x() - ACPos_current.x());

    double dx = qCos(angle) * 1;
    double dy = qSin(angle) * 1;

    QPointF ACPosTO = ACPos_current + QPointF(dx,dy);

    // snap aircraft position to the WP positoin when WP is reached
    double dist  = distance(ACPos_current, QPointF(nxtWP.x(), nxtWP.y()));
    if(dist <= 2)
    {
        ACPosTO = QPointF(nxtWP.x(), nxtWP.y());
    }

    // check for next WP update from flightPlan
    updateNxtWPIdx(ACPos_current);

    return ACPosTO;
}

void MainWindow::updateNxtWPIdx(const QPointF &ACPos)
{
    // update index of next WP is it becomes non-mandatory
    bool mndtry = flightPlan->WP(nxtWP_idx).mandatory();

    if(!mndtry)
    {
        while(flightPlan->WP(nxtWP_idx).mandatory() == false)
        {
            nxtWP_idx += 1;
        }
    }

    // update index of next WP if distance from WP is smaller than threshold (R < 2)
    double dist  = distance(ACPos, QPointF(flightPlan->WP(nxtWP_idx).x(), flightPlan->WP(nxtWP_idx).y()));
    if((dist <= 2) && (nxtWP_idx <= 11))
    {
        nxtWP_idx += 1;

        if(nxtWP_idx > 11)
        {
            stopSimulation();
        }
    }
}

void MainWindow::aircraftClicked()
{
    aircraftWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    aircraftWidget->show();
    aircraftWidget->move(mapToGlobal(myaircraft->pos().toPoint()));
}

void MainWindow::metricsChanged(const QString &metrics)
{
    if(metrics == "ft")
    {
        ftORm = true;
        ui->altitudeUnitLabel->setText("Altitude:[ft]");
    }
    else
    {
        ftORm = false;
        ui->altitudeUnitLabel->setText("Altitude:[m]");
    }

    QPointF ACPos_current = getACpos(*myaircraft);

    disconnect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    disconnect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));
}

void MainWindow::buttonTimeout()
{
    buttonTimer->stop();
    buttonWidget->close();
}

void MainWindow::stopSimulation()
{
    timer->stop();
    simulationTime = 0;
    ui->generateFPButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->runButton->setEnabled(false);
    ui->submitAltFPButton->setEnabled(false);
    ui->acceptButton->setEnabled(false);
    ui->rejectButton->setEnabled(false);
}

QString MainWindow::FP2Message(FlightPlan FP)
{
    // convert flight plan to string (serialization)
    QStringList messageList;

    messageList << QString::number(FP.timeStamp());

    for(int i=0; i<=11; i++)
    {
        QString WP = QString::number(FP.WP(i).x()) + "," + QString::number(FP.WP(i).y())+ "," + QString::number(FP.WP(i).altitude())
                + "," + FP.WP(i).label()+ "," + QString::number(FP.WP(i).mandatory());
        messageList << WP;
    }

    messageList << QString::number(nxtWP_idx);

    return messageList.join(";");
}

FlightPlan MainWindow::message2FP(const QString &message)
{
    // convert received message from string to Flight Plan
    QStringList messageList = message.split(";");

    uint unixtime = messageList[0].toUInt();

    Waypoint WP0(messageList[1].split(",")[0].toDouble(), messageList[1].split(",")[1].toDouble(),messageList[1].split(",")[2].toDouble(),
            messageList[1].split(",")[3], messageList[1].split(",")[4].toInt());
    Waypoint WP1(messageList[2].split(",")[0].toDouble(), messageList[2].split(",")[1].toDouble(),messageList[2].split(",")[2].toDouble(),
            messageList[2].split(",")[3], messageList[2].split(",")[4].toInt());
    Waypoint WP2(messageList[3].split(",")[0].toDouble(), messageList[3].split(",")[1].toDouble(),messageList[3].split(",")[2].toDouble(),
            messageList[3].split(",")[3], messageList[3].split(",")[4].toInt());
    Waypoint WP3(messageList[4].split(",")[0].toDouble(), messageList[4].split(",")[1].toDouble(),messageList[4].split(",")[2].toDouble(),
            messageList[4].split(",")[3], messageList[4].split(",")[4].toInt());
    Waypoint WP4(messageList[5].split(",")[0].toDouble(), messageList[5].split(",")[1].toDouble(),messageList[5].split(",")[2].toDouble(),
            messageList[5].split(",")[3], messageList[5].split(",")[4].toInt());
    Waypoint WP5(messageList[6].split(",")[0].toDouble(), messageList[6].split(",")[1].toDouble(),messageList[6].split(",")[2].toDouble(),
            messageList[6].split(",")[3], messageList[6].split(",")[4].toInt());
    Waypoint WP6(messageList[7].split(",")[0].toDouble(), messageList[7].split(",")[1].toDouble(),messageList[7].split(",")[2].toDouble(),
            messageList[7].split(",")[3], messageList[7].split(",")[4].toInt());
    Waypoint WP7(messageList[8].split(",")[0].toDouble(), messageList[8].split(",")[1].toDouble(),messageList[8].split(",")[2].toDouble(),
            messageList[8].split(",")[3], messageList[8].split(",")[4].toInt());
    Waypoint WP8(messageList[9].split(",")[0].toDouble(), messageList[9].split(",")[1].toDouble(),messageList[9].split(",")[2].toDouble(),
            messageList[9].split(",")[3], messageList[9].split(",")[4].toInt());
    Waypoint WP9(messageList[10].split(",")[0].toDouble(), messageList[10].split(",")[1].toDouble(),messageList[10].split(",")[2].toDouble(),
            messageList[10].split(",")[3], messageList[10].split(",")[4].toInt());
    Waypoint WP10(messageList[11].split(",")[0].toDouble(), messageList[11].split(",")[1].toDouble(),messageList[11].split(",")[2].toDouble(),
            messageList[11].split(",")[3], messageList[11].split(",")[4].toInt());
    Waypoint WP11(messageList[12].split(",")[0].toDouble(), messageList[12].split(",")[1].toDouble(),messageList[12].split(",")[2].toDouble(),
            messageList[12].split(",")[3], messageList[12].split(",")[4].toInt());

    FlightPlan *FP = new FlightPlan(unixtime, WP0, WP1, WP2, WP3, WP4, WP5, WP6, WP7, WP8, WP9, WP10, WP11);

    return *FP;
}

int MainWindow::checkAlternativeFP(FlightPlan FP)
{
    int idx = 0;

    for(int i=0; i<=11; i++)
    {
        if(FP.WP(i).mandatory() != flightPlan->WP(i).mandatory())
        {
            idx = i;
        }
    }
    return idx;
}

void MainWindow::readyRead()
{
    //alternativeFP = new FlightPlan();
    receivedMessage = QString::fromUtf8(client->readAll()); // received message

    alternativeFP = &message2FP(receivedMessage);                   // convertion of message to Flight Plan
    int WPChange_idx = checkAlternativeFP(*alternativeFP);  // index of changed WP

    QPointF ACPos_current = getACpos(*myaircraft);

    disconnect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    disconnect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));

    scene->clear();
    drawGrid();
    drawWP();
    drawAircraft(ACPos_current);
    connect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    connect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));

    if(WPChange_idx != 0)
    {
        // alternative FP has been found
        drawAlternativeRoute(WPChange_idx);
        ui->acceptButton->setEnabled(true);
        ui->rejectButton->setEnabled(true);
        ui->submitAltFPButton->setEnabled(false);
    }
    else
    {
        buttonWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
        buttonWidget->show();
        buttonTimer->start(2000);

        buttonWidget->move(mapToGlobal(ui->submitAltFPButton->pos()).x()+125,mapToGlobal(ui->submitAltFPButton->pos()).y()+25);

        // alternative FP has NOT been found
        drawRoute();
        ui->acceptButton->setEnabled(false);
        ui->rejectButton->setEnabled(false);
    }
}

void MainWindow::connected()
{
    qDebug() << "connected";
}

void MainWindow::generateFP()
{
    ui->runButton->setEnabled(true);

    // index of next Waypoint in route
    nxtWP_idx = 1;

    // init WayPoints
    Waypoint WP0(0, 0, 0, randString(3), true, this);
    Waypoint WP1(10, 10, 10000, randString(3), true, this);
    Waypoint WP10(100, 100, 10000, randString(3), true, this);
    Waypoint WP11(110, 110, 0, randString(3), true, this);

    Waypoint WP2 = generateEnrWP(2);
    Waypoint WP3 = generateEnrWP(3);
    Waypoint WP4 = generateEnrWP(4);
    Waypoint WP5 = generateEnrWP(5);
    Waypoint WP6 = generateEnrWP(6);
    Waypoint WP7 = generateEnrWP(7);
    Waypoint WP8 = generateEnrWP(8);
    Waypoint WP9 = generateEnrWP(9);

    uint unixtime = QDateTime::currentDateTime().toTime_t();
    FlightPlan *initFP = new FlightPlan(unixtime, WP0, WP1, WP2, WP3, WP4, WP5, WP6, WP7, WP8, WP9, WP10, WP11, this);

    flightPlan = initFP;

    myaircraft = new myAircraft();
    myaircraft->setPos(0,0);

    scene->clear();
    drawGrid();
    drawWP();
    drawAircraft(myaircraft->pos());
    connect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    connect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));
    drawRoute();
}

void MainWindow::submitFPAlternative()
{
    client->write(FP2Message(*flightPlan).toUtf8());

    submited = true;
}

void MainWindow::accept()
{
    ui->acceptButton->setEnabled(false);
    ui->rejectButton->setEnabled(false);
    ui->submitAltFPButton->setEnabled(true);

    *flightPlan = message2FP(receivedMessage);

    QPointF ACPos_current = getACpos(*myaircraft);

    disconnect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    disconnect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));

    scene->clear();
    drawGrid();
    drawWP();
    drawAircraft(ACPos_current);
    connect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    connect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));
    drawRoute();

    while(flightPlan->WP(nxtWP_idx).mandatory() == false)
    {
        nxtWP_idx += 1;
    }
}

void MainWindow::reject()
{
    ui->acceptButton->setEnabled(false);
    ui->rejectButton->setEnabled(false);
    ui->submitAltFPButton->setEnabled(true);
    submited = false;

    QPointF ACPos_current = getACpos(*myaircraft);

    disconnect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    disconnect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));

    scene->clear();
    drawGrid();
    drawWP();
    drawAircraft(ACPos_current);
    connect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    connect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));
    drawRoute();
}

void MainWindow::run()
{
    ui->runButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    ui->generateFPButton->setEnabled(false);
    ui->submitAltFPButton->setEnabled(true);
    paused = false;

    if(ui->x2chB->isChecked())
    {
        timer->start(500);
    }
    else if(ui->x05chB->isChecked())
    {
        timer->start(2000);
    }
    else
    {
        timer->start(1000);
    }
}

void MainWindow::pause()
{
    ui->runButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    paused = true;
    timer->stop();
}

void MainWindow::simulationTimeX05Changed()
{
    if(ui->x05chB->isChecked())
    {
       ui->x2chB->setChecked(false);
    }

    if(simulationTime != 0 && !paused)
    {
        run();
    }
}

void MainWindow::simulationTimeX2Changed()
{
    if(ui->x05chB->isChecked())
    {
        ui->x05chB->setChecked(false);
    }

    if(simulationTime != 0 && !paused)
    {
        run();
    }
}

void MainWindow::timeOut()
{
    simulationTime +=1;

    QPointF ACPos_current = getACpos(*myaircraft);
    QPointF ACPos_next = moveAircraftTo(ACPos_current);

    disconnect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    disconnect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));

    scene->clear();

    // draw basic grid on scene
    drawGrid();
    // draw WayPoints on scene
    drawWP();
    // draw aircraft on scene
    drawAircraft(ACPos_next);
    connect(myaircraft, SIGNAL(aircraft_clicked()), this, SLOT(aircraftClicked()));
    connect(aircraftWidget, SIGNAL(metricChanged_signal(QString)), this, SLOT(metricsChanged(QString)));
    // draw behind and front route
    if(submited == true)
    {
        alternativeFP = &message2FP(receivedMessage);
        int WPChange_idx = checkAlternativeFP(*alternativeFP);
        drawAlternativeRoute(WPChange_idx);

        if(nxtWP_idx > WPChange_idx)
        {
            reject();
        }
        else if(WPChange_idx != 0)
        {
            // alternative FP has been found
            drawAlternativeRoute(WPChange_idx);
            ui->acceptButton->setEnabled(true);
            ui->rejectButton->setEnabled(true);
            ui->submitAltFPButton->setEnabled(false);
        }
        else
        {
            // alternative FP has NOT been found
            drawRoute();
            ui->acceptButton->setEnabled(false);
            ui->rejectButton->setEnabled(false);
        }
    }
    else
    {
        drawRoute();
    }
}

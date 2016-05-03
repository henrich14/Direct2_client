#ifndef FLIGHTPLAN_H
#define FLIGHTPLAN_H

#include <QObject>
#include "waypoint.h"

class FlightPlan// : public QObject
{
    //Q_OBJECT
public:

    FlightPlan();

    FlightPlan(uint timestamp, Waypoint WP0, Waypoint WP1, Waypoint WP2, Waypoint WP3, Waypoint WP4, Waypoint WP5, Waypoint WP6,
               Waypoint WP7, Waypoint WP8, Waypoint WP9, Waypoint WP10, Waypoint WP11, QObject *parent = 0);

    Waypoint WP(const int &idx);
    uint timeStamp();

    bool operator==(FlightPlan *FP)
    {
        return ((m_WP0 == FP->WP(0)) && (m_WP1 == FP->WP(1)) && (m_WP2 == FP->WP(2)) &&
                (m_WP3 == FP->WP(3)) && (m_WP4 == FP->WP(4)) && (m_WP5 == FP->WP(5)) &&
                (m_WP6 == FP->WP(6)) && (m_WP7 == FP->WP(7)) && (m_WP8 == FP->WP(8)) &&
                (m_WP9 == FP->WP(9)) && (m_WP10 == FP->WP(10)) && (m_WP11 == FP->WP(11)));
    }

    FlightPlan& operator=(const FlightPlan &FP)
    {
        m_timestamp = FP.m_timestamp;
        m_WP0 = FP.m_WP0;
        m_WP1 = FP.m_WP1;
        m_WP2 = FP.m_WP2;
        m_WP3 = FP.m_WP3;
        m_WP4 = FP.m_WP4;
        m_WP5 = FP.m_WP5;
        m_WP6 = FP.m_WP6;
        m_WP7 = FP.m_WP7;
        m_WP8 = FP.m_WP8;
        m_WP9 = FP.m_WP9;
        m_WP10 = FP.m_WP10;
        m_WP10 = FP.m_WP11;

        return *this;
    }

private:
    uint m_timestamp;
    Waypoint m_WP0;
    Waypoint m_WP1;
    Waypoint m_WP2;
    Waypoint m_WP3;
    Waypoint m_WP4;
    Waypoint m_WP5;
    Waypoint m_WP6;
    Waypoint m_WP7;
    Waypoint m_WP8;
    Waypoint m_WP9;
    Waypoint m_WP10;
    Waypoint m_WP11;
};

#endif // FLIGHTPLAN_H

#include "flightplan.h"

FlightPlan::FlightPlan()
{

}

FlightPlan::FlightPlan(uint timestamp, Waypoint WP0, Waypoint WP1, Waypoint WP2, Waypoint WP3, Waypoint WP4, Waypoint WP5, Waypoint WP6,
                       Waypoint WP7, Waypoint WP8, Waypoint WP9, Waypoint WP10, Waypoint WP11, QObject *parent)// : QObject(parent)
{
    m_timestamp = timestamp;

    m_WP0 = WP0;
    m_WP1 = WP1;
    m_WP2 = WP2;
    m_WP3 = WP3;
    m_WP4 = WP4;
    m_WP5 = WP5;
    m_WP6 = WP6;
    m_WP7 = WP7;
    m_WP8 = WP8;
    m_WP9 = WP9;
    m_WP10 = WP10;
    m_WP11 = WP11;
}

Waypoint FlightPlan::WP(const int &idx)
{
    if(idx == 0) return m_WP0;
    else if(idx == 1) return m_WP1;
    else if(idx == 2) return m_WP2;
    else if(idx == 3) return m_WP3;
    else if(idx == 4) return m_WP4;
    else if(idx == 5) return m_WP5;
    else if(idx == 6) return m_WP6;
    else if(idx == 7) return m_WP7;
    else if(idx == 8) return m_WP8;
    else if(idx == 9) return m_WP9;
    else if(idx == 10) return m_WP10;
    else if(idx == 11) return m_WP11;
    else return m_WP0;
}

uint FlightPlan::timeStamp()
{
    return m_timestamp;
}

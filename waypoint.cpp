#include "waypoint.h"

Waypoint::Waypoint(double x, double y, double altitude, QString label, bool mandatory, QObject *parent)
{
    m_x = x;
    m_y = y;
    m_altitude = altitude;
    m_label = label;
    m_mandatory = mandatory;
}

const double Waypoint::x() const
{
    return m_x;
}

const double Waypoint::y() const
{
    return m_y;
}

const double Waypoint::altitude() const
{
    return m_altitude;
}

const QString Waypoint::label() const
{
    return m_label;
}

const bool Waypoint::mandatory() const
{
    return m_mandatory;
}

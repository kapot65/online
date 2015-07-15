#ifndef EVENT_H
#define EVENT_H

#include <QDataStream>

class Event
{
public:
    Event(){}
    Event(unsigned short data, long time, bool valid);
    ~Event();
    unsigned short data;
    int time;
    bool valid;
    friend inline QDataStream &operator<<(QDataStream &out, const Event &e);
    friend inline QDataStream &operator>>(QDataStream &in, Event &e);
};
inline QDataStream &operator<<(QDataStream &out, const Event &e)
{
    out << e.data << e.time << e.valid;
    return out;
}
inline QDataStream &operator>>(QDataStream &in, Event &e)
{
    in >> e.data >> e.time >> e.valid;
    return in;
}

#endif // EVENT_H

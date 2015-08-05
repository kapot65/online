#ifndef EVENT_H
#define EVENT_H

#include <QDataStream>

/*!
 * \brief Класс события.
 */
class Event
{
public:
    Event(){}
    Event(unsigned short data, long time, bool valid);
    ~Event();
    ///\brief Канал.
    unsigned short data;
    ///\brief Время события.
    int time;
    ///\brief Валидность события.
    bool valid;
    ///\brief Оператор для сериализации события.
    friend inline QDataStream &operator<<(QDataStream &out, const Event &e);
    ///\brief Оператор десериализации события.
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

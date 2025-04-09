#include "event.h"

Event::Event(unsigned short data, long time, bool valid)
{
    this->data = data;
    this->time = time;
    this->valid = valid;
}

Event::~Event()
{

}


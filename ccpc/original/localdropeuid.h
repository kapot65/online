#ifndef LOCALDROPEUID_H
#define LOCALDROPEUID_H

#include <sys/types.h>

class LocalDropEUID
{
public:
    LocalDropEUID();
    ~LocalDropEUID();
private:
    uid_t euid;
};

#endif // LOCALDROPEUID_H

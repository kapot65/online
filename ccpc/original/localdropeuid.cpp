#include "localdropeuid.h"
#include <sys/types.h>
#include <unistd.h>


LocalDropEUID::LocalDropEUID()
{
  euid = geteuid();
  seteuid(getuid());
}

LocalDropEUID::~LocalDropEUID()
{
  seteuid(euid);
}

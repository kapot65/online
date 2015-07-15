
#include "ccpc6.h"
#include <stdio.h>
#include <string.h>

using namespace ccpc;

CamacImplCCPC6::CamacImplCCPC6()
{
  int rc = getBaseAddr(&AMCC_BASE, &ADL_BASE);
  if (rc == -1) throw excCamacNotFound();
}

int CamacImplCCPC6::getBaseAddr(int *o1, int *o2)
{
  const char *PROC_IOPORTS = "/proc/ioports";
  FILE *f;
  int b1, b2, r, n=0;
  char str[1024];

  b1=-1, b2=-1;
  f = fopen(PROC_IOPORTS, "r");
  if (!f) return(-1);
  while(1) {
    int a1, a2;
    char name[1024];
    if (feof(f)) break;
    str[0]=0;
    if (!fgets(str, 1023, f)) break;
    if (!str[0]) break;
    r = sscanf(str, "%04x-%04X%*[ :]%[^\n]", &a1, &a2, name);
//    printf("r=%d, %04X, %04X, %s\n", r, a1, a2, name);
    if (r!=3) {
      printf("Parse error: '%s'\n", str);
      continue;
    }
    if (strstr(name, "10e8:80d8") || strstr(name, "10E8:80D8")) {
      if ((a2-a1) == 0x3F) {
        if (n==0) b1 = a1;
        if (n==1) b2 = a1;
        n++;
      }
    }
  }
  fclose(f);
  if (n==2) {
    if (o1) *o1=b1;
    if (o2) *o2=b2;
    return(0);
  }
  return(-1);
}

void CamacImplCCPC6::exec(CamacOp &op)
{
  // TODO
}

bool CamacImplCCPC6::inf1() const
{
  return false; // TODO
}

bool CamacImplCCPC6::inf2() const
{
  return false; // TODO
}

bool CamacImplCCPC6::lam(int n) const
{
  return false; // TODO
}

void CamacImplCCPC6::readStatus()
{
  // TODO
}

void CamacImplCCPC6::setOut(bool out1, bool out2)
{
  // TODO
}

void CamacImplCCPC6::reset()
{
  // TODO
}

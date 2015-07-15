#ifndef CCPC6CAM_H
#define CCPC6CAM_H

#include "camacinterface.h"
#include "camacop.h"

class Ccpc6Cam: public ccpc::CamacInterface
{
public:
  Ccpc6Cam();
  ~Ccpc6Cam();
  int INITCAM(void);  // Camac controller initialization
  int FNA(int F, int N, int A);
  int FNAW16(int F, int N, int A, int D);
  long FNAR(int F, int N, int A);
  long CAMACSTATUS();
  int X();
  int Q();
private:
  ccpc::CamacInterface *camac;
  ccpc::CamacOp op;
};

#endif // CCPC6CAM_H

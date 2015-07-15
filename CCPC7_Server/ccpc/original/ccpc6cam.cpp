
#include "ccpc6cam.h"
#include "camacinterface.h"
#include "camacop.h"

using namespace ccpc;

Ccpc6Cam::Ccpc6Cam()
{
  camac = new CamacInterface();
}

Ccpc6Cam::~Ccpc6Cam()
{
  delete camac;
}

int Ccpc6Cam::INITCAM(void)  // Camac controller initialization
{
  return(1);
}

int Ccpc6Cam::FNA(int F, int N, int A)
{
op.n = N;
op.a = A;
op.f = F;
camac->exec(op);
return 1;
}

int Ccpc6Cam::FNAW16(int F, int N, int A, int D)
{
op.n = N;
op.a = A;
op.f = F;
op.dir = DW16;
op.data.resize(1);
op.data[0] = D;
camac->exec(op);
return 1;
}

long Ccpc6Cam::FNAR(int F, int N, int A)
{
op.n = N;
op.a = A;
op.f = F;
op.dir = DR16;
camac->exec(op);
if (op.data.size() == 0) return -1;
return op.data[0];
}

long Ccpc6Cam::CAMACSTATUS()
{
return 0;


}
  
int Ccpc6Cam::X()
{
  return op.x;
}


int Ccpc6Cam::Q()
{
  return op.q;
}

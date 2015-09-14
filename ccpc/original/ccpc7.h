#ifndef CCPC7_H
#define CCPC7_H

#include "ccpcisa.h"

namespace ccpc {

class CamacImplCCPC7: public CamacImplISA {
  public:
    CamacImplCCPC7();
	virtual bool lam(int n) const;
  private:
	long readId();
};

}

#endif  /* CCPC7_H */

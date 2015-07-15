#ifndef CCPC6_H
#define CCPC6_H

#include "camacimpl.h"

namespace ccpc {

class CamacImplCCPC6: public CamacImpl {
  public:
	CamacImplCCPC6();
	virtual void reset();
	virtual void exec(CamacOp &op);
	virtual bool inf1() const;
	virtual bool inf2() const;
	virtual bool lam(int n) const;
	virtual void readStatus();
	virtual void setOut(bool out1, bool out2);
  private:
	int getBaseAddr(int *o1, int *o2);

	int ADL_BASE;
	int AMCC_BASE;
};

}

#endif  /* CCPC6_H */

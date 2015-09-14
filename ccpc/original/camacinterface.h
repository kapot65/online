#ifndef CAMACINTERFACE_H
#define CAMACINTERFACE_H

#include "camacop.h"

namespace ccpc {

class CamacImpl;

class CamacInterface {
  public:
	CamacInterface();
	~CamacInterface();
	void reset();
	void exec(CamacSequence &seq);
	void exec(CamacOp &op);
	bool inf1() const;
	bool inf2() const;
	bool lam(int n) const;
	void readStatus();
	void setOut(bool out1, bool out2);
  private:
	CamacImpl *impl;
};

}

#endif  /* CAMACINTERFACE_H */

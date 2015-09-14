#ifndef CAMACIMPL_H
#define CAMACIMPL_H

#include <exception>
#include "camacop.h"

namespace ccpc {

class excCamacNotFound: public std::exception { };

class CamacImpl {
  public:
	CamacImpl();
	virtual ~CamacImpl();
	virtual void reset() = 0;
	virtual void exec(CamacOp &op) = 0;
	virtual bool inf1() const = 0;
	virtual bool inf2() const = 0;
	virtual bool lam(int n) const = 0;
	virtual void readStatus() = 0;
	virtual void setOut(bool out1, bool out2) = 0;
  protected:
	int rData;
	int wData;
	int rData3;
	int wData3;
	int rCtrlStatus;
	int wCtrlStatus;
	int rNaf;
	int wNaf;
};

}

#endif  /* CAMACIMPL_H */

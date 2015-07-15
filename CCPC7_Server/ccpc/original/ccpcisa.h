#ifndef CCPCISA_H
#define CCPCISA_H

#include "camacimpl.h"

namespace ccpc {

// I/O ports
const int CCIO_BASE  = 0x360;
const int CCIO_SIZE  = 0x008;
const int CCIO_DATA  = CCIO_BASE;
const int CCIO_DATA3 = CCIO_BASE+0x02;
const int CCIO_CTRL  = CCIO_BASE+0x04;
const int CCIO_NAF   = CCIO_BASE+0x06;


class CamacImplISA: public CamacImpl {
  public:
	CamacImplISA();
	virtual void reset();
	virtual void exec(CamacOp &op);
	virtual bool inf1() const;
	virtual bool inf2() const;
	virtual bool lam(int n) const;
	virtual void readStatus();
	virtual void setOut(bool out1, bool out2);
  protected:
	int readPort(int port);
	void writePort(int port, int data);
};

}

#endif /* CCPCISA_H */

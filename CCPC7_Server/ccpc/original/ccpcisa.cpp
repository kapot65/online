
#include "ccpcisa.h"

#include <exception>
#include <iostream>

#include <string.h>
#include <errno.h>
#include <sys/io.h>

using namespace ccpc;
using namespace std;

CamacImplISA::CamacImplISA()
{
  if (ioperm(CCIO_BASE, CCIO_SIZE, 1) == -1) {
	cerr << "Can't access I/O ports: " << strerror(errno) << endl;
        throw excCamacNotFound();
  }
}

int NAF2DW(int N, int A, int F)
{
  return N*0x200 + A*0x20 + F;
}

void CamacImplISA::exec(CamacOp &op)
{
  const bool write = op.isWrite();
  const bool read = op.isRead();

  if (op.mode == Zcycle) {
	writePort(CCIO_NAF, 1<<14);
  }
  if (op.mode == Ccycle) {
	writePort(CCIO_NAF, 1<<15);
  }
  if (op.mode == Single) {
	if (write) {
	  long d = (op.data.size()>0)?op.data[0]:0;
	  if (op.dir == DW24) writePort(CCIO_DATA3, d >> 16);
	  if (op.dir == DW24 || op.dir == DW16) writePort(CCIO_DATA, d & 0xFFFF);
	}
	long naf = NAF2DW(op.n, op.a, op.f);
	writePort(CCIO_NAF, naf);
	long status = readPort(CCIO_CTRL);
	op.q = status & 0x1;
	op.x = status & 0x2;
	long d = 0;
	if (read) {
	  if (op.dir == DR24) d |= ((readPort(CCIO_DATA3) & 0xFF) << 16);
	  if (op.dir == DR24 || op.dir == DR16) d |= (readPort(CCIO_DATA) & 0xFFFF);
	  op.data.resize(1); op.data[0] = d;
	}
  }
}

int CamacImplISA::readPort(int port)
{
  int d = 0;
  d = inw(port);
//  cerr << "read port 0x" << hex << port << ": " << d << endl;
  if (port == CCIO_DATA) rData = d;
  if (port == CCIO_DATA3) rData3 = d;
  if (port == CCIO_CTRL) rCtrlStatus = d;
  if (port == CCIO_NAF) rNaf = d;
  return d;
}

void CamacImplISA::writePort(int port, int d)
{
  if (port == CCIO_DATA) wData = d;
  if (port == CCIO_DATA3) wData3 = d;
  if (port == CCIO_CTRL) wCtrlStatus = d;
  if (port == CCIO_NAF) wNaf = d;
//  cerr << "write port 0x" << hex << port << ": " << d << endl;
  outw(d, port);
}

bool CamacImplISA::inf1() const
{
  return rData3 & (1<<15);
}

bool CamacImplISA::inf2() const
{
  return rData3 & (1<<14);
}

bool CamacImplISA::lam(int n) const
{
  if (n<1 || n>22) return false;
  if (n>=1 && n <=16) return rNaf& (1<<(n-1));
  return rData3 & (1<<(n-17+8));
}

void CamacImplISA::readStatus()
{
  readPort(CCIO_DATA3);
  readPort(CCIO_CTRL);
  readPort(CCIO_NAF);
}

void CamacImplISA::setOut(bool out1, bool out2)
{
  wCtrlStatus &= ~0x6;
  wCtrlStatus |= (out1?2:0);
  wCtrlStatus |= (out2?4:0);
  writePort(CCIO_CTRL, wCtrlStatus);
}

void CamacImplISA::reset()
{
  writePort(CCIO_DATA, wData);
  writePort(CCIO_DATA3, wData3);
  writePort(CCIO_CTRL, wCtrlStatus);
}

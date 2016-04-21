
#include "camacinterface.h"
#include "camacimpl.h"
#include "ccpc4.h"
#include "ccpc6.h"
#include "ccpc7.h"
#include <iostream>

using namespace std;
using namespace ccpc;

CamacInterface::CamacInterface()
{
  impl = 0;
  try {
    impl = new CamacImplCCPC6();
  }
  catch (excCamacNotFound &e) {
	impl = 0;
  }
  if (impl) {
    cerr << "CCPC6 Controller detected" << endl;
	return;
  }

  try {
    impl = new CamacImplCCPC7();
  }
  catch (excCamacNotFound &e) {
	impl = 0;
  }
  if (impl) {
    cerr << "CCPC7 Controller detected" << endl;
	return;
  }

  try {
    impl = new CamacImplCCPC4();
  }
  catch (excCamacNotFound &e) {
	impl = 0;
  }
  if (impl) {
    cerr << "CCPC4/5 Controller detected" << endl;
	return;
  }

  cerr << "CAMAC Controller not found" << endl;
  throw excCamacNotFound();
}

CamacInterface::~CamacInterface()
{
  delete impl;
}

void CamacInterface::exec(CamacSequence &seq)
{
  for(CamacSequence::iterator it = seq.begin(); it != seq.end(); ++it) {
	impl->exec(*it);
  }
}

void CamacInterface::exec(CamacOp &op)
{
  impl->exec(op);
}

bool CamacInterface::inf1() const
{
  return impl->inf1();
}

bool CamacInterface::inf2() const
{
  return impl->inf2();
}

bool CamacInterface::lam(int n) const
{
  return impl->lam(n);
}

void CamacInterface::readStatus()
{
  impl->readStatus();
}

void CamacInterface::setOut(bool out1, bool out2)
{
  impl->setOut(out1, out2);
}

void CamacInterface::reset()
{
  impl->reset();
}

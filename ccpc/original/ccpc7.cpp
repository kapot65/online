
#include "ccpc7.h"
#include "ccpcisa.h"
#include <iostream>

using namespace ccpc;
using namespace std;

const int VENDOR_ID = 0xA6;
const int DEVICE_ID_CCPC7 = 0x07;
const int ID_BIT_MASK = 1<<12;

CamacImplCCPC7::CamacImplCCPC7()
 : CamacImplISA()
{
  long id = readId();
  long vendor = (id>>8) & 0xFF;
  long device = (id) & 0xFF;
  if (vendor != VENDOR_ID) {
	throw excCamacNotFound();
  }
  if (device != DEVICE_ID_CCPC7) {
	throw excCamacNotFound();
  }
}

long CamacImplCCPC7::readId()
{
  long id = 0;
  for (int i=0; i<16; i++) {
	int tmp = readPort(CCIO_CTRL);
//    cout << "ID: " << hex << tmp << endl;
	id |= (tmp & ID_BIT_MASK)?(1<<(15-i)):0;
  }
  id &= 0xFFFF;
//  cout << "ID: " << hex << id << endl;
// sync bit sequence
  for (int i=0; i<16; i++) {
	long test = (id << 16) | id;
	test >>= i;
	test &= 0xFFFF;
    long vendor = (test>>8) & 0xFF;
	if (vendor == VENDOR_ID) {
	  id = test;
	  break;
	}
  }
  cout << "ID: " << hex << id << endl;
  return id;
}

bool CamacImplCCPC7::lam(int n) const
{
  if (n==23) return rCtrlStatus&(1<<3);
  return CamacImplISA::lam(n);
}

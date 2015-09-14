#ifndef CAMACOP_H
#define CAMACOP_H

#include <vector>

namespace ccpc {

typedef enum { DN, DR0, DR16, DR24, DW0, DW16, DW24 } CamacDataDir;
typedef enum { Zcycle, Ccycle, Single, QStop, QRepeat, Auto } CamacMode;

class CamacOp {
  public:
	CamacOp() { dir = DN; mode = Single; n=0; a=0; f=0; x=0; q=0; }
	bool isWrite() { return dir == DW0 || dir == DW16 || dir == DW24; }
	bool isRead()  { return dir == DR0 || dir == DR16 || dir == DR24; }

	int n;
	int a;
	int f;
	std::vector<long> data;
	CamacDataDir dir;
	CamacMode mode;
	bool x, q;
};

class CamacSequence: public std::vector<CamacOp> {
};

}

#endif /* CAMACOP_H */

#ifndef ADC8CAM_H
#define ADC8CAM_H

class Ccpc6Cam;

class Adc8Cam
{
public:
  Adc8Cam();
  ~Adc8Cam();
  void SetThreshold(int chanel, int data, int N);
  void SetMainStep(int chanel, int data, int N);
  void SetL1Step(int chanel, int data, int N);
  void SetL2Step(int chanel, int data, int N);
  void SetL1Offset(int chanel, int data, int N);
  void SetL2Offset(int chanel, int data, int N);
  void SetKAmpl(int chanel, int data, int N);
  void SetMaxHistSum(int chanel, int data, int N);
  void SetScopeMode(int N);
  void SetHistMode(int N);
  void StartRun(int mask, int N);
  void StartRead(int chanel, int N);
  void StartReadHist(int chanel, int source, int N);  //source 0-M 1-L1 2-L2
  int ReadReg(int reg, int N);
  int WriteReg(int reg, int data, int N);
  int MemZero(int N);
  void SetEdge(int channel, bool ne, int N);
  void SetTAMode(int channel, bool ne, int N);
  void SetScopeLen(int data, int N);

  Ccpc6Cam *camac;
};

#endif // ADC8CAM_H

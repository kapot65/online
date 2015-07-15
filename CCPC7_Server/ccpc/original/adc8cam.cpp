
#include "adc8cam.h"
#include "ccpc6cam.h"

Adc8Cam::Adc8Cam()
{
  camac = new Ccpc6Cam();
}
Adc8Cam::~Adc8Cam()
{
  delete camac;
}

void Adc8Cam::SetThreshold(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,8+chanel); camac->FNAW16(16,N,0,data); }
void Adc8Cam::SetMainStep(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,16+chanel); camac->FNAW16(16,N,0,data); }
void Adc8Cam::SetL1Step(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,24+chanel); camac->FNAW16(16,N,0,data); }
void Adc8Cam::SetL2Step(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,32+chanel); camac->FNAW16(16,N,0,data); }
void Adc8Cam::SetL1Offset(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,40+chanel); camac->FNAW16(16,N,0,data); }
void Adc8Cam::SetL2Offset(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,48+chanel); camac->FNAW16(16,N,0,data); }
void Adc8Cam::SetKAmpl(int chanel, int data, int N)
{ camac->FNAW16(16,N,1,56+chanel); camac->FNAW16(16,N,0,data); }

void Adc8Cam::SetMaxHistSum(int chanel, int data, int N)
{
 int sendval; 
 sendval=data&0xFFFF;
 camac->FNAW16(16,N,1,72+chanel); camac->FNAW16(16,N,0,sendval);
 sendval=data>>16;
 camac->FNAW16(16,N,1,64+chanel); camac->FNAW16(16,N,0,sendval); 
}
void Adc8Cam::SetScopeMode(int N)
{ camac->FNAW16(16,N,1,0); camac->FNAW16(16,N,0,0x1); }
void Adc8Cam::SetHistMode(int N)
{ camac->FNAW16(16,N,1,0); camac->FNAW16(16,N,0,0); }
void Adc8Cam::StartRun(int mask, int N)
{ camac->FNAW16(16,N,1,3); camac->FNAW16(16,N,0,mask);}
void Adc8Cam::StartRead(int chanel, int N)
{ camac->FNAW16(16,N,1,4); camac->FNAW16(16,N,0,0x1<<chanel); }
void Adc8Cam::StartReadHist(int chanel, int source, int N)  //source 0-M 1-L1 2-L2
   {
     int mask = 0;
     camac->FNAW16(16,N,1,4);
     mask=0x1<<chanel; 
     if(source == 0) mask = mask | 0x100;
     if(source == 1) mask = mask | 0x200;
     if(source == 2) mask = mask | 0x300;
     camac->FNAW16(16,N,0,mask);
   }
int Adc8Cam::ReadReg(int reg, int N)
{ camac->FNAW16(16,N,1,reg); camac->FNA(26,N,0); return camac->FNAR(0,N,0); }
int Adc8Cam::WriteReg(int reg, int data, int N)
{
camac->FNAW16(16,N,1,reg); camac->FNAW16(16,N,0,data);
if (data!=ReadReg(reg, N)) return -1;
return 1;
}
/*
bool MemZero(int N)
  {
    int i;
    camac->FNAW16(16,N,1,1);
    camac->FNAW16(16,N,0,0x1);
    for(i=0;i<100;i++)
      if (ReadReg(1,N) == 0) return true;
    return false;  
  }
*/  
int Adc8Cam::MemZero(int N)
  {
    int i;
    camac->FNAW16(16,N,1,1);
    camac->FNAW16(16,N,0,0x1);
    for(i=0;i<100;i++)
      if (ReadReg(1,N) == 0) return (i);
    return (-1);  
  }
void Adc8Cam::SetEdge(int channel, bool ne, int N)
{
int save;
save=ReadReg(7,N);
if(channel==0)
  if(ne) save=save | 0x1; else save=save&0xFFFE;
if(channel==1)
  if(ne) save=save | 0x2; else save=save&0xFFFD;
if(channel==2)
  if(ne) save=save | 0x4; else save=save&0xFFFB;
if(channel==3)
  if(ne) save=save | 0x8; else save=save&0xFFF7;
if(channel==4)
  if(ne) save=save | 0x10; else save=save&0xFFEF;
if(channel==5)
  if(ne) save=save | 0x20; else save=save&0xFFDF;
if(channel==6)
  if(ne) save=save | 0x40; else save=save&0xFFBF;
if(channel==7)
  if(ne) save=save | 0x80; else save=save&0xFF7F;
WriteReg(7,save,N);
}

void Adc8Cam::SetTAMode(int channel, bool ne, int N)
{
int save;
save=ReadReg(7,N);
if(channel==0)
  if(ne) save=save | 0x100; else save=save&0xFEFF;
if(channel==1)
  if(ne) save=save | 0x200; else save=save&0xFDFF;
if(channel==2)
  if(ne) save=save | 0x400; else save=save&0xFBFF;
if(channel==3)
  if(ne) save=save | 0x800; else save=save&0xF7FF;
if(channel==4)
  if(ne) save=save | 0x1000; else save=save&0xEFFF;
if(channel==5)
  if(ne) save=save | 0x2000; else save=save&0xDFFF;
if(channel==6)
  if(ne) save=save | 0x4000; else save=save&0xBFFF;
if(channel==7)
  if(ne) save=save | 0x8000; else save=save&0x7FFF;
WriteReg(7,save,N);
}

void Adc8Cam::SetScopeLen(int data, int N)
{ camac->FNAW16(16,N,1,80); camac->FNAW16(16,N,0,data); }

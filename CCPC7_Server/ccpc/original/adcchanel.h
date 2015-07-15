#include <qapplication.h>
#include <qtextstream.h>
#include <stdlib.h>
#include <time.h>
#include <qcolor.h> 
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>


typedef struct
{
    QString name;
    bool enabled;
    bool edge;             //false - falling , true -rising
    bool ampmode;       //false - time mode , true - amp mode
    int threshold;
    int mstep;
    int l1step;
    int l2step;
    int l1offset;
    int l2offset;  
    int kamp;
    int bloffset;
    int TrigCount;
    int maxsum;
    QwtPlotCurve curve;    
    QwtPlotMarker mark;    
//    long curve;    
//    long mark;    
    double adcd[14000];  
    double shist[5000];    
    double myampx[5000];    
    double coincidence[130];
    int LastValid;
    QColor color;
    
} adcchanel;

typedef struct
{
    adcchanel chanel[9];
    QString name;
    QString SavePath;
    QString StationSavePath;
    bool running;
    bool curactive;
    bool histmode;
    bool scopeauto;
    bool needhistrestart;
    bool PSpillEnabled;
    time_t timestart,timestop;
    int CFMask;
    int NPresteps;
    int ScopeLength;
    int TrigMask;
    int BlockTime1;
    int BlockTime2;
} adcstation;

typedef struct
{
    adcstation adc[5];
    int camac_positions[5];
    int nocs;   //number of camac stations
    bool fdopened; //file dialog opened    
    QwtPlotCurve curvehist;
    QString fdpath;     //file dialog path
    double histd[5000];
    double myx[15000];
} adcsystem;

#ifndef PHASE_APPROX_CALC_H
#define PHASE_APPROX_CALC_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
typedef vector<double> row;
typedef vector<row> matrix;


struct SystemParams
{
    double         tStep;
    double         tMax;
    unsigned int   tCount;

    double         xStep;
    unsigned int   xCount;
    double         xMax;

    double         yStep;
    unsigned int   yCount;
    double         yMax;

    double         wallWidth;

    double         lm;//lambda metal
    double         cm;//capacity metal
    double         km;//conductivity metal

    double         tempEnv;
    double         kEnv;//conductivity enviroment
    double         delta;
    double         cEnv;//capacity enviroment

    double         heaterW;//heater width
    double         heaterH;//heater height
    double         heaterP;//heater power
    double         lh;//lambda heater
    double         ch;//capacity heater
    double         kh;//conductivity heater

    double         ll;//lambda liquid
    double         cl;//capacity liquid
    double         kl;//conductivity liquid

    double         eps;
};

class HeatCalcThread : public QThread
{
    Q_OBJECT
public:
    HeatCalcThread(vector<matrix> *heat);
    void run();
    void StopAnimation();
    void StartAnimation();
    void Calc(SystemParams &p);

    matrix k;
    matrix c;
    matrix q;
    matrix tempLast;
    matrix tempNext;
    SystemParams p;
    vector<vector<bool>> xBorders;
    vector<vector<bool>> yBorders;

signals:
    void valuesReady(QWaitCondition *cond);
    void calcFinished();

private:
    void CalcAsync();
    double A(unsigned int ir, unsigned int jz, bool napr_r);
    double B(unsigned int ir, unsigned int jz, bool napr_r);
    double C(unsigned int ir, unsigned int jz, bool napr_r);
    double D(unsigned int it, unsigned int ir, unsigned int jz, bool napr_r);
    double D2Temp(unsigned int it, unsigned int ir, unsigned int jz,bool napr_r);
    bool isAnimated = false;
    bool isCalculation = false;
    vector<matrix> *heat;

    QMutex mutex;
    QWaitCondition cond;
};

#endif // PHASE_APPROX_CALC_H

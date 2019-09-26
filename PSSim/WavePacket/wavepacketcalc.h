#ifndef PHASE_APPROX_CALC_H
#define PHASE_APPROX_CALC_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <vector>
#include <complex>
#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

typedef complex<double> cd;
typedef vector<cd> vcd;

struct SystemParams
{
    double tStep;
    double tMax;
    unsigned int tCount;

    double xStep;
    unsigned int xCount;
    double r;

    double integrationStep;

    double sigma;
    double a;
};

class WaveCalcThread : public QThread
{
    Q_OBJECT
public:
    WaveCalcThread(vector<vector<cd>> *waveSamples);
    void run();
    static vcd fft(const vcd &as);
    void stopAnimation();
    void CalcWave(SystemParams &p);

signals:
    void valuesReady(QWaitCondition *cond);
    void calcFinished();

private:
    float calcInputValue(int signalLength, int num, float *lambdas, float *filter);
    bool isAnimated = false;
    vector<vector<cd>> *waveSamples;
    SystemParams params;
    QMutex mutex;
    QWaitCondition cond;
};

#endif // PHASE_APPROX_CALC_H

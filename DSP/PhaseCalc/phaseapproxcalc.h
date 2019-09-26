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

class PhaseApproxCalc : public QThread
{
    Q_OBJECT
public:
    PhaseApproxCalc(int signalLength, float *inputSignalAmp, float *inputSignal, float *calcInputSignal);
    void run();
    static vcd fft(const vcd &as);

signals:
    void valuesReady(QWaitCondition *cond);
    void calcFinished();

private:
    float calcInputValue(int signalLength, int num, float *lambdas, float *filter);
    float function();
    void PhaseApprox();
    int   signalLength;
    float *calcInputSignal;
    float *inputSignalAmp;
    float *inputSignal;
    QMutex mutex;
    QWaitCondition cond;
};

#endif // PHASE_APPROX_CALC_H

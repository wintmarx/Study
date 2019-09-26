#ifndef MHJCALC_H
#define MHJCALC_H

#include <QObject>
#include <QThread>

class MHJCalc : public QThread
{
    Q_OBJECT
public:
    MHJCalc(int signalLength, float *outputSignal, float *filter, float *calcInputSignal);
    void run();
    static float cyclicConvolutionSample(int samplesCount, int sampleNum, float *left, float *right);

signals:
    void valuesReady(float F);
    void calcFinished(float F);

private:
    float calcInputValue(int signalLength, int num, float *lambdas, float *filter);
    float function(int signalLength, float *outputSignal, float *lambdas, float *filter);
    float MHJ(int signalLength, float *outputSignal, float *filter, float *calcInputSignal);
    int   signalLength;
    float *outputSignal;
    float *lambdas;
    float *filter;
    float *calcInputSignal;
    float *inputSignal;
};

#endif // MHJCALC_H

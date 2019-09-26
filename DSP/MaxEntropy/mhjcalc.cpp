#include "mhjcalc.h"

MHJCalc::MHJCalc(int signalLength, float *outputSignal, float *filter, float *calcInputSignal)
{
    this->signalLength = signalLength;
    this->outputSignal = outputSignal;
    this->filter = filter;
    this->calcInputSignal = calcInputSignal;
}

float MHJCalc::cyclicConvolutionSample(int samplesCount, int sampleNum, float *left, float *right)
{
    float convolutionSample = 0;
    for(int j = 0; j < samplesCount; j++)
    {
        convolutionSample += left[j] * right[sampleNum - j + (sampleNum < j ? samplesCount : 0)];
    }
    return convolutionSample;
}

float MHJCalc::calcInputValue(int signalLength, int num, float *lambdas, float *filter)
{
    return exp(-1 - cyclicConvolutionSample(signalLength, num, lambdas, filter));
}

float MHJCalc::function(int signalLength, float *outputSignal, float *lambdas, float *filter)
{
    float result = 0;
    for(int i = 0; i < signalLength; i++)
    {
        inputSignal[i] = calcInputValue(signalLength, i, lambdas, filter);
    }
    for(int i = 0; i < signalLength; i++)
    {
        result += pow(outputSignal[i] - cyclicConvolutionSample(signalLength, i, inputSignal, filter), 2);
    }
    return result;
}

float MHJCalc::MHJ(int signalLength, float *outputSignal, float *filter, float *calcInputSignal)
{
    // kk - количество параметров; x - массив параметров
    float  TAU=1.e-6f; // Точность вычислений
    int i, j, bs, ps;
    float z, h, k, fi, fb;
    float *b = new float[signalLength];
    float *y = new float[signalLength];
    float *p = new float[signalLength];
    float *lambdas = new float[signalLength];
    inputSignal = new float[signalLength];

    h=1.;
    lambdas[0]=1.;
    for( i=1; i < signalLength; i++)
    {
        lambdas[i]=(float)rand()/RAND_MAX; // Задается начальное приближение
    }

    k = h;
    for( i=0; i < signalLength; i++)
    {
        y[i] = p[i] = b[i] = lambdas[i];
    }

    fi = function(signalLength, outputSignal, lambdas, filter);
    ps = 0;   bs = 1;  fb = fi;

    j = 0;
    while(1)
    {
        lambdas[j] = y[j] + k;
        z = function(signalLength, outputSignal, lambdas, filter);
        if (z >= fi)
        {
            lambdas[j] = y[j] - k;
            z = function(signalLength, outputSignal, lambdas, filter);
            if( z<fi )
            {
                y[j] = lambdas[j];
            }
            else
            {
                lambdas[j] = y[j];
            }
        }
        else
        {
            y[j] = lambdas[j];
        }
        fi = function(signalLength, outputSignal, lambdas, filter);

        if ( j < signalLength-1 )
        {
            j++;
            continue;
        }

        for(int it = 0; it < signalLength; it++)
        {
            calcInputSignal[it] = exp(-1 - cyclicConvolutionSample(signalLength, it, lambdas, filter));
        }
        emit valuesReady(fb);

        if ( fi + 1e-8 >= fb )
        {
            if ( ps == 1 && bs == 0)
            {
                for( i=0; i < signalLength; i++)
                {
                    p[i] = y[i] = lambdas[i] = b[i];
                }
                z = function(signalLength, outputSignal, lambdas, filter);
                bs = 1;   ps = 0;   fi = z;   fb = z;   j = 0;
                continue;
            }
            k /= 10.;
            if ( k < TAU )
            {
                break;
            }
            j = 0;
            continue;
        }

        for(i = 0; i < signalLength; i++)
        {
            p[i] = 2 * y[i] - b[i];
            b[i] = y[i];
            lambdas[i] = p[i];

            y[i] = lambdas[i];
        }

        z = function(signalLength, outputSignal, lambdas, filter);
        fb = fi;   ps = 1;   bs = 0;   fi = z;   j = 0;
    } //  end of while(1)

    for( i=0; i<signalLength; i++)
    {
        lambdas[i] = p[i];
    }

    emit calcFinished(fb);

    delete b;
    delete y;
    delete p;
    delete lambdas;
    delete inputSignal;
    return fb;
}

void MHJCalc::run()
{
    MHJ(signalLength, outputSignal, filter, calcInputSignal);
}

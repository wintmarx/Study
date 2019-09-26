#include "phaseapproxcalc.h"

vcd PhaseApproxCalc::fft(const vcd &as)
{
  int n = as.size();
  int k = 0; // Длина n в битах
  while ((1 << k) < n) k++;
  vector<int> rev(n);
  rev[0] = 0;
  int high1 = -1;
  for (int i = 1; i < n; i++) {
    if ((i & (i - 1)) == 0) // Проверка на степень двойки. Если i ей является, то i-1 будет состоять из кучи единиц.
      high1++;
    rev[i] = rev[i ^ (1 << high1)]; // Переворачиваем остаток
    rev[i] |= (1 << (k - high1 - 1)); // Добавляем старший бит
  }

  vcd roots(n);
  for (int i = 0; i < n; i++) {
    double alpha = 2 * M_PI * i / n;
    roots[i] = cd(cos(alpha), sin(alpha));
  }

  vcd cur(n);
  for (int i = 0; i < n; i++)
    cur[i] = as[rev[i]];

  for (int len = 1; len < n; len <<= 1) {
    vcd ncur(n);
    int rstep = roots.size() / (len * 2);
    for (int pdest = 0; pdest < n;) {
      int p1 = pdest;
      for (int i = 0; i < len; i++) {
        cd val = roots[i * rstep] * cur[p1 + len];
        ncur[pdest] = cur[p1] + val;
        ncur[pdest + len] = cur[p1] - val;
        pdest++, p1++;
      }
      pdest += len;
    }
    cur.swap(ncur);
  }
  return cur;
}

PhaseApproxCalc::PhaseApproxCalc(int signalLength, float *inputSignalAmp, float *inputSignal, float *calcInputSignal)
{
    this->signalLength = signalLength;
    this->inputSignalAmp = inputSignalAmp;
    this->calcInputSignal = calcInputSignal;
    this->inputSignal = inputSignal;
}



void PhaseApproxCalc::PhaseApprox()
{
    float TAU = 1.e-2f; // Точность вычислений
    float *phases = new float[signalLength];
    vcd calcComplexSignal(signalLength);
    for(int i = 1; i < signalLength; i++)
    {
        phases[i] = (float)rand()/RAND_MAX * 2 * M_PI; //zero approx
    }

    while(1)
    {
        //gen approx signal's spectrum
        for(int i = 0; i < signalLength; i++)
        {
            calcComplexSignal[i].real(inputSignalAmp[i] * cos(phases[i]));
            calcComplexSignal[i].imag(inputSignalAmp[i] * sin(phases[i]));
        }
        //inverse fft
        calcComplexSignal = fft(calcComplexSignal);
        //use input signal's restrictions that imag part is 0 and real > 0      
        for(int i = 0; i < signalLength; i++)
        {
            if(calcComplexSignal[i].real() < 0)
            {
                calcComplexSignal[i].real(0);
            }
            calcComplexSignal[i].imag(0);
            calcInputSignal[i] = calcComplexSignal[i].real()/signalLength;
            calcComplexSignal[i].real(calcInputSignal[i]);
        }
        float tmp;
        //reverse order because of inverse fft
        for(int i = 1; i < signalLength/2; i++)
        {
            tmp = calcInputSignal[i];
            calcInputSignal[i] = calcInputSignal[signalLength - i];
            calcInputSignal[signalLength - i] = tmp;

            tmp = calcComplexSignal[i].real();
            calcComplexSignal[i].real(calcComplexSignal[signalLength - i].real());
            calcComplexSignal[signalLength - i].real(tmp);
        }

        mutex.lock();
        emit valuesReady(&cond);
        cond.wait(&mutex);
        mutex.unlock();


        //fft
        calcComplexSignal = fft(calcComplexSignal);

        bool isAccuracyReached = true;
        for(int i = 0; i < signalLength; i++)
        {
            float newPhase = (float)atan2(calcComplexSignal[i].imag(), calcComplexSignal[i].real());
            if(abs(phases[i] - newPhase) >= TAU)
            {
                isAccuracyReached = false;
            }
            phases[i] = newPhase;
        }

        if(isAccuracyReached)
        {
            break;
        }
    } //  end of while(1)

    float delta;
    float reverseDelta;
    float minDelta;
    float minReverseDelta;
    float minDeltaOffset;
    float minReverseDeltaOffset;
    for(int i = 0; i < signalLength; i++)
    {
        //calc delta
        delta = 0;
        reverseDelta = 0;
        for(int j = 0; j < signalLength; j++)
        {
            delta        += pow(calcInputSignal[j - i + (j < i ? signalLength : 0)] - inputSignal[j], 2);
            reverseDelta += pow(calcInputSignal[signalLength - j + i - (i > j ? signalLength : 0)] - inputSignal[j], 2);
        }
        if(i == 0 || delta < minDelta)
        {
            minDelta = delta;
            minDeltaOffset = i;
        }
        if(i == 0 || reverseDelta < minReverseDelta)
        {
            minReverseDelta = reverseDelta;
            minReverseDeltaOffset = i;
        }
    }

    if(minReverseDelta < minDelta)
    {
        minDelta = minReverseDelta;
        minDeltaOffset = minReverseDeltaOffset;
        //reverse order
        for(int i = 0; i < signalLength/2; i++)
        {
            delta = calcInputSignal[i];
            calcInputSignal[i] = calcInputSignal[signalLength - i - 1];
            calcInputSignal[signalLength - i - 1] = delta;
            //sleep(1);
        }

    }

    mutex.lock();
    emit valuesReady(&cond);
    cond.wait(&mutex);
    mutex.unlock();

    minDeltaOffset = signalLength - minDeltaOffset - 1;
    for(int j = 0; j < minDeltaOffset; j++)
    {
        delta = calcInputSignal[0];
        for(int i = 0; i < signalLength - 1; i++)
        {
            calcInputSignal[i] = calcInputSignal[i + 1];
        }
        calcInputSignal[signalLength - 1] = delta;
        mutex.lock();
        emit valuesReady(&cond);
        cond.wait(&mutex);
        mutex.unlock();
        //sleep(1);
    }

    emit calcFinished();

    delete phases;
}

void PhaseApproxCalc::run()
{
    PhaseApprox();
}

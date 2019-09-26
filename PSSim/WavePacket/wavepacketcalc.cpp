#include "wavepacketcalc.h"

vcd WaveCalcThread::fft(const vcd &as)
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

void WaveCalcThread::stopAnimation()
{
    isAnimated = false;
}

WaveCalcThread::WaveCalcThread(vector<vector<cd>> *waveSamples)
{
    this->waveSamples = waveSamples;
}

double PotentialWell(SystemParams &p, double x)
{
    if(abs(x) < p.r/2)
        return -50./(pow(cosh(x), 2));
    else
        return 0;
}

cd WaveFunc(SystemParams &p, double x)
{
    return cd(exp(-pow((x - p.a) / (2 * p.sigma), 2)), 0);
}

cd WaveDer(SystemParams &p, vcd &values, unsigned int num)
{
    return (values[num + (num == (values.size() - 1) ? 0 : 1)] -
                    values[num - (num == 0 ? 0 : 1)]) /
                    (p.xStep);
}

cd WaveSecondDer(SystemParams &p, vcd &values, unsigned int num)
{
    return ((values[num + (num == (values.size() - 1) ? 0 : 1)] +
                  values[num - (num == 0 ? 0 : 1)] -
                  2. * values[num]) /
                  (p.xStep * p.xStep));
}

void WaveCalcThread::CalcWave(SystemParams &p)
{
    params = p;
    (*waveSamples).resize(params.tCount);
    for(unsigned int it = 0; it < params.tCount; it++)
    {
        (*waveSamples)[it].resize(params.xCount);
    }

    for(unsigned int ix = 0; ix < params.xCount; ix++)
    {
        (*waveSamples)[0][ix] = WaveFunc(params, -params.r + ix * params.xStep);
    }

    cd a(0, -params.integrationStep/(2 * params.xStep * params.xStep));
    cd b = a;
    cd c(1, 0);
    cd d;

    //pass coeff a
    vcd alpha(params.xCount);
    alpha[0] = cd(0, 0);
    //pass coeff b
    vcd beta(params.xCount);
    beta[0] = cd(0, 0);

    for(unsigned int it = 1; it < params.tCount; it++)
    {
        (*waveSamples)[it][0] = cd(0, 0);
        //forward pass
        for(unsigned int ix = 1; ix < params.xCount; ix++)
        {
            c.imag(params.integrationStep * (PotentialWell(params, -params.r + (ix - 1) * params.xStep)/2. +
                                             1./(params.xStep * params.xStep)));

            d = (*waveSamples)[it - 1][(ix - 1)] +
                    cd(0, params.integrationStep/2.) *
                    (WaveSecondDer(params, (*waveSamples)[it - 1], (ix - 1)) -
                    (*waveSamples)[it - 1][(ix - 1)] * PotentialWell(params, -params.r + (ix - 1) * params.xStep));

            alpha[ix] = -b/(c + a * alpha[ix - 1]);
            beta[ix] = (d - a * beta[ix -1]) / (c + a * alpha[ix - 1]);
        }

        (*waveSamples)[it].back() = cd(0, 0);

        //backward pass
        for(unsigned int ix = params.xCount - 1; ix > 0; ix--)
        {
            (*waveSamples)[it][ix - 1] = alpha[ix] * (*waveSamples)[it][ix] + beta[ix];
        }
    }
}

void WaveCalcThread::run()
{
    isAnimated = true;
    while(isAnimated)
    {
        mutex.lock();
        emit valuesReady(&cond);
        cond.wait(&mutex);
        mutex.unlock();

        usleep(16000);
    }
}

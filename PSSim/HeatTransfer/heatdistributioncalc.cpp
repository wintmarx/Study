#include "heatdistributioncalc.h"

HeatCalcThread::HeatCalcThread(vector<matrix> *heatGridSamples)
{
    this->heat = heatGridSamples;
}

double HeatCalcThread::A(unsigned int ir,unsigned int jz, bool napr_r)
{
    if(napr_r)
    {
        if(xBorders[jz][ir])
        {
            double ccc = k[jz][ir+1]/k[jz][ir-1];
            return 1./(1.+ccc);
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            if(ir == 0)
            {
                return 0;//(omega*p.tStep/(2.*p.xStep))*(1/(p.xStep));
            }
            if(ir == 1)
            {
                return 0;//(omega*p.tStep/(2.*p.xStep))*(1/(p.xStep));
            }
            if(ir == p.xCount - 2)
            {
                return omega*p.tStep/(4.*(p.xStep*p.xStep))*((double)(ir-1)/(double)ir);
            }
            if(ir == p.xCount - 1)
            {
                return omega*p.tStep/(4.*(p.xStep*p.xStep))*((double)(ir-1)/(double)ir);
            }
            return omega*p.tStep/(4.*(p.xStep*p.xStep))*((double)(ir-1)/(double)ir);
        }
    }
    else
    {
        if(yBorders[jz][ir])
        {
            double ttt = 1./(1.+(k[jz+1][ir]/k[jz-1][ir]));
            return ttt;
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            if(jz==0)
            {
                return 0;
            }
            if(jz==1)
            {
                return 0;
            }
            if(jz==p.yCount-2)
            {
                return (omega*p.tStep/(4.*p.yStep*p.yStep));
            }
            if(jz==p.yCount-1)
            {
                return (omega*p.tStep/(4.*p.yStep*p.yStep));
            }
            return (omega*p.tStep/(4.*p.yStep*p.yStep));
        }
    }
}
double HeatCalcThread::B(unsigned int ir,unsigned int jz, bool napr_r)
{
    if(napr_r)
    {
        if(xBorders[jz][ir])
        {
            return 1./(1.+(k[jz][ir-1]/k[jz][ir+1]));
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            if(ir==0)
            {
                return 0;
            }
            if(ir==1)
            {
                return omega*p.tStep/(4.*p.xStep*p.xStep)*((double)(ir+1)/(double)ir);
            }
            if(ir==p.xCount - 2)
            {
                return 0;
            }
            if(ir==p.xCount - 1)
            {
                return 0;
            }
            return omega*p.tStep/(4.*p.xStep*p.xStep)*((double)(ir+1)/(double)ir);
        }
    }
    else
    {
        if(yBorders[jz][ir])
        {
            return 1./(1.+(k[jz-1][ir]/k[jz+1][ir]));
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            if(jz==0)
            {
                return (omega*p.tStep/(4.*p.yStep*p.yStep));
            }
            if(jz==1)
            {
                return (omega*p.tStep/(4.*p.yStep*p.yStep));
            }
            if(jz==p.yCount - 1)
            {
                return 0;
            }
            if(jz==p.yCount - 2)
            {
                return 0;
            }
            return (omega*p.tStep/(4.*p.yStep*p.yStep));
        }
    }
}
double HeatCalcThread::C(unsigned int ir,unsigned int jz, bool napr_r)
{
    if(napr_r)
    {
        if(xBorders[jz][ir])
        {
            return -1;
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            if(ir==0)
            {
                return -1.;
            }
            if(ir==1)
            {
                return -1. - B(ir,jz,1);
            }
            if(ir==p.xCount - 2)
            {
                return -1. -omega*p.tStep/(4.*p.xStep*p.xStep*(double)ir)*((double)(ir+1)*p.xStep/p.delta*(p.kEnv/k[jz][p.xCount - 1])+(double)(ir-1));
            }
            if(ir==p.xCount - 1)
            {
                return -1. -omega*p.tStep/(4.*p.xStep*p.xStep*(double)ir)*((double)(ir+1)*p.xStep/p.delta+(double)(ir-1));
            }
            return -1. -omega*p.tStep/(2.*p.xStep*p.xStep);
        }
    }
    else
    {
        if(yBorders[jz][ir])
        {
            return -1;
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            if(jz==0)
            {
                return -1. -(omega*p.tStep/(4.*p.yStep))*(1./(p.yStep)+1./(p.delta));
            }
            if(jz==1)
            {
                return -1. -(omega*p.tStep/(4.*p.yStep))*(1./(p.yStep)+(p.kEnv/k[0][ir])*1./(p.delta));
            }
            if(jz==p.yCount-2)
            {
                return -1. -(omega*p.tStep/(4.*p.yStep))*(1./(p.yStep)+(p.kEnv/k[p.yCount - 1][ir])*1./(p.delta));
            }
            if(jz==p.yCount-1)
            {
                return -1. -(omega*p.tStep/(2.*p.yStep))*(1./(p.yStep)+1./(p.delta));
            }
            return -1. -omega*p.tStep/(2.*p.yStep*p.yStep);
        }
    }
}
double HeatCalcThread::D(unsigned int it, unsigned int ir,unsigned int jz, bool napr_r)
{
    if(napr_r)
    {
        if(xBorders[jz][ir])
        {
            return 0;
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            double lambda=q[jz][ir]/c[jz][ir];
            if(ir==0)
            {
                return -(*heat)[it - 1][jz][ir]-(omega*p.tStep)*(D2Temp(it - 1,ir,jz,1)/2.+D2Temp(it - 1,ir,jz,0))-p.tStep *lambda;
            }
            if(ir==1)
            {
                return -(*heat)[it - 1][jz][ir]-(omega*p.tStep)*(D2Temp(it - 1,ir,jz,1)/2.+D2Temp(it - 1,ir,jz,0))-p.tStep*lambda;
            }
            if(ir==p.xCount - 2)
            {
                return -(*heat)[it - 1][jz][ir]-(omega*p.tStep)*(D2Temp(it - 1,ir,jz,1)/2.+D2Temp(it - 1,ir,jz,0))-p.tStep*lambda
                    -(omega*p.tStep)/(4.*p.xStep)*((double)(ir+1)/(double)ir)*(p.kEnv/k[jz][p.xCount - 1])*p.tempEnv/p.delta;
            }
            if(ir==p.xCount - 1)
            {
                return -(*heat)[it - 1][jz][ir]-(omega*p.tStep)*(D2Temp(it - 1,ir,jz,1)/2.+D2Temp(it - 1,ir,jz,0))-p.tStep*lambda
                    -(omega*p.tStep)/(4.*p.xStep)*((double)(ir+1)/(double)ir)*p.tempEnv/p.delta;
            }
            return -(*heat)[it - 1][jz][ir]-(omega*p.tStep)*(D2Temp(it - 1,ir,jz,1)/2.+D2Temp(it - 1,ir,jz,0))-p.tStep*lambda;
        }
    }
    else
    {
        if(yBorders[jz][ir])
        {
            return 0;
        }
        else
        {
            double omega=k[jz][ir]/c[jz][ir];
            double lambda=q[jz][ir]/c[jz][ir];
            if(jz==0)
            {
                return -(*heat)[it + 1][jz][ir]-(omega*p.tStep)*(D2Temp(it + 1,ir,jz,1)+D2Temp(it + 1,ir,jz,0)/2.)-p.tStep*lambda
                    -(omega*p.tStep)/(4.*p.yStep)*(p.tempEnv/p.delta);
            }
            if(jz==1)
            {
                return -(*heat)[it + 1][jz][ir]-(omega*p.tStep)*(D2Temp(it + 1,ir,jz,1)+D2Temp(it + 1,ir,jz,0)/2.)-p.tStep*lambda
                    -(omega*p.tStep)/(4.*p.yStep)*(p.tempEnv/p.delta)*(p.kEnv/k[0][ir]);
            }
            if(jz==p.yCount-2)
            {
                return -(*heat)[it + 1][jz][ir]-(omega*p.tStep)*(D2Temp(it + 1,ir,jz,1)+D2Temp(it + 1,ir,jz,0)/2.)-p.tStep*lambda
                    -(omega*p.tStep)/(4.*p.yStep)*(p.tempEnv/p.delta)*(p.kEnv/k[p.yCount - 1][ir]);
            }
            if(jz==p.yCount-1)
            {
                return -(*heat)[it + 1][jz][ir]-(omega*p.tStep)*(D2Temp(it + 1,ir,jz,1)+D2Temp(it + 1,ir,jz,0)/2.)-p.tStep*lambda
                    -(omega*p.tStep)/(4.*p.yStep)*(p.tempEnv/p.delta);
            }
            return -(*heat)[it + 1][jz][ir]-(omega*p.tStep)*(D2Temp(it + 1,ir,jz,1)+D2Temp(it + 1,ir,jz,0)/2.)-p.tStep*lambda;
        }
    }
}
double HeatCalcThread::D2Temp(unsigned int it, unsigned int ir,unsigned int jz,bool napr_r)
{
    if(napr_r)
    {
        if(ir==0||ir==p.xCount - 1||ir==p.xCount-2||ir==1)
        {
            if(ir==0)
            {
                return 0;
            }
            if(ir==1)
            {
                return (1/(2.*p.xStep*p.xStep*(double)ir))*(p.xStep*(double)(ir+1)*(((*heat)[it][jz][ir+1]-(*heat)[it][jz][ir])/p.xStep));
            }
            if(ir==p.xCount-2)//граница
            {
                return (1/(2.*p.xStep*p.xStep*(double)ir))
    *(p.xStep*(double)(ir+1)*(p.kEnv/k[jz][p.xCount - 1])*((p.tempEnv-(*heat)[it][jz][ir])/p.delta)
    -p.xStep*(double)(ir-1)*((*heat)[it][jz][ir]-(*heat)[it][jz][ir-1])/p.xStep);
            }
            if(ir==p.xCount-1)//граница
            {
                return (1/(2.*p.xStep*p.xStep*(double)ir))
                *(p.xStep*(double)(ir+1)*((p.tempEnv-(*heat)[it][jz][ir])/p.delta)
                -p.xStep*(double)(ir-1)*((*heat)[it][jz][ir]-(*heat)[it][jz][ir-1])/p.xStep);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return (1/(2.*p.xStep*p.xStep*(double)ir))
                *(p.xStep*(double)(ir+1)*(((*heat)[it][jz][ir+1]-(*heat)[it][jz][ir])/p.xStep)
                -p.xStep*(double)(ir-1)*((*heat)[it][jz][ir]-(*heat)[it][jz][ir-1])/p.xStep);
        }
    }
    else
    {
        if(jz==0||jz==p.yCount - 1||jz==p.yCount-2||jz==1)
        {
            if(jz==0)
            {
                return (1./(2.*p.yStep))*(((*heat)[it][jz+1][ir]-(*heat)[it][jz][ir])/p.yStep-((*heat)[it][jz][ir]-p.tempEnv)/p.delta);
            }
            if(jz==1)
            {
                return (1./(2.*p.yStep))*(((*heat)[it][jz+1][ir]-(*heat)[it][jz][ir])/p.yStep-(p.kEnv/k[0][ir])*((*heat)[it][jz][ir]-p.tempEnv)/p.delta);
            }
            if(jz==p.yCount-2)
            {
                return (1./(2.*p.yStep))*((p.kEnv/k[p.yCount - 1][ir])*(p.tempEnv-(*heat)[it][jz][ir])/p.delta-((*heat)[it][jz][ir]-(*heat)[it][jz-1][ir])/p.yStep);
            }
            if(jz==p.yCount-1)
            {
                return (1./(2.*p.yStep))*((p.tempEnv-(*heat)[it][jz][ir])/p.delta-((*heat)[it][jz][ir]-(*heat)[it][jz-1][ir])/p.yStep);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return (1./(2.*p.yStep))*(((*heat)[it][jz+1][ir]-(*heat)[it][jz][ir])/p.yStep-((*heat)[it][jz][ir]-(*heat)[it][jz-1][ir])/p.yStep);
        }

    }
}



double FuncDer(matrix &m, double step, unsigned int y, unsigned int x, bool derX)
{
    if(derX)
    {
        if(x == 0)
        {
            x++;
        }
        else if(x == m[y].size() - 1)
        {
            x--;
        }
        return (m[y][x + 1] - m[y][x - 1]) /
                       (2 * step);
    }
    if(y == 0)
    {
        y++;
    }
    else if(y == m.size() - 1)
    {
        y--;
    }
    return (m[y + 1][x] - m[y - 1][x]) /
                   (2 * step);

}

double FuncSecondDer(matrix &m, double step, unsigned int y, unsigned int x, bool derX)
{
    if(derX)
    {
        if(x == 0)
        {
            x++;
        }
        else if(x == m[y].size() - 1)
        {
            x--;
        }
        return ((m[y][x - 1] + m[y][x + 1] - 2. * m[y][x]) /
                          (step * step));
    }
    if(y == 0)
    {
        y++;
    }
    else if(y == m.size() - 1)
    {
        y--;
    }
    return ((m[y + 1][x] + m[y - 1][x] - 2. * m[y][x]) /
                       (step * step));
}

void HeatCalcThread::CalcAsync()
{
    (*heat).resize(
                p.tCount + 1,
                matrix(
                      p.yCount,
                      row(p.xCount, 0)
                     )
                );

    for(unsigned int it = 1; it < p.tCount; it++)
    {
        for(unsigned int iy = 0; iy < p.yCount; iy++)
        {
            for(unsigned int ix = 0; ix < p.xCount; ix++)
            {
                (*heat)[it][iy][ix] = 0;
            }
        }
    }

    double x;
    double y;
    k.resize(p.yCount, row(p.xCount, 0));
    q.resize(p.yCount, row(p.xCount, 0));
    c.resize(p.yCount, row(p.xCount, 0));
    xBorders.resize(p.yCount, vector<bool>(p.xCount, false));
    yBorders.resize(p.yCount, vector<bool>(p.xCount, false));

    for(unsigned int iy = 0; iy < p.yCount; iy++)
    {
        for(unsigned int ix = 0; ix < p.xCount; ix++)
        {
            (*heat)[0][iy][ix] = p.tempEnv;
            x = p.xStep * ix;
            y = p.yStep * iy;
            if(x <= (p.xMax - p.wallWidth) && y >= p.wallWidth && y <= (p.yMax - p.wallWidth))
            {
                k[iy][ix] = p.kl;
                c[iy][ix] = p.cl;
                q[iy][ix] = 0;
            }
            else
            {
                k[iy][ix] = p.km;
                c[iy][ix] = p.cm;
                q[iy][ix] = 0;
            }
            if(x <= p.heaterW && y <= p.heaterH)
            {
                k[iy][ix] = p.kh;
                c[iy][ix] = p.ch;
                q[iy][ix] = p.heaterP;
            }
            xBorders[iy][ix] = false;
            yBorders[iy][ix] = false;
            if(ix > 0 && (abs(k[iy][ix] - k[iy][ix - 1]) > 1e-4))
            {
                xBorders[iy][ix] = true;
            }

            if(iy > 0 && (abs(k[iy][ix] - k[iy - 1][ix]) > 1e-4) && xBorders[iy][ix] == false)
            {
                yBorders[iy][ix] = true;
            }
        }  
    }

    //double a = 0;
    //double b = 0;
    //double c = 0;
    //double d = 0;

    //pass coeff a
    vector<double> alpha;
    //pass coeff b
    vector<double> beta;

    /*double rBoundCondA = (1 - p.eps / (2. * p.kl)) / (1 + p.eps / (2. * p.kl));
    double rBoundCondB = p.eps / (1 + p.eps / (2. * p.kl)) * p.tempEnv;
    double lBoundCondA = 1;
    double lBoundCondB = 0;

    double uBoundCondA = (1 - p.eps / (2. * p.kl)) / (1 + p.eps / (2. * p.kl));
    double uBoundCondB = p.eps / (1 + p.eps / (2. * p.kl)) * p.tempEnv;
    double dBoundCondA = uBoundCondA;
    double dBoundCondB = uBoundCondB;*/

    for(unsigned int it = 1; it < p.tCount; it++)
    {
        //First half-step
        alpha.resize(p.xCount, 0);
        beta.resize(p.xCount, 0);

        //c = 1 + params.lambda * params.lambda * params.tStep / (2. * params.xStep * params.xStep);

        for(unsigned int iy = 0; iy < p.yCount; iy++)
        {
            alpha[0] = 1.;//lBoundCondA;
            beta[0] = 0.;//lBoundCondB;
            //forward pass
            for(unsigned int ix = 1; ix < p.xCount; ix++)
            {
                //a = params.lambda * params.lambda * params.tStep / (8. * params.xStep) * (2./params.xStep - 1/(ix * params.xStep));
                //b = -params.lambda * params.lambda * params.tStep / (8. * params.xStep) * (2./params.xStep + 1/(ix * params.xStep));


                /*double sdx = FuncSecondDer((*heatGridSamples)[it - 1], params.xStep, iy, ix, true);
                double dx = FuncDer((*heatGridSamples)[it - 1], params.xStep, iy, ix, true)/(ix * params.xStep);
                double sdy = FuncSecondDer((*heatGridSamples)[it - 1], params.yStep, iy, ix, false);
                double qxy = Q(params, iy, ix);
                double lastt = (*heatGridSamples)[it - 1][iy][ix];
                d =  lastt +
                     params.tStep/4. *
                         (qxy * 2. +
                          params.lambda * params.lambda *
                              (sdx +
                               dx +
                               sdy * 2.));*/

                alpha[ix] = -B(ix - 1, iy, 1)/(C(ix - 1, iy, 1) + A(ix - 1, iy, 1) * alpha[ix - 1]);
                beta[ix] = (D(it, ix - 1, iy, 1) - A(ix - 1, iy, 1) * beta[ix -1]) / (C(ix - 1, iy, 1) + A(ix - 1, iy, 1) * alpha[ix - 1]);
            }

            //Right boundary condition x
            (*heat)[it + 1][iy][p.xCount - 1] = (beta[p.xCount - 1] + ((p.kEnv/k[iy][p.xCount - 1])/p.delta) * (p.tempEnv - (*heat)[it - 1][iy][p.xCount - 1]) * p.xStep)/(1 - alpha[p.xCount - 1]);
                    //rBoundCondA * tmpMat[iy][params.xCount - 2] + rBoundCondB;

            //backward pass
            for(unsigned int ix = p.xCount - 1; ix > 0; ix--)
            {
                (*heat)[it + 1][iy][ix - 1] = alpha[ix] * (*heat)[it + 1][iy][ix] + beta[ix];
            }

            //Left boundary condition x
            //tmpMat[iy][0] = lBoundCondA * tmpMat[iy][1] + lBoundCondB;
        }

        //Second half-step
        alpha.resize(p.yCount, 0);
        beta.resize(p.yCount, 0);
        //a = -params.lambda * params.lambda * params.tStep / (4. * params.yStep * params.yStep);
        //b = a;
        //c = 1 - 2 * a;
        //d = 0;
        for(unsigned int ix = 0; ix < p.xCount; ix++)
        {
            alpha[0] = 1.;
            beta[0] = (-(p.kEnv/k[0][ix])/p.delta)*((*heat)[it + 1][0][ix]-p.tempEnv)*p.yStep;
            //forward pass
            for(unsigned int iy = 1; iy < p.yCount; iy++)
            {
                /*double sdy = FuncSecondDer(tmpMat, params.yStep, iy, ix, false);
                double dx = FuncDer(tmpMat, params.xStep, iy, ix, true)/(ix * params.xStep);
                double sdx = FuncSecondDer(tmpMat, params.xStep, iy, ix, true);
                double tmt = tmpMat[iy][ix];
                d = tmt +
                    params.tStep/4. *
                        (Q(params, iy, ix) * 2. +
                         params.lambda * params.lambda *
                             (sdy +
                              dx * 2 +
                              sdx * 2.));*/

                alpha[iy] = -B(ix, iy - 1, 0)/(C(ix, iy - 1, 0) + A(ix, iy - 1, 0) * alpha[iy - 1]);
                beta[iy] = (D(it, ix, iy - 1, 0) - A(ix, iy - 1, 0) * beta[iy -1]) / (C(ix, iy - 1, 0) + A(ix, iy - 1, 0) * alpha[iy - 1]);
            }

            //Upper boundary condition y
            (*heat)[it][p.yCount - 1][ix] = (beta[p.yCount - 1]+((p.kEnv/k[p.yCount - 1][ix])/ p.delta)*(p.tempEnv-(*heat)[it + 1][p.yCount - 1][ix])*(p.yStep))/(1-alpha[p.yCount - 1]);
                    //uBoundCondA * (*heatGridSamples)[it][params.yCount - 2][ix] + uBoundCondB;

            //backward pass
            for(unsigned int iy = p.yCount - 1; iy > 0; iy--)
            {
                (*heat)[it][iy - 1][ix] = alpha[iy] * (*heat)[it][iy][ix] + beta[iy];
            }

            //Down boundary condition y
            //(*heatGridSamples)[it][0][ix] = dBoundCondA * (*heatGridSamples)[it][1][ix] + dBoundCondB;
        }

        //Left x boundary cond
        /*for(unsigned int iy = 0; iy < p.yCount; iy++)
        {
            (*heat)[it][iy][0] = lBoundCondA * (*heat)[it][iy][1] + lBoundCondB;
        }*/
        emit calcFinished();
    }
    isCalculation = false;
}

void HeatCalcThread::run()
{
    while(isAnimated)
    {
        mutex.lock();
        emit valuesReady(&cond);
        cond.wait(&mutex);
        mutex.unlock();

        usleep(16000);
    }
    if(!isAnimated && isCalculation)
    {
       CalcAsync();
    }

}

void HeatCalcThread::StartAnimation()
{
    if(isCalculation || isAnimated)
    {
        return;
    }
    isAnimated = true;
    start();
}

void HeatCalcThread::Calc(SystemParams &p)
{
    if(isCalculation)
    {
        return;
    }
    wait();
    this->p = p;
    isAnimated = false;
    isCalculation = true;
    start();
}

void HeatCalcThread::StopAnimation()
{
    isAnimated = false;
}

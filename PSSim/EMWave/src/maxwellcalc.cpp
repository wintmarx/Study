#include "maxwellcalc.h"
#include <QFile>
#include <QTextStream>

float Der(float xNext, float xPrev, float dt)
{
    return (xNext - xPrev)/dt;
}

float MaxwellCalc::Hz(uint x, uint y, uint t)
{
    GridPoint &f = grid[t-1][y][x];
    GridPoint &fnx = grid[t-1][y][x-1];
    GridPoint &fny = grid[t-1][y-1][x];
    return f.h.z() - p.dt/ f.mPerm * (Der(f.e.y(), fnx.e.y(), p.gridStep) - Der(f.e.x(), fny.e.x(), p.gridStep));
}

float MaxwellCalc::Ex(uint x, uint y, uint t)
{
    /*float c = (1+grid[t-1][y][x].eCond*p.dt/(2.f*grid[t-1][y][x].dPerm));
    return (-c+2.f)/c * grid[t-1][y][x].e.x() +
             (2.f*p.dt/grid[t-1][y][x].dPerm)/c * Der(grid[t][y + 1][x].h.z(), grid[t][y][x].h.z(), 2.f * p.gridStep);*/
    return (1 - grid[t-1][y][x].eCond * p.dt / grid[t-1][y][x].dPerm) * grid[t-1][y][x].e.x() +
             (p.dt/grid[t-1][y][x].dPerm) * Der(grid[t][y + 1][x].h.z(), grid[t][y][x].h.z(), p.gridStep);
}

float MaxwellCalc::Ey(uint x, uint y, uint t)
{
    /*float c = (1.f+grid[t-1][y][x].eCond*p.dt/(2.f*grid[t-1][y][x].dPerm));
    return (-c+2.f)/c * grid[t - 1][y][x].e.y() -
             (2.f*p.dt/grid[t-1][y][x].dPerm)/c * Der(grid[t][y][x + 1].h.z(), grid[t][y][x].h.z(), 2.f * p.gridStep);*/

    return (1 - grid[t-1][y][x].eCond * p.dt / grid[t-1][y][x].dPerm) * grid[t - 1][y][x].e.y() -
             (p.dt/grid[t-1][y][x].dPerm) * Der(grid[t][y][x + 1].h.z(), grid[t][y][x].h.z(), p.gridStep);
}

float MaxwellCalc::HzyPML(uint x, uint y, uint t)
{
    float mCond = p.eCondPML * p.mPermVacuum / p.dPermVacuum;
    int n = y - p.gridStepsCount + p.pmlCount;
    if (n > 0)
    {
        mCond *= static_cast<float>(n)/p.pmlCount;
    }
    else if ((n = (p.pmlCount - y)) > 0)
    {
        mCond *= static_cast<float>(n)/p.pmlCount;
    }
    return /*(1 - mCond * p.dt / p.mPermVacuum) **/ grid[t-1][y][x].h.y() +
            (p.dt/p.mPermVacuum) * Der(grid[t-1][y][x].e.x(), grid[t-1][y-1][x].e.x(), p.gridStep);
}

float MaxwellCalc::HzxPML(uint x, uint y, uint t)
{
    float mCond = p.eCondPML * p.mPermVacuum / p.dPermVacuum;
    int n = x - p.gridStepsCount + p.pmlCount;
    if (n > 0)
    {
        mCond *= static_cast<float>(n)/p.pmlCount;
    }
    else if ((n = (p.pmlCount - x)) > 0)
    {
        mCond *= static_cast<float>(n)/p.pmlCount;
    }
    return (1 - mCond * p.dt/p.mPermVacuum) * grid[t-1][y][x].h.x() -
            (2.f * p.dt/p.mPermVacuum) * Der(grid[t-1][y][x].e.y(), grid[t-1][y][x-1].e.y(), p.gridStep);
}

float MaxwellCalc::ExPML(uint x, uint y, uint t)
{
    float eCond = p.eCondPML;
    int n = y - p.gridStepsCount + p.pmlCount;
    if (n > 0)
    {
        eCond *= static_cast<float>(n)/p.pmlCount;
    }
    else if ((n = (p.pmlCount - y)) > 0)
    {
        eCond *= static_cast<float>(n)/p.pmlCount;
    }
    return (1 - eCond * p.dt/p.dPermVacuum) * grid[t-1][y][x].e.x() +
            (p.dt/p.dPermVacuum) * Der(grid[t][y+1][x].h.z(), grid[t][y][x].h.z(), p.gridStep);
}

float MaxwellCalc::EyPML(uint x, uint y, uint t)
{
    float eCond = p.eCondPML;
    int n = x - p.gridStepsCount + p.pmlCount;
    if (n > 0)
    {
        eCond *= static_cast<float>(n)/p.pmlCount;
    }
    else if ((n = (p.pmlCount - x)) > 0)
    {
        eCond *= static_cast<float>(n)/p.pmlCount;
    }

    return (1 - eCond * p.dt/p.dPermVacuum) * grid[t-1][y][x].e.y() -
            (p.dt/p.dPermVacuum) * Der(grid[t][y][x+1].h.z(), grid[t][y][x].h.z(), p.gridStep);
}

MaxwellCalc::MaxwellCalc()
{
    isReady = false;
}

bool MaxwellCalc::Calc(SystemParams &p)
{
    this->p = p;
    grid.resize(p.timeStepsCount);
    QFile file("field.cfg");

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QTextStream in(&file);
    QString line = "";
    for(uint t = 0; t < p.timeStepsCount; t++)
    {
        grid[t].resize(p.gridStepsCount);
        for(uint i = 0; i < p.gridStepsCount; i++)
        {

            if (t == 0)
            {
                line = in.readLine();
            }
            grid[t][i].resize(p.gridStepsCount);
            for(uint j = 0; j < p.gridStepsCount; j++)
            {
                if (t == 0)
                {
                    grid[t][i][j].type = GridPointType(line.at(j).toLatin1() - '0');
                    switch(grid[t][i][j].type)
                    {
                        case VACUUM:
                            grid[t][i][j].dPerm = p.dPermVacuum;
                            grid[t][i][j].eCond = p.eCondVacuum;
                            grid[t][i][j].mPerm = p.mPermVacuum;
                            break;
                        case METAL:
                            grid[t][i][j].dPerm = p.dPermMetal;
                            grid[t][i][j].eCond = p.eCondMetal;
                            grid[t][i][j].mPerm = p.mPermMetal;
                            break;
                        case CHARGE:
                            grid[t][i][j].dPerm = p.dPermVacuum;
                            grid[t][i][j].eCond = p.eCondVacuum;
                            grid[t][i][j].mPerm = p.mPermVacuum;
                            break;
                    }

                }
                else
                {
                    grid[t][i][j].dPerm = grid[t - 1][i][j].dPerm;
                    grid[t][i][j].eCond = grid[t - 1][i][j].eCond;
                    grid[t][i][j].mPerm = grid[t - 1][i][j].mPerm;
                    grid[t][i][j].e = grid[t - 1][i][j].e;
                    grid[t][i][j].type = grid[t - 1][i][j].type;
                }
                /*if (grid[t][i][j].type == CHARGE)
                {
                    grid[t][i][j].e.setX(p.chargeEx * cosf(float(M_PI * t) / p.timeStepsCount * 5.f));
                    grid[t][i][j].e.setY(p.chargeEy * cosf(float(M_PI * t) / p.timeStepsCount * 5.f));
                }*/

            }
        }
        if(t == 0)
        {
            file.close();
        }
    }

    for(uint t = 1; t < p.timeStepsCount; t++)
    {
        for(uint i = p.gridStepsCount - 2; i > 1; i--)
        {
            for(uint j = 1; j < p.gridStepsCount - 1; j++)
            {
                if((j >= p.gridStepsCount - p.pmlCount) || j < p.pmlCount || (i >= p.gridStepsCount - p.pmlCount) || i < p.pmlCount)
                {
                    grid[t][i][j].h.setX(HzxPML(j, i, t));
                    grid[t][i][j].h.setY(HzyPML(j, i, t));
                    grid[t][i][j].h.setZ(grid[t][i][j].h.x() + grid[t][i][j].h.y());
                }
                else
                {
                    grid[t][i][j].h.setZ(Hz(j, i, t));
                }

            }
        }
        for(uint i = 1; i < p.gridStepsCount - 1; i++)
        {
            for(uint j = p.gridStepsCount - 2; j > 1; j--)
            {
                if (grid[t][i][j].type == METAL)
                {
                    grid[t][i][j].e.setX(0);
                    grid[t][i][j].e.setY(0);
                    continue;
                }
                if((j >= p.gridStepsCount - p.pmlCount) || j < p.pmlCount || (i >= p.gridStepsCount - p.pmlCount) || i < p.pmlCount)
                {
                    grid[t][i][j].e.setX(ExPML(j, i, t));
                    grid[t][i][j].e.setY(EyPML(j, i, t));
                }
                else
                {
                    grid[t][i][j].e.setX(Ex(j, i, t));
                    grid[t][i][j].e.setY(Ey(j, i, t));
                }
                if (grid[t][i][j].type == CHARGE)
                {
                    grid[t][i][j].e.setX(grid[t][i][j].e.x() + p.chargeEx * cosf(float(M_PI * t) / p.timeStepsCount * 5.f));
                    grid[t][i][j].e.setY(grid[t][i][j].e.y() + p.chargeEy * cosf(float(M_PI * t) / p.timeStepsCount * 5.f));
                }
            }
        }
    }
    isReady = true;
    return true;
}

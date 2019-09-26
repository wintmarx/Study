#include "diodecalc.h"

DiodeCalcThread::DiodeCalcThread()
{
    isAnimated = false;
    isCalculation = false;

    a = nullptr;
    b = nullptr;
    x = nullptr;
}

DiodeCalcThread::~DiodeCalcThread()
{
    delete[] a;
    delete[] b;
    delete[] x;
}

//метод Качмаржа
/* матрица А, столбец свободных членов, массив неизвестных,
nn - количество неизвестных;  ny - количество уравнений*/
void kazf(float* a, float* b, float* x, int nn, int ny)
{
    float eps = 1.e-6f;
    //float s;
    int i, j, k;
    float s1, s2, fa1, t;
    float *x1;

    x1 = new float[nn];

    x[0] = 0.5f;
    for (i = 1; i<nn; i++)  x[i] = 0.f;

    s1 = s2 = 1.f;
    while (s1 > eps*s2)
    {
        for (i = 0; i < nn; i++)
        {
            x1[i] = x[i];
        }
        for (i = 0; i < ny; i++)
        {
            s1 = 0.0;
            s2 = 0.0;

            for (j = 0; j < nn; j++)
            {
                fa1 = a[i * nn + j];
                s1 += fa1 * x[j];
                s2 += fa1 * fa1;
            }

            t = (b[i] - s1) / s2;

            for (k = 0; k < nn; k++)
            {
                x[k] += a[i * nn + k] * t;
            }
        }

        s1 = 0.0;
        s2 = 0.0;
        for (i = 0; i < nn; i++)
        {
            s1 += (x[i] - x1[i]) * (x[i] - x1[i]);
            s2 += x[i] * x[i];
        }
        s1 = sqrtf(s1);
        s2 = sqrtf(s2);
    }
    delete[] x1;
}

void DiodeCalcThread::run()
{
    points.clear();
    particles.clear();
    tris.clear();
    SetupBorders();
    TriangulateGrid();
    //Find all border indices
    borderIndices.clear();
    innerIndices.clear();
    for(uint i = 0; i < points.size(); i++)
    {
        if(points[i].isBorder)
        {
            borderIndices.push_back(i);
        }
        else
        {
            innerIndices.push_back(i);
        }
    }

    if (a)
    {
        delete[] a;
    }
    if (b)
    {
        delete[] b;
    }
    if (x)
    {
        delete[] x;
    }

    a = new float[innerIndices.size() * innerIndices.size()];
    b = new float[innerIndices.size()];
    x = new float[innerIndices.size()];

    CreateVoronoiCells(points, tris);

    globalEField.setX((p.leftPotential - p.rightPotential) * p.width);

    GenerateParticles();

    if(isCalculation)
    {
        iv.clear();
        qDebug("Calc started");
        float uStep = (p.uMax - p.uMin)/p.uStepsCount;
        float charge;
        for (float u = p.uMin; u < p.uMax; u += uStep)
        {
            particles.clear();
            GenerateParticles();
            globalEField.setX((-u) * p.width);
            charge = 0;
            for(float t = 0; t < (p.dt * 200); t += p.dt)
            {
                UpdateParticles();
                charge += erasedParticlesCount /** p.charge*/;
            }
            iv.push_back(make_pair(u, /*-*/charge/* / (p.dt * 100)*/));
            qDebug("calc for u = %f completed, charge = %f", (double)(u), (double)(iv.back().second));
        }

        emit calcFinished();
        return;
    }


    while(isAnimated)
    {   
        UpdateParticles();
        mutex.lock();
        emit valuesReady(&cond);
        cond.wait(&mutex);
        mutex.unlock();
    }
}

void DiodeCalcThread::StartAnimation()
{
    if(/*isCalculation || */isAnimated)
    {
        return;
    }
    isAnimated = true;
    start();
}

void DiodeCalcThread::Calc(SystemParams &p)
{
    if (isAnimated)
    {
        mutex.lock();
        isAnimated = false;
        mutex.unlock();
        quit();
        return;
    }
    wait();
    this->p = p;
    isAnimated = true;
    isCalculation = false;
    start();
}

void DiodeCalcThread::CalcIV(SystemParams &p)
{
    if (isCalculation)
    {
        isCalculation = false;
        return;
    }
    wait();
    this->p = p;
    isCalculation = true;
    isAnimated = false;
    start();
}

void DiodeCalcThread::StopAnimation()
{
    isAnimated = false;
    wait();
}

void DiodeCalcThread::ShareParticlesCharge(std::vector<Particle> &particles, std::vector<Point> &points)
{
    for(uint i = 0; i < particles.size(); i++)
    {
        for(uint j = 0; j < points.size(); j++)
        {
            if(!points[j].isActive)
            {
                continue;
            }
            if (IsPointInsideVCell(particles[i].pos, points[j].vCellIdx))
            {
                points[j].ro += p.charge / 0.001;//points[j].cellSquare;
                break;
            }
        }
    }
}

void DiodeCalcThread::AdvectParticle(float dt, Particle &p, Triangle &t, std::vector<Point> &points)
{
    points[t.p1].setZ(points[t.p1].u);
    points[t.p2].setZ(points[t.p2].u);
    points[t.p3].setZ(points[t.p3].u);
    QVector3D eField = GetNormalCoeffs(t, points);
    if (eField.z() < 0)
    {
       eField = -eField;
    }
    eField.setZ(0);
    //eField = -eField;
    points[t.p1].setZ(0);
    points[t.p2].setZ(0);
    points[t.p3].setZ(0);

    //qDebug("field %f:%f, gfield %f", eField.x(), eField.y(), globalEField.x());

    p.pos += p.vel * dt;

    p.vel += (eField + globalEField) * this->p.charge / this->p.mass * dt;

    //p.pos += (eField + globalEField) * this->p.charge / this->p.mass * dt * dt / 2.f;
}

void DiodeCalcThread::UpdateParticles()
{
    erasedParticlesCount = 0;

    ShareParticlesCharge(particles, points);

    //TODO: Make all points ro 0 every time step;
    GalerkinMethod(points, tris, borderIndices, innerIndices, a, b, x);

    for(auto it = particles.begin(); it < particles.end(); it++)
    {
        for(uint j = 0; j < tris.size(); j++)
        {
            if(!points[tris[j].p1].isActive ||
               !points[tris[j].p2].isActive ||
               !points[tris[j].p3].isActive)
            {
                continue;
            }

            if(IsPointInsideTriangle(it->pos, tris[j], points))
            {
                AdvectParticle(p.dt, *it, tris[j], points);
                break;
            }
        }
        //qDebug("pos: %f; %f, pWidth: %f", it->pos.x(), it->pos.y(), p.width);
        if(it->pos.x() > p.width - p.gridDeviation) {
            erasedParticlesCount++;
        }
        if(it->pos.y() > step.y() * (p.activeGridStart + (p.activeGridSize - 1)) - p.gridDeviation ||
           it->pos.y() < step.y() * p.activeGridStart + p.gridDeviation ||
           it->pos.x() > p.width - p.gridDeviation ||
           it->pos.x() <= p.gridDeviation)
        {
            /*if(rand() > RAND_MAX/2)
            {
                particles.erase(it);
            }
            else
            {*/
                it->pos.setX(static_cast<float>(rand())/RAND_MAX * step.x() * 2 + p.gridDeviation);
                it->pos.setY(static_cast<float>(rand())/RAND_MAX * (step.y() * (p.activeGridSize - 1) - p.gridDeviation) + step.y() * p.activeGridStart + p.gridDeviation);
                it->vel.setX(static_cast<float>(rand())/RAND_MAX * 500);
                it->vel.setY(static_cast<float>(rand())/RAND_MAX * 500 - 250);
            //}
        }
    }

    //GenerateParticles();
}

void DiodeCalcThread::GenerateParticles()
{
    //particles.clear();
    //particles.resize(p.ui->particlesAmountEdit->text().toUInt());
    particles.resize(p.particlesAmount);
    for(uint i = 0; i < particles.size(); i++)
    {
        particles[i].pos.setX(static_cast<float>(rand())/RAND_MAX * step.x() * 2);
        particles[i].pos.setY(static_cast<float>(rand())/RAND_MAX * step.y() * (p.activeGridSize - 1) + step.y() * p.activeGridStart);
        particles[i].vel.setX(static_cast<float>(rand())/RAND_MAX * 500);
        particles[i].vel.setY(static_cast<float>(rand())/RAND_MAX * 500 - 250);
    }
    //ui->bifurPlot->setParticles(&particles);
}

void DiodeCalcThread::GalerkinMethod(std::vector<Point> &points,
                                std::vector<Triangle> &tris,
                                std::vector<uint> &borderIndices,
                                std::vector<uint> &innerIndices,
                                float *a,
                                float *b,
                                float *x)
{

    QVector3D n1;
    QVector3D n2;
    float area;
    float fullArea = 0;
    for (uint i = 0; i < innerIndices.size(); i++)
    {
        for (uint j = 0; j < innerIndices.size(); j++)
        {
            a[j * innerIndices.size() + i] = 0;
            fullArea = 0;
            for (uint t = 0; t < tris.size(); t++)
            {
                //if tri contains point i  and point j
                if ((tris[t].p1 == innerIndices[i] ||
                     tris[t].p2 == innerIndices[i] ||
                     tris[t].p3 == innerIndices[i]) &&
                    (tris[t].p1 == innerIndices[j] ||
                     tris[t].p2 == innerIndices[j] ||
                     tris[t].p3 == innerIndices[j]))
                {
                    //set z peak to i point
                    points[innerIndices[i]].setZ(1.f);
                    //calculate i plane coeffs
                    n1 = GetNormalCoeffs(tris[t], points);
                    //remove z peak from i point
                    points[innerIndices[i]].setZ(0.f);

                    if(n1.z() < 0)
                    {
                        n1 = -n1;
                    }

                    //set z peak to j point
                    points[innerIndices[j]].setZ(1.f);
                    //calculate j plane coeffs
                    n2 = GetNormalCoeffs(tris[t], points);
                    //remove z peak from j point
                    points[innerIndices[j]].setZ(0.f);

                    if(n2.z() < 0)
                    {
                        n2 = -n2;
                    }

                    area = 0.0001f;//GetNormalCoeffs(tris[t], points).length()/2.f;
                    fullArea += area;

                    //add triangle's sum to integral
                    a[j * innerIndices.size() + i] += area * area * (n1.x() * n2.x() + n1.y() * n2.y());
                }
            }
            a[j * innerIndices.size() + i] -= points[innerIndices[i]].ro * fullArea / 3.f;
        }

        b[i] = 0;
        fullArea = 0;
        for(uint j = 0; j < borderIndices.size(); j++)
        {
            for (uint t = 0; t < tris.size(); t++)
            {
                // if tri contains point i  and point j
                if ((tris[t].p1 == innerIndices[i] ||
                     tris[t].p2 == innerIndices[i] ||
                     tris[t].p3 == innerIndices[i]) &&
                    (tris[t].p1 == borderIndices[j] ||
                     tris[t].p2 == borderIndices[j] ||
                     tris[t].p3 == borderIndices[j]))
                {
                    // set z peak to i point
                    points[innerIndices[i]].setZ(1.f);
                    // calculate i plane coeffs
                    n1 = GetNormalCoeffs(tris[t], points);
                    // remove z peak from i point
                    points[innerIndices[i]].setZ(0.f);

                    if(n1.z() < 0)
                    {
                        n1 = -n1;
                    }

                    // set z peak to j point
                    points[borderIndices[j]].setZ(1.f);
                    //calculate j plane coeffs
                    n2 = GetNormalCoeffs(tris[t], points);
                    //remove z peak from j point
                    points[borderIndices[j]].setZ(0.f);

                    if(n2.z() < 0)
                    {
                        n2 = -n2;
                    }

                    area = 0.0001f;//GetNormalCoeffs(tris[t], points).length()/2.f;

                    fullArea += area;

                    //add triangle's sum to integral
                    b[i] += points[borderIndices[j]].u * area * area * (n1.x() * n2.x() + n1.y() * n2.y());
                }
            }
            b[i] -= points[innerIndices[i]].ro * fullArea / 3.f;
        }
    }

    /*for(uint i = 0; i < innerIndices.size(); i++)
    {
        for(uint j = 0; j < innerIndices.size(); j++)
        {
            qDebug("a[%d, %d] = %f", i, j, a[j * innerIndices.size() + i]);
        }
        qDebug("b[%d] = %f", i, b[i]);
    }*/

    kazf(a, b, x, innerIndices.size(), innerIndices.size());

    //set min/max only from active points;
    /*bool minmaxset = false;
    float max = 0;
    float min = 0;
    uint steps = 10;*/
    for(uint i = 0; i < innerIndices.size(); i++)
    {
        points[innerIndices[i]].u = -x[i];

        points[innerIndices[i]].ro = 0;

        /*if(!points[innerIndices[i]].isActive)
        {
            continue;
        }

        if(!minmaxset)
        {
            max = points[innerIndices[i]].u;
            min = points[innerIndices[i]].u;
            minmaxset = true;
            continue;
        }

        if(points[innerIndices[i]].u > max)
        {
            max = points[innerIndices[i]].u;
        }
        else if(points[innerIndices[i]].u < min)
        {
            min = points[innerIndices[i]].u;
        }*/
    }
}

QVector2D GetIsolineIntersection(Point &h, Point &l, float isoVal)
{
    return
    {
      h.x() + (l.x() - h.x()) * ((h.u - isoVal)/(h.u - l.u)),
      h.y() + (l.y() - h.y()) * ((h.u - isoVal)/(h.u - l.u))
    };
}

void DiodeCalcThread::CreateIsolines(float max, float min, uint stepsCount)
{
    float step = std::fabs(max - min) / stepsCount;
    isolines.clear();
    float value = 0;
    for(uint i = 0; i < tris.size(); i++)
    {
        if(!points[tris[i].p1].isActive ||
           !points[tris[i].p2].isActive ||
           !points[tris[i].p3].isActive)
        {
            continue;
        }
        if(points[tris[i].p1].u > points[tris[i].p2].u)
        {
            std::swap(tris[i].p1, tris[i].p2);
        }
        if(points[tris[i].p2].u > points[tris[i].p3].u)
        {
            std::swap(tris[i].p2, tris[i].p3);
        }
        if(points[tris[i].p1].u > points[tris[i].p2].u)
        {
            std::swap(tris[i].p1, tris[i].p2);
        }
        for(uint j = 0; j < stepsCount; j++)
        {
            value = min + j * step;
            if(value < points[tris[i].p3].u && value > points[tris[i].p1].u)
            {
                isolines.push_back
                ({
                    GetIsolineIntersection(points[tris[i].p3], points[tris[i].p1], value),
                     {0, 0}
                });
                if(value < points[tris[i].p2].u && value > points[tris[i].p1].u)
                {
                    isolines.back().p2 = GetIsolineIntersection(points[tris[i].p2], points[tris[i].p1], value);
                }
                else
                {
                    isolines.back().p2 = GetIsolineIntersection(points[tris[i].p3], points[tris[i].p2], value);
                }

            }
        }
    }
}

float FindCentralX(Point &p1, Point &p2, Point &p3)
{
    return 0.5f * ((p2.x() * p2.x() - p1.x() * p1.x() + p2.y() * p2.y() - p1.y() * p1.y()) * (p3.y() - p1.y()) -
                   (p3.x() * p3.x() - p1.x() * p1.x() + p3.y() * p3.y() - p1.y() * p1.y()) * (p2.y() - p1.y())) /
                  ((p2.x() - p1.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p2.y() - p1.y()));
}

float FindCentralY(Point &p1, Point &p2, Point &p3)
{
    return 0.5f * ((p3.x() * p3.x() - p1.x() * p1.x() + p3.y() * p3.y() - p1.y() * p1.y()) * (p2.x() - p1.x()) -
                   (p2.x() * p2.x() - p1.x() * p1.x() + p2.y() * p2.y() - p1.y() * p1.y()) * (p3.x() - p1.x())) /
                  ((p2.x() - p1.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p2.y() - p1.y()));
}

float DiodeCalcThread::randDeviation()
{
    return static_cast<float>(rand())/RAND_MAX * p.gridDeviation;
}

void DiodeCalcThread::Triangulation(std::vector<Point> &points, std::vector<uint> &indices)
{
    Point c;
    float radius;
    bool isGoodTri;
    for (uint i = 0; i < indices.size(); i++)
    {
        for (uint j = i + 1; j < indices.size(); j++)
        {
            for (uint k = j + 1; k < indices.size(); k++)
            {
                c.setX(FindCentralX(points[indices[i]], points[indices[j]], points[indices[k]]));
                c.setY(FindCentralY(points[indices[i]], points[indices[j]], points[indices[k]]));
                radius = c.distanceToPoint(points[indices[i]]);
                isGoodTri = true;
                for (uint n = 0; n < points.size(); n++)
                {
                    if(n == indices[i] || n == indices[j] || n == indices[k])
                    {
                        continue;
                    }
                    if(c.distanceToPoint(points[n]) < radius)
                    {
                        isGoodTri = false;
                        break;
                    }
                }
                if (isGoodTri)
                {
                    tris.push_back({indices[i], indices[j], indices[k], radius, c});
                }
            }
        }
    }
}

void DiodeCalcThread::Triangulation(std::vector<Point> &points, std::vector<Triangle> &tris)
{
    Point c;
    float radius;
    bool isGoodTri;
    for (uint i = 0; i < points.size(); i++)
    {
        for (uint j = i + 1; j < points.size(); j++)
        {
            for (uint k = j + 1; k < points.size(); k++)
            {
                c.setX(FindCentralX(points[i], points[j], points[k]));
                c.setY(FindCentralY(points[i], points[j], points[k]));
                radius = c.distanceToPoint(points[i]);
                isGoodTri = true;
                for (uint n = 0; n < points.size(); n++)
                {
                    if(n == i || n == j || n == k)
                    {
                        continue;
                    }
                    if(c.distanceToPoint(points[n]) < radius)
                    {
                        isGoodTri = false;
                        break;
                    }
                }
                if (isGoodTri)
                {
                    tris.push_back({i, j, k, radius, c});
                }
            }
        }
    }


}

void DiodeCalcThread::CreateVoronoiCells(std::vector<Point> &points, std::vector<Triangle> &tris)
{
    for_each(points.begin(), points.end(), [](Point &p){ p.vCellIdx.clear(); });

    for (uint i = 0; i < tris.size(); i++)
    {
        points[tris[i].p1].vCellIdx.push_back(i);
        points[tris[i].p2].vCellIdx.push_back(i);
        points[tris[i].p3].vCellIdx.push_back(i);
    }

    for_each(points.begin(), points.end(),
    [&tris](Point &p)
    {
        if(!p.isActive || p.isBorder)
        {
            return;
        }
        std::sort(p.vCellIdx.begin() + 1, p.vCellIdx.end(),
        [&p, &tris](uint &a, uint &b)
        {
             return ((tris[a].c.x() - tris[p.vCellIdx[0]].c.x()) * (tris[b].c.y() - tris[p.vCellIdx[0]].c.y()) - (tris[a].c.y() - tris[p.vCellIdx[0]].c.y()) * (tris[b].c.x() - tris[p.vCellIdx[0]].c.x())) > 0;
        });
        p.cellSquare = QVector3D::crossProduct(tris[p.vCellIdx[0]].c - p, tris[p.vCellIdx.back()].c - p).length() * 0.5f;
        for (uint i = 1; i < p.vCellIdx.size(); i++)
        {
            p.cellSquare += QVector3D::crossProduct(tris[p.vCellIdx[i]].c - p, tris[p.vCellIdx[i - 1]].c - p).length() * 0.5f;
        }
    });
}

void DiodeCalcThread::eraseHSTris(std::vector<uint> &hsIndices)
{
    for(uint i = 0; i < hsIndices.size(); i++)
    {
        for(uint j = 0; j < tris.size(); j++)
        {
            if(tris[j].p1 == hsIndices[i] ||
               tris[j].p2 == hsIndices[i] ||
               tris[j].p3 == hsIndices[i])
            {
                tris.erase(tris.begin() + j--);
            }
        }
    }

    while(!hsIndices.empty())
    {
        points.erase(points.begin() + hsIndices.back());
        for(uint i = 0; i < hsIndices.size() - 1; i++)
        {
            if(hsIndices[i] > hsIndices.back())
            {
                hsIndices[i]--;
            }
        }
        for(uint j = 0; j < tris.size(); j++)
        {
            if(tris[j].p1 > hsIndices.back())
            {
                tris[j].p1--;
            }
            if(tris[j].p2 > hsIndices.back())
            {
                tris[j].p2--;
            }
            if(tris[j].p3 > hsIndices.back())
            {
                tris[j].p3--;
            }
        }
        hsIndices.pop_back();
    }
}

void DiodeCalcThread::TriangulateGrid()
{
    Point tmp;
    tmp.u = 0;
    tmp.ro = 0;
    std::vector<uint> hsIndices;

    //Hyperstructure points
    tmp.setX(p.width/2.f + randDeviation());
    tmp.setY(p.height + p.height/2.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(p.width/2.f + randDeviation());
    tmp.setY(0.f - p.height/2.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(0.f - p.width/2.f + randDeviation());
    tmp.setY(p.height/2.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(p.width + p.width/2.f + randDeviation());
    tmp.setY(p.height/2.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    // Triangulate initial grid
    Triangulation(points, tris);

    // Add points to the grid one by one and retriangulate broken part of the grid after each new point
    std::vector<uint> brokenPoints;
    for (uint i = 1; i < p.stepsCountY; i++)
    {
        for (uint j = 1; j < p.stepsCountX; j++)
        {
            brokenPoints.clear();
            tmp.setX(j * step.x() + randDeviation());
            tmp.setY(i * step.y() + randDeviation());
            tmp.isActive = (i >= p.activeGridStart && i < p.activeGridStart + p.activeGridSize);
            tmp.isBorder = false;
            points.push_back(tmp);
            for (uint k = 0; k < tris.size(); k++)
            {
                if (tmp.distanceToPoint(tris[k].c) < tris[k].r)
                {
                    bool isUniqueP1 = true;
                    bool isUniqueP2 = true;
                    bool isUniqueP3 = true;
                    for (uint n = 0; n < brokenPoints.size(); n++)
                    {
                        if (tris[k].p1 == brokenPoints[n])
                        {
                            isUniqueP1 = false;
                        }
                        if (tris[k].p2 == brokenPoints[n])
                        {
                            isUniqueP2 = false;
                        }
                        if (tris[k].p3 == brokenPoints[n])
                        {
                            isUniqueP3 = false;
                        }
                        if(!isUniqueP1 && !isUniqueP2 && !isUniqueP3)
                        {
                            break;
                        }
                    }
                    if(isUniqueP1)
                    {
                        brokenPoints.push_back(tris[k].p1);
                    }
                    if(isUniqueP2)
                    {
                        brokenPoints.push_back(tris[k].p2);
                    }
                    if(isUniqueP3)
                    {
                        brokenPoints.push_back(tris[k].p3);
                    }
                    tris.erase(tris.begin() + k--);
                }
            }
            if(!brokenPoints.empty())
            {
                brokenPoints.push_back(points.size() - 1);
                Triangulation(points, brokenPoints);
            }
        }
    }
    eraseHSTris(hsIndices);
}

void DiodeCalcThread::SetupBorders()
{
    Point tmp;
    tmp.ro = 0;
    points.clear();
    tris.clear();
    step.setX(static_cast<float>(p.width) / p.stepsCountX);
    step.setY(static_cast<float>(p.height) / p.stepsCountY);

    // Box's points
    for (uint i = 0; i < p.stepsCountX + 1; i++)
    {
        tmp.isActive = false;
        //up side
        tmp.setX(i * step.x() + randDeviation());
        tmp.setY(0 + randDeviation());
        tmp.isBorder = true;
        tmp.u = 0;
        points.push_back(tmp);

        //down side
        tmp.setX(i * step.x() + randDeviation());
        tmp.setY(p.height + randDeviation());
        tmp.isBorder = true;
        tmp.u = 0;
        points.push_back(tmp);
    }

    for (uint i = 1; i < p.stepsCountY; i++)
    {
        tmp.isActive = (i >= p.activeGridStart && i < p.activeGridStart + p.activeGridSize);
        //right side
        tmp.setX(p.width + randDeviation());
        tmp.setY(i * step.y() + randDeviation());
        tmp.isBorder = true;
        tmp.u = 0;//ui->vRightEdit->text().toFloat();
        points.push_back(tmp);

        //left side
        tmp.setX(0 + randDeviation());
        tmp.setY(i * step.y() + randDeviation());
        tmp.isBorder = true;
        tmp.u = 0;//ui->vLeftEdit->text().toFloat();
        points.push_back(tmp);
    }
}

QVector3D DiodeCalcThread::GetNormalCoeffs(Triangle &t, std::vector<Point> &points)
{
    /*qDebug("npx %f, %f, %f", points[t.p1].x(), points[t.p2].x(), points[t.p3].x());
    qDebug("npy %f, %f, %f", points[t.p1].y(), points[t.p2].y(), points[t.p3].y());
    qDebug("npz %f, %f, %f", points[t.p1].z(), points[t.p2].z(), points[t.p3].z());
    qDebug("--------------------");*/
    return /*QVector3D::normal(points[t.p1], points[t.p2], points[t.p3]);*/QVector3D::crossProduct(points[t.p2] - points[t.p1], points[t.p3] - points[t.p1]);
}

bool DiodeCalcThread::IsPointInsideVCell(QVector3D &p, std::vector<uint> &vCell)
{
    for(uint i = 0; i < vCell.size(); i++)
    {
        if (QVector3D::crossProduct(tris[vCell[i]].c - p, tris[vCell[i]].c - tris[vCell[(i == 0 ? vCell.size() : i) - 1]].c).z() < 0)
        {
            return false;
        }
    }
    return true;
}

bool DiodeCalcThread::IsPointInsideTriangle(QVector3D &p, Triangle &t, std::vector<Point> &points)
{
    float a = (points[t.p1].x() - p.x()) * (points[t.p2].y() - points[t.p1].y()) - (points[t.p2].x() - points[t.p1].x()) * (points[t.p1].y() - p.y());
    float b = (points[t.p2].x() - p.x()) * (points[t.p3].y() - points[t.p2].y()) - (points[t.p3].x() - points[t.p2].x()) * (points[t.p2].y() - p.y());
    float c = (points[t.p3].x() - p.x()) * (points[t.p1].y() - points[t.p3].y()) - (points[t.p1].x() - points[t.p3].x()) * (points[t.p3].y() - p.y());

    return ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0));
}

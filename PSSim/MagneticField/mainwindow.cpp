#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>

float MainWindow::randDeviation()
{
    return static_cast<float>(rand())/RAND_MAX * gridDeviation;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    srand(static_cast<uint>(time(nullptr)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//метод Качмаржа
void kazf(float* a, float* b, float* x, int nn, int ny)
/* матрица А, столбец свободных членов, массив неизвестных,
nn - количество неизвестных;  ny - количество уравнений*/
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

void MainWindow::Triangulation(std::vector<Point> &points, std::vector<uint> &indices)
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

    //ui->bifurPlot->setData(&tris, &points);
    ui->bifurPlot->update();
}

void MainWindow::Triangulation(std::vector<Point> &points)
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

    ui->bifurPlot->update();
}

void MainWindow::NeighborSearch()
{
    pointsNeighbors.clear();
    pointsNeighbors.resize(points.size());
    for(uint i = 0; i < tris.size(); i++)
    {
        pointsNeighbors[tris[i].p1].insert(tris[i].p2);
        pointsNeighbors[tris[i].p1].insert(tris[i].p3);
        pointsNeighbors[tris[i].p2].insert(tris[i].p1);
        pointsNeighbors[tris[i].p2].insert(tris[i].p3);
        pointsNeighbors[tris[i].p3].insert(tris[i].p1);
        pointsNeighbors[tris[i].p3].insert(tris[i].p2);
    }
}

void MainWindow::eraseHSTris(std::vector<uint> &hsIndices)
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

    ui->bifurPlot->update();
}

void MainWindow::on_startBtn_clicked()
{
    Point tmp;
    tmp.u = 0;
    std::vector<uint> hsIndices;
    float dist;

    //Hyperstructure points
    tmp.setX(ui->bifurPlot->width()/2.f + randDeviation());
    tmp.setY(ui->bifurPlot->height() + 200.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(ui->bifurPlot->width()/2.f + randDeviation());
    tmp.setY(0.f - 200 + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(0.f - 200 + randDeviation());
    tmp.setY(ui->bifurPlot->height()/2.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(ui->bifurPlot->width() + 200.f + randDeviation());
    tmp.setY(ui->bifurPlot->height()/2.f + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    // Hyperstructure ring's points
    float ringStep = static_cast<float>(M_PI) / ringPointsCount;
    for (uint i = 1; i < ringPointsCount; i++)
    {
        tmp.setX(ringCenter.x() + (ringRadius - ringWidth/2.f) * sinf(i * ringStep + ringAngle) + randDeviation());
        tmp.setY(ringCenter.y() + (ringRadius - ringWidth/2.f) * cosf(i * ringStep + ringAngle) + randDeviation());
        points.push_back(tmp);
        hsIndices.push_back(points.size() - 1);
    }

    tmp.setX(ringCenter.x() + (ringRadius - ringWidth * 3.f/4.f) * sinf(1 * ringStep + ringAngle) + randDeviation());
    tmp.setY(ringCenter.y() + (ringRadius - ringWidth * 3.f/4.f) * cosf(1 * ringStep + ringAngle) + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(ringCenter.x() + (ringRadius - ringWidth * 1.f/4.f) * sinf(1 * ringStep + ringAngle) + randDeviation());
    tmp.setY(ringCenter.y() + (ringRadius - ringWidth * 1.f/4.f) * cosf(1 * ringStep + ringAngle) + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(ringCenter.x() + (ringRadius - ringWidth * 3.f/4.f) * sinf((ringPointsCount - 1) * ringStep + ringAngle) + randDeviation());
    tmp.setY(ringCenter.y() + (ringRadius - ringWidth * 3.f/4.f) * cosf((ringPointsCount - 1) * ringStep + ringAngle) + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    tmp.setX(ringCenter.x() + (ringRadius - ringWidth * 1.f/4.f) * sinf((ringPointsCount - 1) * ringStep + ringAngle) + randDeviation());
    tmp.setY(ringCenter.y() + (ringRadius - ringWidth * 1.f/4.f) * cosf((ringPointsCount - 1) * ringStep + ringAngle) + randDeviation());
    points.push_back(tmp);
    hsIndices.push_back(points.size() - 1);

    // Triangulate initial grid
    Triangulation(points);

    // Add points to the grid one by one and retriangulate broken part of the grid after each new point
    std::vector<uint> brokenPoints;
    for (uint i = 1; i < stepsCount; i++)
    {
        for (uint j = 1; j < stepsCount; j++)
        {
            brokenPoints.clear();
            tmp.setX(j * step.x() + randDeviation());
            tmp.setY(i * step.y() + randDeviation());
            tmp.isBorder = false;
            dist = ringCenter.distanceToPoint(tmp);
            if (dist < ringRadius && dist > ringRadius - ringWidth)
            {
                if ((tmp.y() - ringCenter.y()) * cosf(ringAngle - static_cast<float>(M_PI_2)) + (tmp.x() - ringCenter.x()) * sinf(ringAngle - static_cast<float>(M_PI_2)) < 0)
                {
                    continue;
                }
            }
            points.push_back(tmp);
            //ui->bifurPlot->update();
            //this->repaint();
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

void MainWindow::on_redrawBtn_clicked()
{
    ui->bifurPlot->setData(&tris, &points);
    Point tmp;
    points.clear();
    tris.clear();

    // Grid params
    gridDeviation = ui->deviationEdit->text().toFloat();
    stepsCount = ui->gridSizeEdit->text().toUInt();
    step.setX(static_cast<float>(ui->bifurPlot->width()) / stepsCount);
    step.setY(static_cast<float>(ui->bifurPlot->height()) / stepsCount);
    ringPointsCount = ui->ringPointsEdit->text().toUInt();
    ringRadius = ui->ringRadiusEdit->text().toUInt();
    ringWidth = ui->ringWidthEdit->text().toUInt();
    ringCenter = QVector3D(ui->ringXEdit->text().toFloat(), ui->ringYEdit->text().toFloat(), 0);
    ringAngle = ui->angleEdit->text().toFloat() / 180.f * static_cast<float>(M_PI);

    // Add ring's points
    float ringStep = static_cast<float>(M_PI) / ringPointsCount;
    for (uint i = 0; i < ringPointsCount + 1; i++)
    {
        tmp.setX(ringCenter.x() + ringRadius * sinf(i * ringStep + ringAngle) + randDeviation());
        tmp.setY(ringCenter.y() + ringRadius * cosf(i * ringStep + ringAngle) + randDeviation());
        tmp.isBorder = true;
        tmp.u = 10;
        points.push_back(tmp);

        tmp.setX(ringCenter.x() + (ringRadius - ringWidth) * sinf(i * ringStep + ringAngle) + randDeviation());
        tmp.setY(ringCenter.y() + (ringRadius - ringWidth) * cosf(i * ringStep + ringAngle) + randDeviation());
        tmp.isBorder = true;
        tmp.u = 10;
        points.push_back(tmp);
    }

    tmp.setX(ringCenter.x() + (ringRadius - ringWidth/2.f) * sinf(0 + ringAngle) + randDeviation());
    tmp.setY(ringCenter.y() + (ringRadius - ringWidth/2.f) * cosf(0 + ringAngle) + randDeviation());
    tmp.isBorder = true;
    tmp.u = 10;
    points.push_back(tmp);

    tmp.setX(ringCenter.x() + (ringRadius - ringWidth/2.f) * sinf(-static_cast<float>(M_PI) + ringAngle) + randDeviation());
    tmp.setY(ringCenter.y() + (ringRadius - ringWidth/2.f) * cosf(-static_cast<float>(M_PI) + ringAngle) + randDeviation());
    tmp.isBorder = true;
    tmp.u = 10;
    points.push_back(tmp);

    // Box's points
    for (uint i = 0; i < stepsCount + 1; i++)
    {
        tmp.setX(i * step.x() + randDeviation());
        tmp.setY(0 + randDeviation());
        tmp.isBorder = true;
        tmp.u = 0;
        points.push_back(tmp);

        tmp.setX(i * step.x() + randDeviation());
        tmp.setY(ui->bifurPlot->height() + randDeviation());
        tmp.isBorder = true;
        tmp.u = 0;
        points.push_back(tmp);

        if (i != 0 && i != stepsCount)
        {
            tmp.setX(ui->bifurPlot->width() + randDeviation());
            tmp.setY(i * step.y() + randDeviation());
            tmp.isBorder = true;
            tmp.u = 0;
            points.push_back(tmp);

            tmp.setX(0 + randDeviation());
            tmp.setY(i * step.y() + randDeviation());
            tmp.isBorder = true;
            tmp.u = 0;
            points.push_back(tmp);
        }
    }
    ui->bifurPlot->update();
}

QVector3D MainWindow::GetNormalCoeffs(Triangle &t)
{
    /*qDebug("npx %f, %f, %f", points[t.p1].x(), points[t.p2].x(), points[t.p3].x());
    qDebug("npy %f, %f, %f", points[t.p1].y(), points[t.p2].y(), points[t.p3].y());
    qDebug("npz %f, %f, %f", points[t.p1].z(), points[t.p2].z(), points[t.p3].z());
    qDebug("--------------------");*/
    return QVector3D::normal(points[t.p1], points[t.p2], points[t.p3]);//QVector3D::crossProduct(points[t.p2] - points[t.p1], points[t.p3] - points[t.p1]);
}

void MainWindow::on_calcBtn_clicked()
{
    std::vector<uint> borderIndices;
    std::vector<uint> innerIndices;
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
    float *a = new float[innerIndices.size() * innerIndices.size()];
    float *b = new float[innerIndices.size()];
    float *x = new float[innerIndices.size()];

    //NeighborSearch();

    QVector3D n1;
    QVector3D n2;
    float area;
    for (uint i = 0; i < innerIndices.size(); i++)
    {
        for (uint j = 0; j < innerIndices.size(); j++)
        {
            a[j * innerIndices.size() + i] = 0;
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
                    n1 = GetNormalCoeffs(tris[t]);
                    //remove z peak from i point
                    points[innerIndices[i]].setZ(0.f);

                    if(n1.z() < 0)
                    {
                        n1 = -n1;
                    }

                    //set z peak to j point
                    points[innerIndices[j]].setZ(1.f);
                    //calculate j plane coeffs
                    n2 = GetNormalCoeffs(tris[t]);
                    //remove z peak from j point
                    points[innerIndices[j]].setZ(0.f);

                    if(n2.z() < 0)
                    {
                        n2 = -n2;
                    }

                    area = GetNormalCoeffs(tris[t]).length()/2.f;

                    //add triangle's sum to integral
                    a[j * innerIndices.size() + i] +=  area * area * (n1.x() * n2.x() + n1.y() * n2.y());
                }
            }
        }

        b[i] = 0;
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
                    n1 = GetNormalCoeffs(tris[t]);
                    // remove z peak from i point
                    points[innerIndices[i]].setZ(0.f);

                    if(n1.z() < 0)
                    {
                        n1 = -n1;
                    }

                    // set z peak to j point
                    points[borderIndices[j]].setZ(1.f);
                    //calculate j plane coeffs
                    n2 = GetNormalCoeffs(tris[t]);
                    //remove z peak from j point
                    points[borderIndices[j]].setZ(0.f);

                    if(n2.z() < 0)
                    {
                        n2 = -n2;
                    }

                    area = GetNormalCoeffs(tris[t]).length()/2.f;

                    //add triangle's sum to integral
                    b[i] += points[borderIndices[j]].u * area * area * (n1.x() * n2.x() + n1.y() * n2.y());
                }
            }
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

    float max = -x[0];
    float min = -x[0];
    uint steps = 5;
    for(uint i = 0; i < innerIndices.size(); i++)
    {
        points[innerIndices[i]].u = -x[i];
        if(-x[i] > max)
        {
            max = -x[i];
        }
        else if(-x[i] < min)
        {
            min = -x[i];
        }
    }

    delete[] a;
    delete[] b;
    delete[] x;

    CreateIsolines(max, min, steps);
}

QVector2D GetIsolineIntersection(Point &h, Point &l, float isoVal)
{
    return
    {
      h.x() + (l.x() - h.x()) * ((h.u - isoVal)/(h.u - l.u)),
      h.y() + (l.y() - h.y()) * ((h.u - isoVal)/(h.u - l.u))
    };
}

void MainWindow::CreateIsolines(float max, float min, uint stepsCount)
{
    float step = std::fabs(max - min) / stepsCount;
    isolines.clear();
    float value = 0;
    for(uint i = 0; i < tris.size(); i++)
    {
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
    ui->bifurPlot->setIsolines(&isolines);
    ui->bifurPlot->update();
}

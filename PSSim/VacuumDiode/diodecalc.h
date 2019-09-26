#ifndef PHASE_APPROX_CALC_H
#define PHASE_APPROX_CALC_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

#include "diagramplot.h"

using namespace std;

struct SystemParams
{
    float dt;
    float charge;
    float mass;
    uint stepsCountX;
    uint stepsCountY;
    uint activeGridStart;
    uint activeGridSize;
    float width;
    float height;
    float viewCoeff;
    float leftPotential;
    float rightPotential;
    float gridDeviation;
    uint particlesAmount;
    float uMin;
    float uMax;
    uint uStepsCount;
};

class DiodeCalcThread : public QThread
{
    Q_OBJECT
public:
    DiodeCalcThread();
    ~DiodeCalcThread();
    void run();
    void StopAnimation();
    void StartAnimation();
    void Calc(SystemParams &p);
    void CalcIV(SystemParams &p);


    SystemParams p;

    std::vector<Point> points;
    std::vector<Line> isolines;
    std::vector<Triangle> tris;
    std::vector<Particle> particles;
    std::vector<std::pair<float, float>> iv;

signals:
    void valuesReady(QWaitCondition *cond);
    void calcFinished();

private:
    bool isAnimated = false;
    bool isCalculation = false;

    QMutex mutex;
    QWaitCondition cond;

    float randDeviation();
    void SetupBorders();
    void TriangulateGrid();
    void Triangulation(std::vector<Point> &points, std::vector<Triangle> &tris);
    void Triangulation(std::vector<Point> &points, std::vector<uint> &indices);
    void GalerkinMethod(std::vector<Point> &points,
                        std::vector<Triangle> &tris,
                        std::vector<uint> &borderIndices,
                        std::vector<uint> &innerIndices,
                        float *a,
                        float *b,
                        float *x);
    void CreateVoronoiCells(std::vector<Point> &points, std::vector<Triangle> &tris);
    void CreateIsolines(float max, float min, uint stepsCountX);
    void GenerateParticles();
    void UpdateParticles();
    bool IsPointInsideVCell(QVector3D &p, std::vector<uint> &vCell);
    bool IsPointInsideTriangle(QVector3D &p, Triangle &t, std::vector<Point> &points);
    void AdvectParticle(float dt, Particle &p, Triangle &t, std::vector<Point> &points);
    QVector3D GetNormalCoeffs(Triangle &t, std::vector<Point> &points);
    void eraseHSTris(std::vector<uint> &hsIndices);
    void ShareParticlesCharge(std::vector<Particle> &particles, std::vector<Point> &points);

    std::vector<uint> borderIndices;
    std::vector<uint> innerIndices;
    QVector2D step;
    float *a;
    float *b;
    float *x;

    uint erasedParticlesCount;

    QVector3D globalEField;
};

#endif // PHASE_APPROX_CALC_H

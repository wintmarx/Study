#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <complex>

using namespace std;

#define DTOR(X) (M_PI * X / 180.f)
#define RTOD(X) (180.f * X / M_PI)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->phaseDiagram->xAxis->setRange(0, 100);
    ui->phaseDiagram->yAxis->setRange(-16, 16);
    ui->phaseDiagram->setInteraction(QCP::iRangeDrag, true);
    ui->phaseDiagram->setInteraction(QCP::iRangeZoom, true);
    ui->phaseDiagram->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    curve = new QCPCurve(ui->phaseDiagram->xAxis, ui->phaseDiagram->yAxis);
}

MainWindow::~MainWindow()
{
    delete ui;
}

typedef struct
{
    float lambda;
    float m;
    float r;
    float l;
    float h;
    float w;
    float u;
    float a;
    float r0;
    float r1;
}SystemParam;

float RateFunc(float u, float a, SystemParam &param)
{
    return u;
}

const float g = 9.8f;

float AccelFunc(float u, float a, SystemParam &param)
{
    float mG = -param.m * g * (param.l + param.r) * sin(a);
    //float f = param.k * (u < 0 ? 1 : -1);
    float f = ((param.w > u) ? 1 : -1) * ((param.r0 - param.r1)/(1 + param.lambda * fabs(param.w - u)) + param.r1);
    float mF = f * param.r;
    return ((mF + mG) / (param.m * pow(param.l + param.r, 2)));
}

void SimulateSystemStep(SystemParam &param)
{
    float K = param.h * RateFunc(param.u, param.a, param);
    float L = param.h * AccelFunc(param.u, param.a, param);
    float da = K/6;
    float du = L/6;
    float tmpK = param.h * RateFunc(param.u + L/2, param.a + K/2, param);
    L = param.h * AccelFunc(param.u + L/2, param.a + K/2, param);
    K = tmpK;
    da += K/3;
    du += L/3;
    tmpK = param.h * RateFunc(param.u + L/2, param.a + K/2, param);
    L = param.h * AccelFunc(param.u + L/2, param.a + K/2, param);
    K = tmpK;
    da += K/3;
    du += L/3;
    tmpK = param.h * RateFunc(param.u + L, param.a + K, param);
    L = param.h * AccelFunc(param.u + L, param.a + K, param);
    K = tmpK;
    da += K/6;
    du += L/6;

    param.u += du;
    param.a += da;
}


void MainWindow::on_startBtn_clicked()
{
    ui->phaseDiagram->clearGraphs();
    ui->phaseDiagram->addGraph();

    ui->phaseDiagram->removePlottable(curve);
    curve = new QCPCurve(ui->phaseDiagram->xAxis, ui->phaseDiagram->yAxis);

    SystemParam param;
    param.lambda = ui->fLambdaEdit->text().toFloat();
    param.r0 = ui->staticFrictionEdit->text().toFloat();
    param.r1 = ui->infVelFrictionEdit->text().toFloat();
    param.m = ui->massEdit->text().toFloat();
    param.r = ui->radiusEdit->text().toFloat();
    param.l = ui->lengthEdit->text().toFloat();
    param.h = ui->timestepEdit->text().toFloat();
    param.w = DTOR(ui->tubeRateEdit->text().toFloat());
    param.u = DTOR(ui->initialRateEdit->text().toFloat());
    param.a = DTOR(ui->initialAngleEdit->text().toFloat());

    for(int i = 0; i < static_cast<int>(60.f/param.h); i++)
    {
        curve->addData(RTOD(param.a), RTOD(param.u));
        SimulateSystemStep(param); 
    }

    curve->rescaleAxes();
    ui->phaseDiagram->rescaleAxes();
    ui->phaseDiagram->replot();
}

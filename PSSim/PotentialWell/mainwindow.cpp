#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <complex>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->uPlot->xAxis->setRange(0, 100);
    ui->uPlot->yAxis->setRange(-16, 16);
    ui->uPlot->setInteraction(QCP::iRangeDrag, true);
    ui->uPlot->setInteraction(QCP::iRangeZoom, true);
    ui->uPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    ui->phaseFuncPlot->xAxis->setRange(0, 100);
    ui->phaseFuncPlot->yAxis->setRange(-16, 16);
    ui->phaseFuncPlot->setInteraction(QCP::iRangeDrag, true);
    ui->phaseFuncPlot->setInteraction(QCP::iRangeZoom, true);
    ui->phaseFuncPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    ui->waveFuncPlot->xAxis->setRange(0, 100);
    ui->waveFuncPlot->yAxis->setRange(-16, 16);
    ui->waveFuncPlot->setInteraction(QCP::iRangeDrag, true);
    ui->waveFuncPlot->setInteraction(QCP::iRangeZoom, true);
    ui->waveFuncPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

MainWindow::~MainWindow()
{
    delete ui;
}

double PotentialEnergy(SystemParams &p, double z)
{
    return (2 * z <= p.a && 2 * z >= -p.a) ? 0 : p.U0;
}

double PhaseFuncDer(SystemParams &p, double z, double phaseFunc)
{
    return (PotentialEnergy(p, z) - p.e) * pow(cos(phaseFunc), 2) - pow(sin(phaseFunc), 2);
}

double RadialFuncDer(SystemParams &p, double z, double radialFunc, double phaseFunc)
{
    return radialFunc * (1 + PotentialEnergy(p, z) - p.e) * cos(phaseFunc) * sin(phaseFunc);
}

void Euler(SystemParams &p, std::vector<double> &funcValues, std::vector<double> &keys)
{
    for(unsigned int i = 1; i < funcValues.size(); i++)
    {
        funcValues[i] = funcValues[i - 1] + (keys[i] - keys[i - 1]) * PhaseFuncDer(p, keys[i - 1], funcValues[i - 1]);
    }
}

void Euler(SystemParams &p, std::vector<double> &funcValues, std::vector<double> &keys, std::vector<double> &phase)
{
    for(unsigned int i = 1; i < funcValues.size(); i++)
    {
        funcValues[i] = funcValues[i - 1] + (keys[i] - keys[i-1]) * RadialFuncDer(p, keys[i - 1], funcValues[i - 1], phase[i - 1]);
    }
}

void PhaseFunc(SystemParams &p, std::vector<double> &funcValues, std::vector<double> &energyValues)
{
    std::vector<double> eulerValues(p.zCount);
    eulerValues[0] = M_PI_2;
    std::vector<double> zValues(p.zCount);
    for(unsigned int i = 0; i < p.zCount; i++)
    {
        zValues[i] = -p.R + i * p.zStep;
    }

    for(unsigned int i = 0; i < p.eCount; i++)
    {
        energyValues[i] = p.eMin + i * p.eStep;
        p.e = energyValues[i];
        Euler(p, eulerValues, zValues);
        funcValues[i] = eulerValues.back();
    }
}

double PhaseFunc(SystemParams &p, double energyValue)
{
    std::vector<double> eulerValues(p.zCount);
    eulerValues[0] = M_PI_2;
    std::vector<double> zValues(p.zCount);
    for(unsigned int i = 0; i < p.zCount; i++)
    {
        zValues[i] = -p.R + i * p.zStep;
    }

    p.e = energyValue;
    Euler(p, eulerValues, zValues);
    return eulerValues.back();
}

bool isSignsEqual(double left, double right)
{
    return copysign(1.0, left) == copysign(1.0, right);
}

double StaticEnergyLevel(SystemParams &p, std::vector<double> &phaseFuncValues)
{
    double e = 1e-8;
    double offset = -(2 * p.k + 1) * M_PI_2;

    double l = p.eMin;
    double r = p.eMax;
    double m;

    double fl = PhaseFunc(p, l);
    double fr = PhaseFunc(p, r);
    double fm;
    while(abs(l - r) > e)
    {
        m = (l + r)/2.;
        fm = PhaseFunc(p, m);

        if(!isSignsEqual(fl - offset, fm - offset))
        {
            r = m;
            fr = fm;
        }
        else
        {
            l = m;
            fl = fm;
        }
    }

    return (l + r)/2;
}

void MainWindow::on_startBtn_clicked()
{
    ui->uPlot->clearGraphs();
    ui->uPlot->addGraph();
    ui->phaseFuncPlot->clearGraphs();
    ui->phaseFuncPlot->addGraph();
    ui->phaseFuncPlot->addGraph();

    p.R = ui->rEdit->text().toDouble();
    p.U0 = ui->u0Edit->text().toDouble();
    p.a = ui->wellSizeEdit->text().toDouble();
    p.zStep = ui->zStepEdit->text().toDouble();
    p.eStep = ui->eStepEdit->text().toDouble();
    p.eMin = ui->eMinEdit->text().toDouble();
    p.eMax = ui->eMaxEdit->text().toDouble();

    p.zCount = static_cast<unsigned int>(2*p.R/ p.zStep + 1);
    p.eCount = static_cast<unsigned int>((p.eMax - p.eMin)/ p.eStep + 1);

    for(unsigned int i = 0; i < p.zCount; i++)
    {
        ui->uPlot->graph(0)->addData(-p.R + i * p.zStep, PotentialEnergy(p, -p.R + i * p.zStep));
    }

    phaseFuncValues.resize(p.eCount);
    energyValues.resize(p.eCount);

    PhaseFunc(p, phaseFuncValues, energyValues);

    for(unsigned int i = 0; i < p.eCount; i++)
    {
        ui->phaseFuncPlot->graph(0)->addData(energyValues[i], phaseFuncValues[i]);
    }

    ui->uPlot->rescaleAxes();
    ui->uPlot->replot();
    ui->phaseFuncPlot->rescaleAxes();
    ui->phaseFuncPlot->replot();
}

void MainWindow::on_waveBtn_clicked()
{
    ui->waveFuncPlot->clearGraphs();
    ui->waveFuncPlot->addGraph();

    p.k = ui->kEdit->text().toInt();
    p.e = StaticEnergyLevel(p, phaseFuncValues);
    ui->phaseFuncPlot->graph(1)->setData({energyValues.front(), energyValues.back()}, {-(2. * p.k + 1.) * M_PI_2, -(2. * p.k + 1.) * M_PI_2});

    std::vector<double> phaseValues(p.zCount);
    phaseValues[0] = M_PI_2;
    std::vector<double> radialValues(p.zCount);
    radialValues[0] = 2;
    std::vector<double> zValues(p.zCount);
    for(unsigned int i = 0; i < p.zCount; i++)
    {
        zValues[i] = -p.R + i * p.zStep;
    }

    Euler(p, phaseValues, zValues);
    Euler(p, radialValues, zValues, phaseValues);

    for(unsigned int i = 0; i < p.zCount; i++)
    {
        ui->waveFuncPlot->graph(0)->addData(zValues[i], radialValues[i] * cos(phaseValues[i]));
    }

    ui->waveFuncPlot->rescaleAxes();
    ui->waveFuncPlot->replot();
    ui->phaseFuncPlot->replot();
}

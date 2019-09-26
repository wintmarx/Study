#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <complex>

using namespace std;


const int signalLength = 500;
int n1 = 150;
int n2 = 350;
double w1 = 0.2;
double w2 = 0.3;
double w0 = 0.1;
int L;

double signalValues[signalLength];
double epsilonValues[signalLength];
double noiseValues[signalLength];
double roundedSignalValues[signalLength];

double clearSignalEnergy;
double noisySignalEnergy;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->cleanPlot->xAxis->setRange(0, signalLength);
    ui->cleanPlot->yAxis->setRange(-16, 16);
    ui->cleanPlot->setInteraction(QCP::iRangeDrag, true);
    ui->cleanPlot->setInteraction(QCP::iRangeZoom, true);
    ui->cleanPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->epsilonPlot->xAxis->setRange(0, signalLength);
    ui->epsilonPlot->yAxis->setRange(0, 3.5);
    ui->epsilonPlot->setInteraction(QCP::iRangeDrag, true);
    ui->epsilonPlot->setInteraction(QCP::iRangeZoom, true);
    ui->epsilonPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->fourierPlot->xAxis->setRange(0, signalLength);
    ui->fourierPlot->yAxis->setRange(0, 1);
    ui->fourierPlot->setInteraction(QCP::iRangeDrag, true);
    ui->fourierPlot->setInteraction(QCP::iRangeZoom, true);
    ui->fourierPlot->axisRect()->setRangeZoom(Qt::Horizontal);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void GenerateNoise(double signalEnergy, double noisePart)
{
    srand(time(NULL) | clock());
    int randCount = 20;
    double mult = 0;
    noisySignalEnergy = 0;
    for(int i = 0; i < signalLength; i++)
    {
        noiseValues[i] = 0;
        for(int j = 0; j < randCount; j++)
        {
            noiseValues[i] += rand() * 2./RAND_MAX - 1.;
        }
        noiseValues[i] /= randCount;
        mult += noiseValues[i] * noiseValues[i];
    }
    mult = sqrt(signalEnergy * noisePart / (100 * mult));
    for(int i = 0; i < signalLength; i++)
    {
        noiseValues[i] *= mult;
        noisySignalEnergy += pow(noiseValues[i] + signalValues[i], 2);
    }
}

void MainWindow::on_genButton_clicked()
{
    ui->cleanPlot->clearGraphs();
    ui->epsilonPlot->clearGraphs();
    ui->fourierPlot->clearGraphs();

    ui->cleanPlot->addGraph();
    ui->cleanPlot->addGraph();
    clearSignalEnergy = 0;
    n1 = ui->n1Edit->text().toInt();
    n2 = ui->n2Edit->text().toInt();
    w1 = ui->w1Edit->text().toDouble();
    w2 = ui->w2Edit->text().toDouble();
    w0 = ui->w0Edit->text().toDouble();
    L = (int)(ui->upperLEdit->text().toDouble()/w0);
    double phase = 0;
    for(int i = 0; i<signalLength; i++)
    {
        if(i < n1)
        {
            signalValues[i] = 10 * sin(w1 * i);
        }
        else if(i == n1)
        {
            phase = (w1 - w0) * i;
            signalValues[i] = 10 *sin(w0 * i + phase);
        }
        else if(i < n2)
        {
            signalValues[i] = 10 *sin(w0 * i + phase);
        }
        else if(i == n2)
        {
            phase = (w0 - w2) * i + phase;
            signalValues[i] = 10 *sin(w2 * i + phase);
        }
        else
        {
            signalValues[i] = 10 *sin(w2 * i + phase);
        }
        clearSignalEnergy += pow(signalValues[i], 2);
    }

    GenerateNoise(clearSignalEnergy, ui->noisePartEdit->text().toDouble());

    for(int i = 0; i < signalLength; i++)
    {
        signalValues[i] += noiseValues[i];
        ui->cleanPlot->graph(0)->addData(i,  signalValues[i]);
    }

    epsilonValues[0] = 0;
    epsilonValues[1] = 0;

    double a1= -2 * cos(w0);
    double a2 = 1;
    double min = 0;
    double max = 0;
    ui->epsilonPlot->addGraph();
    for(int i = 2; i<signalLength; i++)
    {
        epsilonValues[i] = signalValues[i] + a1 * signalValues[i-1] + a2 * signalValues[i-2];
        ui->epsilonPlot->graph(0)->addData(i,  pow(epsilonValues[i], 2));
    }

    ui->fourierPlot->addGraph();
    ui->fourierPlot->addGraph();
    for(int i = 0; i < signalLength - L; i++)
    {
        roundedSignalValues[i] = 0;
        for(int j = 0; j < L; j++)
        {
          roundedSignalValues[i] += pow(epsilonValues[i + j], 2);
        }
        roundedSignalValues[i] /= 1. * L;

        if(i == 0)
        {
            max = roundedSignalValues[i];
            min = roundedSignalValues[i];
        }

        if(roundedSignalValues[i] < min)
            min = roundedSignalValues[i];

        if(roundedSignalValues[i] > max)
            max = roundedSignalValues[i];

        ui->fourierPlot->graph(0)->addData(i, roundedSignalValues[i]);
    }

    ui->fourierPlot->yAxis->setRange(min, max);

    int n1 = -1;
    int n2 = -1;

    double pLevel = ui->pLevelEdit->text().toDouble();

    ui->fourierPlot->graph(1)->setPen(QPen(QColor(255, 0,0)));
    ui->fourierPlot->graph(1)->addData(0, pLevel);
    ui->fourierPlot->graph(1)->addData(signalLength, pLevel);
    for(int i =0; i < signalLength - L - 1; i++)
    {
        if(roundedSignalValues[i] < pLevel && roundedSignalValues[i + 1] >= pLevel)
        {
                n2 = i;
        }
        else if(roundedSignalValues[i] >= pLevel && roundedSignalValues[i + 1] < pLevel)
        {
                n1 = i;
        }
    }

    ui->n1CalcEdit->setText(QString::number(n1));
    ui->n2CalcEdit->setText(QString::number(n2));
    ui->cleanPlot->replot();
    ui->epsilonPlot->replot();
    ui->fourierPlot->replot();
}

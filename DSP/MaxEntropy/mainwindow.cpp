#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mhjcalc.h"
#include <math.h>
#include <time.h>
#include <vector>

using namespace std;

const int signalLength = 60;
float inputValues[signalLength];
float noiseValues[signalLength];
float outputValues[signalLength];
float filterValues[signalLength];
float calcInputValues[signalLength];
float clearSignalEnergy;
float noiseEnergy;
MHJCalc *mhjCalc;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->cleanPlot->setInteraction(QCP::iRangeDrag, true);
    ui->cleanPlot->setInteraction(QCP::iRangeZoom, true);
    ui->cleanPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->noisyPlot->setInteraction(QCP::iRangeDrag, true);
    ui->noisyPlot->setInteraction(QCP::iRangeZoom, true);
    ui->noisyPlot->axisRect()->setRangeZoom(Qt::Horizontal);

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
    noiseEnergy = 0;
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
        noiseEnergy += pow(noiseValues[i], 2);
    }
}

double signalFunc(double a, double t, double t0, double d)
{
    return a * exp(-pow((t - t0)/d, 2));
}

void MainWindow::redrawCalcInputSignal(float F)
{
    ui->cleanPlot->graph(1)->data().data()->clear();
    for(int i = 0; i < signalLength; i++)
    {
        ui->cleanPlot->graph(1)->addData(i, calcInputValues[i]);
    }
    ui->cleanPlot->rescaleAxes(true);
    ui->cleanPlot->replot();
    ui->fEdit->setText(QString::number(F));
}

void MainWindow::onMHJFinished(float F)
{
    redrawCalcInputSignal(F);
    delete mhjCalc;
    ui->genButton->setEnabled(true);
    ui->calcButton->setEnabled(true);
    float delta = 0;
    for(int i = 0; i < signalLength; i++)
    {
        delta += pow(calcInputValues[i] - inputValues[i], 2);
    }
    ui->deltaEdit->setText(QString::number(delta));
}

void MainWindow::on_genButton_clicked()
{
    ui->cleanPlot->clearGraphs();
    ui->noisyPlot->clearGraphs();
    ui->fourierPlot->clearGraphs();

    //generate input signal
    ui->cleanPlot->addGraph();
    for(int i = 0; i < signalLength; i++)
    {
        inputValues[i] = 0;
        for(int j = 0; j < ui->signalsTable->rowCount(); j++)
        {
            inputValues[i] += signalFunc(ui->signalsTable->item(j,0)->text().toDouble(),
                                    i,
                                    ui->signalsTable->item(j,1)->text().toDouble(),
                                    ui->signalsTable->item(j,2)->text().toDouble());
        }
        ui->cleanPlot->graph(0)->addData(i,  inputValues[i]);
    }

    //generate filter
    ui->fourierPlot->addGraph();
    for(int i = 0; i < signalLength; i++)
    {
        filterValues[i] = signalFunc(1, i, (i < signalLength/2 ? 0 : signalLength - 1), ui->dHEdit->text().toDouble());
        ui->fourierPlot->graph(0)->addData(i,  filterValues[i]);
    }

    //calculate output
    ui->noisyPlot->addGraph();
    clearSignalEnergy = 0;
    for(int i = 0; i < signalLength; i++)
    {
        outputValues[i] = 0;
        for(int j = 0; j < signalLength; j++)
        {
            outputValues[i] += filterValues[i - j + (i < j ? signalLength : 0)] * inputValues[j];
        }
        clearSignalEnergy += pow(outputValues[i], 2);
    }

    GenerateNoise(clearSignalEnergy, ui->noisePartEdit->text().toDouble());

    for(int i = 0; i < signalLength; i++)
    {
        outputValues[i] += noiseValues[i];
        ui->noisyPlot->graph(0)->addData(i,  outputValues[i]);
    }

    ui->cleanPlot->rescaleAxes(true);
    ui->cleanPlot->replot();
    ui->noisyPlot->rescaleAxes(true);
    ui->noisyPlot->replot();
    ui->fourierPlot->rescaleAxes(true);
    ui->fourierPlot->replot();
}

void MainWindow::on_addRowButton_clicked()
{
    if(ui->signalsTable->rowCount() < 5)
        ui->signalsTable->setRowCount(ui->signalsTable->rowCount()+1);
}

void MainWindow::on_delRowButton_clicked()
{
    if(ui->signalsTable->rowCount() > 1)
        ui->signalsTable->setRowCount(ui->signalsTable->rowCount()-1);
}

void MainWindow::on_calcButton_clicked()
{
    ui->genButton->setEnabled(false);
    ui->calcButton->setEnabled(false);
    ui->cleanPlot->addGraph();
    ui->cleanPlot->graph(1)->setPen(QPen(QColor(255, 0, 0)));

    mhjCalc = new MHJCalc(signalLength, outputValues, filterValues, calcInputValues);
    QObject::connect(mhjCalc, SIGNAL(valuesReady(float)), this, SLOT(redrawCalcInputSignal(float)));
    QObject::connect(mhjCalc, SIGNAL(calcFinished(float)), this, SLOT(onMHJFinished(float)));
    mhjCalc->start();
}

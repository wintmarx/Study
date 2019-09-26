#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>

using namespace std;

const int signalLength = 512;
float inputValues[signalLength];
float inputSignalAmp[signalLength];
float clearSignalEnergy;
float noiseValues[signalLength];
float noiseEnergy;
float calcInputValues[signalLength];

PhaseApproxCalc *phaseApproxCalc;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->cleanPlot->setInteraction(QCP::iRangeDrag, true);
    ui->cleanPlot->setInteraction(QCP::iRangeZoom, true);
    ui->cleanPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->fourierPlot->setInteraction(QCP::iRangeDrag, true);
    ui->fourierPlot->setInteraction(QCP::iRangeZoom, true);
    ui->fourierPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    srand(time(NULL) | clock());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void GenerateNoise(double signalEnergy, double noisePart)
{
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

void MainWindow::redrawCalcInputSignal(QWaitCondition *cond)
{
    ui->cleanPlot->graph(1)->data().data()->clear();
    for(int i = 0; i < signalLength; i++)
    {
        ui->cleanPlot->graph(1)->addData(i, calcInputValues[i]);
    }
    ui->cleanPlot->rescaleAxes(true);
    ui->cleanPlot->replot();
    cond->wakeOne();
}

void MainWindow::onPhaseApproxFinished()
{ 
    ui->genButton->setEnabled(true);
    ui->calcButton->setEnabled(true);
    float delta = 0;
    for(int i = 0; i < signalLength; i++)
    {
        delta += pow(calcInputValues[i] - inputValues[i], 2);
    }
    ui->deltaEdit->setText(QString::number(delta));
    delete phaseApproxCalc;
}

double signalFunc(double a, double t, double t0, double d)
{
    return a * exp(-pow((t - t0)/d, 2));
}

/**
 * @brief on gen button clicked
 *
 * @details Generates initial data: input signal, noize, input signal spectrum amplitude
 *          Plots initial data.
 */
void MainWindow::on_genButton_clicked()
{  
    ui->fourierPlot->clearGraphs();
    ui->fourierPlot->addGraph();
    ui->cleanPlot->clearGraphs();
    ui->cleanPlot->addGraph();
    ui->cleanPlot->addGraph();
    ui->cleanPlot->graph(1)->setPen(QPen(QColor(255, 0, 0)));

    //generate input signal
    clearSignalEnergy = 0;
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
        //ui->cleanPlot->graph(0)->addData(i,  inputValues[i]);
        clearSignalEnergy += pow(inputValues[i], 2);
    }

    //Add noise
    GenerateNoise(clearSignalEnergy, ui->noisePartEdit->text().toDouble());
    vcd spectrum;

    //calc amp of input signal
    for(int i = 0; i < signalLength; i++)
    {
        inputValues[i] += noiseValues[i];
        inputValues[i] = fabs(inputValues[i]);
        ui->cleanPlot->graph(0)->addData(i,  inputValues[i]);
        spectrum.push_back(cd(inputValues[i], 0));
    }
    spectrum = PhaseApproxCalc::fft(spectrum);
    float tmp;
    for(int i = 0; i<signalLength; i++)
    {
        tmp = sqrt(pow(spectrum[i].imag(), 2) + pow(spectrum[i].real(), 2));
        inputSignalAmp[i] = tmp;
        ui->fourierPlot->graph(0)->addData(i * 1./signalLength, tmp);
    }

    spectrum.clear();
    ui->cleanPlot->rescaleAxes(true);
    ui->cleanPlot->replot();
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
    phaseApproxCalc = new PhaseApproxCalc(signalLength, inputSignalAmp, inputValues, calcInputValues);
    QObject::connect(phaseApproxCalc, SIGNAL(valuesReady(QWaitCondition*)), this, SLOT(redrawCalcInputSignal(QWaitCondition*)));
    QObject::connect(phaseApproxCalc, SIGNAL(calcFinished()), this, SLOT(onPhaseApproxFinished()));
    phaseApproxCalc->start();
}

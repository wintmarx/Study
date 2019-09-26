#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>

using namespace std;

WaveCalcThread *waveCalcThread;
vector<vcd> waveSamples;
SystemParams p;
vector<vcd> spectrum;
bool isThreadAnimated;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->wavePlot->setInteraction(QCP::iRangeDrag, true);
    ui->wavePlot->setInteraction(QCP::iRangeZoom, true);
    ui->wavePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    ui->cutPlot->setInteraction(QCP::iRangeDrag, true);
    ui->cutPlot->setInteraction(QCP::iRangeZoom, true);
    ui->cutPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    ui->fourierPlot->setInteraction(QCP::iRangeDrag, true);
    ui->fourierPlot->setInteraction(QCP::iRangeZoom, true);
    ui->fourierPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);

    waveCalcThread = new WaveCalcThread(&waveSamples);
    QObject::connect(waveCalcThread, SIGNAL(valuesReady(QWaitCondition*)), this, SLOT(redrawCalcInputSignal(QWaitCondition*)));
    QObject::connect(waveCalcThread, SIGNAL(calcFinished()), this, SLOT(onPhaseApproxFinished()));

    isThreadAnimated = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

unsigned int animationTime = 0;

void MainWindow::redrawCalcInputSignal(QWaitCondition *cond)
{
    ui->wavePlot->graph(0)->data().data()->clear();
    for(unsigned int i = 0; i < p.xCount; i++)
    {
        ui->wavePlot->graph(0)->addData(-p.r + i * p.xStep, waveSamples[animationTime][i].real());
    }
    if(++animationTime > p.tCount - 1)
    {
        animationTime = 0;
    }
    //ui->wavePlot->rescaleAxes(true);
    ui->wavePlot->replot();
    cond->wakeOne();
}

void MainWindow::onPhaseApproxFinished()
{ 

}

void MainWindow::on_genButton_clicked()
{  
    ui->wavePlot->clearGraphs();
    ui->wavePlot->addGraph();
    ui->wavePlot->addGraph();

    p.a = ui->aEdit->text().toDouble();
    p.integrationStep = ui->integrationStepEdit->text().toDouble();
    p.r = ui->rEdit->text().toDouble();
    p.sigma = ui->sigmaEdit->text().toDouble();
    p.xStep = ui->xStepEdit->text().toDouble();
    p.tMax = ui->tMaxEdit->text().toDouble();
    p.tStep = static_cast<unsigned int>(p.tMax/ p.tCount);
    p.tCount = ui->tCountEdit->text().toUInt();
    p.xCount = static_cast<unsigned int>(2. * p.r/ p.xStep + 1);

    waveCalcThread->CalcWave(p);

    ui->waveSlider->setRange(0, p.xCount);

    for(unsigned int i = 0; i < p.xCount; i++)
    {
        ui->wavePlot->graph(0)->addData(-p.r + i * p.xStep, waveSamples[0][i].real());
    }
    ui->wavePlot->rescaleAxes(true);
    ui->wavePlot->replot();
}

void MainWindow::on_calcButton_clicked()
{
    ui->fourierPlot->clearGraphs();
    ui->fourierPlot->addGraph();
    ui->fourierPlot->addGraph();

    spectrum.clear();
    spectrum.resize(p.xCount);

    for(unsigned int ix = 0; ix < p.xCount; ix++)
    {

        spectrum[ix].resize(p.tCount);
        for(unsigned int it = 0; it < p.tCount; it++)
        {
            spectrum[ix][it] = waveSamples[it][ix];
        }

        spectrum[ix] = WaveCalcThread::fft(spectrum[ix]);
    }

    unsigned int layer = ui->waveSlider->value();

    for(unsigned int it = 0; it < p.tCount; it++)
    {
        ui->fourierPlot->graph(0)->addData(it, sqrt(pow(spectrum[layer][it].imag(), 2) + pow(spectrum[layer][it].real(), 2)));
    }

    ui->fourierSlider->setRange(0, p.tCount);
    ui->fourierSlider->setTickInterval(1);

    //spectrum.clear();

    ui->fourierPlot->rescaleAxes(true);
    ui->fourierPlot->replot();
}

void MainWindow::on_animateButton_clicked()
{
    if(!isThreadAnimated)
    {
        animationTime = 0;
        isThreadAnimated = true;
        waveCalcThread->start();
    }
    else
    {
        isThreadAnimated = false;
        waveCalcThread->stopAnimation();
    }
}

void MainWindow::on_waveSlider_valueChanged(int value)
{
    ui->wavePlot->graph(1)->data().data()->clear();
    ui->wavePlot->graph(1)->addData(-p.r + value * p.xStep, 0);
    ui->wavePlot->graph(1)->addData(-p.r + value * p.xStep, 1);
    ui->wavePlot->replot();
}

void MainWindow::on_fourierSlider_valueChanged(int value)
{
    ui->fourierPlot->graph(1)->data().data()->clear();
    ui->fourierPlot->graph(1)->addData(value, 0);
    ui->fourierPlot->graph(1)->addData(value, 1000);
    ui->fourierPlot->replot();
}

void MainWindow::on_cutButton_clicked()
{
    ui->cutPlot->clearGraphs();
    ui->cutPlot->addGraph();

    for(unsigned int ix = 0; ix < p.xCount; ix++)
    {
        ui->cutPlot->graph(0)->addData(-p.r + ix * p.xStep,
                                       sqrt(pow(spectrum[ix][ui->fourierSlider->value()].real(),2) +
                                       pow(spectrum[ix][ui->fourierSlider->value()].imag(),2)));
    }

    ui->cutPlot->rescaleAxes(true);
    ui->cutPlot->replot();
}

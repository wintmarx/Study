#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>

using namespace std;

HeatCalcThread *heatCalcThread;
SystemParams p;
bool isThreadAnimated;

QCPColorMap *heatMap;
QCPMarginGroup *marginGroup;
QCPColorScale *colorScale;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->heatPlot->setInteraction(QCP::iRangeDrag, true);
    ui->heatPlot->setInteraction(QCP::iRangeZoom, true);
    ui->heatPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
    ui->heatPlot->axisRect()->setupFullAxesBox(true);

    heatMap = new QCPColorMap(ui->heatPlot->xAxis, ui->heatPlot->yAxis);


    colorScale = new QCPColorScale(ui->heatPlot);
    ui->heatPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    heatMap->setColorScale(colorScale); // associate the color map with the color scale

    QCPColorGradient gr(QCPColorGradient::gpJet);
    //gr.setLevelCount(8);
    heatMap->setGradient(gr);//QCPColorGradient::gpHot/*gpJet*/);
    //heatMap->setInterpolate(false);

    marginGroup = new QCPMarginGroup(ui->heatPlot);
    ui->heatPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    heatGridSamples = new vector<matrix>();
    heatCalcThread = new HeatCalcThread(heatGridSamples);
    QObject::connect(heatCalcThread, SIGNAL(valuesReady(QWaitCondition*)), this, SLOT(redrawCalcInputSignal(QWaitCondition*)));
    QObject::connect(heatCalcThread, SIGNAL(calcFinished()), this, SLOT(onPhaseApproxFinished()));

    isThreadAnimated = false;

    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

unsigned int animationTime = 0;

unsigned int progressValue = 0;

void MainWindow::redrawCalcInputSignal(QWaitCondition *cond)
{
    ui->timeSlider->setValue(animationTime++);
    if(animationTime > p.tCount - 1)
    {
        animationTime = 0;
    }


    cond->wakeOne();
}

void MainWindow::onPhaseApproxFinished()
{ 
    progressValue++;
    ui->progressBar->setValue(progressValue);
}

void MainWindow::DrawHeatMap(unsigned int time)
{
    //heatMap->data()->fill(0);
    for (unsigned int iy = 0; iy < p.yCount; iy++)
    {
        for (unsigned int ix = 0; ix < p.xCount; ix++)
        {
            if(ui->cBox->isChecked())
            {
                heatMap->data()->setCell(ix, iy, heatCalcThread->c[iy][ix]);
            }
            else if(ui->qBox->isChecked())
            {
                heatMap->data()->setCell(ix, iy, heatCalcThread->q[iy][ix]);
            }
            else if(ui->tempBox->isChecked())
            {
                heatMap->data()->setCell(ix, iy, heatCalcThread->xBorders[iy][ix] + heatCalcThread->yBorders[iy][ix] + /*heatCalcThread->q[iy][ix]  + heatCalcThread->k[iy][ix] +*/ (*heatGridSamples)[time][iy][ix]);//(*heatGridSamples)[time][iy][ix]);
            }
            else if(ui->boundsBox->isChecked())
            {
                heatMap->data()->setCell(ix, iy, heatCalcThread->xBorders[iy][ix] + heatCalcThread->yBorders[iy][ix]);
            }
            else if(ui->kBox->isChecked())
            {
                heatMap->data()->setCell(ix, iy, heatCalcThread->k[iy][ix]);
            }

        }
    }
    //heatMap->rescaleDataRange();
    ui->heatPlot->rescaleAxes(true);
    ui->heatPlot->replot();
}

void MainWindow::on_genButton_clicked()
{  
    ui->heatPlot->clearGraphs();
    ui->heatPlot->addGraph();

    p.tStep = ui->tStepEdit->text().toDouble();
    p.tMax = ui->tMaxEdit->text().toDouble();
    p.tCount = static_cast<unsigned int>(p.tMax/ p.tStep + 1);

    p.xMax = ui->xMaxEdit->text().toDouble();
    p.xCount = ui->xCountEdit->text().toUInt();
    p.xStep = p.xMax/ p.xCount;

    p.yMax = ui->yMaxEdit->text().toDouble();
    p.yCount = ui->yCountEdit->text().toUInt();
    p.yStep = p.yMax/ p.yCount;

    p.cm = ui->cmEdit->text().toDouble();
    p.ch = ui->chEdit->text().toDouble();
    p.cl = ui->clEdit->text().toDouble();

    p.tempEnv = ui->tEnvEdit->text().toDouble();
    p.cEnv = ui->cEnvEdit->text().toDouble();
    p.kEnv = ui->kEnvEdit->text().toDouble();

    p.heaterW = ui->heaterWEdit->text().toDouble();
    p.heaterH = ui->heaterHEdit->text().toDouble();
    p.heaterP = ui->heaterPEdit->text().toDouble();

    p.km = ui->kmEdit->text().toDouble();
    p.kl = ui->klEdit->text().toDouble();
    p.kh = ui->khEdit->text().toDouble();

    p.wallWidth = ui->wallWidthEdit->text().toDouble();

    p.delta=sqrt(p.kEnv*p.tempEnv/p.cEnv);

    p.eps = p.kEnv/ p.delta;

    heatMap->data()->setSize(p.xCount, p.yCount);
    heatMap->data()->setRange(QCPRange(0, p.xMax), QCPRange(0, p.yMax));
    heatMap->setDataRange(QCPRange(295, 400));

    ui->progressBar->setRange(0, p.tCount - 1);
    progressValue = 0;

    ui->timeSlider->setRange(0, p.tCount - 1);

    heatCalcThread->Calc(p);
}

void MainWindow::on_animateButton_clicked()
{
    if(!isThreadAnimated)
    {
        animationTime = 0;
        isThreadAnimated = true;
        heatCalcThread->StartAnimation();
    }
    else
    {
        isThreadAnimated = false;
        heatCalcThread->StopAnimation();
    }
}

void MainWindow::on_timeSlider_valueChanged(int value)
{
    DrawHeatMap(value);
}


void MainWindow::on_qBox_clicked()
{
    ui->qBox->setChecked(true);
    ui->cBox->setChecked(false);
    ui->tempBox->setChecked(false);
    ui->kBox->setChecked(false);
    ui->boundsBox->setChecked(false);
    heatMap->setDataRange(QCPRange(0, 1000000));
    DrawHeatMap(animationTime);
}

void MainWindow::on_kBox_clicked()
{
    ui->kBox->setChecked(true);
    ui->cBox->setChecked(false);
    ui->tempBox->setChecked(false);
    ui->qBox->setChecked(false);
    ui->boundsBox->setChecked(false);
    heatMap->setDataRange(QCPRange(0, 100));
    DrawHeatMap(animationTime);
}

void MainWindow::on_cBox_clicked()
{
    ui->cBox->setChecked(true);
    ui->qBox->setChecked(false);
    ui->tempBox->setChecked(false);
    ui->kBox->setChecked(false);
    ui->boundsBox->setChecked(false);
    heatMap->setDataRange(QCPRange(400, 500));
    DrawHeatMap(animationTime);
}

void MainWindow::on_tempBox_clicked()
{
    ui->tempBox->setChecked(true);
    ui->cBox->setChecked(false);
    ui->qBox->setChecked(false);
    ui->kBox->setChecked(false);
    ui->boundsBox->setChecked(false);
    heatMap->setDataRange(QCPRange(295, 400));
    DrawHeatMap(animationTime);
}

void MainWindow::on_boundsBox_clicked()
{
    ui->boundsBox->setChecked(true);
    ui->cBox->setChecked(false);
    ui->tempBox->setChecked(false);
    ui->kBox->setChecked(false);
    ui->qBox->setChecked(false);
    heatMap->setDataRange(QCPRange(0, 1.5));
    DrawHeatMap(animationTime);
}

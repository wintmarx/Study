 #include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    srand(static_cast<uint>(time(nullptr)));
    connect(&diodeCalcThread, SIGNAL(valuesReady(QWaitCondition*)), this, SLOT(redrawCalcData(QWaitCondition*)));
    connect(&diodeCalcThread, SIGNAL(calcFinished()), this, SLOT(onCalcFinished()));

    ui->ivPlot->setInteraction(QCP::iRangeDrag, true);
    ui->ivPlot->setInteraction(QCP::iRangeZoom, true);
    ui->ivPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::redrawCalcData(QWaitCondition *cond)
{
    ui->bifurPlot->setData(&diodeCalcThread.tris, &diodeCalcThread.points);
    ui->bifurPlot->setParticles(&diodeCalcThread.particles);
    ui->bifurPlot->update();
    cond->wakeOne();
}

void MainWindow::onCalcFinished()
{
    ui->ivPlot->clearGraphs();
    ui->ivPlot->addGraph();
    for(uint i = 0; i < diodeCalcThread.iv.size(); i++)
    {
        ui->ivPlot->graph(0)->addData(diodeCalcThread.iv[i].first, diodeCalcThread.iv[i].second);
    }
    ui->ivPlot->rescaleAxes(true);
    ui->ivPlot->replot();
}

void MainWindow::on_startBtn_clicked()
{

}

void MainWindow::on_redrawBtn_clicked()
{


}

void MainWindow::on_calcBtn_clicked()
{
    SystemParams p;

    p.leftPotential = ui->vLeftEdit->text().toFloat();
    p.rightPotential = ui->vRightEdit->text().toFloat();
    p.width = ui->bifurPlot->width() / 10000.f;
    p.height = ui->bifurPlot->height() / 10000.f;
    p.charge = ui->chargeEdit->text().toFloat();
    p.mass = ui->massEdit->text().toFloat();
    p.dt = ui->dtEdit->text().toFloat();
    p.particlesAmount = ui->particlesAmountEdit->text().toUInt();

    // Grid params
    p.gridDeviation = ui->deviationEdit->text().toFloat() / 10000.f;
    p.stepsCountX = ui->gridSizeXEdit->text().toUInt();
    p.stepsCountY = ui->gridSizeYEdit->text().toUInt();
    p.activeGridSize = ui->activeGridSizeEdit->text().toUInt();
    p.activeGridStart = static_cast<uint>(round((p.stepsCountY + 1 - p.activeGridSize)/2.));

    p.uMax =  ui->uMaxEdit->text().toFloat();
    p.uMin =  ui->uMinEdit->text().toFloat();
    p.uStepsCount =  ui->uStepsCountEdit->text().toUInt();

    diodeCalcThread.Calc(p);
}

void MainWindow::on_plotIVBtn_clicked()
{

    SystemParams p;

    p.leftPotential = ui->vLeftEdit->text().toFloat();
    p.rightPotential = ui->vRightEdit->text().toFloat();
    p.width = ui->bifurPlot->width()/10000.f;
    p.height = ui->bifurPlot->height()/10000.f;
    p.charge = ui->chargeEdit->text().toFloat();
    p.mass = ui->massEdit->text().toFloat();
    p.dt = ui->dtEdit->text().toFloat();
    p.particlesAmount = ui->particlesAmountEdit->text().toUInt();

    // Grid params
    p.gridDeviation = ui->deviationEdit->text().toFloat()/10000.f;
    p.stepsCountX = ui->gridSizeXEdit->text().toUInt();
    p.stepsCountY = ui->gridSizeYEdit->text().toUInt();
    p.activeGridSize = ui->activeGridSizeEdit->text().toUInt();
    p.activeGridStart = static_cast<uint>(round((p.stepsCountY + 1 - p.activeGridSize)/2.));

    p.uMax =  ui->uMaxEdit->text().toFloat();
    p.uMin =  ui->uMinEdit->text().toFloat();
    p.uStepsCount =  ui->uStepsCountEdit->text().toUInt();

    diodeCalcThread.CalcIV(p);
}

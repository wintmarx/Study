#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <complex>
#include "semiconductorplatesolver.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->viPlot->xAxis->setRange(0, 100);
    ui->viPlot->yAxis->setRange(-16, 16);
    ui->viPlot->setInteraction(QCP::iRangeDrag, true);
    ui->viPlot->setInteraction(QCP::iRangeZoom, true);
    ui->viPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startBtn_clicked()
{
    ui->viPlot->clearGraphs();
    ui->viPlot->addGraph();

    Problem p;
    p.eps = 0.001;
    p.is_left_field = false;
    p.left = 0;
    p.length = ui->widthSiEdit->text().toDouble();
    p.n0 = ui->nBEdit->text().toDouble();
    p.nA = 0;
    p.nD = ui->nPEdit->text().toDouble();
    p.right = 0;
    p.size = ui->sizeEdit->text().toInt();

    SemiconductorPlateSolver solver(p);
    std::vector<double> values;
    std::vector<double> keys;

    solver.solveVIChar(values, keys, ui->vMinEdit->text().toDouble(), ui->vMaxEdit->text().toDouble());

    for(unsigned int i = 0 ; i < keys.size(); i++)
    {
        ui->viPlot->graph(0)->addData(keys[i], values[i]);
    }

    ui->viPlot->rescaleAxes();
    ui->viPlot->replot();
}

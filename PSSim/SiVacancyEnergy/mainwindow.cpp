#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    simulation = new Simulation(ui->view);
    connect(simulation, SIGNAL(LoadingTick(float)), this, SLOT(UpdateLoadingBar(float)));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), simulation, SLOT(SetSimTimestep(int)));
    connect(simulation, SIGNAL(ActiveTimestepChanged(int)), this, SLOT(UpdateSlider(int)));
    connect(simulation, SIGNAL(DrawPlot(const QVector<double>*, const QVector<double>*, const QColor*)), this, SLOT(DrawPlot(const QVector<double>*, const QVector<double>*, const QColor*)));
    connect(simulation, SIGNAL(RemovePlots()), this, SLOT(RemovePlots()));
    simulation->start();
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    ui->plot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

MainWindow::~MainWindow()
{
    simulation->isActive = false;
    delete simulation;
    delete ui;
}

void MainWindow::UpdateLoadingBar(float percentage)
{
    ui->loadingBar->setValue(static_cast<int>(std::roundf(percentage * ui->loadingBar->maximum())));
}

void MainWindow::UpdateSlider(int value)
{
    ui->horizontalSlider->setValue(value);
}

void MainWindow::DrawPlot(const QVector<double> *keys, const QVector<double> *values, const QColor *color)
{
    ui->plot->addGraph();
    for (int i = 0; i < keys->size(); i++)
    {
        ui->plot->graph(ui->plot->graphCount() - 1)->addData(keys->at(i), values->at(i));
    }
    ui->plot->graph(ui->plot->graphCount() - 1)->setPen(QPen(*color));
    ui->plot->rescaleAxes();
    ui->plot->replot();
}

void MainWindow::RemovePlots()
{
    ui->plot->clearGraphs();
}

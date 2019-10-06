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
    simulation->start();
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

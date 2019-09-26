#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    simulation = new Simulation(ui->view);
    simulation->start();
}

MainWindow::~MainWindow()
{
    simulation->isActive = false;
    delete simulation;
    delete ui;
}

void MainWindow::on_close_clicked()
{
    simulation->isActive = false;
}

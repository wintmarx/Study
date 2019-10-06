#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "simulation.h"
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Simulation *simulation;

public slots:
    void UpdateLoadingBar(float percentage);
    void UpdateSlider(int value);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

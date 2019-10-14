#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "simulation.h"
#include "qcustomplot.h"

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
    void DrawPlot(const QVector<double> *keys, const QVector<double> *values, const QColor *color);
    void RemovePlots();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

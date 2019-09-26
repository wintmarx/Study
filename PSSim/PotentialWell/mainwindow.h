#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"

typedef struct {
    double R;
    double eMin;
    double eMax;
    double zStep;
    double U0;
    double a;
    double eStep;
    double e;
    int k;
    unsigned int eCount;
    unsigned int zCount;
    int align;
} SystemParams;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_startBtn_clicked();

    void on_waveBtn_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<double> phaseFuncValues;
    std::vector<double> energyValues;
    SystemParams p;
};

#endif // MAINWINDOW_H

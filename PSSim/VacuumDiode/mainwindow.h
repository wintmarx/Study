#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector3D>
#include <QVector2D>
#include <QTimer>
#include <set>
#include "diagramplot.h"
#include "diodecalc.h"

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
    void on_redrawBtn_clicked();
    void on_calcBtn_clicked();
    void redrawCalcData(QWaitCondition *cond);
    void onCalcFinished();

    void on_plotIVBtn_clicked();

private:
    DiodeCalcThread diodeCalcThread;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

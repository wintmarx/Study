#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "heatdistributioncalc.h"

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
   void on_genButton_clicked();
   
   void on_animateButton_clicked();

   void on_timeSlider_valueChanged(int value);

   void on_qBox_clicked();

   void on_kBox_clicked();

   void on_cBox_clicked();

   void on_tempBox_clicked();

   void on_boundsBox_clicked();

public slots:
    void redrawCalcInputSignal(QWaitCondition *cond);
    void onPhaseApproxFinished();
	
private:
    Ui::MainWindow *ui;
    void DrawHeatMap(unsigned int time);
    vector<matrix> *heatGridSamples;
};

#endif // MAINWINDOW_H

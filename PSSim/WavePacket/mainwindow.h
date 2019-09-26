#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "wavepacketcalc.h"

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
   
   void on_calcButton_clicked();

   void on_animateButton_clicked();

   void on_waveSlider_valueChanged(int value);

   void on_fourierSlider_valueChanged(int value);

   void on_cutButton_clicked();

public slots:
    void redrawCalcInputSignal(QWaitCondition *cond);
    void onPhaseApproxFinished();
	
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

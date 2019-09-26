#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include "qchartviewer.h"
#include "maxwellcalc.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void drawChart(QChartViewer *viewer);

    MaxwellCalc maxwell;

    // 3D view angles
    double m_elevationAngle;
    double m_rotationAngle;

    // Keep track of mouse drag
    int m_lastMouseX;
    int m_lastMouseY;
    bool m_isDragging;

private slots:
    void onViewPortChanged();
    void onMouseMoveChart(QMouseEvent *event);
    void onMouseUpChart(QMouseEvent *event);
    void on_calcButton_clicked();
    void on_slider_valueChanged(int value);
    void on_checkBox_stateChanged(int arg1);
};

#endif // MAINWINDOW_H

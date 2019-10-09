#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <cfloat>
#include <cmath>
#include <vector>

#include "qchartviewer.h"
#include "antennacell.h"

struct AntennasGrid
{
    std::vector<AntennaCell*> ant;
    uint sizeX;
    uint sizeY;
};

enum class SignalDrawMode
{
    Amp,
    Re,
    Im
};

typedef struct
{
    double re;
    double im;
    inline double Magnitude() const
    {
        return std::sqrt(MagnitudeSqr());
    }
    inline double MagnitudeSqr() const
    {
        return re * re + im * im;
    }
} Complex;

struct Pattern
{
    std::vector<Complex> data;
    double max = -DBL_MAX;
    double min = DBL_MAX;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_calcButton_clicked();
    void on_genGridButton_clicked();
    void onViewPortChanged();
    void onMouseMoveChart(QMouseEvent *event);
    void onMouseUpChart(QMouseEvent *event);

private:
    void Draw3DPattern(const Pattern &pattern, double dr, int size, QChartViewer *viewer);
    void Update3DPattern(double *dataX, double *dataZ, int size, QChartViewer *viewer);
    void DrawSignal(const Pattern &pattern, SignalDrawMode mode, int size, QGraphicsScene *scene, double scale = 1.);
    void GenerateGrid(AntennasGrid &grid, double w, double h, uint cellsx, uint cellsy);
    Ui::MainWindow *ui;
    AntennasGrid grid;

    // 3D view angles
    double m_elevationAngle;
    double m_rotationAngle;

    // Keep track of mouse drag
    int m_lastMouseX;
    int m_lastMouseY;
    bool m_isDragging;
};

#endif // MAINWINDOW_H

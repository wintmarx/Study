#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector3D>
#include <QVector2D>
#include <set>
#include "diagramplot.h"

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

private:
    Ui::MainWindow *ui;
    float randDeviation();
    void Triangulation(std::vector<Point> &points);
    void Triangulation(std::vector<Point> &points, std::vector<uint> &indices);
    void NeighborSearch();
    void CreateIsolines(float max, float min, uint stepsCount);
    QVector3D GetNormalCoeffs(Triangle &t);
    void eraseHSTris(std::vector<uint> &hsIndices);
    std::vector<Point> points;
    std::vector<Line> isolines;
    std::vector<std::set<uint>> pointsNeighbors;
    std::vector<Triangle> tris;
    QVector2D step;
    uint stepsCount;
    uint ringPointsCount;
    uint ringRadius;
    uint ringWidth;
    QVector3D ringCenter;
    float ringAngle;
    float gridDeviation;
};

#endif // MAINWINDOW_H

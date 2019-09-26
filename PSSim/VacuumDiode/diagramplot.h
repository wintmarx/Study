#ifndef DIAGRAMPLOT_H
#define DIAGRAMPLOT_H

#include <QWidget>
#include <QVector3D>
#include <QVector2D>
#include <set>

struct Triangle {
    uint p1;
    uint p2;
    uint p3;
    float r;
    QVector3D c;
};

struct Line {
    QVector2D p1;
    QVector2D p2;
};

struct Point : QVector3D {
    bool isBorder;
    bool isActive;
    float u;
    float ro;
    std::vector<uint> vCellIdx;
    float cellSquare;
};

struct Particle{
    QVector3D vel;
    QVector3D pos;
};

class DiagramPlot : public QWidget
{
    Q_OBJECT
public:
    explicit DiagramPlot(QWidget *parent = nullptr);
    bool isDataLoaded = false;
    void paintEvent(QPaintEvent *e);
    void setData(std::vector<Triangle> *triangles, std::vector<Point> *points);
    void setIsolines(std::vector<Line> *isolines);
    void setParticles(std::vector<Particle> *particles);
    std::vector<Triangle> *triangles;
    std::vector<Point> *points;
    std::vector<Line> *isolines;
    std::vector<Particle> *particles;

signals:

public slots:
};

#endif // DIAGRAMPLOT_H

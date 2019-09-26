#include "diagramplot.h"
#include <QPainter>

DiagramPlot::DiagramPlot(QWidget *parent) : QWidget(parent)
{
    isolines = nullptr;
    particles = nullptr;
    srand(time(NULL));
}

void DiagramPlot::paintEvent(QPaintEvent *e)
{
    if(isDataLoaded == false)
    {
        return;
    }
    //setAttribute(Qt::WA_OpaquePaintEvent);
    QPainter painter(this);
    QPen linepen(Qt::red);
    linepen.setCapStyle(Qt::RoundCap);
    linepen.setWidth(1);

    QPen dotpen(Qt::blue);
    dotpen.setCapStyle(Qt::RoundCap);
    dotpen.setWidth(5);

    QPen dotpenyellow(Qt::black);
    dotpenyellow.setCapStyle(Qt::RoundCap);
    dotpenyellow.setWidth(5);

    QPen circlepen(Qt::green);
    circlepen.setCapStyle(Qt::RoundCap);
    circlepen.setWidth(1);

    float sc = 10000;

    painter.setRenderHint(QPainter::Antialiasing,true);

    for(unsigned int i = 0; i < triangles->size(); i++)
    {
        painter.setPen(linepen);
        uint p1 = triangles->at(i).p1;
        uint p2 = triangles->at(i).p2;
        uint p3 = triangles->at(i).p3;
        if(!points->at(p1).isActive ||
           !points->at(p2).isActive ||
           !points->at(p3).isActive)
        {
            continue;
        }
        painter.drawLine(points->at(p1).toPointF() * sc, points->at(p2).toPointF() * sc);
        painter.drawLine(points->at(p1).toPointF() * sc, points->at(p3).toPointF() * sc);
        painter.drawLine(points->at(p2).toPointF() * sc, points->at(p3).toPointF() * sc);
        //painter.drawText(((points->at(p1) + points->at(p2) + points->at(p3))/3.).toPointF() * sc, QString::number(points->at(i).u));
        //painter.setPen(circlepen);
        //painter.drawEllipse(triangles->at(i).c, triangles->at(i).r, triangles->at(i).r);
    }

    for(uint i = 0; i < points->size(); i++)
    {
        painter.setPen(points->at(i).isActive ? dotpen : dotpenyellow);
        if(!points->at(i).isActive)
        {
            continue;
        }
        painter.drawPoint(points->at(i).toPointF() * sc);
        painter.drawText(points->at(i).toPointF() * sc, QString::number(points->at(i).u));
        //painter.drawText(points->at(i).toPointF() + QPointF(0, 10), QString::number(points->at(i).ro));
        /*circlepen.setColor(QColor(rand()%255, rand()%255, rand()%255));
        painter.setPen(circlepen);
        for(unsigned int j = 1; j < points->at(i).vCellIdx.size(); j++)
        {
            painter.drawLine(triangles->at(points->at(i).vCellIdx[j]).c.toPointF(), triangles->at(points->at(i).vCellIdx[j - 1]).c.toPointF());
        }
        if(points->at(i).vCellIdx.size() > 2)
        {
            painter.drawLine(triangles->at(points->at(i).vCellIdx[0]).c.toPointF(), triangles->at(points->at(i).vCellIdx.back()).c.toPointF());
        }*/
    }

    if(isolines)
    {
        painter.setPen(circlepen);
        for(uint i = 0; i < isolines->size(); i++)
        {
            painter.drawLine(isolines->at(i).p1.toPointF() * sc, isolines->at(i).p2.toPointF() * sc);
        }
    }

    if(particles)
    {
        painter.setPen(dotpenyellow);
        for(uint i = 0; i < particles->size(); i++)
        {
            painter.drawPoint(particles->at(i).pos.toPointF() * sc);
            painter.drawText(particles->at(i).pos.toPointF() * sc, QString::number(particles->at(i).vel.x()) + ":" + QString::number(particles->at(i).vel.y()));
        }
    }


}

 void DiagramPlot::setParticles(std::vector<Particle> *particles)
 {
     this->particles = particles;
 }

void DiagramPlot::setData(std::vector<Triangle> *triangles, std::vector<Point> *points)
{
    isDataLoaded = true;
    this->triangles = triangles;
    this->points = points;
}

void DiagramPlot::setIsolines(std::vector<Line> *isolines)
{
    this->isolines = isolines;
}

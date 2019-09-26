#include "diagramplot.h"
#include <QPainter>

DiagramPlot::DiagramPlot(QWidget *parent) : QWidget(parent)
{
    isolines = nullptr;
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

    QPen circlepen(Qt::green);
    circlepen.setCapStyle(Qt::RoundCap);
    circlepen.setWidth(2);

    painter.setRenderHint(QPainter::Antialiasing,true);

    for(unsigned int i = 0; i < triangles->size(); i++)
    {
        painter.setPen(linepen);
        uint p1 = triangles->at(i).p1;
        uint p2 = triangles->at(i).p2;
        uint p3 = triangles->at(i).p3;
        painter.drawLine(points->at(p1).toPointF(), points->at(p2).toPointF());
        painter.drawLine(points->at(p1).toPointF(), points->at(p3).toPointF());
        painter.drawLine(points->at(p2).toPointF(), points->at(p3).toPointF());
        //painter.setPen(circlepen);
        //painter.drawEllipse(triangles->at(i).c, triangles->at(i).r, triangles->at(i).r);
    }

    for(unsigned int i = 0; i < points->size(); i++)
    {
        painter.setPen(dotpen);
        painter.drawPoint(points->at(i).toPointF());
        painter.drawText(points->at(i).toPointF(), QString::number(points->at(i).u));
    }

    if(isolines == nullptr)
    {
        return;
    }
    painter.setPen(circlepen);
    for(uint i = 0; i < isolines->size(); i++)
    {
        painter.drawLine(isolines->at(i).p1.toPointF(), isolines->at(i).p2.toPointF());
    }

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

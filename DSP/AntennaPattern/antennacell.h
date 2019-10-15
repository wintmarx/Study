#ifndef ANTENNARECT_H
#define ANTENNARECT_H

#include <QGraphicsItem>
#include <QObject>
#include <QPainter>

class AntennaCell : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    AntennaCell(double width, double height, QObject *parent = nullptr);
    ~AntennaCell();
    bool isActive = false;
    double w = 0.;
    double h = 0.;

private:
    QRectF boundingRect() const;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // ANTENNARECT_H

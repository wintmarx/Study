#include "antennacell.h"

AntennaCell::AntennaCell(double width, double height, QObject *parent) :
    QObject(parent), QGraphicsItem(), w(width), h(height)
{

}

AntennaCell::~AntennaCell()
{

}

void AntennaCell::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    isActive = !isActive;
    update();
}

QRectF AntennaCell::boundingRect() const
{
    return QRectF (-w/2., -h/2., w, h);
}

void AntennaCell::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setPen(Qt::black);
    painter->setBrush(isActive ? Qt::blue : Qt:: white);
    painter->drawRect(boundingRect());
}

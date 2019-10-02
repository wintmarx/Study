#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QWheelEvent>
#include <QKeyEvent>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class GLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = nullptr);
    virtual ~GLWidget() override;
    void setViewMatrix(const double *mat);
    void DrawTriangle(const glm::dvec3 &p, const glm::vec3 &c);
    void DrawCube(const glm::dvec3 &p, const glm::vec3 &c, double s = 1., bool lines = false);
    void DrawLine(const glm::dvec3 &b, const glm::dvec3 &e, const glm::vec3 &c);
    glm::ivec4 viewport;
    glm::dmat4 proj;

signals:
    void keyPressEventSignal(QKeyEvent*);
    void keyReleaseEventSignal(QKeyEvent*);
    void mousePressEventSignal(QMouseEvent*);
    void mouseReleaseEventSignal(QMouseEvent*);
    void wheelEventSignal(QWheelEvent*);
    void mouseMoveEventSignal(QMouseEvent*);
    void Draw();
    void Resized(int, int);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // GLWIDGET_H

#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) :
    QOpenGLWidget (parent),
    viewport(0),
    proj(1.)
{

}

GLWidget::~GLWidget()
{

}

void GLWidget::initializeGL()
{
    glClearColor(1, 1, 1, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_MULTISAMPLE);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void GLWidget::DrawTriangle(const glm::dvec3 &p, const glm::vec3 &c)
{
    glColor3fv(&c[0]);
    glVertex3d(p.x - 0.5, p.y - 0.5, p.z + 1.);
    glVertex3d(p.x + 0.5, p.y - 0.5, p.z + 1.);
    glVertex3d(p.x + 0., p.y + 0.5, p.z + 1.);
}

void GLWidget::DrawCube(const glm::dvec3 &p, const glm::vec3 &c, double s)
{
    glBegin(GL_QUADS);
    s *= 0.5;
    glColor3fv(&c[0]);
    //glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3d(p.x + s, p.y + s, p.z - s);
    glVertex3d(p.x - s, p.y + s, p.z - s);
    glVertex3d(p.x - s, p.y + s, p.z + s);
    glVertex3d(p.x + s, p.y + s, p.z + s);

    // Bottom face (y = -1.0f)
    //glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3d(p.x + s, p.y - s, p.z + s);
    glVertex3d(p.x - s, p.y - s, p.z + s);
    glVertex3d(p.x - s, p.y - s, p.z - s);
    glVertex3d(p.x + s, p.y - s, p.z - s);

    // Front face  (z = 1.0f)
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3d(p.x + s, p.y + s, p.z + s);
    glVertex3d(p.x - s, p.y + s, p.z + s);
    glVertex3d(p.x - s, p.y - s, p.z + s);
    glVertex3d(p.x + s, p.y - s, p.z + s);

    // Back face (z = -1.0f)
    //glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3d(p.x + s, p.y - s, p.z - s);
    glVertex3d(p.x - s, p.y - s, p.z - s);
    glVertex3d(p.x - s, p.y + s, p.z - s);
    glVertex3d(p.x + s, p.y + s, p.z - s);

    // Left face (x = -1.0f)
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3d(p.x - s, p.y + s, p.z + s);
    glVertex3d(p.x - s, p.y + s, p.z - s);
    glVertex3d(p.x - s, p.y - s, p.z - s);
    glVertex3d(p.x - s, p.y - s, p.z + s);

    // Right face (x = 1.0f)
    //glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3d(p.x + s, p.y + s, p.z - s);
    glVertex3d(p.x + s, p.y + s, p.z + s);
    glVertex3d(p.x + s, p.y - s, p.z + s);
    glVertex3d(p.x + s, p.y - s, p.z - s);
    glEnd();
}

/*void DrawCell(const glm::dvec3 &p, double size = 1.)
{
    glBegin(GL_QUADS);
    DrawCube(p, 0.04 * size);
    DrawCube(p + glm::dvec3(.50 * size, .50 * size, .00 * size), 0.04 * size);
    DrawCube(p + glm::dvec3(.50 * size, .00 * size, .50 * size), 0.04 * size);
    DrawCube(p + glm::dvec3(.00 * size, .50 * size, .50 * size), 0.04 * size);
    DrawCube(p + glm::dvec3(.25 * size, .25 * size, .25 * size), 0.04 * size);
    DrawCube(p + glm::dvec3(.25 * size, .75 * size, .75 * size), 0.04 * size);
    DrawCube(p + glm::dvec3(.75 * size, .75 * size, .25 * size), 0.04 * size);
    DrawCube(p + glm::dvec3(.75 * size, .25 * size, .75 * size), 0.04 * size);
    glEnd();

    glBegin(GL_LINES);
    glColor3f(1., 0., 0.);
    glVertex3dv(&p[0]);
    glVertex3dv(&(p + glm::dvec3(.25 * size, .25 * size, .25 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.25 * size, .25 * size, .25 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.50 * size, .50 * size, .00 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.50 * size, .50 * size, .00 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.75 * size, .75 * size, .25 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.25 * size, .25 * size, .25 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.00 * size, .50 * size, .50 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.00 * size, .50 * size, .50 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.25 * size, .75 * size, .75 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.25 * size, .75 * size, .75 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.50 * size,       size, .50 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.25 * size, .75 * size, .75 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.50 * size, .50 * size,       size))[0]);

    glVertex3dv(&(p + glm::dvec3(.25 * size, .25 * size, .25 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.50 * size, .00 * size, .50 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.50 * size, .00 * size, .50 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.75 * size, .25 * size, .75 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.75 * size, .25 * size, .75 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(      size, .00 * size,       size))[0]);

    glVertex3dv(&(p + glm::dvec3(.75 * size, .25 * size, .75 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(      size, .50 * size, .50 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(      size, .50 * size, .50 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.75 * size, .75 * size, .25 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.75 * size, .75 * size, .25 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(      size,       size, .00 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.75 * size, .75 * size, .25 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.50 * size,       size, .50 * size))[0]);

    glVertex3dv(&(p + glm::dvec3(.75 * size, .25 * size, .75 * size))[0]);
    glVertex3dv(&(p + glm::dvec3(.50 * size, .50 * size,       size))[0]);
    glEnd();
}*/

void GLWidget::setViewMatrix(const double *view)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(view);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
        DrawCube(zcp + glm::dvec3(.5, .5, .5));
        /*DrawCube(zcp + glm::dvec3(1.5, .5, .5));
        DrawCube(zcp + glm::dvec3(1.5, .5, 1.5));
        DrawCube(zcp + glm::dvec3(.5, .5, 1.5));
        DrawCube(zcp + glm::dvec3(.5, 1.5, .5));
        DrawCube(zcp + glm::dvec3(1.5, 1.5, .5));
        DrawCube(zcp + glm::dvec3(1.5, 1.5, 1.5));
        DrawCube(zcp + glm::dvec3(.5, 1.5, 1.5));*/
    /*glEnd();
    glEnable(GL_CULL_FACE);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    DrawCell(zcp);*/
    /*DrawCell(zcp + glm::dvec3(1., 0., 0.));
    DrawCell(zcp + glm::dvec3(1., 0., 1.));
    DrawCell(zcp + glm::dvec3(0., 0., 1.));
    DrawCell(zcp + glm::dvec3(0., 1., 0.));
    DrawCell(zcp + glm::dvec3(1., 1., 0.));
    DrawCell(zcp + glm::dvec3(1., 1., 1.));
    DrawCell(zcp + glm::dvec3(0., 1., 1.));*/
    emit Draw();
}

void GLWidget::resizeGL(int w, int h)
{
    viewport.z = w;
    viewport.w = h;
    glMatrixMode(GL_PROJECTION);
    proj = glm::perspectiveRH_NO(75., double(w)/ h, 0.1, 100.);
    glLoadMatrixd(&proj[0][0]);
    emit Resized(w, h);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    emit wheelEventSignal(event);
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    emit keyPressEventSignal(event);
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleaseEventSignal(event);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoveEventSignal(event);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    emit mousePressEventSignal(event);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseReleaseEventSignal(event);
}

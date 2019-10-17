#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    viewport(0),
    proj(1.)
{
    timer = new QTimer(this);
    timer->setInterval(16);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setMajorVersion(2);
    format.setMinorVersion(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    //format.setSamples(0);
    //QSurfaceFormat::setDefaultFormat(format);
    setFormat(format);
}

GLWidget::~GLWidget()
{
    timer->stop();
    delete timer;
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
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

void GLWidget::DrawCube(const glm::dvec3 &p, const glm::vec3 &c, double s, bool lines)
{
    if (lines)
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glDisable(GL_CULL_FACE);
    }
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

    if (lines)
    {
        glEnable(GL_CULL_FACE);
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    }
}

void GLWidget::DrawLine(const glm::dvec3 &b, const glm::dvec3 &e, const glm::vec3 &c)
{
    glBegin(GL_LINES);
    glColor3fv(&c[0]);
    glVertex3dv(&b[0]);
    glVertex3dv(&e[0]);
    glEnd();
}

void GLWidget::setViewMatrix(const double *view)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(view);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    emit Draw();
}

void GLWidget::resizeGL(int w, int h)
{
    viewport.z = w;
    viewport.w = h;
    glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
    glMatrixMode(GL_PROJECTION);
    proj = glm::perspectiveRH_NO(45., double(w)/ h, 0.1, 100.);
    //proj = glm::frustumRH_NO(-1., 1., -1., 1., 1., 100.);
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

glm::dvec3 GLWidget::ScreenToWorld(const QPoint &screen, const glm::dmat4 &modelview)
{
    makeCurrent();
    float depth = 1.f;
    glReadPixels(screen.x(), viewport[3] - screen.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    qDebug("%d, %d, depth: %f", screen.x(),  viewport[3] - screen.y(), depth);
    return glm::unProjectNO(glm::dvec3(screen.x(), screen.y(), depth), modelview, proj, viewport);
}

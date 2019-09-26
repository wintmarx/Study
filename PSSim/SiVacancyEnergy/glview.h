#ifndef GLVIEW_H
#define GLVIEW_H

#include <QGLWidget>
#include <QtOpenGL>

class GLView : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    GLView();
};

#endif // GLVIEW_H

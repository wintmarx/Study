#ifndef SIMULATION_H
#define SIMULATION_H

#include <QThread>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include "glwidget.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "crystal.h"
#include <mutex>

struct Camera
{
    glm::dvec3 p;
    glm::dvec3 d;
    glm::dvec3 u;
    glm::dvec3 r;
    static constexpr double v = 0.000005;
};

class Simulation : public QThread
{
    Q_OBJECT
public:
    Simulation(GLWidget *widget);
    virtual ~Simulation() override;
    bool isActive = true;

public slots:
    void MainLoop();
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void onWidgetResized(int w, int h);
    void Draw();

signals:
    void Finished();

private:
    void run() override;
    inline int KeyToAddr(int key) { return key | ((Qt::Key_Escape & key) >> 2); }
    inline int AddrToKey(int addr) { return addr | ((0x0100 & addr) << 2); }
    void Update();
    void HandleInput();
    double TwoParticleIteractionEnergy(const Crystal &c, const glm::dvec3 &r1, const glm::dvec3 &r2);
    double ThreeParticleIteractionEnergy(const Crystal &c, const glm::dvec3 &r1, const glm::dvec3 &r2, const glm::dvec3 &r3);

    Crystal crystal;

    GLWidget *widget = nullptr;
    const glm::dvec3 up = glm::dvec3(0., -1., 0.);
    glm::dmat4 view;
    Camera camera;

    glm::dvec2 angleCoeff;

    static const int keysStates = 352;
    bool keysState[keysStates];
    static const int mouseStates = 3;
    bool mouseState[mouseStates];
    QPoint mouse;
    std::mutex aMutex;
    std::mutex cameraMutex;
};

#endif // SIMULATION_H

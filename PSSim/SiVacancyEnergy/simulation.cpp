#include "simulation.h"

Simulation::Simulation(GLWidget *widget) :
    crystal({0., 0., 0.}, {3, 3, 3}),
    angleCoeff(1.),
    mouse(0., 0.)
{
    camera.p = glm::dvec3(0., 0., 20.);
    camera.d = glm::dvec3(0., 0., -1.);
    camera.u = glm::dvec3(0., -1., 0.);
    camera.r = glm::dvec3(1., 0., 0.);

    for (int i = 0; i < keysStates; i++)
    {
        keysState[i] = false;
    }

    for (int i = 0; i < mouseStates; i++)
    {
        mouseState[i] = false;
    }

    this->widget = widget;
    connect(widget, SIGNAL (keyPressEventSignal(QKeyEvent*)), this, SLOT (keyPressEvent(QKeyEvent*)));
    connect(widget, SIGNAL (keyReleaseEventSignal(QKeyEvent*)), this, SLOT (keyReleaseEvent(QKeyEvent*)));
    connect(widget, SIGNAL (wheelEventSignal(QWheelEvent*)), this, SLOT (wheelEvent(QWheelEvent*)));
    connect(widget, SIGNAL (mouseMoveEventSignal(QMouseEvent*)), this, SLOT (mouseMoveEvent(QMouseEvent*)));
    connect(widget, SIGNAL (mousePressEventSignal(QMouseEvent*)), this, SLOT (mousePressEvent(QMouseEvent*)));
    connect(widget, SIGNAL (mouseReleaseEventSignal(QMouseEvent*)), this, SLOT (mouseReleaseEvent(QMouseEvent*)));
    connect(widget, SIGNAL (Resized(int, int)), this, SLOT (onWidgetResized(int, int)));
    connect(widget, SIGNAL (Draw()), this, SLOT (Draw()));
}

Simulation::~Simulation()
{
    requestInterruption();
    quit();
    wait();
}

void Simulation::run()
{
    MainLoop();
}

void Simulation::MainLoop()
{
    while(isActive)
    {
        HandleInput();
        Update();
    }
    emit Finished();
}

void Simulation::Update()
{
    mutex.lock();
    view = glm::lookAtRH(camera.p, camera.p + camera.d, up);
    mutex.unlock();
}

void Simulation::HandleInput()
{
    mutex.lock();
    if(keysState[KeyToAddr(Qt::Key_W)])
    {
        camera.p += camera.d * camera.v;
    }
    if(keysState[KeyToAddr(Qt::Key_S)])
    {
        camera.p -= camera.d * camera.v;
    }
    if(keysState[KeyToAddr(Qt::Key_A)])
    {
        camera.p -= camera.r * camera.v;
    }
    if(keysState[KeyToAddr(Qt::Key_D)])
    {
        camera.p += camera.r * camera.v;
    }
    if(keysState[KeyToAddr(Qt::Key_Space)])
    {
        camera.p -= camera.u * camera.v;
    }
    mutex.unlock();
}

void Simulation::Draw()
{
    mutex.lock();
    widget->setViewMatrix(&view[0][0]);
    for(uint i = 0; i < crystal.atoms.size(); i++)
    {
        glm::vec3 c(0.f, 0.f, 0.f);
        if (crystal.atoms[i].isBoundary)
        {
            c.r = 1.f;
        }
        widget->DrawCube(crystal.atoms[i].p, c, Crystal::atomSize);
    }
    for(uint i = 0; i < crystal.bonds.size(); i++)
    {
        const std::pair<uint, uint> &b = crystal.bonds[i];
        const std::vector<Atom> &v = crystal.atoms;
        widget->DrawLine(v[b.first].p, v[b.second].p, glm::vec3(0.3f, 0.3f, 0.3f));
    }
    widget->DrawCube(crystal.p + glm::dvec3(crystal.sizeInCells.x * Crystal::cellSize / 2.), glm::vec3(0.3f, 0.3f, 0.3f), crystal.sizeInCells.x * Crystal::cellSize, true);

    widget->update();
    mutex.unlock();
}

void Simulation::onWidgetResized(int w, int h)
{
    mutex.lock();
    angleCoeff.x = 2. * glm::two_pi<double>() / double(w);
    angleCoeff.y = 2. * glm::two_pi<double>() / double(h);
    mutex.unlock();
}

void Simulation::wheelEvent(QWheelEvent *event)
{
    mutex.lock();
    camera.p += camera.d * double(event->angleDelta().y())/8.;
    mutex.unlock();
}

void Simulation::keyPressEvent(QKeyEvent *event)
{
    mutex.lock();
    keysState[KeyToAddr(event->key())] = true;
    mutex.unlock();
}

void Simulation::keyReleaseEvent(QKeyEvent *event)
{
    mutex.lock();
    keysState[KeyToAddr(event->key())] = false;
    mutex.unlock();
}

void Simulation::mouseMoveEvent(QMouseEvent *event)
{
    mutex.lock();
    camera.r = glm::normalize(glm::cross(up, camera.d));
    glm::qua<double> r = glm::angleAxis((event->pos().y() - mouse.y()) * angleCoeff.y, camera.r);
    r += glm::angleAxis((mouse.x() - event->pos().x()) * angleCoeff.x, up);
    camera.d = camera.d * r;
    mouse = event->pos();
    mutex.unlock();
}

void Simulation::mousePressEvent(QMouseEvent *event)
{
    mutex.lock();
    mouse = event->pos();
    mutex.unlock();
}

void Simulation::mouseReleaseEvent(QMouseEvent *event)
{
    mutex.lock();
    mouse = event->pos();
    mutex.unlock();
}

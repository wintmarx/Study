#include "simulation.h"

Simulation::Simulation(GLWidget *widget) :
    crystal({0., 0., 10.}, {3, 3, 3}),
    angleCoeff(1.),
    mouse(0., 0.)
{
    camera.p = glm::dvec3(0., 0., 30.);
    camera.d = glm::dvec3(0., 0., -1.);
    camera.u = glm::dvec3(0., 1., 0.);
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
    cameraMutex.lock();
    view = glm::lookAtRH(camera.p, camera.p + camera.d, up);
    cameraMutex.unlock();
    aMutex.lock();
    for(Atom &a : crystal.atoms) {
        /*if (a.isBoundary)
        {
            continue;
        }*/
        a.u = 0.;
        const auto& neighboors = a.neighboors;
        for(uint j = 0; j < neighboors.size(); j++) {
            a.u += TwoParticleIteractionEnergy(crystal, a.p, neighboors[j]->p);
            for(uint k = j + 1; k < neighboors.size(); k++) {
                a.u += ThreeParticleIteractionEnergy(crystal, a.p, neighboors[j]->p, neighboors[k]->p);
            }
        }
        if (a.u > crystal.maxEnergy)
        {
            crystal.maxEnergy = a.u;
        }
        if (a.u < crystal.minEnergy)
        {
            crystal.minEnergy = a.u;
        }
    }
    aMutex.unlock();
}

double Simulation::TwoParticleIteractionEnergy(const Crystal &c, const glm::dvec3 &r1, const glm::dvec3 &r2)
{
    double d = glm::length(r1 - r2);
    return c.epsilon * c.A * (c.B * std::pow(d/c.zeroPotentialR, -c.r) - std::pow(d/c.zeroPotentialR, -c.q)) * std::exp(c.zeroPotentialR/(d - c.potentialCutoffR));
}

inline double h(const Crystal &c, double d12, double d13, double cos213)
{
    if (d12 >= c.potentialCutoffR || d13 >= c.potentialCutoffR)
    {
        return 0.;
    }
    return c.lambda * std::exp(c.gamma * c.zeroPotentialR * (1./(d12 - c.potentialCutoffR) + 1./(d13 - c.potentialCutoffR))) * (cos213 + 1./3.) * (cos213 + 1./3.);
}

inline double cos(double a, double b, double c)
{
    return 0.5 * (b * b + c * c - a * a) / (b * c);
}

double Simulation::ThreeParticleIteractionEnergy(const Crystal &c, const glm::dvec3 &r1, const glm::dvec3 &r2, const glm::dvec3 &r3)
{
    double d12 = glm::length(r1 - r2);
    double d23 = glm::length(r2 - r3);
    double d31 = glm::length(r3 - r1);
    return h(c, d12, d31, cos(d23, d31, d12)) + h(c, d12, d23, cos(d31, d12, d23)) + h(c, d31, d23, cos(d12, d23, d31));
}

void Simulation::HandleInput()
{
    cameraMutex.lock();
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
        camera.p += camera.u * camera.v;
    }
    cameraMutex.unlock();
}

void Simulation::Draw()
{
    glm::vec3 c;
    glm::vec3 bColor(0.3f, 0.3f, 0.3f);
    const std::vector<Atom> &atoms = crystal.atoms;
    cameraMutex.lock();
    widget->setViewMatrix(&view[0][0]);
    cameraMutex.unlock();
    aMutex.lock();
    for(const Atom& a : atoms)
    {
        double range = crystal.maxEnergy - crystal.minEnergy;
        float rate = (a.u - crystal.minEnergy) / range;
        c.r = a.u >= 0. ? rate : 0;
        c.g = 0.;
        c.b = a.u < 0. ? rate : 0;
        /*if (a.isBoundary)
        {
            c.r = 1.f;
        }*/
        widget->DrawCube(a.p, c, Crystal::atomSize);
    }
    for(const auto& b : crystal.bonds)
    {
        widget->DrawLine(atoms[b.first].p, atoms[b.second].p, bColor);
    }
    //widget->DrawCube(crystal.p + glm::dvec3(crystal.sizeInCells.x * Crystal::latticeConst / 2.), glm::vec3(0.3f, 0.3f, 0.3f), crystal.sizeInCells.x * Crystal::latticeConst, true);
    aMutex.unlock();
    widget->update();
}

void Simulation::onWidgetResized(int w, int h)
{
    angleCoeff.x = 0.5 * glm::two_pi<double>() / double(w);
    angleCoeff.y = 0.5 * glm::two_pi<double>() / double(h);
}

void Simulation::wheelEvent(QWheelEvent *event)
{
    cameraMutex.lock();
    camera.p += camera.d * double(event->angleDelta().y())/8.;
    cameraMutex.unlock();
}

void Simulation::keyPressEvent(QKeyEvent *event)
{
    keysState[KeyToAddr(event->key())] = true;
}

void Simulation::keyReleaseEvent(QKeyEvent *event)
{
    keysState[KeyToAddr(event->key())] = false;
}

void Simulation::mouseMoveEvent(QMouseEvent *event)
{
    cameraMutex.lock();
    camera.r = glm::normalize(glm::cross(camera.d, up));
    glm::qua<double> r = glm::angleAxis((event->pos().y() - mouse.y()) * angleCoeff.y, camera.r);
    r *= glm::angleAxis((event->pos().x() - mouse.x()) * angleCoeff.x, up);
    camera.d = camera.d * r;
    cameraMutex.unlock();
    mouse = event->pos();
}

void Simulation::mousePressEvent(QMouseEvent *event)
{
    mouse = event->pos();
}

void Simulation::mouseReleaseEvent(QMouseEvent *event)
{
    mouse = event->pos();
}

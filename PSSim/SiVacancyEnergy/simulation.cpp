#include "simulation.h"

Simulation::Simulation(GLWidget *widget) :
    curTimestep(0.),
    angleCoeff(1.),
    mouse(0., 0.)
{
    camera.p = glm::dvec3(0., 0., 11.);
    camera.d = glm::dvec3(0., 0., -1.);
    camera.u = glm::dvec3(0., 1., 0.);
    camera.r = glm::dvec3(1., 0., 0.);

    for (int i = 0; i < keysStates; i++)
    {
        keysState[i] = false;
        keysBlock[i] = false;
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

void Simulation::SetSimTimestep(int step)
{
    const uint &s = static_cast<uint>(step);
    if (s >= simTimesteps || !isLoaded)
    {
        return;
    }
    curTimestep = s;
}


void Simulation::run()
{
    MainLoop();
}

void Simulation::MainLoop()
{
    crystal.reserve(simTimesteps);
    crystal.emplace_back(glm::dvec3(0., 0., 10.), glm::uvec3(3, 3, 3));
    float loading = 0.3f;//30%
    emit LoadingTick(loading);
    glm::dvec2 ux(0.);
    glm::dvec2 uy(0.);
    glm::dvec2 uz(0.);
    glm::dvec3 lastF;
    double d = Crystal::equalR * 0.01;
    double initEnergy = 0.;
    for(Atom &a : crystal.back().atoms)
    {
        const auto& n = a.neighboors;
        for(uint j = 0; j < n.size(); j++)
        {
            //initEnergy += TwoParticleIteractionEnergy(crystal.back(), a.p, n[j]->p);
            for(uint k = j + 1; k < n.size(); k++)
            {
                initEnergy += ThreeParticleIteractionEnergy(crystal.back(), a.p, n[j]->p, n[k]->p);
            }
        }
    }

    qDebug("init energy %f", initEnergy);



    for (uint i = 1; i < simTimesteps; i++)
    {
        crystal.emplace_back(crystal.back());

        for(Atom &a : crystal.back().atoms)
        {
            if (a.isBoundary)
            {
                continue;
            }
            ux[0] = 0.;
            ux[1] = 0.;
            uy[0] = 0.;
            uy[1] = 0.;
            uz[0] = 0.;
            uz[1] = 0.;
            const auto& neighboors = a.neighboors;
            for(uint j = 0; j < neighboors.size(); j++) {
                ux[0] += TwoParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(-d, 0., 0.), neighboors[j]->p);
                ux[1] += TwoParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3( d, 0., 0.), neighboors[j]->p);
                uy[0] += TwoParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0., -d, 0.), neighboors[j]->p);
                uy[1] += TwoParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0.,  d, 0.), neighboors[j]->p);
                uz[0] += TwoParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0., 0., -d), neighboors[j]->p);
                uz[1] += TwoParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0., 0.,  d), neighboors[j]->p);
                for(uint k = j + 1; k < neighboors.size(); k++) {
                    ux[0] += ThreeParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(-d, 0., 0.), neighboors[j]->p, neighboors[k]->p);
                    ux[1] += ThreeParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3( d, 0., 0.), neighboors[j]->p, neighboors[k]->p);
                    uy[0] += ThreeParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0., -d, 0.), neighboors[j]->p, neighboors[k]->p);
                    uy[1] += ThreeParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0.,  d, 0.), neighboors[j]->p, neighboors[k]->p);
                    uz[0] += ThreeParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0., 0., -d), neighboors[j]->p, neighboors[k]->p);
                    uz[1] += ThreeParticleIteractionEnergy(crystal.back(), a.p - glm::dvec3(0., 0.,  d), neighboors[j]->p, neighboors[k]->p);
                }
            }
            lastF = a.f;
            a.f.x = Der(ux[1], ux[0], d + d);
            a.f.y = Der(uy[1], uy[0], d + d);
            a.f.z = Der(uz[1], uz[0], d + d);
            a.p = a.p + a.v * dt + lastF * 0.5 / a.m * dt * dt;
            a.v = a.v + (a.f + lastF) * 0.5 / a.m * dt;
        }
        loading += (1.f - loading) / (simTimesteps - i);
        emit LoadingTick(loading);
    }
    isLoaded = true;
    while(isActive)
    {
        HandleInput();
        Update();
    }
    emit Finished();
}

static QVector<double> ks;
static QVector<double> fx;
static QVector<double> fy;
static QVector<double> fz;
static QVector<double> vx;
static QVector<double> vy;
static QVector<double> vz;
const QColor fxc(Qt::black);
const QColor fyc(Qt::blue);
const QColor fzc(Qt::cyan);
const QColor vxc(Qt::red);
const QColor vyc(Qt::green);
const QColor vzc(Qt::yellow);

void Simulation::Update()
{
    cameraMutex.lock();
    view = glm::lookAtRH(camera.p, camera.p + camera.d, up);
    cameraMutex.unlock();

    if (selAtom >= 0 && !selAtomPlotted) {
        selAtomPlotted = true;
        ks.resize(simTimesteps);
        fx.resize(simTimesteps);
        fy.resize(simTimesteps);
        fz.resize(simTimesteps);
        vx.resize(simTimesteps);
        vy.resize(simTimesteps);
        vz.resize(simTimesteps);
        for (uint i = 0 ; i < simTimesteps; i++) {
            ks[i] = i;
            fx[i] = crystal[i].atoms[selAtom].f.x;
            fy[i] = crystal[i].atoms[selAtom].f.y;
            fz[i] = crystal[i].atoms[selAtom].f.z;
            vx[i] = crystal[i].atoms[selAtom].v.x;
            vy[i] = crystal[i].atoms[selAtom].v.y;
            vz[i] = crystal[i].atoms[selAtom].v.z;
        }

        emit RemovePlots();
        emit DrawPlot(&ks, &fx, &fxc);
        emit DrawPlot(&ks, &fy, &fyc);
        emit DrawPlot(&ks, &fz, &fzc);
        emit DrawPlot(&ks, &vx, &vxc);
        emit DrawPlot(&ks, &vy, &vyc);
        emit DrawPlot(&ks, &vz, &vzc);
    }
}

double Simulation::TwoParticleIteractionEnergy(const Crystal &c, const glm::dvec3 &r1, const glm::dvec3 &r2)
{
    double d = glm::length(r1 - r2);
    if (d >= c.potentialCutoffR)
    {
        return 0.;
    }
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
    if (keysState[KeyToAddr(Qt::Key_W)])
    {
        camera.p += camera.d * camera.v;
    }
    if (keysState[KeyToAddr(Qt::Key_S)])
    {
        camera.p -= camera.d * camera.v;
    }
    if (keysState[KeyToAddr(Qt::Key_A)])
    {
        camera.p -= camera.r * camera.v;
    }
    if (keysState[KeyToAddr(Qt::Key_D)])
    {
        camera.p += camera.r * camera.v;
    }
    if (keysState[KeyToAddr(Qt::Key_Space)])
    {
        camera.p += camera.u * camera.v;
    }
    cameraMutex.unlock();
}

void Simulation::Draw()
{
    if (!isLoaded)
    {
        return;
    }
    glm::vec3 c(0.);
    glm::vec3 bColor(0.3f, 0.3f, 0.3f);
    const Crystal& crystal = this->crystal[curTimestep];
    const std::vector<Atom> &atoms = crystal.atoms;
    cameraMutex.lock();
    widget->setViewMatrix(&view[0][0]);
    cameraMutex.unlock();
    aMutex.lock();
    int i = 0;
    for(const Atom& a : atoms)
    {
        c.b = i == selAtom? 1.f : 0.f;
        /*if (selAtom >= 0) {
            c.g = std::find(atoms[selAtom].neighboors.begin(), atoms[selAtom].neighboors.end(), &a) != atoms[selAtom].neighboors.end() ? 1.f : 0.f;
        }*/
        c.r = a.isBoundary ? 1.f : 0.f;
        /*if (selAtom < 0 || c.g  == 1.f)
        {
            widget->DrawCube(a.p, c, Crystal::atomSize);
        }*/
        i++;
    }
    for(const auto& b : crystal.bonds)
    {
        widget->DrawLine(atoms[b.first].p, atoms[b.second].p, bColor);
    }
    widget->DrawCube(crystal.p + glm::dvec3(crystal.sizeInCells.x * Crystal::latticeConst / 2.), glm::vec3(0.3f, 0.3f, 0.3f), crystal.sizeInCells.x * Crystal::latticeConst, true);
    aMutex.unlock();
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
    if (event->key() == Qt::Key_Left)
    {
        if (curTimestep > 0)
        {
            emit(ActiveTimestepChanged(static_cast<int>(curTimestep - 1)));
        }
    }
    if (event->key() == Qt::Key_Right)
    {
        if (curTimestep < simTimesteps - 1)
        {
            emit(ActiveTimestepChanged(static_cast<int>(curTimestep + 1)));
        }
    }
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
    if (event->button() != Qt::RightButton) {
        return;
    }
    selAtom = -1;
    glm::dvec3 m = widget->ScreenToWorld(mouse, view);
    for(uint i = 0; i < crystal[curTimestep].atoms.size(); i++)
    {
        if (glm::length(crystal[curTimestep].atoms[i].p - m) < 2 * Crystal::atomSize) {
           selAtom = static_cast<int>(i);
           selAtomPlotted = false;
           break;
        }
    }
}

void Simulation::mouseReleaseEvent(QMouseEvent *event)
{
    mouse = event->pos();
}

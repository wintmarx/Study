#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <algorithm>

using namespace std;

static QGraphicsScene *antennaScene;
static QGraphicsScene *patternScene;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    antennaScene = new QGraphicsScene();
    antennaScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    patternScene = new QGraphicsScene();
    ui->antennaView->setScene(antennaScene);
    ui->antennaView->setRenderHint(QPainter::Antialiasing);
    ui->antennaView->setCacheMode(QGraphicsView::CacheBackground);
    ui->antennaView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    ui->antennaView->show();
    ui->patternView->setScene(patternScene);
    ui->patternView->show();
    srand(static_cast<uint>(time(nullptr)));

    // Chart Viewer
    connect(ui->chartViewer, SIGNAL(viewPortChanged()), this, SLOT(onViewPortChanged()));
    connect(ui->chartViewer, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(onMouseMoveChart(QMouseEvent *)));
    connect(ui->chartViewer, SIGNAL(clicked(QMouseEvent *)), this, SLOT(onMouseUpChart(QMouseEvent *)));

    // 3D view angles
    m_elevationAngle = 30;
    m_rotationAngle = 45;

    // Keep track of mouse drag
    m_isDragging = false;
    m_lastMouseX = -1;
    m_lastMouseY = -1;
}

MainWindow::~MainWindow()
{
    for (uint i = 0; i < grid.ant.size(); i++)
    {
        antennaScene->removeItem(grid.ant[i]);
        delete grid.ant[i];
    }
    delete antennaScene;
    delete patternScene;
    delete ui;
}

void MainWindow::onMouseMoveChart(QMouseEvent *event)
{
    qDebug("onMouseMoveChart");
    int mouseX = ui->chartViewer->getChartMouseX();
    int mouseY = ui->chartViewer->getChartMouseY();

    // Drag occurs if mouse is moving and mouse button is down
    if (event->buttons() & Qt::LeftButton)
    {
        if (m_isDragging)
        {
            // The chart is configured to rotate by 90 degrees when the mouse moves from
            // left to right, which is the plot region width (360 pixels). Similarly, the
            // elevation changes by 90 degrees when the mouse moves from top to buttom,
            // which is the plot region height (270 pixels).
            m_rotationAngle += (m_lastMouseX - mouseX) * 90.0 / ui->chartViewer->width() * 2.;
            m_elevationAngle += (mouseY - m_lastMouseY) * 90.0 / ui->chartViewer->height() * 2.;
            ui->chartViewer->updateViewPort(true, false);
        }

        // Keep track of the last mouse position
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
        m_isDragging = true;
    }
}

void MainWindow::onMouseUpChart(QMouseEvent *event)
{
    qDebug("onMouseUpChart");
    if (m_isDragging && (event->button() == Qt::LeftButton))
    {
        // mouse up means not dragging
        m_isDragging = false;
        ui->chartViewer->updateViewPort(true, false);
    }
}

void MainWindow::GenerateGrid(AntennasGrid &grid, double w, double h, uint cellsx, uint cellsy)
{
    double cellw = (w - 2.)/cellsx;
    double cellh = (h - 2.)/cellsy;
    grid.ant.reserve(cellsx * cellsy);
    grid.sizeX = cellsx;
    grid.sizeY = cellsy;
    for (uint i = 0; i < cellsy; i++)
    {
        for (uint j = 0; j < cellsx; j++)
        {
            grid.ant.push_back(new AntennaCell(cellw, cellh));
            grid.ant[i * cellsx + j]->setPos(cellw * (j + 0.5), cellh * (i + 0.5));
        }
    }

}

template<typename T>
inline T clamp(T val, T lo, T hi)
{
    return std::min(std::max(val, lo), hi);
}

void MainWindow::DrawSignal(const Pattern &pattern, SignalDrawMode mode, int size, QGraphicsScene *scene, double scale)
{
    vector<uchar> img(pattern.data.size());
    for(uint i = 0; i < img.size(); i++)
    {
        double src = 0.;
        switch(mode)
        {
            case SignalDrawMode::Amp:
                src = std::round(255. * pattern.data[i].Magnitude()/pattern.max);
            break;
            case SignalDrawMode::Re:
                src = std::round(pattern.data[i].re);
            break;
            case SignalDrawMode::Im:
                src = std::round(pattern.data[i].im);
            break;
        }
        img[i] = static_cast<uchar>(clamp(scale * src, 0., 255.));
    }
    QPixmap pixmap;
    pixmap.convertFromImage(QImage((const uchar*)img.data(), size, size, size * (int)sizeof(uchar), QImage::Format_Grayscale8));
    scene->clear();
    scene->setSceneRect(0., 0., size, size);
    scene->addPixmap(pixmap);
}


void MainWindow::Update3DPattern(double *dataX, double *dataZ, int size, QChartViewer *viewer) {
    SurfaceChart *c = new SurfaceChart(size, size);

    // Set the center of the plot region at (330, 290), and set width x depth x height to
    // 360 x 360 x 270 pixels
    c->setPlotRegion(size/2, size/2, size/2, size/2, size/2);

    // Set the data to use to plot the chart
    c->setData(DoubleArray(dataX, size), DoubleArray(dataX, size),
        DoubleArray(dataZ, size * size));

    // Spline interpolate data to a 80 x 80 grid for a smooth surface
    c->setInterpolation(80, 80);

    // Set the view angles
    c->setViewAngle(m_elevationAngle, m_rotationAngle);

    // Add a color axis (the legend) in which the left center is anchored at (660, 270). Set
    // the length to 200 pixels and the labels on the right side.
    c->setColorAxis(600, 270, Chart::Left, 200, Chart::Right);

    // Set the x, y and z axis titles using 10 points Arial Bold font
    c->xAxis()->setTitle("X", "arialbd.ttf", 15);
    c->yAxis()->setTitle("Y", "arialbd.ttf", 15);

    // Set axis label font
    c->xAxis()->setLabelStyle("arial.ttf", 10);
    c->yAxis()->setLabelStyle("arial.ttf", 10);
    c->zAxis()->setLabelStyle("arial.ttf", 10);
    c->colorAxis()->setLabelStyle("arial.ttf", 10);

    // Output the chart
    BaseChart *chart = viewer->getChart();
    if (chart != nullptr)
    {
       delete chart;
    }
    viewer->setChart(c);
    // Update the viewport to display the chart
    ui->chartViewer->updateViewPort(true, false);
}

double *dataXT = nullptr;
double *dataZT = nullptr;
int sizeT = 0;

void MainWindow::Draw3DPattern(const Pattern &pattern, double dr, int size, QChartViewer *viewer) {
    double *dataX = new double[size];
    double *dataZ = new double[size * size];
    for(int i = 0; i < size; i++)
    {
        dataX[i] = dr * (i - size * 0.5);
        for(int j = 0; j < size; j++)
        {
            dataZ[i * size + j] = pattern.data[i * size + j].Magnitude()/pattern.max;
        }
    }
    dataXT = dataX;
    dataZT = dataZ;
    sizeT = size;
    Update3DPattern(dataX, dataZ, size, viewer);
}

//
// View port changed event
//
void MainWindow::onViewPortChanged()
{
    // Update the chart if necessary
    if (ui->chartViewer->needUpdateChart() && dataXT != nullptr && dataZT != nullptr)
        Update3DPattern(dataXT, dataZT, sizeT, ui->chartViewer);

    /*SurfaceChart* c = (SurfaceChart*)ui->chartViewer->getChart();
    if (c == nullptr)
    {
        return;
    }
    c->setViewAngle(m_elevationAngle, m_rotationAngle);
     ui->chartViewer->setChart(c);
    // Update the viewport to display the chart
    ui->chartViewer->updateViewPort(true, true);*/
}

void MainWindow::on_calcButton_clicked()
{
    const uint size = ui->sizeEdit->text().toUInt();
    const double freq = ui->freqEdit->text().toDouble();
    const double lambda = 3.e8/freq;
    const double k = M_2_PI / lambda;
    const double d = ui->kEdit->text().toDouble() * lambda;
    const double R = ui->rEdit->text().toDouble();
    const double dr = 2. * R/size;
    const double A0 = ui->a0Edit->text().toDouble();
    double x = -R;
    double y = R;
    double l = 0.;
    double z = 0.;
    double dx = 0.;
    double dy = 0.;
    double r = 0.;
    Pattern pattern;
    pattern.data.resize(size * size, {0., 0});
    for (uint i = 0; i < size; i++, y-=dr)
    {
        x = -R;
        for (uint j = 0; j < size; j++, x+=dr)
        {
            l = sqrt(x*x + y*y);
            if (l > R)
            {
                continue;
            }
            z = sqrt(R * R - l * l);
            for (uint a = 0; a < grid.ant.size(); a++)
            {
                if (grid.ant[a]->isActive)
                {
                    dy = (0.5 * grid.sizeY - 1 - a / grid.sizeX + 0.5) * d * 10.;
                    dx = (a % grid.sizeX - 0.5 * grid.sizeX + 0.5) * d * 10.;
                    r = sqrt((x - dx) * (x - dx) + (y - dy) * (y - dy) + z * z);
                    pattern.data[i * size + j].re += cos(k * r) * (A0 / r);
                    pattern.data[i * size + j].im += -sin(k * r) * (A0 / r);
                }
            }
            double tmp = pattern.data[i * size + j].Magnitude();
            if (tmp > pattern.max)
            {
                pattern.max = tmp;
            }
            if (tmp < pattern.min)
            {
                pattern.min = tmp;
            }
        }
    }
    DrawSignal(pattern, SignalDrawMode::Amp, size, patternScene, ui->scaleEdit->text().toDouble());
    Draw3DPattern(pattern, dr, size, ui->chartViewer);
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::on_genGridButton_clicked()
{
    for (uint i = 0; i < grid.ant.size(); i++)
    {
        antennaScene->removeItem(grid.ant[i]);
        delete grid.ant[i];
    }
    grid.ant.clear();
    antennaScene->setSceneRect(0., 0., ui->antennaView->width(), ui->antennaView->height());
    ui->tabWidget->setCurrentIndex(0);
    GenerateGrid(grid, ui->antennaView->width(), ui->antennaView->height(), ui->cellsxEdit->text().toUInt(), ui->cellsyEdit->text().toUInt());
    for (uint i = 0; i < grid.ant.size(); i++)
    {
        antennaScene->addItem(grid.ant[i]);
    }
}

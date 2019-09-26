#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QCheckBox>
#include <QMouseEvent>
#include <QMessageBox>
#include "chartdir.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Chart Viewer
    connect(ui->chartViewer, SIGNAL(viewPortChanged()),SLOT(onViewPortChanged()));
    connect(ui->chartViewer, SIGNAL(mouseMove(QMouseEvent *)), SLOT(onMouseMoveChart(QMouseEvent *)));
    connect(ui->chartViewer, SIGNAL(clicked(QMouseEvent *)), SLOT(onMouseUpChart(QMouseEvent *)));

    // 3D view angles
    m_elevationAngle = 30;
    m_rotationAngle = 45;

    // Keep track of mouse drag
    m_isDragging = false;
    m_lastMouseX = -1;
    m_lastMouseY = -1;

    // Update the viewport to display the chart
    ui->chartViewer->updateViewPort(true, false);
}

MainWindow::~MainWindow()
{
    delete ui->chartViewer->getChart();
    delete ui;
}

//
// View port changed event
//
void MainWindow::onViewPortChanged()
{
    // Update the chart if necessary
    if (ui->chartViewer->needUpdateChart())
        drawChart(ui->chartViewer);
}

int curStep = 0;

//
// Draw chart
//
void MainWindow::drawChart(QChartViewer *viewer)
{
    if (!maxwell.isReady)
    {
        return;
    }
    const int dataSize = int(maxwell.p.gridStepsCount);
    double dataX[dataSize];
    double dataZ[dataSize * dataSize];
    for(int i = 0; i < dataSize; i++)
    {
        dataX[i] = i * double(maxwell.p.gridStep);
        for(int j = 0; j < dataSize; j++)
        {
            dataZ[i * dataSize + j] = double(ui->checkBox->isChecked() ?
                                                 maxwell.grid[curStep][i][j].h.length() :
                                                 maxwell.grid[curStep][i][j].e.length());

            if(dataZ[i * dataSize + j] > 1)
            {
                dataZ[i * dataSize + j] = 1;
            }

            /*qDebug("E (%d, %d, %d) %f, %f, %f\tH %f, %f, %f",
                   curStep, i, j,
                   maxwell.grid[curStep][i][j].e.x(), maxwell.grid[curStep][i][j].e.y(), maxwell.grid[curStep][i][j].e.z(),
                   maxwell.grid[curStep][i][j].h.x(), maxwell.grid[curStep][i][j].h.y(), maxwell.grid[curStep][i][j].h.z());
            */
        }
    }



    dataZ[0] = 1;

    // Create a SurfaceChart object of size 720 x 600 pixels
    SurfaceChart *c = new SurfaceChart(ui->chartViewer->width(), ui->chartViewer->height());

    // Set the center of the plot region at (330, 290), and set width x depth x height to
    // 360 x 360 x 270 pixels
    c->setPlotRegion(ui->chartViewer->width()/2, ui->chartViewer->height()/2, ui->chartViewer->width()/2., ui->chartViewer->width()/2., ui->chartViewer->height()/2);

    // Set the data to use to plot the chart
    c->setData(DoubleArray(dataX, dataSize), DoubleArray(dataX, dataSize),
        DoubleArray(dataZ, dataSize * dataSize));

    // Spline interpolate data to a 80 x 80 grid for a smooth surface
    //c->setInterpolation(80, 80);

    // Set the view angles
    c->setViewAngle(m_elevationAngle, m_rotationAngle);

    // Add a color axis (the legend) in which the left center is anchored at (660, 270). Set
    // the length to 200 pixels and the labels on the right side.
    c->setColorAxis(650, 270, Chart::Left, 200, Chart::Right);

    // Set the x, y and z axis titles using 10 points Arial Bold font
    c->xAxis()->setTitle("X", "arialbd.ttf", 15);
    c->yAxis()->setTitle("Y", "arialbd.ttf", 15);

    // Set axis label font
    c->xAxis()->setLabelStyle("arial.ttf", 10);
    c->yAxis()->setLabelStyle("arial.ttf", 10);
    c->zAxis()->setLabelStyle("arial.ttf", 10);
    c->colorAxis()->setLabelStyle("arial.ttf", 10);

    // Output the chart
    delete viewer->getChart();
    viewer->setChart(c);
}

void MainWindow::onMouseMoveChart(QMouseEvent *event)
{
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
    if (m_isDragging && (event->button() == Qt::LeftButton))
    {
        // mouse up means not dragging
        m_isDragging = false;
        ui->chartViewer->updateViewPort(true, false);
    }
}

void MainWindow::on_calcButton_clicked()
{
    SystemParams p;
    p.gridSize = ui->gridSizeEdit->text().toFloat();
    p.gridStepsCount = ui->gridStepsEdit->text().toUInt();
    p.gridStep = p.gridSize/(p.gridStepsCount - 1);
    p.dt = ui->dtEdit->text().toFloat();
    p.timeStepsCount = ui->timeStepsEdit->text().toUInt();

    p.mPermMetal = ui->mPermMetalEdit->text().toFloat(); //mu
    p.dPermMetal = ui->dPermMetalEdit->text().toFloat(); //epsilon
    p.eCondMetal = ui->eCondMetalEdit->text().toFloat(); //sigma

    p.mPermVacuum = ui->mPermVacuumEdit->text().toFloat(); //mu
    p.dPermVacuum = ui->dPermVacuumEdit->text().toFloat(); //epsilon
    p.eCondVacuum = ui->eCondVacuumEdit->text().toFloat(); //sigma

    p.mPermPML = ui->mPermPMLEdit->text().toFloat(); //mu
    p.dPermPML = ui->dPermPMLEdit->text().toFloat(); //epsilon
    p.eCondPML = ui->eCondPMLEdit->text().toFloat(); //sigma

    p.pmlCount = ui->pmlCountEdit->text().toUInt();

    p.chargeEx = ui->chargeExEdit->text().toFloat();
    p.chargeEy = ui->chargeEyEdit->text().toFloat();

    ui->slider->setRange(0, p.timeStepsCount - 1);

    if (!maxwell.Calc(p))
    {
        QMessageBox messageBox;
        messageBox.critical(nullptr,"Error","field.cfg is missing in build dir");
    }
}

void MainWindow::on_slider_valueChanged(int value)
{
    curStep = value;
    drawChart(ui->chartViewer);
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    drawChart(ui->chartViewer);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <algorithm>

using namespace std;

const int imgDefaultSize = 500;

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
    GenerateGrid(grid, ui->antennaView->width(), ui->antennaView->height(), 5, 5);
    for (uint i = 0; i < grid.size(); i++)
    {
        antennaScene->addItem(grid[i]);
    }
}

MainWindow::~MainWindow()
{
    for (uint i = 0; i < grid.size(); i++)
    {
        antennaScene->removeItem(grid[i]);
        delete grid[i];
    }
    delete antennaScene;
    delete patternScene;
    delete ui;
}

void MainWindow::GenerateGrid(AntennasGrid &grid, double w, double h, uint cellsx, uint cellsy)
{
    double cellw = (w - 2.)/cellsx;
    double cellh = (h - 2.)/cellsy;
    grid.reserve(cellsx * cellsy);
    for (uint i = 0; i < cellsy; i++)
    {
        for (uint j = 0; j < cellsx; j++)
        {
            grid.push_back(new AntennaCell(cellw, cellh));
            grid[i * cellsx + j]->setPos(cellw * (j - 0.5) + 1., cellh * (i - 0.5) + 1.);
        }
    }

}

template<typename T>
inline T clamp(T val, T lo, T hi)
{
    return std::min(std::max(val, lo), hi);
}

typedef struct
{
    float re;
    float im;
    inline float Magnitude() const
    {
        return sqrtf(MagnitudeSqr());
    }
    inline float MagnitudeSqr() const
    {
        return re * re + im * im;
    }
} Complex;


enum class SignalDrawMode
{
    Amp,
    Real,
    Image
};

void DrawSignal(const vector<Complex> &signal, SignalDrawMode mode, uint size, QGraphicsScene *scene, float scale = 1.)
{
    vector<uchar> img(signal.size());
    for(uint i = 0; i < img.size(); i++)
    {
        float src = 0.f;
        switch(mode)
        {
            case SignalDrawMode::Amp:
                src = std::roundf(signal[i].Magnitude());
            break;
            case SignalDrawMode::Real:
                src = std::roundf(signal[i].re);
            break;
            case SignalDrawMode::Image:
                src = std::roundf(signal[i].im);
            break;
        }
        img[i] = static_cast<uchar>(clamp(scale * src, 0.f, 255.f));
    }
    QPixmap pixmap;
    pixmap.convertFromImage(QImage((const uchar*)img.data(), size, size, QImage::Format_Grayscale8));
    scene->clear();
    scene->addPixmap(pixmap);
}

void fft(Complex *signal, int n, int is)
{
    int i, j, istep;
    int m, mmax;
    float r, r1, theta, w_r, w_i, temp_r, temp_i;
    float pi = 3.1415926f;

    r = pi*is;
    j = 0;
    for (i = 0; i<n; i++)
    {
        if (i<j)
        {
            temp_r = signal[j].re;
            temp_i = signal[j].im;
            signal[j].re = signal[i].re;
            signal[j].im = signal[i].im;
            signal[i].re = temp_r;
            signal[i].im = temp_i;
        }
        m = n >> 1;
        while (j >= m) { j -= m; m = (m + 1) >> 1; }
        j += m;
    }
    mmax = 1;
    while (mmax<n)
    {
        istep = mmax << 1;
        r1 = r / (float)mmax;
        for (m = 0; m<mmax; m++)
        {
            theta = r1*m;
            w_r = cosf(theta);
            w_i = sinf(theta);
            for (i = m; i<n; i += istep)
            {
                j = i + mmax;
                temp_r = w_r*signal[j].re - w_i*signal[j].im;
                temp_i = w_r*signal[j].im + w_i*signal[j].re;
                signal[j].re = signal[i].re - temp_r;
                signal[j].im = signal[i].im - temp_i;
                signal[i].re += temp_r;
                signal[i].im += temp_i;
            }
        }
        mmax = istep;
    }

    if (is <= 0)
    {
        return;
    }

    for (i = 0; i<n; i++)
    {
        signal[i].re /= (float)n;
        signal[i].im /= (float)n;
    }
}

void Transpose(vector<Complex> &signal, uint size)
{
    for(uint i = 0; i < size; i++)
    {
        for(uint j = i + 1; j < size; j++)
        {
            std::swap(signal[i * size + j], signal[j * size + i]);
        }
    }
}

void fft2D(vector<Complex> &signal, uint size, int is)
{
    for(uint i = 0; i < size; i++)
    {
        fft(&signal[i * size], size, is);
    }

    Transpose(signal, size);

    for(uint i = 0; i < size; i++)
    {
        fft(&signal[i * size], size, is);
    }

    Transpose(signal, size);
}

static vector<Complex> cleanSignal;
static uint dimSize;

void MainWindow::on_calcButton_clicked()
{
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <QFileDialog>

using namespace std;

//typedef complex<double> cd;
//typedef vector<cd> vcd;

const int imgDefaultSize = 500;

QGraphicsScene *cleanScene;
QGraphicsScene *noiseScene;
QGraphicsScene *fftScene;
QGraphicsScene *restoredScene;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*ui->cleanPlot->xAxis->setRange(0, signalLength);
    ui->cleanPlot->yAxis->setRange(-16, 16);
    ui->cleanPlot->setInteraction(QCP::iRangeDrag, true);
    ui->cleanPlot->setInteraction(QCP::iRangeZoom, true);
    ui->cleanPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->noisyPlot->xAxis->setRange(0, signalLength);
    ui->noisyPlot->yAxis->setRange(-16, 16);
    ui->noisyPlot->setInteraction(QCP::iRangeDrag, true);
    ui->noisyPlot->setInteraction(QCP::iRangeZoom, true);
    ui->noisyPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->fourierPlot->xAxis->setRange(0, 1);
    ui->fourierPlot->yAxis->setRange(0, 1024);
    ui->fourierPlot->setInteraction(QCP::iRangeDrag, true);
    ui->fourierPlot->setInteraction(QCP::iRangeZoom, true);
    ui->fourierPlot->axisRect()->setRangeZoom(Qt::Horizontal);*/

    ui->signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->signalsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    cleanScene = new QGraphicsScene();
    noiseScene = new QGraphicsScene();
    fftScene = new QGraphicsScene();
    restoredScene = new QGraphicsScene();
    ui->cleanView->setScene(cleanScene);
    ui->cleanView->show();
    ui->noiseView->setScene(noiseScene);
    ui->noiseView->show();
    ui->fftView->setScene(fftScene);
    ui->fftView->show();
    ui->restoredView->setScene(restoredScene);
    ui->restoredView->show();
    srand(static_cast<uint>(time(nullptr)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

float GaussFunc(float x, float y, float sx, float sy)
{
    return std::exp(-0.5f * (x * x / sx / sx + y * y / sy / sy));
}

template<typename T>
T clamp(T val, T lo, T hi)
{
    return std::min(std::max(val, lo), hi);
}

/*vcd fft(const vcd &as) {
  int n = as.size();
  int k = 0; // Длина n в битах
  while ((1 << k) < n) k++;
  vector<int> rev(n);
  rev[0] = 0;
  int high1 = -1;
  for (int i = 1; i < n; i++) {
    if ((i & (i - 1)) == 0) // Проверка на степень двойки. Если i ей является, то i-1 будет состоять из кучи единиц.
      high1++;
    rev[i] = rev[i ^ (1 << high1)]; // Переворачиваем остаток
    rev[i] |= (1 << (k - high1 - 1)); // Добавляем старший бит
  }

  vcd roots(n);
  for (int i = 0; i < n; i++) {
    double alpha = 2 * M_PI * i / n;
    roots[i] = cd(cos(alpha), sin(alpha));
  }

  vcd cur(n);
  for (int i = 0; i < n; i++)
    cur[i] = as[rev[i]];

  for (int len = 1; len < n; len <<= 1) {
    vcd ncur(n);
    int rstep = roots.size() / (len * 2);
    for (int pdest = 0; pdest < n;) {
      int p1 = pdest;
      for (int i = 0; i < len; i++) {
        cd val = roots[i * rstep] * cur[p1 + len];
        ncur[pdest] = cur[p1] + val;
        ncur[pdest + len] = cur[p1] - val;
        pdest++, p1++;
      }
      pdest += len;
    }
    cur.swap(ncur);
  }
  return cur;
}*/

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
    pixmap.convertFromImage(QImage((const uchar*)img.data(), size, size, size * (int)sizeof(uchar), QImage::Format_Grayscale8));
    scene->clear();
    scene->setSceneRect(0., 0., size, size);
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

vector<Complex> GenerateNoise(vector<Complex> &signal, float noiseRate)
{
    vector<Complex> noise(signal.size());
    float signalEnergy = 0.f;
    for(uint i = 0; i < signal.size(); i++)
    {
        signalEnergy += signal[i].MagnitudeSqr();

    }
    const uint randCount = 20;
    float mult = 0.;
    for(uint i = 0; i < noise.size(); i++)
    {
        noise[i].re = 0;
        for(uint j = 0; j < randCount; j++)
        {
            noise[i].re += rand() * 2.f / RAND_MAX - 1.f;
        }
        noise[i].re /= randCount;
        mult += noise[i].re * noise[i].re;
    }
    mult = sqrtf(signalEnergy * 0.01f * noiseRate / mult);
    for(uint i = 0; i < noise.size(); i++)
    {
        noise[i].re *= mult;
        noise[i].re += signal[i].re;
        noise[i].im += signal[i].im;
    }
    return noise;
}

void SwapQuarters(vector<Complex> &signal, uint size)
{
    uint hSize = size/2;
    for(uint i = 0; i < hSize; i++)
    {
        for(uint j = 0; j < hSize; j++)
        {
            std::swap(signal[i * size + j], signal[(i + hSize) * size + j + hSize]);
            std::swap(signal[(i + hSize) * size + j], signal[i * size + j + hSize]);
        }
    }
}

void ClearSpectrum(vector<Complex> &spectrum, uint size, float clipEnergy)
{
    /*noisySignal[0].re = 0;
    noisySignal[0].im = 0;

    noisySignal[size - 1].re = 0;
    noisySignal[size - 1].im = 0;

    noisySignal[noisySignal.size() - 1].re = 0;
    noisySignal[noisySignal.size() - 1].im = 0;

    noisySignal[noisySignal.size() - 1 - size].re = 0;
    noisySignal[noisySignal.size() - 1 - size].im = 0;*/

    float tmpEnergy = 0.f;
    bool clip = false;
    for(uint i = 0; i < size/2; i++)
    {
        for(uint j = 0; j <= i; j++)
        {
            if (clip)
            {
                spectrum[i * size + j].re = 0.f;
                spectrum[i * size + j].im = 0.f;

                spectrum[(size - 1 - i) * size + j].re = 0.f;
                spectrum[(size - 1 - i) * size + j].im = 0.f;

                spectrum[i * size + size - 1 - j].re = 0.f;
                spectrum[i * size + size - 1 - j].im = 0.f;

                spectrum[(size - 1 - i) * size + size - 1 - j].re = 0.f;
                spectrum[(size - 1 - i) * size + size - 1 - j].im = 0.f;

                if(j == i)
                {
                    continue;
                }

                spectrum[j * size + i].re = 0.f;
                spectrum[j * size + i].im = 0.f;

                spectrum[(size - 1 - j) * size + i].re = 0.f;
                spectrum[(size - 1 - j) * size + i].im = 0.f;

                spectrum[j * size + size - 1 - i].re = 0.f;
                spectrum[j * size + size - 1 - i].im = 0.f;

                spectrum[(size - 1 - j) * size + size - 1 - i].re = 0.f;
                spectrum[(size - 1 - j) * size + size - 1 - i].im = 0.f;
            }
            else
            {
               tmpEnergy += spectrum[i * size + j].MagnitudeSqr();
               tmpEnergy += spectrum[(size - 1 - i) * size + j].MagnitudeSqr();
               tmpEnergy += spectrum[i * size + size - 1 - j].MagnitudeSqr();
               tmpEnergy += spectrum[(size - 1 - i) * size + size - 1 - j].MagnitudeSqr();

               if(j == i)
               {
                   continue;
               }

               tmpEnergy += spectrum[j * size + i].MagnitudeSqr();
               tmpEnergy += spectrum[(size - 1 - j) * size + i].MagnitudeSqr();
               tmpEnergy += spectrum[j * size + size - 1 - i].MagnitudeSqr();
               tmpEnergy += spectrum[(size - 1 - j) * size + size - 1 - i].MagnitudeSqr();
            }
        }
        if (tmpEnergy >= clipEnergy)
        {
           clip = true;
        }
    }
}

float lerp(float s, float e, float t)
{
    return s + (e - s) * t;
}
float blerp(float c00, float c10, float c01, float c11, float tx, float ty)
{
    return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

vector<Complex> scale(vector<Complex> &src, uint size, uint newSize){
    uint x, y;
    float coeff = float(size - 1) / newSize;
    vector<Complex> dst(newSize * newSize);
    for(x = 0, y = 0; y < newSize; x++)
    {
        if(x > newSize)
        {
            x = 0;
            y++;
        }
        float gx = x * coeff;
        float gy = y * coeff;
        uint gxi = static_cast<uint>(roundf(gx));
        uint gyi = static_cast<uint>(roundf(gy));
        float c00 = src[gyi * size + gxi].re;
        float c10 = src[gyi * size + gxi + 1].re;
        float c01 = src[(gyi + 1) * size + gxi].re;
        float c11 = src[(gyi + 1) * size + gxi + 1].re;
        dst[y * newSize + x].re = blerp(c00, c10, c01, c11, gx - gxi, gy -gyi);
    }
    return dst;
}

static vector<Complex> cleanSignal;
static vector<Complex> noisySignal;
static uint dimSize;
static uint dimSizeScaled;

void ProcessSignal(vector<Complex> &signal, uint size, float noiseRate)
{
    noisySignal = GenerateNoise(signal, noiseRate);
    DrawSignal(noisySignal, SignalDrawMode::Real, size, noiseScene);

    uint nearestPow = 1;
    while(nearestPow < size) nearestPow <<= 1;
    dimSizeScaled = (nearestPow - size) < (size - (nearestPow >> 1)) ? nearestPow : (nearestPow >> 1);
    if (dimSizeScaled != size)
    {
        noisySignal = scale(noisySignal, size, dimSizeScaled);
    }

    fft2D(noisySignal, dimSizeScaled, -1);
    SwapQuarters(noisySignal, dimSizeScaled);
    DrawSignal(noisySignal, SignalDrawMode::Amp, dimSizeScaled, fftScene, 0.01f);
    SwapQuarters(noisySignal, dimSizeScaled);
}

void MainWindow::on_openButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open BMP"), ".", tr("*.bmp"));
    if(filename.isEmpty())
    {
        return;
    }
    QImage img(filename);
    img = img.convertToFormat(QImage::Format_Grayscale8);
    uchar *data = img.bits();
    if (img.isNull() || data == nullptr)
    {
        return;
    }
    uint size = img.width();
    cleanSignal.clear();
    cleanSignal.resize(size * size, Complex{0.f, 0.f});
    for(uint i = 0; i < cleanSignal.size(); i++)
    {
        cleanSignal[i].re = data[i];
    }
    dimSize = size;
    DrawSignal(cleanSignal, SignalDrawMode::Real, size, cleanScene);
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_genButton_clicked()
{
    const uint size = ui->sizeEdit->text().toUInt();
    cleanSignal.clear();
    cleanSignal.resize(size * size, Complex{0., 0.});
    for(int s = 0; s < ui->signalsTable->rowCount(); s++)
    {
        int x0 = ui->signalsTable->item(s, 0)->text().toInt();
        int y0 = ui->signalsTable->item(s, 1)->text().toInt();
        float sx = ui->signalsTable->item(s, 2)->text().toFloat();
        float sy = ui->signalsTable->item(s, 3)->text().toFloat();
        float a = ui->signalsTable->item(s, 4)->text().toFloat();
        for(int y = 0; y < size; y++)
        {
            for(int x = 0; x < size; x++)
            {
                cleanSignal[y * size + x].re += a * GaussFunc(x - x0, y - y0, sx, sy);
            }
        }
    }

    dimSize = size;
    DrawSignal(cleanSignal, SignalDrawMode::Real, size, cleanScene);
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_noiseButton_clicked()
{
    if (cleanSignal.empty())
    {
        return;
    }

    float noiseRate = ui->noiseRateEdit->text().toFloat();
    ProcessSignal(cleanSignal, dimSize, noiseRate);
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::on_clearButton_clicked()
{
    if (noisySignal.empty())
    {
        return;
    }

    vector<Complex> copySignal(noisySignal);

    float clipRate = ui->clipRateEdit->text().toFloat();
    float clipEnergy = 0.f;
    for(uint i = 0; i < copySignal.size(); i++)
    {
        clipEnergy += copySignal[i].MagnitudeSqr();
    }
    clipEnergy *= clipRate * 0.01f;

    ClearSpectrum(copySignal, dimSizeScaled, clipEnergy);

    fft2D(copySignal, dimSizeScaled, 1);
    if (dimSizeScaled != dimSize) {
       copySignal = scale(copySignal, dimSizeScaled, dimSize);
    }
    DrawSignal(copySignal, SignalDrawMode::Real, dimSize, restoredScene);
    ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::on_addRowButton_clicked()
{
    if(ui->signalsTable->rowCount() < 5)
        ui->signalsTable->setRowCount(ui->signalsTable->rowCount()+1);
}

void MainWindow::on_delRowButton_clicked()
{
    if(ui->signalsTable->rowCount() > 1)
        ui->signalsTable->setRowCount(ui->signalsTable->rowCount()-1);
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>

using namespace std;

const int imgDefaultSize = 500;

static QGraphicsScene *cleanScene;
static QGraphicsScene *noiseScene;
static QGraphicsScene *fftScene;
static QGraphicsScene *restoredScene;
static QGraphicsRectItem *clipRect = nullptr;
static QGraphicsPixmapItem *clearPixmap = nullptr;
static QGraphicsPixmapItem *noisyPixmap = nullptr;
static QGraphicsPixmapItem *fftPixmap = nullptr;
static QGraphicsPixmapItem *restoredPixmap = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->signalsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    cleanScene = new QGraphicsScene();
    noiseScene = new QGraphicsScene();
    fftScene = new QGraphicsScene();
    restoredScene = new QGraphicsScene();
    clearPixmap = new QGraphicsPixmapItem();
    ui->cleanView->setScene(cleanScene);
    cleanScene->addItem(clearPixmap);
    ui->cleanView->show();
    ui->noiseView->setScene(noiseScene);
    noisyPixmap = new QGraphicsPixmapItem();
    noiseScene->addItem(noisyPixmap);
    ui->noiseView->show();
    ui->fftView->setScene(fftScene);
    fftPixmap = new QGraphicsPixmapItem();
    clipRect = new QGraphicsRectItem();
    clipRect->setPen(QPen(Qt::red));
    fftScene->addItem(fftPixmap);
    fftScene->addItem(clipRect);
    ui->fftView->show();
    ui->restoredView->setScene(restoredScene);
    restoredPixmap = new QGraphicsPixmapItem();
    restoredScene->addItem(restoredPixmap);
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

QPixmap DrawSignal(const vector<Complex> &signal, SignalDrawMode mode, uint size, bool lg = false, float scale = 1.)
{
    vector<uchar> img(signal.size());
    float max = 1.;
    switch(mode)
    {
        case SignalDrawMode::Amp:
            if (!lg)
            {
                break;
            }
            max = std::max_element(signal.begin(), signal.end(), [](const Complex& a, const Complex&b) {
                return a.Magnitude() < b.Magnitude();
            })->Magnitude() * scale * 0.001f;
        break;
        case SignalDrawMode::Real:
            max = std::max_element(signal.begin(), signal.end(), [](const Complex& a, const Complex&b) {
                return a.re < b.re;
            })->re;
        break;
        case SignalDrawMode::Image:
            max = std::max_element(signal.begin(), signal.end(), [](const Complex& a, const Complex&b) {
                return a.im < b.im;
            })->im;
        break;
    }
    for(uint i = 0; i < img.size(); i++)
    {
        float src = 0.f;
        switch(mode)
        {
            case SignalDrawMode::Amp:
                src = signal[i].Magnitude();
            break;
            case SignalDrawMode::Real:
                src = signal[i].re;
            break;
            case SignalDrawMode::Image:
                src = signal[i].im;
            break;
        }
        src = scale * src / max;
        if(lg) {
            src = std::log10(src);
        }
        img[i] = static_cast<uchar>(clamp(std::roundf(255.f * src), 0.f, 255.f));
    }
    QPixmap pixmap;
    pixmap.convertFromImage(QImage((const uchar*)img.data(), size, size, size * (int)sizeof(uchar), QImage::Format_Grayscale8));
    return pixmap;
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

uint ClearSpectrum(vector<Complex> &spectrum, uint size, float clipEnergy)
{
    float tmpEnergy = 0.f;
    bool clip = false;
    uint clipi = 0;
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
           if(!clip)
           {
               clipi = i;
           }
           clip = true;
        }
    }
    return clipi;
}

float lerp(float s, float e, float t)
{
    return s + (e - s) * t;
}
float blerp(float c00, float c10, float c01, float c11, float tx, float ty)
{
    return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

vector<Complex> scale(vector<Complex> &src, uint size, uint newSize)
{
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

float Error(const vector<Complex>& n, const vector<Complex>& s)
{
    float enn = 0.;
    float ens = 0.;
    for (uint i = 0; i < n.size(); i++)
    {
        enn += (n[i].re - s[i].re) * (n[i].re - s[i].re) + (n[i].im - s[i].im) * (n[i].im - s[i].im);
        ens += s[i].MagnitudeSqr();
    }
    return enn/ens;
}

static vector<Complex> cleanSignal;
static vector<Complex> noisySignal;
static vector<Complex> fftSignal;
static uint dimSize;
static uint dimSizeScaled;

void ProcessSignal(vector<Complex> &signal, uint size, float noiseRate)
{
    noisySignal = GenerateNoise(signal, noiseRate);

    noisyPixmap->setPixmap(DrawSignal(noisySignal, SignalDrawMode::Real, size));
    noiseScene->update();

    uint nearestPow = 1;
    while(nearestPow < size) nearestPow <<= 1;
    dimSizeScaled = (nearestPow - size) < (size - (nearestPow >> 1)) ? nearestPow : (nearestPow >> 1);
    if (dimSizeScaled != size)
    {
        fftSignal = scale(noisySignal, size, dimSizeScaled);
    } else {
        fftSignal = noisySignal;
    }

    fft2D(fftSignal, dimSizeScaled, -1);
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

    clearPixmap->setPixmap(DrawSignal(cleanSignal, SignalDrawMode::Real, size));
    cleanScene->update();

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

    clearPixmap->setPixmap(DrawSignal(cleanSignal, SignalDrawMode::Real, size));
    cleanScene->update();

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

    SwapQuarters(fftSignal, dimSizeScaled);
    fftPixmap->setPixmap(DrawSignal(fftSignal, SignalDrawMode::Amp, dimSizeScaled, ui->logCheckBox->checkState() == Qt::Checked, 0.01f / 255.f));
    SwapQuarters(fftSignal, dimSizeScaled);
    fftScene->update();

    ui->ntosEdit->setText(QString::number(Error(noisySignal, cleanSignal)));
    ui->tabWidget->setCurrentIndex(1);
    ui->logCheckBox->setCheckState(Qt::Unchecked);
}

void MainWindow::on_clearButton_clicked()
{
    if (fftSignal.empty())
    {
        return;
    }

    vector<Complex> copySignal(fftSignal);

    float clipRate = ui->clipRateEdit->text().toFloat();
    float clipEnergy = 0.f;
    for(uint i = 0; i < copySignal.size(); i++)
    {
        clipEnergy += copySignal[i].MagnitudeSqr();
    }
    clipEnergy *= clipRate * 0.01f;

    uint clip = 2 * ClearSpectrum(copySignal, dimSizeScaled, clipEnergy);
    clipRect->setRect(dimSizeScaled/2 - clip/2 - 1, dimSizeScaled/2 - clip/2 - 1, clip + 1, clip + 1);
    fftScene->update();

    fft2D(copySignal, dimSizeScaled, 1);
    if (dimSizeScaled != dimSize) {
       copySignal = scale(copySignal, dimSizeScaled, dimSize);
    }

    restoredPixmap->setPixmap(DrawSignal(copySignal, SignalDrawMode::Real, dimSize));
    restoredScene->update();

    ui->rtosEdit->setText(QString::number(Error(copySignal, cleanSignal)));

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

void MainWindow::on_logCheckBox_stateChanged(int arg1)
{
    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        SwapQuarters(fftSignal, dimSizeScaled);
        fftPixmap->setPixmap(DrawSignal(fftSignal, SignalDrawMode::Amp, dimSizeScaled, arg1 == Qt::Checked, 0.01f / 255.f));
        SwapQuarters(fftSignal, dimSizeScaled);
        fftScene->update();
        break;
    case 3:
        break;
    }
}

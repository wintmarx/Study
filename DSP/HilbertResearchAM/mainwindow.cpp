#define _USE_MATH_DEFINES

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <complex>

using namespace std;

typedef complex<double> cd;
typedef vector<cd> vcd;

vcd fft(const vcd &as)
{
    int n = as.size();
    int k = 0; // Длина n в битах
    while ((1 << k) < n) k++;
    vector<int> rev(n);
    rev[0] = 0;
    int high1 = -1;
    for (int i = 1; i < n; i++)
    {
        // Проверка на степень двойки. Если i ей является, то i-1 будет состоять из кучи единиц.
        if ((i & (i - 1)) == 0)
        {
            high1++;
        }
        // Переворачиваем остаток
        rev[i] = rev[i ^ (1 << high1)];
        // Добавляем старший бит
        rev[i] |= (1 << (k - high1 - 1));
    }

    vcd roots(n);
    for (int i = 0; i < n; i++)
    {
        double alpha = 2 * M_PI * i / n;
        roots[i] = cd(cos(alpha), sin(alpha));
    }

    vcd cur(n);
    for (int i = 0; i < n; i++)
    {
        cur[i] = as[rev[i]];
    }

    for (int len = 1; len < n; len <<= 1)
    {
        vcd ncur(n);
        int rstep = roots.size() / (len * 2);
        for (int pdest = 0; pdest < n;)
        {
            int p1 = pdest;
            for (int i = 0; i < len; i++)
            {
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
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->researchPlot->setInteraction(QCP::iRangeDrag, true);
    ui->researchPlot->setInteraction(QCP::iRangeZoom, true);
    //ui->researchPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    //ui->researchPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    ui->cleanPlot->setInteraction(QCP::iRangeDrag, true);
    ui->cleanPlot->setInteraction(QCP::iRangeZoom, true);
    ui->cleanPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->cleanPlot->axisRect()->setRangeZoom(Qt::Horizontal);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void GenerateNoise(double *signal, int length, double signalEnergy, double noisePart)
{
    srand(time(NULL) | clock());
    int randCount = 20;
    double mult = 0;
    double *noise = (double*)malloc(sizeof(double) * length);
    for(int i = 0; i < length; i++)
    {
        noise[i] = 0;
        for(int j = 0; j < randCount; j++)
        {
            noise[i] += rand() * 2./RAND_MAX - 1.;
        }
        noise[i] /= randCount;
        mult += noise[i] * noise[i];
    }
    mult = sqrt(signalEnergy * noisePart / (100 * mult));
    for(int i = 0; i < length; i++)
    {
        noise[i] *= mult;
        signal[i] += noise[i];
    }
    free(noise);
}

int generateSignal(double **signalValues, double m, int signalFreq, int samplingFreq)
{
    int signalLength = (int)round(1./ signalFreq * 10 * samplingFreq);

    int expandedSignalLength;
    for(expandedSignalLength = 1; expandedSignalLength < signalLength; expandedSignalLength<<=1);

    (*signalValues) = new double[expandedSignalLength];
    memset(*signalValues, 0, expandedSignalLength * sizeof(double));

    for(int i = 0; i < signalLength; i++)
    {
        (*signalValues)[i] = (1 + m * cos(2 * M_PI * signalFreq * i * 1./samplingFreq));
    }

    return expandedSignalLength;
}

void generateCarrierSignal(double *carrierSignal, double *signal, int signalLength, int carrierFreq, int samplingFreq, double deviationCoeff)
{
    double sign = -1;
    double w = 2 * M_PI * carrierFreq;
    double samplingT = 1./samplingFreq;
    double freqDev = w/8. * deviationCoeff;
    double phase = -2 * freqDev;
    double curPhase = 0;

    for(int i = 0, c = 1; i < signalLength; i++)
    {
        carrierSignal[i] = (signal[i] * sin((w + sign * freqDev) * i * samplingT + curPhase));

        if(i > 1 && carrierSignal[i - 1] > carrierSignal[i] && carrierSignal[i - 1] > carrierSignal[i - 2])
        {
            if(c >= 10)
            {
                sign = -sign;
                curPhase = sign * phase * i * samplingT + curPhase;
                c = 0;
                i--;
            }
            else
            {
                c++;
            }
        }
    }
}

void hilbertTransform(double *hilbert, double *signal, int signalLength)
{
    vcd transform;
    for(int i = 0; i < signalLength; i++)
    {
        transform.push_back(signal[i]);
    }

    transform = fft(transform);

    for(int i = 0; i < signalLength/2; i++)
    {
        transform[signalLength/2 + i].real(0);
        transform[signalLength/2 + i].imag(0);
        transform[i].real(transform[i].real() * 2);
        transform[i].imag(transform[i].imag() * 2);
    }

    transform = fft(transform);

    hilbert[0] = transform[0].imag()/signalLength;
    for(int i = 1; i < signalLength; i++)
    {
        hilbert[i] = transform[signalLength - i].imag()/signalLength;
    }
}

void MainWindow::on_genButton_clicked()
{
    double *signalValues;
    double *carrierValues;
    double *hilbertValues;
    int length;
    double signalEnergy;
    double m = 0.5;
    double deviationCoeff = 0.5;
    int signalFreq = 3000;
    int samplingFreq = 500000;
    int carrierFreq = 50000;

    m = ui->mEdit->text().toDouble();
    deviationCoeff = ui->devCoeffEdit->text().toDouble();
    signalFreq = ui->signalFreqEdit->text().toInt();
    carrierFreq = ui->carrierFreqEdit->text().toInt();
    samplingFreq = ui->samplingFreqEdit->text().toInt();

    length = generateSignal(&signalValues, m, signalFreq, samplingFreq);
    carrierValues = new double[length];
    hilbertValues = new double[length];

    generateCarrierSignal(carrierValues, signalValues, length, carrierFreq, samplingFreq, deviationCoeff);

    //GenerateNoise(signalValues, length, signalEnergy, ui->noisePartEdit->text().toDouble());

    ui->cleanPlot->clearGraphs();
    ui->cleanPlot->addGraph();

    ui->cleanPlot->addGraph();
    ui->cleanPlot->graph(1)->setPen(QPen(QColor(255, 0, 0)));

    double samplingT = 1./samplingFreq;
    for(int i = 0; i < length; i++)
    {
        ui->cleanPlot->graph(0)->addData(i * samplingT, carrierValues[i]);
        ui->cleanPlot->graph(1)->addData(i * samplingT, signalValues[i]);
    }

    hilbertTransform(hilbertValues, carrierValues, length);

    ui->cleanPlot->addGraph();
    ui->cleanPlot->graph(2)->setPen(QPen(QColor(0, 255, 0)));
    double epsilon = 0;
    double calSignalSample;
    for(int i = 0; i < length; i++)
    {
        calSignalSample = sqrt(pow(hilbertValues[i], 2) + pow(carrierValues[i], 2));
        epsilon += pow(calSignalSample - signalValues[i], 2);
        ui->cleanPlot->graph(2)->addData(i * samplingT, calSignalSample);
    }
    ui->eEdit->setText(QString::number(epsilon));

    ui->cleanPlot->rescaleAxes(true);
    ui->cleanPlot->replot();

    delete[] signalValues;
    delete[] carrierValues;
    delete[] hilbertValues;
}

void MainWindow::on_researchButton_clicked()
{
    double *signalValues;
    double *carrierValues;
    double *hilbertValues;
    int length;
    double signalEnergy;
    double nMin = 0.1;
    double nMax = 0.5;
    double nStep;
    int passesCount = 100;
    int samplingFreq = 1000000;
    int carrierFreq = 50000;
    int signalFreq = 6000;
    double m = 0.5;
    double epsilon;

    m = ui->mEdit->text().toDouble();
    signalFreq = ui->signalFreqEdit->text().toInt();
    carrierFreq = ui->carrierFreqEdit->text().toInt();
    samplingFreq = ui->samplingFreqEdit->text().toInt();
    nMin = ui->nMinEdit->text().toDouble();
    nMax = ui->nMaxEdit->text().toDouble();
    passesCount = ui->passesCountEdit->text().toInt();

    nStep = (nMax - nMin)/passesCount;

    ui->researchPlot->clearGraphs();
    ui->researchPlot->addGraph();

    length = generateSignal(&signalValues, m, signalFreq, samplingFreq);
    carrierValues = new double[length];
    hilbertValues = new double[length];

    for(double n = nMin; n <= nMax; n += nStep)
    {
        memset(carrierValues, 0, length * sizeof(double));
        memset(hilbertValues, 0, length * sizeof(double));
        generateCarrierSignal(carrierValues, signalValues, length, carrierFreq, samplingFreq, n);
        hilbertTransform(hilbertValues, carrierValues, length);
        epsilon = 0;
        for(int i = 0; i < length; i++)
        {
            epsilon += pow(sqrt(pow(hilbertValues[i], 2) + pow(carrierValues[i], 2)) - signalValues[i], 2);
        }
        ui->researchPlot->graph(0)->addData(n, epsilon);
    }

    ui->researchPlot->rescaleAxes(true);
    ui->researchPlot->replot();

    delete[] signalValues;
    delete[] carrierValues;
    delete[] hilbertValues;
}

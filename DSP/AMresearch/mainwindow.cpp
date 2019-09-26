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

vcd fft(const vcd &as) {
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

    ui->fourierPlot->setInteraction(QCP::iRangeDrag, true);
    ui->fourierPlot->setInteraction(QCP::iRangeZoom, true);
    ui->fourierPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->fourierPlot->axisRect()->setRangeZoom(Qt::Horizontal);
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

void generate_data(int **data, int count)
{
    srand(time(NULL) | clock());
    *data = (int*)malloc(sizeof(int) * count);
    rand();
    for(int i = 0; i < count ; i++)
    {
        (*data)[i] = (rand() < (RAND_MAX / 2) ? 1 : 0);
    }
}

int calculate_width(double *signal, int *width, int length, double level)
{
    *width = 0;
    uint8_t flag = 0;
    int start;
    int i = 0;
    for(i; i < length/2; i++)
    {
        if(!(flag & 1) && signal[i] <= level && signal[i + 1] > level)
        {
            *width -= i;
            flag |= 1;
            start = i;
        }
        if(!(flag & 2) && signal[length/2 - i] < level && signal[length/2 - i - 1] >= level)
        {
            *width += length/2 - i;
            flag |= 2;
        }
        if(flag == (2 | 1))
        {
            break;
        }
    }
    return start;
}

void generate_signal(int *data, double **signalValues, int *length, int *expandedLength, double *energy, int bitrate, int dataSize, int samplingFreq, int carrierFreq)
{
    int signalLength = dataSize * (int)round(1.0 / bitrate * samplingFreq);
    *energy = 0;
    int expandedSignalLength;
    int nearestPow = 1;
    for(nearestPow = 1; nearestPow < signalLength; nearestPow *= 2);

    expandedSignalLength = nearestPow;
    *length = signalLength;
    *expandedLength = nearestPow;
    *signalValues = (double*)malloc(expandedSignalLength * sizeof(double));


    double phase = 0;
    int modulationPeriodSamples = (int)round(1.0 / bitrate * samplingFreq);
    for(int i = 0, j = 0; i < signalLength; i++)
    {
        if(i % modulationPeriodSamples == 0)
            phase = M_PI * (data[j++]);
        (*signalValues)[i] = cos(2.0 * M_PI * carrierFreq * i/samplingFreq + phase);
        *energy += (*signalValues)[i] * (*signalValues)[i];
    }

    for(int i = signalLength; i < expandedSignalLength; i++)
    {
        (*signalValues)[i] = 0;
    }
}

void MainWindow::on_genButton_clicked()
{
    double *signalValues;
    double *spectrum;
    int *data;
    int length;
    int expandedLength;
    double signalEnergy;
    int dataSize = 100;
    int samplingFreq = 250000;
    int bitrate = 9600;
    int carrierFreq = 25000;
    vcd transformedSignalValues;

    bitrate = ui->bitrateEdit->text().toInt();
    carrierFreq = ui->carrierFreqEdit->text().toInt();
    samplingFreq = ui->samplingFreqEdit->text().toInt();
    dataSize = ui->dataSizeEdit->text().toInt();
    generate_data(&data, dataSize);
    ui->deltaEdit->setText("");
    for(int i = 0; i < dataSize; i++)
    {
        ui->deltaEdit->setText(ui->deltaEdit->text().append(QString::number(data[i])));
    }
    generate_signal(data, &signalValues, &length, &expandedLength, &signalEnergy, bitrate, dataSize, samplingFreq, carrierFreq);
    GenerateNoise(signalValues, length, signalEnergy, ui->noisePartEdit->text().toDouble());
    double minSignalLevel = signalValues[0];
    double maxSignalLevel = signalValues[0];
    for(int i = 1; i < length; i++)
    {
        if(signalValues[i] < minSignalLevel)
        {
            minSignalLevel = signalValues[i];
        }
        else if(signalValues[i] > maxSignalLevel)
        {
            maxSignalLevel = signalValues[i];
        }
    }
    ui->cleanPlot->yAxis->setRange(minSignalLevel, maxSignalLevel);

    ui->cleanPlot->clearGraphs();
    ui->cleanPlot->addGraph();
    ui->cleanPlot->xAxis->setRange(0, length * 1.0 /samplingFreq);

    ui->fourierPlot->clearGraphs();
    ui->fourierPlot->addGraph();
    ui->fourierPlot->addGraph();
    ui->fourierPlot->xAxis->setRange(0, samplingFreq);

    for(int i = 0; i < expandedLength; i++)
    {
        if(i < length)
        {
            ui->cleanPlot->graph(0)->addData(i * 1.0 / samplingFreq,  signalValues[i]);
        }
        transformedSignalValues.push_back(cd(signalValues[i], 0));
    }

    transformedSignalValues = fft(transformedSignalValues);

    spectrum = (double*)malloc(sizeof(double) * expandedLength);
    double maxSpectrumAmp = 0;
    for(int i = 0; i < expandedLength; i++)
    {
        spectrum[i] = sqrt(pow(transformedSignalValues[i].imag(),2) + pow(transformedSignalValues[i].real(),2));
        ui->fourierPlot->graph(0)->addData(i*1./expandedLength * samplingFreq, spectrum[i]);
        if(spectrum[i] > maxSpectrumAmp)
        {
            maxSpectrumAmp = spectrum[i];
        }
    }
    ui->fourierPlot->yAxis->setRange(0, maxSpectrumAmp);
    maxSpectrumAmp /= sqrt(2);
    ui->fourierPlot->setBackground(QBrush(QColor(128,128,128)));
    ui->fourierPlot->graph(0)->setPen(QPen(QColor(0, 0, 0)));
    ui->fourierPlot->graph(1)->setPen(QPen(QColor(255, 0, 0)));
    ui->fourierPlot->graph(1)->addData(0, maxSpectrumAmp);
    ui->fourierPlot->graph(1)->addData(samplingFreq, maxSpectrumAmp);
    int width;
    ui->fourierPlot->addGraph();
    int startIndex = calculate_width(spectrum, &width, expandedLength, maxSpectrumAmp);
    width *= 1./expandedLength * samplingFreq;
    startIndex *= 1./expandedLength * samplingFreq;
    ui->fourierPlot->graph(2)->setPen(QPen(QColor(255, 255, 0)));
    ui->fourierPlot->graph(2)->addData(startIndex, maxSpectrumAmp);
    ui->fourierPlot->graph(2)->addData(startIndex + width, maxSpectrumAmp);

    ui->widthEdit->setText(QString::number(width));

    ui->fourierPlot->replot();
    ui->cleanPlot->replot();
    free(signalValues);
    free(data);
    free(spectrum);
    transformedSignalValues.clear();
}

void MainWindow::on_pushButton_clicked()
{
    double *signalValues;
    double *spectrum;
    int *researchValues;
    int researchLength;
    int *data;
    int length;
    int expandedLength;
    double signalEnergy;
    int dataSize = 100;
    int samplingFreq = 250000;
    int bitrateMin = 1000;
    int bitrateMax = 12000;
    int bitrateStep = 500;
    int carrierFreq = 25000;
    vcd transformedSignalValues;

    bitrateMin = ui->bitrateMinEdit->text().toInt();
    bitrateMax = ui->bitrateMaxEdit->text().toInt();
    bitrateStep = ui->bitrateStepEdit->text().toInt();

    ui->researchPlot->clearGraphs();
    ui->researchPlot->addGraph();
    ui->researchPlot->xAxis->setRange(bitrateMin, bitrateMax);

    carrierFreq = ui->carrierFreqEdit->text().toInt();
    samplingFreq = ui->samplingFreqEdit->text().toInt();

    dataSize = ui->dataSizeEdit->text().toInt();

    researchLength = (bitrateMax - bitrateMin) / bitrateStep + 1;
    researchValues = (int*)malloc(sizeof(int) * researchLength);
    memset(researchValues, 0, researchLength * sizeof(int));

    int averagePassCount = ui->passCountEdit->text().toInt();

    for(int k = 0; k < averagePassCount; k++)
    {
        generate_data(&data, dataSize);
       /* ui->deltaEdit->setText("");
        for(int i = 0; i < dataSize; i++)
        {
            ui->deltaEdit->setText(ui->deltaEdit->text().append(QString::number(data[i])));
        }*/

        for(int bitrate = bitrateMin, pass = 0; bitrate <= bitrateMax; bitrate += bitrateStep, pass++)
        {
            generate_signal(data, &signalValues, &length, &expandedLength, &signalEnergy, bitrate, dataSize, samplingFreq, carrierFreq);
            GenerateNoise(signalValues, length, signalEnergy, ui->noisePartEdit->text().toDouble());
            for(int i = 0; i < expandedLength; i++)
            {
                transformedSignalValues.push_back(cd(signalValues[i], 0));
            }
            transformedSignalValues = fft(transformedSignalValues);
            spectrum = (double*)malloc(sizeof(double) * expandedLength);
            double maxSpectrumAmp = 0;
            for(int i = 0; i < expandedLength; i++)
            {
                spectrum[i] = sqrt(pow(transformedSignalValues[i].imag(),2) + pow(transformedSignalValues[i].real(),2));
                if(spectrum[i] > maxSpectrumAmp)
                {
                    maxSpectrumAmp = spectrum[i];
                }
            }
            maxSpectrumAmp /= sqrt(2);

           int tmpResearchValue = researchValues[pass];
           calculate_width(spectrum, researchValues + pass, expandedLength, maxSpectrumAmp);
           researchValues[pass] *= 1./expandedLength * samplingFreq;
           researchValues[pass] += tmpResearchValue;

            free(signalValues);
            free(spectrum);
            transformedSignalValues.clear();
        }
    }

    researchValues[0] /= averagePassCount;
    ui->researchPlot->graph(0)->addData(bitrateMin, researchValues[0]);
    int maxResearchValue = researchValues[0];
    int minResearchValue = researchValues[0];

    for(int i = 1; i < researchLength; i++)
    {
        researchValues[i] /= averagePassCount;
        ui->researchPlot->graph(0)->addData(i * bitrateStep + bitrateMin, researchValues[i]);
        if(researchValues[i] > maxResearchValue)
        {
            maxResearchValue = researchValues[i];
        }
        if(researchValues[i] < minResearchValue)
        {
            minResearchValue = researchValues[i];
        }
    }



    ui->researchPlot->yAxis->setRange(minResearchValue, maxResearchValue);
    ui->researchPlot->replot();
    free(data);
}

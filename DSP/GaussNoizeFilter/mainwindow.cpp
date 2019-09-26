#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <complex>

using namespace std;

typedef complex<double> cd;
typedef vector<cd> vcd;

const int signalLength = 512;
double signalValues[signalLength];
double noiseValues[signalLength];
double clearSignalEnergy;
double noisySignalEnergy;
vcd transformedSignalValues;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->cleanPlot->xAxis->setRange(0, signalLength);
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
    ui->fourierPlot->axisRect()->setRangeZoom(Qt::Horizontal);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void GenerateNoise(double signalEnergy, double noisePart)
{
    srand(time(NULL) | clock());
    int randCount = 20;
    double mult = 0;
    noisySignalEnergy = 0;
    for(int i = 0; i < signalLength; i++)
    {
        noiseValues[i] = 0;
        for(int j = 0; j < randCount; j++)
        {
            noiseValues[i] += rand() * 2./RAND_MAX - 1.;
        }
        noiseValues[i] /= randCount;
        mult += noiseValues[i] * noiseValues[i];
    }
    mult = sqrt(signalEnergy * noisePart / (100 * mult));
    for(int i = 0; i < signalLength; i++)
    {
        noiseValues[i] *= mult;
        noisySignalEnergy += pow(noiseValues[i] + signalValues[i], 2);
    }
}

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

void MainWindow::on_genButton_clicked()
{
    ui->cleanPlot->clearGraphs();
    ui->noisyPlot->clearGraphs();
    ui->fourierPlot->clearGraphs();

    ui->cleanPlot->addGraph();
    clearSignalEnergy = 0;
    for(int i = 0; i<signalLength; i++)
    {
        signalValues[i] = 0;
        for(int j = 0; j < ui->signalsTable->rowCount(); j++)
            signalValues[i] += ui->signalsTable->item(j,0)->text().toDouble() * sin(2 * 3.1415 * ui->signalsTable->item(j,1)->text().toDouble() * i + ui->signalsTable->item(j,2)->text().toDouble());

        ui->cleanPlot->graph(0)->addData(i,  signalValues[i]);
        clearSignalEnergy+= signalValues[i] * signalValues[i];
    }
    GenerateNoise(clearSignalEnergy, ui->noisePartEdit->text().toDouble());

    ui->noisyPlot->addGraph();
    ui->noisyPlot->graph(0)->setPen(QPen(QColor(255, 0, 0)));
    transformedSignalValues.clear();
    for(int i = 0; i<signalLength; i++)
    {
        ui->noisyPlot->graph(0)->addData(i,  signalValues[i] + noiseValues[i]);
        transformedSignalValues.push_back(cd(signalValues[i] + noiseValues[i], 0));
    }

    transformedSignalValues = fft(transformedSignalValues);

    ui->fourierPlot->addGraph();

    double fourierEnergy = 0;
    double tmp = 0;
    for(int i = 0; i < signalLength; i++)
    {
        tmp = pow(transformedSignalValues[i].imag(),2) + pow(transformedSignalValues[i].real(),2);
        ui->fourierPlot->graph(0)->addData(i*1./signalLength, sqrt(tmp));
        fourierEnergy += tmp;
    }

    double currentEnergy = 0;
    int clippingIndex = 0;
    double energyClipping = ui->energyClippingEdit->text().toDouble();
    while(currentEnergy < fourierEnergy * energyClipping / 100)
    {
            currentEnergy += pow(transformedSignalValues[clippingIndex].real(),2)
                    + pow(transformedSignalValues[clippingIndex].imag(),2)
                    + pow(transformedSignalValues[signalLength - 1 - clippingIndex].real(),2)
                    + pow(transformedSignalValues[signalLength - 1 - clippingIndex].imag(),2);
            clippingIndex++;
    }

    for(int i = clippingIndex; i < signalLength - clippingIndex; i++)
    {
        transformedSignalValues[i].real(0);
        transformedSignalValues[i].imag(0);
    }

    ui->fourierPlot->addGraph();
    ui->fourierPlot->graph(1)->setPen(QPen(QColor(255, 0, 0)));
    for(int i = 0; i < signalLength; i++)
    {
        tmp = pow(transformedSignalValues[i].imag(),2) + pow(transformedSignalValues[i].real(),2);
        ui->fourierPlot->graph(1)->addData(i*1./signalLength, sqrt(tmp));
    }

    transformedSignalValues = fft(transformedSignalValues);
    ui->cleanPlot->addGraph();
    ui->cleanPlot->graph(1)->setPen(QPen(QColor(255, 0, 0)));
    ui->cleanPlot->graph(1)->addData(0, transformedSignalValues[0].real()/signalLength);
    double delta = 0;
    for(int i = 1; i < signalLength; i++)
    {
        ui->cleanPlot->graph(1)->addData(i, transformedSignalValues[signalLength - i].real()/signalLength);
        delta += pow(transformedSignalValues[signalLength - i].real()/signalLength - signalValues[i],2);

    }

    ui->deltaEdit->setText(QString::number(delta));
    ui->cleanPlot->replot();
    ui->noisyPlot->replot();
    ui->fourierPlot->replot();
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

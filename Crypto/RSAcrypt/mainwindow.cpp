#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "rsa.h"
#include <ctime>
#include <QFileDialog>

RSA rsa;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    srand(time(NULL) | clock());
    ui->encryptButton->setEnabled(false);
    ui->decryptButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString bytes2Str(char* bytes, int size) {
  char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B','C','D','E','F'};
  QString str;
  for (int i = 0; i < size; ++i) {
    str.append(QChar(hex[(bytes[size - i - 1] & 0xF0) >> 4]));
    str.append(QChar(hex[ bytes[size - i - 1] & 0x0F      ]));
  }
  //qDebug("bytes2Str: bytes: %#llX", *((uint64_t*)bytes));
  //qDebug("bytes2Str: str: %s", str.toLatin1().data());
  return str;
}

char* str2Bytes(QString string)
{
    char *bytes = new char[string.size()/2];
    char tmp[3];
    tmp[2] = '\0';
    for(int i = 0; i < string.size()/2; i++)
    {
        tmp[0] = string.at(2 * i).toLatin1();
        tmp[1] = string.at(2 * i + 1).toLatin1();
        bytes[string.size()/2 - 1 - i] = strtol(tmp, nullptr, 16);
    }
    //qDebug("bytes2Str: bytes: %#llX", *((uint64_t*)bytes));
    //qDebug("str2Bytes: str: %s", string.toLatin1().data());
    return bytes;
}

void MainWindow::on_generateKeyButton_clicked()
{
    RSAKeys k = rsa.generateKeys();
    ui->publicKeyTextEdit->document()->setPlainText(bytes2Str((char*)&k.pub, 4));
    ui->privateKeyTextEdit->document()->setPlainText(bytes2Str((char*)&k.priv, 4));
    ui->keyModTextEdit->document()->setPlainText(bytes2Str((char*)&k.module, 4));
    qDebug("pub: %#X, priv: %#X, mod: %#X", k.pub, k.priv, k.module);
    ui->encryptButton->setEnabled(true);
    ui->decryptButton->setEnabled(true);
}

void MainWindow::on_decryptButton_clicked()
{
    ui->encryptButton->setEnabled(false);
    ui->decryptButton->setEnabled(false);

    QString sourceText;
    uint32_t resultLength;
    QString string;
    RSAKeys keys;

    keys.pub = 0;
    memcpy(&keys.pub, str2Bytes(ui->publicKeyTextEdit->document()->toPlainText()), ui->publicKeyTextEdit->document()->toPlainText().size()/2);

    keys.priv = 0;
    memcpy(&keys.priv, str2Bytes(ui->privateKeyTextEdit->document()->toPlainText()), ui->privateKeyTextEdit->document()->toPlainText().size()/2);

    keys.module = 0;
    memcpy(&keys.module, str2Bytes(ui->keyModTextEdit->document()->toPlainText()), ui->keyModTextEdit->document()->toPlainText().size()/2);

    //decrypt------------------
    sourceText = ui->sourceTextEdit->document()->toPlainText();
    char* decryptedText;
    resultLength = sourceText.size()/4;
    rsa.decrypt((uint32_t*)str2Bytes(sourceText), &decryptedText, resultLength/2, keys);
    string.clear();
    for(uint32_t i = 0; i < resultLength/2; i++)
    {
        string.append(decryptedText[i]);
    }
    ui->decryptedTextEdit->document()->setPlainText(string);
    //--------------------------

    //encrypt-------------------
    sourceText = ui->decryptedTextEdit->document()->toPlainText();
    uint32_t* encryptedText;
    resultLength = sourceText.size();
    rsa.encrypt(sourceText.toLatin1().data(), &encryptedText, resultLength, keys);
    string = bytes2Str((char*)encryptedText, resultLength * 4);
    ui->encryptedTextEdit->document()->setPlainText(string.toUpper());
    //--------------------------

    ui->encryptButton->setEnabled(true);
    ui->decryptButton->setEnabled(true);
}

void MainWindow::on_encryptButton_clicked()
{
    ui->encryptButton->setEnabled(false);
    ui->decryptButton->setEnabled(false);

    QString sourceText;
    uint32_t resultLength;
    QString string;
    RSAKeys keys;

    keys.pub = 0;
    memcpy(&keys.pub, str2Bytes(ui->publicKeyTextEdit->document()->toPlainText()), ui->publicKeyTextEdit->document()->toPlainText().size()/2);

    keys.priv = 0;
    memcpy(&keys.priv, str2Bytes(ui->privateKeyTextEdit->document()->toPlainText()), ui->privateKeyTextEdit->document()->toPlainText().size()/2);

    keys.module = 0;
    memcpy(&keys.module, str2Bytes(ui->keyModTextEdit->document()->toPlainText()), ui->keyModTextEdit->document()->toPlainText().size()/2);

    qDebug("pub: %#X, priv: %#X, mod: %#X", keys.pub, keys.priv, keys.module);
    //encrypt--------------
    sourceText = ui->sourceTextEdit->document()->toPlainText();
    uint32_t* encryptedText;
    resultLength = sourceText.size();
    rsa.encrypt(sourceText.toLatin1().data(), &encryptedText, resultLength, keys);
    string = bytes2Str((char*)encryptedText, resultLength * 4);
    ui->encryptedTextEdit->document()->setPlainText(string.toUpper());
    //---------------------

    //decrypt--------------
    sourceText = ui->encryptedTextEdit->document()->toPlainText();
    char* decryptedText;
    rsa.decrypt((uint32_t*)str2Bytes(sourceText), &decryptedText, resultLength, keys);
    string.clear();
    for(uint32_t i = 0; i < resultLength; i++)
    {
        string.append(decryptedText[i]);
    }
    ui->decryptedTextEdit->document()->setPlainText(string);
    //-----------------------

    ui->encryptButton->setEnabled(true);
    ui->decryptButton->setEnabled(true);
}

void MainWindow::on_loadFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream in(&file);
    QString text;
    while(!in.atEnd()) {
        text.append(in.readLine());
    }
    ui->sourceTextEdit->setPlainText(text);

    file.close();
}

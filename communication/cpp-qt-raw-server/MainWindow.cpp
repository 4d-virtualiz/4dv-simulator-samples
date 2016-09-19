#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QTcpSocket>
#include <sstream>
#include <iomanip>
#include <iostream>

#define ERR_ABORT(MSG)   \
  do { if (!QString(MSG).isEmpty()) qDebug() << QString(MSG); qApp->quit(); } while(0)

///////////////////////////////////////////
///////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow),
    tcpServer_(0),
    s0_(0.),
    s1_(0.),
    stopLoop_(false)
{
  ui->setupUi(this);
  QObject::connect(ui->doubleSpinBox0, SIGNAL(valueChanged(double)), this, SLOT(recChangeS0(double)));
  QObject::connect(ui->doubleSpinBox1, SIGNAL(valueChanged(double)), this, SLOT(recChangeS1(double)));
  QObject::connect(ui->doubleSpinBox2, SIGNAL(valueChanged(double)), this, SLOT(recChangeS2(double)));
  QObject::connect(ui->doubleSpinBox3, SIGNAL(valueChanged(double)), this, SLOT(recChangeS3(double)));
  QObject::connect(ui->doubleSpinBox4, SIGNAL(valueChanged(double)), this, SLOT(recChangeS4(double)));
  QObject::connect(ui->doubleSpinBox5, SIGNAL(valueChanged(double)), this, SLOT(recChangeS5(double)));
  QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanQuit()));

  startServer();
}

///////////////////////////////////////////
///////////////////////////////////////////
MainWindow::~MainWindow()
{
  delete ui;
}

///////////////////////////////////////////
///////////////////////////////////////////
double MainWindow::s0()
{
  double ret = 0.;

  {
    QMutexLocker  lock(&mutex_);
    ret = s0_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
double MainWindow::s1()
{
  double ret = 0.;

  {
    QMutexLocker  lock(&mutex_);
    ret = s1_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
double MainWindow::s2()
{
  double ret = 0.;

  {
    QMutexLocker  lock(&mutex_);
    ret = s2_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
double MainWindow::s3()
{
  double ret = 0.;

  {
    QMutexLocker  lock(&mutex_);
    ret = s3_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
double MainWindow::s4()
{
  double ret = 0.;

  {
    QMutexLocker  lock(&mutex_);
    ret = s4_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
double MainWindow::s5()
{
  double ret = 0.;

  {
    QMutexLocker  lock(&mutex_);
    ret = s5_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
bool MainWindow::stopLoop()
{
  bool ret = false;

  {
    QMutexLocker  lock(&mutex_);
    ret = stopLoop_;
  }

  return ret;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recTcpServerConnection(qintptr socketDescriptor)
{
  QtConcurrent::run(this, &MainWindow::mainWritingLoop, socketDescriptor);
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::mainWritingLoop(qintptr socketDescriptor)
{
  QTcpSocket* sock = new QTcpSocket();

  qDebug() << "Connection established";

  if (!sock->setSocketDescriptor(socketDescriptor))
  {
    ERR_ABORT("setSocketDescriptor");
  }

  while (!stopLoop())
  {
    if (sock->state() == QAbstractSocket::ConnectedState)
    {
      QTextStream txtStream(sock);

      /*  ----------------------------------
       *  RAW PROTOCOL FORMAT :
       *    <START>TimeStamp 1 double 0.'\n'
       *           [INPUT_NAME COUNT TYPE VALUE'\n']
       *           '\n'<END>
       *  ----------------------------------
       */
      /* Mandatory header */
      txtStream << "TimeStamp 1 double 0." << endl;
      /* 'Raw datagram' body */
      txtStream << QString("s0 1 double %1").arg(QString::number(this->s0(), 'f', 10)) << endl;
      txtStream << QString("s1 1 double %1").arg(QString::number(this->s1(), 'f', 10)) << endl;
      txtStream << QString("s2 1 double %1").arg(QString::number(this->s2(), 'f', 10)) << endl;
      txtStream << QString("s3 1 double %1").arg(QString::number(this->s3(), 'f', 10)) << endl;
      txtStream << QString("s4 1 double %1").arg(QString::number(this->s4(), 'f', 10)) << endl;
      txtStream << QString("s5 1 double %1").arg(QString::number(this->s5(), 'f', 10)) << endl;
      /* Mandatory footer */
      txtStream << endl;

      /* Send data */
      txtStream.flush();
      sock->waitForBytesWritten();
    }
    else
      return;

    QThread::usleep(SLEEP_TIME);
  }
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::cleanQuit()
{
  this->recChangeLoopStop(true);
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeS0(double inValue)
{
  QMutexLocker locker(&mutex_);
  s0_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeS1(double inValue)
{
  QMutexLocker locker(&mutex_);
  s1_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeS2(double inValue)
{
  QMutexLocker locker(&mutex_);
  s2_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeS3(double inValue)
{
  QMutexLocker locker(&mutex_);
  s3_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeS4(double inValue)
{
  QMutexLocker locker(&mutex_);
  s4_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeS5(double inValue)
{
  QMutexLocker locker(&mutex_);
  s5_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::recChangeLoopStop(bool inValue)
{
  QMutexLocker locker(&mutex_);
  stopLoop_ = inValue;
}

///////////////////////////////////////////
///////////////////////////////////////////
void MainWindow::startServer()
{
  tcpServer_ = new MyTcpServer(this);

  if (!tcpServer_->listen(QHostAddress::Any, SERVER_PORT))
  {
    qDebug() << "error while listening on " << SERVER_PORT;
    ERR_ABORT("");
  }

  QObject::connect(tcpServer_, SIGNAL(sockDescArrived(qintptr)), this, SLOT(recTcpServerConnection(qintptr)));
}

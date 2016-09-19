#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtCore>
#include <QMainWindow>
#include <QTcpServer>
#include <cstdlib>
#include <cassert>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class MainWindow;
}


#define SERVER_PORT (quint16)3001

// usecs
#define SLEEP_TIME  (unsigned long)5000
class MyTcpServer : public QTcpServer
{
  Q_OBJECT
public:
  MyTcpServer(QObject* inParent = 0)
    :QTcpServer(inParent)
  {}

  virtual ~MyTcpServer() {}

  virtual void incomingConnection(qintptr socketDescriptor) { Q_EMIT sockDescArrived(socketDescriptor); }

protected:
  Q_SIGNAL  void sockDescArrived(qintptr);
};


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  double  s0();
  double  s1();
  double  s2();
  double  s3();
  double  s4();
  double  s5();

  bool    stopLoop();

private slots:
  void  recTcpServerConnection(qintptr socketDescriptor);
  void  mainWritingLoop(qintptr socketDescriptor);
  void  cleanQuit();

  void  recChangeS0(double inValue);
  void  recChangeS1(double inValue);
  void  recChangeS2(double inValue);
  void  recChangeS3(double inValue);
  void  recChangeS4(double inValue);
  void  recChangeS5(double inValue);
  void  recChangeLoopStop(bool inValue);

protected:
  void  startServer();

  Ui::MainWindow *ui;

  MyTcpServer*   tcpServer_;

  double  s0_;
  double  s1_;
  double  s2_;
  double  s3_;
  double  s4_;
  double  s5_;

  QMutex  mutex_;

  bool    stopLoop_;
};

#endif // MAINWINDOW_HPP

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QtSerialPort/QtSerialPort>


class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void readData();
    void on_pbConnect_clicked();
    void on_pbSend_clicked();

    void on_pbAt_clicked();

    void on_pbRst_clicked();

    void on_pbCwjap_clicked();

    void on_pbCwqap_clicked();

    void on_pbVer_clicked();

private:
    QSerialPort *serial;
};

#endif // MAINWINDOW_H

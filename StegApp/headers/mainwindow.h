#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Image.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_EncodeImageUploadBtn_clicked();

    void on_TextUploadBtn_clicked();

    void on_EncodeBtn_clicked();

    void on_DecodeBtn_clicked();

private:
    Ui::MainWindow *ui;
    Image *image = nullptr;
    void EncodeNoID(QString);
    void EncodeIncremental(QString);
    void EncodeUniqueID(QString);
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QFile f(":qdarkstyle/dark/darkstyle.qss");
    if (!f.exists())   {
        printf("Unable to set stylesheet, file not found\n");
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_EncodeImageUploadBtn_clicked()
{
    //opens a file input that filters for images only
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "",
                                                   tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif)"));
    if (!fileName.isEmpty()) {
        QImage image(fileName);

        // Scale the image while preserving the aspect ratio and then set it on the label
        QPixmap pixmap = QPixmap::fromImage(image);
        ui->ImageLabel->setPixmap(pixmap.scaled(ui->ImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    }

}

void MainWindow::on_TextUploadBtn_clicked()
{
    //opens a file input that filters for images only
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Text"), "",
                                                   tr("*.txt"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            file.close();

            // Assuming ui->textEdit is your QTextEdit
            ui->TextInputBox->setText(fileContent);
        } else {
            // Handle error if the file couldn't be opened
            QMessageBox::critical(this, tr("Error"), tr("Could not open the file"));
        }
    }
}

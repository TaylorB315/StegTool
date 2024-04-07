#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>
#include <cmath>
#include <QDebug>

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
        // Scale the image while preserving the aspect ratio and then set it on the label
        QPixmap pixmap;
        if (pixmap.load(fileName)){
            ui->ImageLabel->setPixmap(pixmap.scaled(ui->ImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->ImageFrame->setStyleSheet("");
            ui->OutputBox->setText("");
            this->image = new Image(fileName.toStdString().c_str());
        }
        else{
            qDebug() << "pixmap failed to load";
        }

    }

}

void MainWindow::on_TextUploadBtn_clicked()
{
    //opens a file input that filters for txt only
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Text"), "",
                                                   tr("*.txt"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            file.close();

            ui->TextInputBox->setText(fileContent);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Could not open the file"));
        }
    }
}

void MainWindow::on_EncodeBtn_clicked()
{
    QString input = ui->TextInputBox->toPlainText();
    if(input.length()>0 && this->image != nullptr){
        bool redundancy = false;
        if (ui->RedundancyCheck->checkState() == Qt::Checked){
            redundancy = true;
        }

        int noise = 0;
        switch(ui->NoiseSelect->currentIndex()){
        case(0):
            noise = 1;
            break;
        case(1):
            noise = 3;
            break;
        case(2):
            noise = 4;
            break;
        case(3):
            noise = 6;
            break;
        }
        bool success = false;
        switch(ui->LabelSelect->currentIndex()){
        case(0):
            success = EncodeNoID(input, noise, redundancy);
            break;
        case(1):
            success = EncodeIncremental(input, noise, redundancy);
            break;
        case(2):
            success = EncodeUniqueID(input, noise, redundancy);
            break;
        }
        if(success){
            ui->ImageFrame->setStyleSheet("QFrame { border: 2px solid green; }");
            //Image has been edited for encoding so needs to be reloaded to avoid encoding over previous work
            QString fileName = this->image->filename;
            this->image = new Image(fileName.toStdString().c_str());
        }
        else{
            ui->ImageFrame->setStyleSheet("QFrame { border: 2px solid red; }");
            //Reload image here for safety, not sure if neccessary
            QString fileName = this->image->filename;
            this->image = new Image(fileName.toStdString().c_str());

            QMessageBox::critical(this, tr("Error"), tr("You have entered too much text for this image, please shorten the input or increase the noise"));
        }

    }
    else{
        QMessageBox::critical(this, tr("Error"), tr("Please upload an image and add text before generating"));
    }
}

bool MainWindow::EncodeNoID(QString input, int noise, bool redundancy){
    for (int i = 0; i < ui->NumImages->value(); ++i){
        if(!(this->image->encode((input.toStdString().c_str()),(QString::number(i)).toStdString().c_str(),noise, redundancy))){
            return false;
            break;
        }
    }
    return true;
}

bool MainWindow::EncodeIncremental(QString input, int noise, bool redundancy){
    QString inputId;
    for (int i = 0; i < ui->NumImages->value(); ++i){
        inputId = input + QString::number(i);
        if(!(this->image->encode((inputId.toStdString().c_str()),(QString::number(i)).toStdString().c_str(),noise, redundancy))){
            return false;
            break;
        }
    }
    return true;
}

bool MainWindow::EncodeUniqueID(QString input, int noise, bool redundancy){
    QString uniqueId;
    for (int i = 0; i < ui->NumImages->value(); ++i){
        uniqueId =(QUuid::createUuid().toString());
        if(!(this->image->encode(((input+uniqueId).toStdString().c_str()),(uniqueId).toStdString().c_str(),noise, redundancy))){
            return false;
            break;
        }
    }
    return true;
}

void MainWindow::on_DecodeBtn_clicked()
{
    QString message = this->image->decode();
    if(message.length() > 0) {
        ui->OutputBox->setText(message);
        if(ui->saveText->checkState() == Qt::Checked){
            if(ui->FilenameBox->text() == ""){
                QFile file("../StegApp/output.txt");

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    qDebug() << "Cannot open file for writing: " << file.errorString();
                    return;
                }

                QTextStream out(&file);
                out << message;
                file.close();
            }
            else{
                QFile file("../StegApp/" + ui->FilenameBox->text() + ".txt");

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    qDebug() << "Cannot open file for writing: " << file.errorString();
                    return;
                }

                QTextStream out(&file);
                out << message;
                file.close();
            }
        }
    }
    else{
        QMessageBox::critical(this, tr("Error"), tr("Please upload an encoded image"));
    }
}

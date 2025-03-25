#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QWidget>
#include <QPaintEvent>
#include <vector>
#include <memory>

#include "customimagewidget.h"
#include "Filters/Filter.h"
#include "Image.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    private slots:
        void openImage();
        void saveImage();
        void updateUI();

private:
    CustomImageWidget *imageWidget;
    QTextEdit *infoTextEdit;
    Image currentImage;
    QString filePath;
    std::vector<std::unique_ptr<Filter>> filters;

    void createMenuBar();
    void updateImageInfo();
};

#endif // MAINWINDOW_H
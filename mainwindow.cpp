#include "mainwindow.h"
#include <QImageReader>
#include <QMessageBox>
#include <QFileInfo>
#include <QPixmap>
#include <QPushButton>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
    setWindowTitle("Image Editor");
    setGeometry(100, 100, 800, 600);

    // Vytvoření centrálního widgetu
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QPushButton *invertButton = new QPushButton("Invert Colors", this);
    QPushButton *rotateButton = new QPushButton("Rotate 90°", this);
    QPushButton *flipButton = new QPushButton("Flip Horizontal", this);

    connect(invertButton, &QPushButton::clicked, this, &MainWindow::invertColors);
    connect(rotateButton, &QPushButton::clicked, this, &MainWindow::rotateImage);
    connect(flipButton, &QPushButton::clicked, this, &MainWindow::flipImage);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(invertButton);
    buttonLayout->addWidget(rotateButton);
    buttonLayout->addWidget(flipButton);

    mainLayout->addLayout(buttonLayout);

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(imageLabel);

    infoTextEdit = new QTextEdit(this);
    infoTextEdit->setReadOnly(true);
    mainLayout->addWidget(infoTextEdit);

    // Vytvoření menu
    createMenuBar();
}

MainWindow::~MainWindow() {}

void MainWindow::createMenuBar() {
    // Vytvoření hlavní menu lišty
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // Vytvoření menu "Soubor"
    QMenu *fileMenu = menuBar->addMenu(tr("Soubor"));

    // Vytvoření akcí pro menu
    QAction *openAction = new QAction(tr("Otevřít"), this);
    QAction *saveAction = new QAction(tr("Uložit"), this);

    // Připojení akcí na sloty
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);

    // Přidání akcí do menu
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
}

void MainWindow::openImage() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Images (*.bmp)"));
    if (fileName.isEmpty()) return;

    if (!image.load(fileName)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to open the image!"));
        return;
    }

    filePath = fileName;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    updateImageInfo();
}

void MainWindow::saveImage() {
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Error"), tr("No image to save!"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.bmp)"));
    if (fileName.isEmpty()) return;

    if (!image.save(fileName)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save the image!"));
    }
}

void MainWindow::invertColors() {
    if (image.isNull()) return;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor color = image.pixelColor(x, y);
            color.setRed(255 - color.red());
            color.setGreen(255 - color.green());
            color.setBlue(255 - color.blue());
            image.setPixelColor(x, y, color);
        }
    }
    imageLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::rotateImage() {
    if (image.isNull()) return;

    image = image.transformed(QTransform().rotate(90));
    imageLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::flipImage() {
    if (image.isNull()) return;

    image = image.mirrored(true, false);
    imageLabel->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::updateImageInfo() {
    if (filePath.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    infoTextEdit->clear();
    infoTextEdit->append("Image Info:");
    infoTextEdit->append("Width: " + QString::number(image.width()));
    infoTextEdit->append("Height: " + QString::number(image.height()));
    infoTextEdit->append("Size: " + QString::number(fileInfo.size()) + " bytes");
    infoTextEdit->append("Format: " + image.format());
}

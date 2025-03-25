#include "mainwindow.h"

#include <iostream>
#include <QImageReader>
#include <QMessageBox>
#include <QFileInfo>
#include <QPushButton>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QPainter>


#include "styles.h"
#include "Filters/FlipFilter.h"
#include "Filters/InvertFilter.h"
#include "Filters/RotateFilter.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
    setWindowTitle("Image Editor");
    setGeometry(100, 100, 950, 600);

    // Vytvoření centrálního widgetu
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Hlavní layout bude horizontální, aby textové pole mohlo být vpravo
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Levá část - obrázek a tlačítka
    QVBoxLayout *leftLayout = new QVBoxLayout();

    // Tlačítka pro úpravy
    filters.push_back(std::make_unique<InvertFilter>());
    filters.push_back(std::make_unique<RotateFilter>());
    filters.push_back(std::make_unique<FlipFilter>());

    // Vytvoření tlačítek pro filtry dynamicky
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    for (const auto& filter : filters) {
        QPushButton *button = new QPushButton(filter->name(), this);
        button->setStyleSheet(Styles::ButtonStyle);  // Aplikace stylu na tlačítko
        buttonLayout->addWidget(button);

        connect(button, &QPushButton::clicked, [this, &filter]() {
            if (currentImage.isEmpty()) return;
            currentImage.applyFilter(*filter);
            updateUI();
});
    }

    // Nastavení vlastností layoutu pro zarovnání tlačítek
    buttonLayout->setSpacing(10);  // Mezera mezi tlačítky
    buttonLayout->setContentsMargins(10, 10, 10, 10);  // Okraje kolem tlačítek
    buttonLayout->setAlignment(Qt::AlignCenter);  // Zarovnání na střed

    // Přidání layoutu tlačítek do hlavního layoutu
    leftLayout->addLayout(buttonLayout);

    // Widget pro zobrazení obrázku
    imageWidget = new CustomImageWidget(this);
    leftLayout->addWidget(imageWidget, 1);  // 1 = stretch faktor pro zvětšení

    // Přidání levé části do hlavního layoutu
    mainLayout->addLayout(leftLayout, 3);  // 3 = 75% šířky

    // Pravá část - informace o obrázku
    infoTextEdit = new QTextEdit(this);
    infoTextEdit->setReadOnly(true);
    infoTextEdit->setMinimumWidth(250);
    infoTextEdit->setMaximumWidth(350);

    // Nastavení stylu pro panel s informacemi
    infoTextEdit->setStyleSheet(Styles::InfoHeaderStyle);

    // Přidání pravé části do hlavního layoutu
    mainLayout->addWidget(infoTextEdit, 1);  // 1 = 25% šířky

    // Přidání záhlaví pro panel s informacemi
    QVBoxLayout *rightLayout = new QVBoxLayout();
    QLabel *infoHeaderLabel = new QLabel("Image Information", this);
    infoHeaderLabel->setStyleSheet(Styles::InfoHeaderStyle);
    rightLayout->addWidget(infoHeaderLabel);
    rightLayout->addWidget(infoTextEdit);

    // Přidání tlačítek pro zoom pod informačním panelem
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    QPushButton *zoomInButton = new QPushButton("+", this);
    QPushButton *zoomOutButton = new QPushButton("-", this);
    QPushButton *zoomResetButton = new QPushButton("reset", this);

    // Nastavení stylu pro tlačítka zoomu
    zoomInButton->setStyleSheet(Styles::ButtonStyle);
    zoomOutButton->setStyleSheet(Styles::ButtonStyle);
    zoomResetButton->setStyleSheet(Styles::ButtonStyle);

    // Připojení signálů
    connect(zoomInButton, &QPushButton::clicked, [this]() {
        imageWidget->zoomIn();
    });
    connect(zoomOutButton, &QPushButton::clicked, [this]() {
        imageWidget->zoomOut();
    });
    connect(zoomResetButton, &QPushButton::clicked, [this]() {
        imageWidget->resetZoom();
    });

    // Přidání tlačítek do layoutu
    zoomLayout->addWidget(zoomInButton);
    zoomLayout->addWidget(zoomResetButton);
    zoomLayout->addWidget(zoomOutButton);

    // Přidání layoutu s tlačítky pro zoom do pravého panelu
    rightLayout->addLayout(zoomLayout);

    // Přidání pravé části do hlavního layoutu
    mainLayout->addLayout(rightLayout, 1);  // 1 = 25% šířky

    // Vytvoření menu
    createMenuBar();
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenuBar() {
    // Vytvoření hlavní menu lišty
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // Vytvoření menu "Soubor"
    QMenu *fileMenu = menuBar->addMenu(tr("Soubor"));

    // Vytvoření akcí pro menu
    QAction *openAction = new QAction(tr("Otevřít"), this);
    QAction *saveAction = new QAction(tr("Uložit"), this);
    QAction *exitAction = new QAction(tr("Zavřít aplikaci"), this);

    // Přidání klávesových zkratek
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

    // Přidání tooltipů
    openAction->setToolTip(tr("Otevřít obrázek (Ctrl+O)"));
    saveAction->setToolTip(tr("Uložit obrázek (Ctrl+S)"));
    exitAction->setToolTip(tr("Zavřít aplikaci"));

    // Připojení akcí na sloty
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    // Přidání akcí do menu
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
}

void MainWindow::openImage() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Images (*.bmp)"));
    if (fileName.isEmpty()) return;

    if (currentImage.loadFromFile(fileName)) {
        filePath = fileName;
        updateUI();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Nelze otevřít soubor!"));
    }
}

void MainWindow::saveImage() {
    if (currentImage.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No image to save!"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("Images (*.bmp)"));
    if (fileName.isEmpty()) return;

    bool useCopyMethod = false;

    // Pokud obrázek nebyl modifikován, nabídneme možnost prostého kopírování
    if (!currentImage.isModified()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Způsob uložení"),
            tr("Obrázek nebyl modifikován. Chcete jej uložit kopírováním původního souboru?\n\n"
               "Ano = kopírování (zachová přesně původní soubor)\n"
               "Ne = znovu vygenerovat data"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        useCopyMethod = (reply == QMessageBox::Yes);

        if (useCopyMethod) {
            std::cout << "File not modified, Copying image to: " << fileName.toStdString() << std::endl;
            if (QFile::exists(fileName)) {
                QFile::remove(fileName);  // Odstraní existující soubor se stejným názvem
            }

            if (QFile::copy(filePath, fileName)) {
                return;  // Úspěšné kopírování
            }
            // Pokud kopírování selže, přejdeme k alternativní metodě
            std::cout << "Copy failed, using manual data generation instead" << std::endl;
        }
    }

    // Použití metody třídy Image pro uložení souboru
    if (currentImage.saveToFile(fileName)) {
        std::cout << "Image saved successfully to: " << fileName.toStdString() << std::endl;
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to save the image!"));
    }
}

void MainWindow::updateUI() {
    if (!currentImage.isEmpty()) {
        imageWidget->resetZoom();
        imageWidget->setImage(currentImage.toQImage());
        updateImageInfo();
    }
}

void MainWindow::updateImageInfo() {
    if (filePath.isEmpty() || currentImage.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    infoTextEdit->clear();
    infoTextEdit->append("Image Info:");
    infoTextEdit->append("File Path: " + filePath);
    infoTextEdit->append("Width: " + QString::number(currentImage.width()));
    infoTextEdit->append("Height: " + QString::number(currentImage.height()));
    infoTextEdit->append("Size: " + QString::number(fileInfo.size()) + " bytes");
    infoTextEdit->append("Format: " + QString::number(currentImage.bitsPerPixel()) + "-bit BMP");

    // Získání BMP header dat z objektu Image
    const Image::BMPFileHeader& bmpFileHeader = currentImage.getFileHeader();
    const Image::BMPInfoHeader& bmpInfoHeader = currentImage.getInfoHeader();

    infoTextEdit->append("\nBMP File Header:");
    infoTextEdit->append("bfType: " + QString(bmpFileHeader.bfType[0]) + QString(bmpFileHeader.bfType[1]));
    infoTextEdit->append("bfSize: " + QString::number(bmpFileHeader.bfSize) + " bytes");
    infoTextEdit->append("bfReserved1: " + QString::number(bmpFileHeader.bfReserved1));
    infoTextEdit->append("bfReserved2: " + QString::number(bmpFileHeader.bfReserved2));
    infoTextEdit->append("bfOffBits: " + QString::number(bmpFileHeader.bfOffBits) + " bytes");

    infoTextEdit->append("\nBMP Info Header:");
    infoTextEdit->append("biSize: " + QString::number(bmpInfoHeader.biSize) + " bytes");
    infoTextEdit->append("biWidth: " + QString::number(bmpInfoHeader.biWidth) + " pixels");
    infoTextEdit->append("biHeight: " + QString::number(bmpInfoHeader.biHeight) + " pixels");
    infoTextEdit->append("biPlanes: " + QString::number(bmpInfoHeader.biPlanes));
    infoTextEdit->append("biBitCount: " + QString::number(bmpInfoHeader.biBitCount) + " bits");

    // Výpis typu komprese
    QString compressionType;
    switch(bmpInfoHeader.biCompression) {
        case 0: compressionType = "BI_RGB (0) - nekomprimovaný"; break;
        case 1: compressionType = "BI_RLE8 (1) - 8-bit RLE komprese"; break;
        case 2: compressionType = "BI_RLE4 (2) - 4-bit RLE komprese"; break;
        case 3: compressionType = "BI_BITFIELDS (3) - bitové masky"; break;
        default: compressionType = QString::number(bmpInfoHeader.biCompression) + " - neznámý typ"; break;
    }
    infoTextEdit->append("biCompression: " + compressionType);

    infoTextEdit->append("biSizeImage: " + QString::number(bmpInfoHeader.biSizeImage) + " bytes");
    infoTextEdit->append("biXPelsPerMeter: " + QString::number(bmpInfoHeader.biXPelsPerMeter));
    infoTextEdit->append("biYPelsPerMeter: " + QString::number(bmpInfoHeader.biYPelsPerMeter));
    infoTextEdit->append("biClrUsed: " + QString::number(bmpInfoHeader.biClrUsed));
    infoTextEdit->append("biClrImportant: " + QString::number(bmpInfoHeader.biClrImportant));

    // Pro obrázky s paletou vypíšeme informace o paletě
    if (currentImage.bitsPerPixel() <= 8) {
        const QVector<QRgb>& palette = currentImage.palette();

        infoTextEdit->append("\nPalette Info:");
        infoTextEdit->append("Palette Size: " + QString::number(palette.size()) + " colors");

        // Pro menší palety můžeme vypsat i barvy
        if (palette.size() <= 256) {
            infoTextEdit->append("\nPalette Colors (RGB):");
            for (int i = 0; i < palette.size(); i++) {
                QRgb color = palette[i];
                QString colorStr = QString("Color %1: R=%2, G=%3, B=%4")
                    .arg(i, 2, 10, QChar('0'))
                    .arg(qRed(color), 3, 10, QChar('0'))
                    .arg(qGreen(color), 3, 10, QChar('0'))
                    .arg(qBlue(color), 3, 10, QChar('0'));
                infoTextEdit->append(colorStr);
            }
        }
    }
}
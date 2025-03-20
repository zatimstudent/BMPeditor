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

    // Použití vlastní implementace pro načtení BMP souboru
    if (!loadBMPFile(fileName)) {
        return;
    }

    filePath = fileName;
    // Použití vlastní implementace pro vykreslení BMP souboru
    renderCustomBMP();
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

void MainWindow::parseBMPHeader(const QByteArray &bmpData) {
    //clear the text edit
    infoTextEdit->clear();

    if (bmpData.size() < 54) {
        infoTextEdit->append("Invalid BMP file: header too small.");
        return;
    }

    const char* data = bmpData.constData();

    // --- BMP File Header ---
    quint16 signature = *reinterpret_cast<const quint16*>(data);
    quint32 fileSize = *reinterpret_cast<const quint32*>(data + 2);
    quint16 reserved1 = *reinterpret_cast<const quint16*>(data + 6);
    quint16 reserved2 = *reinterpret_cast<const quint16*>(data + 8);
    quint32 dataOffset = *reinterpret_cast<const quint32*>(data + 10);

    // --- DIB Header ---
    quint32 dibHeaderSize = *reinterpret_cast<const quint32*>(data + 14);
    qint32 width = *reinterpret_cast<const qint32*>(data + 18);
    qint32 height = *reinterpret_cast<const qint32*>(data + 22);
    quint16 planes = *reinterpret_cast<const quint16*>(data + 26);
    quint16 bitsPerPixel = *reinterpret_cast<const quint16*>(data + 28);
    quint32 compression = *reinterpret_cast<const quint32*>(data + 30);
    quint32 imageSize = *reinterpret_cast<const quint32*>(data + 34);
    qint32 xPixelsPerMeter = *reinterpret_cast<const qint32*>(data + 38);
    qint32 yPixelsPerMeter = *reinterpret_cast<const qint32*>(data + 42);
    quint32 colorsUsed = *reinterpret_cast<const quint32*>(data + 46);
    quint32 importantColors = *reinterpret_cast<const quint32*>(data + 50);

    // --- Výpis hlavičky ---
    infoTextEdit->append("=== BMP Header Info ===");
    infoTextEdit->append("Signature: " + QString(QChar(signature & 0xFF)) + QString(QChar((signature >> 8) & 0xFF))); //
    infoTextEdit->append("File size: " + QString::number(fileSize) + " bytes");
    infoTextEdit->append("Data offset: " + QString::number(dataOffset) + " bytes");
    infoTextEdit->append("DIB header size: " + QString::number(dibHeaderSize) + " bytes");
    infoTextEdit->append("Width: " + QString::number(width));
    infoTextEdit->append("Height: " + QString::number(height));
    infoTextEdit->append("Planes: " + QString::number(planes));
    infoTextEdit->append("Bits per pixel: " + QString::number(bitsPerPixel));

    QString compressionStr;
    switch (compression) {
        case 0: compressionStr = "BI_RGB (no compression)"; break;
        case 1: compressionStr = "BI_RLE8"; break;
        case 2: compressionStr = "BI_RLE4"; break;
        default: compressionStr = "Unknown (" + QString::number(compression) + ")"; break;
    }
    infoTextEdit->append("Compression: " + compressionStr);
    infoTextEdit->append("Image size: " + QString::number(imageSize) + " bytes");
    infoTextEdit->append("X pixels per meter: " + QString::number(xPixelsPerMeter));
    infoTextEdit->append("Y pixels per meter: " + QString::number(yPixelsPerMeter));
    infoTextEdit->append("Colors used: " + QString::number(colorsUsed));
    infoTextEdit->append("Important colors: " + QString::number(importantColors));

    // --- Barevná paleta ---
    int paletteSize = 0;
    if (bitsPerPixel == 1) paletteSize = 2;
    else if (bitsPerPixel == 4) paletteSize = 16;
    else if (bitsPerPixel == 8) paletteSize = 256;

    if (paletteSize > 0) {
        infoTextEdit->append("Palette (" + QString::number(paletteSize) + " colors):");
        for (int i = 0; i < paletteSize; i++) {
            int offset = 14 + dibHeaderSize + i * 4;
            if (offset + 3 >= bmpData.size()) break;
            auto blue = static_cast<quint8>(bmpData[offset]);
            auto green = static_cast<quint8>(bmpData[offset + 1]);
            auto red = static_cast<quint8>(bmpData[offset + 2]);
            infoTextEdit->append(QString("Color %1: R=%2, G=%3, B=%4")
                                 .arg(i)
                                 .arg(red)
                                 .arg(green)
                                 .arg(blue));
        }
    } else if (bitsPerPixel == 24) {
        infoTextEdit->append("No palette (24-bit true color).");
    } else {
        infoTextEdit->append("Unknown or unsupported palette configuration.");
    }

    infoTextEdit->append("========================");
}

// Pomocná funkce pro čtení dat z BMP souboru
bool MainWindow::loadBMPFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Nelze otevřít soubor!"));
        return false;
    }

    // Čtení BMP hlavičky
    QByteArray header = file.read(54);
    if (header.size() < 54 || header[0] != 'B' || header[1] != 'M') {
        QMessageBox::warning(this, tr("Error"), tr("Neplatný formát BMP!"));
        file.close();
        return false;
    }

    // Získání informací o obrázku
    int width = *reinterpret_cast<int*>(header.data() + 18);
    int height = *reinterpret_cast<int*>(header.data() + 22);
    int bitsPerPixel = *reinterpret_cast<short*>(header.data() + 28);
    int compression = *reinterpret_cast<int*>(header.data() + 30);

    // Kontrola podporovaných formátů
    if (compression != 0) {
        QMessageBox::warning(this, tr("Error"), tr("Komprimované BMP soubory nejsou podporovány!"));
        file.close();
        return false;
    }

    if (bitsPerPixel != 1 && bitsPerPixel != 2 && bitsPerPixel != 4 &&
        bitsPerPixel != 8 && bitsPerPixel != 24) {
        QMessageBox::warning(this, tr("Error"), tr("Nepodporovaná bitová hloubka!"));
        file.close();
        return false;
    }

    // Získání offsetu dat
    int dataOffset = *reinterpret_cast<int*>(header.data() + 10);

    // Přeskočení zbytku hlavičky
    file.seek(dataOffset);

    // Nastavení velikosti obrázku
    customBMPWidth = width;
    customBMPHeight = height;
    customBMPBitsPerPixel = bitsPerPixel;

    // Výpočet velikosti řádku (musí být zarovnán na 4 bajty)
    int bytesPerRow = ((width * bitsPerPixel + 31) / 32) * 4;

    // Čtení palety barev (pro 1, 2, 4 a 8 bitové obrázky)
    customBMPPalette.clear();
    if (bitsPerPixel <= 8) {
        int paletteOffset = 54;  // Standardní offset pro paletu barev
        int paletteSize = 1 << bitsPerPixel;  // 2^bitsPerPixel

        file.seek(paletteOffset);
        QByteArray paletteData = file.read(paletteSize * 4);  // 4 bajty na barvu (BGRA)

        for (int i = 0; i < paletteSize; i++) {
            int blue = static_cast<unsigned char>(paletteData[i*4]);
            int green = static_cast<unsigned char>(paletteData[i*4+1]);
            int red = static_cast<unsigned char>(paletteData[i*4+2]);
            customBMPPalette.append(qRgb(red, green, blue));
        }

        file.seek(dataOffset);  // Návrat na začátek dat
    }

    // Čtení dat obrázku
    customBMPData = file.readAll();
    file.close();

    return true;
}

// Vlastní vykreslování BMP souboru
void MainWindow::renderCustomBMP() {
    if (customBMPData.isEmpty()) return;

    // Vytvoření prázdného obrázku
    QImage renderedImage(customBMPWidth, customBMPHeight, QImage::Format_RGB32);

    // Výpočet velikosti řádku (musí být zarovnán na 4 bajty)
    int bytesPerRow = ((customBMPWidth * customBMPBitsPerPixel + 31) / 32) * 4;

    // Vykreslení podle bitové hloubky
    for (int y = 0; y < customBMPHeight; y++) {
        for (int x = 0; x < customBMPWidth; x++) {
            QRgb pixelColor;

            // Pozice v datech (BMP ukládá data odspodu nahoru)
            int row = customBMPHeight - 1 - y;
            int byteIndex = row * bytesPerRow;

            if (customBMPBitsPerPixel == 24) {
                // 24 bitů = 3 bajty na pixel
                int index = byteIndex + x * 3;
                if (index + 2 < customBMPData.size()) {
                    int blue = static_cast<unsigned char>(customBMPData[index]);
                    int green = static_cast<unsigned char>(customBMPData[index + 1]);
                    int red = static_cast<unsigned char>(customBMPData[index + 2]);
                    pixelColor = qRgb(red, green, blue);
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            } else if (customBMPBitsPerPixel == 8) {
                // 8 bitů = 1 bajt na pixel
                int index = byteIndex + x;
                if (index < customBMPData.size()) {
                    int colorIndex = static_cast<unsigned char>(customBMPData[index]);
                    pixelColor = customBMPPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            } else if (customBMPBitsPerPixel == 4) {
                // 4 bity = 2 pixely na bajt
                int index = byteIndex + x / 2;
                if (index < customBMPData.size()) {
                    int value = static_cast<unsigned char>(customBMPData[index]);
                    int colorIndex;
                    if (x % 2 == 0) {
                        // Horní 4 bity
                        colorIndex = (value >> 4) & 0x0F;
                    } else {
                        // Dolní 4 bity
                        colorIndex = value & 0x0F;
                    }
                    pixelColor = customBMPPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            } else if (customBMPBitsPerPixel == 2) {
                // 2 bity = 4 pixely na bajt
                int index = byteIndex + x / 4;
                if (index < customBMPData.size()) {
                    int value = static_cast<unsigned char>(customBMPData[index]);
                    int shift = 6 - (x % 4) * 2;  // 6, 4, 2, 0
                    int colorIndex = (value >> shift) & 0x03;
                    pixelColor = customBMPPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            } else if (customBMPBitsPerPixel == 1) {
                // 1 bit = 8 pixelů na bajt
                int index = byteIndex + x / 8;
                if (index < customBMPData.size()) {
                    int value = static_cast<unsigned char>(customBMPData[index]);
                    int shift = 7 - (x % 8);  // 7, 6, 5, 4, 3, 2, 1, 0
                    int colorIndex = (value >> shift) & 0x01;
                    pixelColor = customBMPPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            } else {
                pixelColor = qRgb(0, 0, 0);  // Fallback pro nepodporované formáty
            }

            renderedImage.setPixel(x, y, pixelColor);
        }
    }

    // Nastavení vykresleného obrázku
    imageLabel->setPixmap(QPixmap::fromImage(renderedImage));
    image = renderedImage;  // Uložení do proměnné image pro další úpravy

    // use parseBMPHeader to display BMP header information
    parseBMPHeader(customBMPData);

    // Aktualizace informací o obrázku
    // infoTextEdit->clear();
    // infoTextEdit->append("Image Info:");
    // infoTextEdit->append("Width: " + QString::number(customBMPWidth));
    // infoTextEdit->append("Height: " + QString::number(customBMPHeight));
    // infoTextEdit->append("Bits per pixel: " + QString::number(customBMPBitsPerPixel));
    // infoTextEdit->append("Format: BMP");
}
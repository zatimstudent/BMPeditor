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
        : QMainWindow(parent), imageModified(false)
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
            if (image.isNull()) return;
            image = filter->apply(image);
            imageWidget->setImage(image);
            imageModified = true;

            // Aktualizace velikosti (pro případ, že filtr změní rozměry)
            customBMPWidth = image.width();
            customBMPHeight = image.height();
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
    infoHeaderLabel->setStyleSheet(Styles::InfoPanelStyle);
    rightLayout->addWidget(infoHeaderLabel);
    rightLayout->addWidget(infoTextEdit);
    mainLayout->addLayout(rightLayout, 1);

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

    bool useCopyMethod = false;

    // Pokud obrázek nebyl modifikován, nabídneme možnost prostého kopírování
    if (!imageModified) {
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

    // Implementace pro uložení obrázku se zachováním původní struktury
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Nelze vytvořit soubor!"));
        return;
    }

    std::cout << "Saving image to: " << fileName.toStdString() << std::endl;

    // 1. Zápis file header (14 bajtů)
    file.write(bmpFileHeader.bfType, 2);
    file.write(reinterpret_cast<char*>(&bmpFileHeader.bfSize), 4);
    file.write(reinterpret_cast<char*>(&bmpFileHeader.bfReserved1), 2);
    file.write(reinterpret_cast<char*>(&bmpFileHeader.bfReserved2), 2);
    file.write(reinterpret_cast<char*>(&bmpFileHeader.bfOffBits), 4);

    // 2. Zápis info header (40 bajtů)
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biSize), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biWidth), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biHeight), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biPlanes), 2);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biBitCount), 2);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biCompression), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biSizeImage), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biXPelsPerMeter), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biYPelsPerMeter), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biClrUsed), 4);
    file.write(reinterpret_cast<char*>(&bmpInfoHeader.biClrImportant), 4);

    // 3. Zápis palety barev (pokud existuje)
    if (customBMPBitsPerPixel <= 8 && !customBMPPalette.isEmpty()) {
        for (QRgb color : customBMPPalette) {
            char paletteEntry[4];
            paletteEntry[0] = static_cast<char>(qBlue(color));  // B
            paletteEntry[1] = static_cast<char>(qGreen(color)); // G
            paletteEntry[2] = static_cast<char>(qRed(color));   // R
            paletteEntry[3] = 0; // Reserved
            file.write(paletteEntry, 4);
        }
    }

    // 4. Zápis obrazových dat
    if (imageModified) {
        std::cout << "Writing modified image data..." << std::endl;
        // Výpočet velikosti řádku (musí být zarovnán na 4 bajty)
        int bytesPerRow = ((customBMPWidth * customBMPBitsPerPixel + 31) / 32) * 4;
        QByteArray dataToSave(bytesPerRow * customBMPHeight, 0);

        // Konverze pixelů z upraveného QImage zpět do formátu BMP
        for (int y = 0; y < image.height(); y++) {
            for (int x = 0; x < image.width(); x++) {
                QRgb pixel = image.pixel(x, y);

                // Pozice v datech (BMP ukládá data odspodu nahoru, pokud biHeight > 0)
                int row = (bmpInfoHeader.biHeight > 0) ? customBMPHeight - 1 - y : y;
                int byteIndex = row * bytesPerRow;

                if (customBMPBitsPerPixel == 24) {
                    // 24 bitů = 3 bajty na pixel
                    int index = byteIndex + x * 3;
                    if (index + 2 < dataToSave.size()) {
                        dataToSave[index] = static_cast<char>(qBlue(pixel));
                        dataToSave[index + 1] = static_cast<char>(qGreen(pixel));
                        dataToSave[index + 2] = static_cast<char>(qRed(pixel));
                    }
                }
                else if (customBMPBitsPerPixel == 8) {
                    // 8 bitů = 1 bajt na pixel
                    int index = byteIndex + x;
                    if (index < dataToSave.size()) {
                        // Nalezení nejbližší barvy v paletě
                        QRgb currentPixel = pixel;
                        int bestMatch = 0;
                        int bestDiff = INT_MAX;

                        for (int i = 0; i < customBMPPalette.size(); i++) {
                            QRgb paletteColor = customBMPPalette[i];
                            int rDiff = qRed(currentPixel) - qRed(paletteColor);
                            int gDiff = qGreen(currentPixel) - qGreen(paletteColor);
                            int bDiff = qBlue(currentPixel) - qBlue(paletteColor);

                            // Výpočet vzdálenosti v RGB prostoru
                            int diff = rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;

                            if (diff < bestDiff) {
                                bestDiff = diff;
                                bestMatch = i;
                            }
                        }

                        dataToSave[index] = static_cast<char>(bestMatch);
                    }
                }
                else if (customBMPBitsPerPixel == 4) {
                    // 4 bity = 2 pixely na bajt
                    int bytePos = byteIndex + x / 2;
                    if (bytePos < dataToSave.size()) {
                        // Nalezení nejbližší barvy v paletě
                        QRgb currentPixel = pixel;
                        int bestMatch = 0;
                        int bestDiff = INT_MAX;

                        for (int i = 0; i < customBMPPalette.size() && i < 16; i++) {
                            QRgb paletteColor = customBMPPalette[i];
                            int rDiff = qRed(currentPixel) - qRed(paletteColor);
                            int gDiff = qGreen(currentPixel) - qGreen(paletteColor);
                            int bDiff = qBlue(currentPixel) - qBlue(paletteColor);

                            int diff = rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;

                            if (diff < bestDiff) {
                                bestDiff = diff;
                                bestMatch = i;
                            }
                        }

                        // Aktualizace hodnoty v bajtu
                        if (x % 2 == 0) {
                            // Horní 4 bity (první pixel v bajtu)
                            dataToSave[bytePos] = (dataToSave[bytePos] & 0x0F) | (bestMatch << 4);
                        } else {
                            // Dolní 4 bity (druhý pixel v bajtu)
                            dataToSave[bytePos] = (dataToSave[bytePos] & 0xF0) | bestMatch;
                        }
                    }
                }
                else if (customBMPBitsPerPixel == 1) {
                    // 1 bit = 8 pixelů na bajt
                    int bytePos = byteIndex + x / 8;
                    if (bytePos < dataToSave.size()) {
                        // Pro 1-bitový obrázek máme jen 2 barvy (obvykle černá a bílá)
                        QRgb currentPixel = pixel;
                        int bestMatch = 0;

                        // Máme jen dvě barvy, takže stačí porovnat s první
                        if (customBMPPalette.size() >= 2) {
                            QRgb color0 = customBMPPalette[0];
                            QRgb color1 = customBMPPalette[1];

                            int rDiff0 = qRed(currentPixel) - qRed(color0);
                            int gDiff0 = qGreen(currentPixel) - qGreen(color0);
                            int bDiff0 = qBlue(currentPixel) - qBlue(color0);

                            int rDiff1 = qRed(currentPixel) - qRed(color1);
                            int gDiff1 = qGreen(currentPixel) - qGreen(color1);
                            int bDiff1 = qBlue(currentPixel) - qBlue(color1);

                            int diff0 = rDiff0 * rDiff0 + gDiff0 * gDiff0 + bDiff0 * bDiff0;
                            int diff1 = rDiff1 * rDiff1 + gDiff1 * gDiff1 + bDiff1 * bDiff1;

                            bestMatch = (diff1 < diff0) ? 1 : 0;
                        }

                        // Určíme, který bit v bajtu aktualizujeme
                        int bitPos = 7 - (x % 8); // 7, 6, 5, 4, 3, 2, 1, 0

                        unsigned char currentByte = static_cast<unsigned char>(dataToSave[bytePos]);
                        if (bestMatch == 0) {
                            // Nastavíme bit na 0
                            currentByte &= ~(1 << bitPos);
                        } else {
                            // Nastavíme bit na 1
                            currentByte |= (1 << bitPos);
                        }
                        dataToSave[bytePos] = static_cast<char>(currentByte);
                    }
                }
            }
        }
        file.write(dataToSave);
    } else {
        std::cout << "Writing original image data..." << std::endl;
        file.write(customBMPData);
    }

    file.close();
}

void MainWindow::updateImageInfo() {
    if (filePath.isEmpty()) return;

    QFileInfo fileInfo(filePath);
    infoTextEdit->clear();
    infoTextEdit->append("Image Info:");
    infoTextEdit->append("File Path: " + filePath);
    infoTextEdit->append("Width: " + QString::number(image.width()));
    infoTextEdit->append("Height: " + QString::number(image.height()));
    infoTextEdit->append("Size: " + QString::number(fileInfo.size()) + " bytes");
    infoTextEdit->append("Format: " + QString::number(customBMPBitsPerPixel) + "-bit BMP");

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

    // Výpis typu komprese i když náš use case je vždy nekomprimovaný BMP
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
    if (customBMPBitsPerPixel <= 8) {
        infoTextEdit->append("\nPalette Info:");
        infoTextEdit->append("Palette Size: " + QString::number(customBMPPalette.size()) + " colors");

        // Pro menší palety můžeme vypsat i barvy
        if (customBMPPalette.size() <= 256) {
            infoTextEdit->append("\nPalette Colors (RGB):");
            for (int i = 0; i < customBMPPalette.size(); i++) {
                QRgb color = customBMPPalette[i];
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

// Pomocná funkce pro čtení dat z BMP souboru
bool MainWindow::loadBMPFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Nelze otevřít soubor!"));
        return false;
    }

    // Čtení kompletní BMP hlavičky
    // Načtení file header (14 bajtů)
    QByteArray fileHeaderData = file.read(14);
    if (fileHeaderData.size() < 14) {
        QMessageBox::warning(this, tr("Error"), tr("Neplatný formát BMP - příliš krátká hlavička!"));
        file.close();
        return false;
    }

    // Kontrola signatury "BM"
    if (fileHeaderData[0] != 'B' || fileHeaderData[1] != 'M') {
        QMessageBox::warning(this, tr("Error"), tr("Neplatný formát BMP - chybí signatura BM!"));
        file.close();
        return false;
    }

    // Naplnění file header struktury
    bmpFileHeader.bfType[0] = fileHeaderData[0];
    bmpFileHeader.bfType[1] = fileHeaderData[1];
    bmpFileHeader.bfSize = *reinterpret_cast<const uint32_t*>(fileHeaderData.data() + 2);
    bmpFileHeader.bfReserved1 = *reinterpret_cast<const uint16_t*>(fileHeaderData.data() + 6);
    bmpFileHeader.bfReserved2 = *reinterpret_cast<const uint16_t*>(fileHeaderData.data() + 8);
    bmpFileHeader.bfOffBits = *reinterpret_cast<const uint32_t*>(fileHeaderData.data() + 10);

    // Načtení info header (40 bajtů pro BITMAPINFOHEADER)
    QByteArray infoHeaderData = file.read(40);
    if (infoHeaderData.size() < 40) {
        QMessageBox::warning(this, tr("Error"), tr("Neplatný formát BMP - příliš krátká info hlavička!"));
        file.close();
        return false;
    }

    // Naplnění info header struktury
    bmpInfoHeader.biSize = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 0);
    bmpInfoHeader.biWidth = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 4);
    bmpInfoHeader.biHeight = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 8);
    bmpInfoHeader.biPlanes = *reinterpret_cast<const uint16_t*>(infoHeaderData.data() + 12);
    bmpInfoHeader.biBitCount = *reinterpret_cast<const uint16_t*>(infoHeaderData.data() + 14);
    bmpInfoHeader.biCompression = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 16);
    bmpInfoHeader.biSizeImage = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 20);
    bmpInfoHeader.biXPelsPerMeter = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 24);
    bmpInfoHeader.biYPelsPerMeter = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 28);
    bmpInfoHeader.biClrUsed = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 32);
    bmpInfoHeader.biClrImportant = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 36);

    // Kontrola podporovaných formátů
    if (bmpInfoHeader.biCompression != 0) {
        QMessageBox::warning(this, tr("Error"), tr("Komprimované BMP soubory nejsou podporovány!"));
        file.close();
        return false;
    }

    if (bmpInfoHeader.biBitCount != 1 && bmpInfoHeader.biBitCount != 4 &&
        bmpInfoHeader.biBitCount != 8 && bmpInfoHeader.biBitCount != 24) {
        QMessageBox::warning(this, tr("Error"), tr("Nepodporovaná bitová hloubka!"));
        file.close();
        return false;
    }

    // Nastavení velikosti obrázku
    customBMPWidth = bmpInfoHeader.biWidth;
    customBMPHeight = abs(bmpInfoHeader.biHeight); // Výška může být záporná, pokud data jdou odshora dolů
    customBMPBitsPerPixel = bmpInfoHeader.biBitCount;

    // Čtení palety barev (pro 1, 4 a 8 bitové obrázky)
    customBMPPalette.clear();
    if (customBMPBitsPerPixel <= 8) {
        // Zjištění velikosti palety - pokud biClrUsed je 0, použij maximální počet barev
        int paletteSize = (bmpInfoHeader.biClrUsed > 0) ? bmpInfoHeader.biClrUsed : (1 << customBMPBitsPerPixel);

        // Načtení palety
        QByteArray paletteData = file.read(paletteSize * 4);  // 4 bajty na barvu (BGRA)

        for (int i = 0; i < paletteSize && i*4 < paletteData.size(); i++) {
            int blue = static_cast<unsigned char>(paletteData[i*4]);
            int green = static_cast<unsigned char>(paletteData[i*4+1]);
            int red = static_cast<unsigned char>(paletteData[i*4+2]);
            customBMPPalette.append(qRgb(red, green, blue));
        }
    }

    // Posun na začátek obrazových dat
    file.seek(bmpFileHeader.bfOffBits);

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

            // Pozice v datech (BMP ukládá data odspodu nahoru, pokud biHeight > 0)
            int row = (bmpInfoHeader.biHeight > 0) ? customBMPHeight - 1 - y : y;
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
    imageWidget->setImage(renderedImage);
    image = renderedImage;
    imageModified = false;

    // Aktualizace informací o obrázku
    updateImageInfo();
}

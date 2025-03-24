#include "Image.h"
#include "Filters/Filter.h"
#include "customimagewidget.h"

Image::Image() : imageWidth(0), imageHeight(0), imageBitsPerPixel(0), modified(false) {
    // Inicializace struktur
    fileHeader = {0};
    infoHeader = {0};
}

Image::~Image() = default;

bool Image::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Načtení file header
    QByteArray fileHeaderData = file.read(14);
    if (fileHeaderData.size() < 14 || fileHeaderData[0] != 'B' || fileHeaderData[1] != 'M') {
        file.close();
        return false;
    }

    // Parsování file header
    fileHeader.bfType[0] = fileHeaderData[0];
    fileHeader.bfType[1] = fileHeaderData[1];
    fileHeader.bfSize = *reinterpret_cast<const uint32_t*>(fileHeaderData.data() + 2);
    fileHeader.bfReserved1 = *reinterpret_cast<const uint16_t*>(fileHeaderData.data() + 6);
    fileHeader.bfReserved2 = *reinterpret_cast<const uint16_t*>(fileHeaderData.data() + 8);
    fileHeader.bfOffBits = *reinterpret_cast<const uint32_t*>(fileHeaderData.data() + 10);

    // Načtení info header
    QByteArray infoHeaderData = file.read(40);
    if (infoHeaderData.size() < 40) {
        file.close();
        return false;
    }

    // Parsování info header
    infoHeader.biSize = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 0);
    infoHeader.biWidth = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 4);
    infoHeader.biHeight = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 8);
    infoHeader.biPlanes = *reinterpret_cast<const uint16_t*>(infoHeaderData.data() + 12);
    infoHeader.biBitCount = *reinterpret_cast<const uint16_t*>(infoHeaderData.data() + 14);
    infoHeader.biCompression = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 16);
    infoHeader.biSizeImage = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 20);
    infoHeader.biXPelsPerMeter = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 24);
    infoHeader.biYPelsPerMeter = *reinterpret_cast<const int32_t*>(infoHeaderData.data() + 28);
    infoHeader.biClrUsed = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 32);
    infoHeader.biClrImportant = *reinterpret_cast<const uint32_t*>(infoHeaderData.data() + 36);

    // Kontrola podporovaných formátů
    if (infoHeader.biCompression != 0 || 
        (infoHeader.biBitCount != 1 && infoHeader.biBitCount != 4 && 
         infoHeader.biBitCount != 8 && infoHeader.biBitCount != 24)) {
        file.close();
        return false;
    }

    // Nastavení základních parametrů
    imageWidth = infoHeader.biWidth;
    imageHeight = abs(infoHeader.biHeight);
    imageBitsPerPixel = infoHeader.biBitCount;

    // Načtení palety
    colorPalette.clear();
    if (imageBitsPerPixel <= 8) {
        int paletteSize = (infoHeader.biClrUsed > 0) ? infoHeader.biClrUsed : (1 << imageBitsPerPixel);
        QByteArray paletteData = file.read(paletteSize * 4);

        for (int i = 0; i < paletteSize && i*4 < paletteData.size(); i++) {
            int blue = static_cast<unsigned char>(paletteData[i*4]);
            int green = static_cast<unsigned char>(paletteData[i*4+1]);
            int red = static_cast<unsigned char>(paletteData[i*4+2]);
            colorPalette.append(qRgb(red, green, blue));
        }
    }

    // Čtení dat obrázku
    file.seek(fileHeader.bfOffBits);
    rawData = file.readAll();
    file.close();

    // Převedení raw dat do QImage
    renderFromRawData();
    sourceFilePath = filePath;
    modified = false;
    
    return true;
}

bool Image::saveToFile(const QString &filePath) const {
    // Kontrola, zda je obrázek prázdný
    if (isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    // 1. Zápis file header (14 bajtů)
    file.write(fileHeader.bfType, 2);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfSize), 4);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfReserved1), 2);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfReserved2), 2);
    file.write(reinterpret_cast<const char*>(&fileHeader.bfOffBits), 4);

    // 2. Zápis info header (40 bajtů)
    file.write(reinterpret_cast<const char*>(&infoHeader.biSize), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biWidth), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biHeight), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biPlanes), 2);
    file.write(reinterpret_cast<const char*>(&infoHeader.biBitCount), 2);
    file.write(reinterpret_cast<const char*>(&infoHeader.biCompression), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biSizeImage), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biXPelsPerMeter), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biYPelsPerMeter), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biClrUsed), 4);
    file.write(reinterpret_cast<const char*>(&infoHeader.biClrImportant), 4);

    // 3. Zápis palety barev (pokud existuje)
    if (imageBitsPerPixel <= 8 && !colorPalette.isEmpty()) {
        for (QRgb color : colorPalette) {
            char paletteEntry[4];
            paletteEntry[0] = static_cast<char>(qBlue(color));  // B
            paletteEntry[1] = static_cast<char>(qGreen(color)); // G
            paletteEntry[2] = static_cast<char>(qRed(color));   // R
            paletteEntry[3] = 0; // Reserved
            file.write(paletteEntry, 4);
        }
    }

    // 4. Zápis obrazových dat
    if (modified) {
        // Výpočet velikosti řádku (musí být zarovnán na 4 bajty)
        int bytesPerRow = calculateRowSize();
        QByteArray dataToSave(bytesPerRow * imageHeight, 0);

        // Konverze pixelů z QImage zpět do formátu BMP
        for (int y = 0; y < qImage.height(); y++) {
            for (int x = 0; x < qImage.width(); x++) {
                QRgb pixel = qImage.pixel(x, y);

                // Pozice v datech (BMP ukládá data odspodu nahoru, pokud biHeight > 0)
                int row = (infoHeader.biHeight > 0) ? imageHeight - 1 - y : y;
                int byteIndex = row * bytesPerRow;

                if (imageBitsPerPixel == 24) {
                    // 24 bitů = 3 bajty na pixel
                    int index = byteIndex + x * 3;
                    if (index + 2 < dataToSave.size()) {
                        dataToSave[index] = static_cast<char>(qBlue(pixel));
                        dataToSave[index + 1] = static_cast<char>(qGreen(pixel));
                        dataToSave[index + 2] = static_cast<char>(qRed(pixel));
                    }
                }
                else if (imageBitsPerPixel == 8) {
                    // 8 bitů = 1 bajt na pixel
                    int index = byteIndex + x;
                    if (index < dataToSave.size()) {
                        // Nalezení nejbližší barvy v paletě
                        QRgb currentPixel = pixel;
                        int bestMatch = 0;
                        int bestDiff = INT_MAX;

                        for (int i = 0; i < colorPalette.size(); i++) {
                            QRgb paletteColor = colorPalette[i];
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
                else if (imageBitsPerPixel == 4) {
                    // 4 bity = 2 pixely na bajt
                    int bytePos = byteIndex + x / 2;
                    if (bytePos < dataToSave.size()) {
                        // Nalezení nejbližší barvy v paletě
                        QRgb currentPixel = pixel;
                        int bestMatch = 0;
                        int bestDiff = INT_MAX;

                        for (int i = 0; i < colorPalette.size() && i < 16; i++) {
                            QRgb paletteColor = colorPalette[i];
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
                        unsigned char currentByte = static_cast<unsigned char>(dataToSave[bytePos]);
                        if (x % 2 == 0) {
                            // Horní 4 bity (první pixel v bajtu)
                            currentByte = (currentByte & 0x0F) | (bestMatch << 4);
                        } else {
                            // Dolní 4 bity (druhý pixel v bajtu)
                            currentByte = (currentByte & 0xF0) | bestMatch;
                        }
                        dataToSave[bytePos] = static_cast<char>(currentByte);
                    }
                }
                else if (imageBitsPerPixel == 1) {
                    // 1 bit = 8 pixelů na bajt
                    int bytePos = byteIndex + x / 8;
                    if (bytePos < dataToSave.size()) {
                        // Pro 1-bitový obrázek máme jen 2 barvy (obvykle černá a bílá)
                        QRgb currentPixel = pixel;
                        int bestMatch = 0;

                        // Máme jen dvě barvy, takže stačí porovnat s první
                        if (colorPalette.size() >= 2) {
                            QRgb color0 = colorPalette[0];
                            QRgb color1 = colorPalette[1];

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
        // Použití původních dat, pokud obrázek nebyl upraven
        file.write(rawData);
    }

    file.close();
    return true;
}

int Image::calculateRowSize() const {
    return ((imageWidth * imageBitsPerPixel + 31) / 32) * 4;
}

const Image::BMPFileHeader & Image::getFileHeader() const {
    return fileHeader;
}

const Image::BMPInfoHeader & Image::getInfoHeader() const {
    return infoHeader;
}

void Image::renderFromRawData() {
    // Vytvoření prázdného obrázku
    qImage = QImage(imageWidth, imageHeight, QImage::Format_RGB32);
    int bytesPerRow = calculateRowSize();

    // Vykreslení podle bitové hloubky
    for (int y = 0; y < imageHeight; y++) {
        for (int x = 0; x < imageWidth; x++) {
            QRgb pixelColor;

            // Pozice v datech (BMP ukládá data odspodu nahoru, pokud biHeight > 0)
            int row = (infoHeader.biHeight > 0) ? imageHeight - 1 - y : y;
            int byteIndex = row * bytesPerRow;

            if (imageBitsPerPixel == 24) {
                // 24 bitů = 3 bajty na pixel
                int index = byteIndex + x * 3;
                if (index + 2 < rawData.size()) {
                    int blue = static_cast<unsigned char>(rawData[index]);
                    int green = static_cast<unsigned char>(rawData[index + 1]);
                    int red = static_cast<unsigned char>(rawData[index + 2]);
                    pixelColor = qRgb(red, green, blue);
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            }
            else if (imageBitsPerPixel == 8) {
                // 8 bitů = 1 bajt na pixel
                int index = byteIndex + x;
                if (index < rawData.size()) {
                    int colorIndex = static_cast<unsigned char>(rawData[index]);
                    pixelColor = colorPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            }
            else if (imageBitsPerPixel == 4) {
                // 4 bity = 2 pixely na bajt
                int index = byteIndex + x / 2;
                if (index < rawData.size()) {
                    int value = static_cast<unsigned char>(rawData[index]);
                    int colorIndex;
                    if (x % 2 == 0) {
                        // Horní 4 bity
                        colorIndex = (value >> 4) & 0x0F;
                    } else {
                        // Dolní 4 bity
                        colorIndex = value & 0x0F;
                    }
                    pixelColor = colorPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            }
            else if (imageBitsPerPixel == 1) {
                // 1 bit = 8 pixelů na bajt
                int index = byteIndex + x / 8;
                if (index < rawData.size()) {
                    int value = static_cast<unsigned char>(rawData[index]);
                    int shift = 7 - (x % 8);  // 7, 6, 5, 4, 3, 2, 1, 0
                    int colorIndex = (value >> shift) & 0x01;
                    pixelColor = colorPalette.value(colorIndex, qRgb(0, 0, 0));
                } else {
                    pixelColor = qRgb(0, 0, 0);
                }
            }
            else {
                pixelColor = qRgb(0, 0, 0);  // Fallback pro nepodporované formáty
            }

            qImage.setPixel(x, y, pixelColor);
        }
    }

}

void Image::applyFilter(const Filter &filter) {
    qImage = filter.apply(qImage);
    imageWidth = qImage.width();
    imageHeight = qImage.height();
    modified = true;
}

QImage Image::toQImage() const {
    // Kontrola, zda je qImage platný
    if (qImage.isNull() || qImage.width() != imageWidth || qImage.height() != imageHeight) {
        // V případě nekonzistence interních dat, znovu vygenerujeme QImage z raw dat
        if (!rawData.isEmpty()) {
            return QImage(imageWidth, imageHeight, QImage::Format_RGB32);
        }
        // Pokud nemáme ani raw data, vrátíme prázdný obrázek
        return QImage();
    }

    // Vrácení interního QImage objektu
    return qImage;
}

bool Image::isModified() const {
    return modified;
}

bool Image::isEmpty() const {
    // Obrázek je prázdný, pokud nemá rozměry (šířka nebo výška je 0)
    // nebo pokud je objekt QImage prázdný
    return imageWidth <= 0 || imageHeight <= 0 || qImage.isNull();
}

int Image::width() const {
    return imageWidth;
}

int Image::height() const {
    return imageHeight;
}

int Image::bitsPerPixel() const {
    return imageBitsPerPixel;
}

const QVector<QRgb> & Image::palette() const {
    return colorPalette;
}

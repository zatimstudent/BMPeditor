#ifndef IMAGE_H
#define IMAGE_H

#include <QImage>
#include <QString>
#include <QVector>

class Image {
public:
    Image();
    ~Image();

    bool loadFromFile(const QString &filePath);
    bool saveToFile(const QString &filePath) const;
    void applyFilter(const class Filter &filter);

    QImage toQImage() const;

    bool isModified() const;
    bool isEmpty() const;

    // Gettery pro metadata
    int width() const;
    int height() const;
    int bitsPerPixel() const;
    const QVector<QRgb>& palette() const;

    // Gettery pro BMP header informace
    struct BMPFileHeader {
        char bfType[2];     // musí být "BM" pro BMP soubory
        uint32_t bfSize;    // velikost souboru v bajtech
        uint16_t bfReserved1; // rezervováno, musí být 0
        uint16_t bfReserved2; // rezervováno, musí být 0
        uint32_t bfOffBits;  // offset, kde začínají data obrázku
    };

    struct BMPInfoHeader {
        uint32_t biSize;           // velikost této hlavičky (40 bajtů)
        int32_t biWidth;          // šířka obrázku v pixelech
        int32_t biHeight;         // výška obrázku v pixelech
        uint16_t biPlanes;         // počet barevných rovin (musí být 1)
        uint16_t biBitCount;       // počet bitů na pixel (1, 4, 8, 24)
        uint32_t biCompression;    // typ komprese
        uint32_t biSizeImage;      // velikost obrázku v bajtech
        int32_t biXPelsPerMeter;  // horizontální rozlišení
        int32_t biYPelsPerMeter;  // vertikální rozlišení
        uint32_t biClrUsed;        // počet použitých barev
        uint32_t biClrImportant;   // počet důležitých barev
    };

    const BMPFileHeader& getFileHeader() const;
    const BMPInfoHeader& getInfoHeader() const;

private:
    QImage qImage;
    QByteArray rawData;
    QVector<QRgb> colorPalette;
    int imageWidth;
    int imageHeight;
    int imageBitsPerPixel;
    bool modified;
    QString sourceFilePath;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    void renderFromRawData();
    int calculateRowSize() const;
};

#endif // IMAGE_H
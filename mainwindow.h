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

#include "customimagewidget.h"
#include "Filters/Filter.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImage();
    void saveImage();

private:
    CustomImageWidget *imageWidget;
    QTextEdit *infoTextEdit;
    QImage image;
    QString filePath;  // Uchování cesty k souboru
    bool imageModified;
    std::vector<std::unique_ptr<Filter>> filters;


    // Proměnné pro vlastní vykreslování BMP souboru
    QByteArray customBMPData;
    QVector<QRgb> customBMPPalette;
    int customBMPWidth;
    int customBMPHeight;
    int customBMPBitsPerPixel;

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

    BMPFileHeader bmpFileHeader;
    BMPInfoHeader bmpInfoHeader;

    void renderCustomBMP();
    bool loadCustomBMP(const QString &fileName);
    void updateImageInfo();
    bool loadBMPFile(const QString &filePath);
    void createMenuBar(); // Funkce pro vytvoření menu
};

#endif // MAINWINDOW_H
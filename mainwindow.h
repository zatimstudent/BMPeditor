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

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openImage();      // Slot pro otevření obrázku
    void saveImage();      // Slot pro uložení obrázku
    void invertColors();
    void rotateImage();
    void flipImage();

private:
    QLabel *imageLabel;
    QTextEdit *infoTextEdit;
    QImage image;
    QString filePath;  // Uchování cesty k souboru

    // Proměnné pro vlastní vykreslování BMP souboru
    QByteArray customBMPData;
    QVector<QRgb> customBMPPalette;
    int customBMPWidth;
    int customBMPHeight;
    int customBMPBitsPerPixel;

    void renderCustomBMP();
    bool loadCustomBMP(const QString &fileName);

    void updateImageInfo();

bool loadBMPFile(const QString &filePath);

void createMenuBar(); // Funkce pro vytvoření menu

};

#endif // MAINWINDOW_H

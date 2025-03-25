#include "customimagewidget.h"

#include <iostream>
#include <QPainter>
#include <QScrollBar>

CustomImageWidget::CustomImageWidget(QWidget* parent)
    : QWidget(parent), zoomFactor(1.0) {}

void CustomImageWidget::setImage(const QImage& newImage) {
    image = newImage;
    update(); // Vyvolá překreslení
}

void CustomImageWidget::setZoomFactor(double factor) {
    // Omezení faktoru zoomu na rozumné hodnoty
    zoomFactor = qBound(0.1, factor, 10.0);
    update(); // Překreslit s novým faktorem
}

double CustomImageWidget::getZoomFactor() const {
    return zoomFactor;
}

void CustomImageWidget::zoomIn() {
    setZoomFactor(zoomFactor * 1.25); // Zvětšení o 25%
}

void CustomImageWidget::zoomOut() {
    setZoomFactor(zoomFactor / 1.25); // Zmenšení o 20%
}

void CustomImageWidget::resetZoom() {
    setZoomFactor(1.0);
}

void CustomImageWidget::paintEvent(QPaintEvent* event) {
    if (image.isNull()) return;

    QPainter painter(this);

    int scaledWidth = qRound(image.width() * zoomFactor);
    int scaledHeight = qRound(image.height() * zoomFactor);

    // Vytvoření nového obrázku se zvětšenými pixely
    QImage scaledImage(scaledWidth, scaledHeight, QImage::Format_RGB32);

    // Vyplnění obrázku zvětšenými pixely
    for (int y = 0; y < image.height(); y++) {
        int scaledY = qRound(y * zoomFactor);
        int pixelHeight = qRound((y + 1) * zoomFactor) - scaledY;

        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            int scaledX = qRound(x * zoomFactor);
            int pixelWidth = qRound((x + 1) * zoomFactor) - scaledX;

            // Vyplnění bloku pixelů stejnou barvou
            for (int sy = 0; sy < pixelHeight; sy++) {
                for (int sx = 0; sx < pixelWidth; sx++) {
                    if (scaledX + sx < scaledWidth && scaledY + sy < scaledHeight) {
                        scaledImage.setPixel(scaledX + sx, scaledY + sy, pixel);
                    }
                }
            }
        }
    }

    // Vykreslení připraveného obrázku
    int x_offset = (width() - scaledWidth) / 2;
    int y_offset = (height() - scaledHeight) / 2;
    painter.drawImage(x_offset, y_offset, scaledImage);
}
void CustomImageWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
    event->accept();
}
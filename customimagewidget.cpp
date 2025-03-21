#include "customimagewidget.h"

#include <QPainter>

CustomImageWidget::CustomImageWidget(QWidget* parent) : QWidget(parent) {}

void CustomImageWidget::setImage(const QImage& newImage) {
    image = newImage;
    setMinimumSize(image.size());
    update(); // Vyvolá překreslení
}

void CustomImageWidget::paintEvent(QPaintEvent* event) {
    if (image.isNull()) return;

    QPainter painter(this);

    // Výpočet pozice pro centrování obrázku
    int x_offset = (width() - image.width()) / 2;
    int y_offset = (height() - image.height()) / 2;

    // Vykreslení pixel po pixelu s offsetem pro centrování
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            QRgb pixel = image.pixel(x, y);
            painter.setPen(QColor(pixel));
            painter.drawPoint(x + x_offset, y + y_offset);
        }
    }
}
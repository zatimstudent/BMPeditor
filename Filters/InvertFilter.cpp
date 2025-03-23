#include "InvertFilter.h"

QImage InvertFilter::apply(const QImage& image) {
    if (image.isNull()) return image;

    QImage result = image.copy();
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QColor color = result.pixelColor(x, y);
            color.setRed(255 - color.red());
            color.setGreen(255 - color.green());
            color.setBlue(255 - color.blue());
            result.setPixelColor(x, y, color);
        }
    }
    return result;
}
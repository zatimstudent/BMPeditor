#include "RotateFilter.h"
#include <QTransform>

QImage RotateFilter::apply(const QImage& image) {
    if (image.isNull()) return image;
    return image.transformed(QTransform().rotate(90));
}
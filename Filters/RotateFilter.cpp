#include "RotateFilter.h"
#include <QTransform>

QImage RotateFilter::apply(const QImage& image) const {
    if (image.isNull()) return image;
    return image.transformed(QTransform().rotate(90));
}
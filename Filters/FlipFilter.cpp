#include "FlipFilter.h"

QImage FlipFilter::apply(const QImage& image) const {
    if (image.isNull()) return image;
    return image.mirrored(true, false);
}
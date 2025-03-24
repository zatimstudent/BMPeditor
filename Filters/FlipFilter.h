#ifndef FLIPFILTER_H
#define FLIPFILTER_H

#include "Filter.h"

class FlipFilter : public Filter {
public:
QImage apply(const QImage& image) const override;
    QString name() const override { return "Flip Horizontal"; }
};

#endif // FLIPFILTER_H
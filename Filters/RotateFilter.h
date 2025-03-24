#ifndef ROTATEFILTER_H
#define ROTATEFILTER_H

#include "Filter.h"

class RotateFilter : public Filter {
public:
    QImage apply(const QImage& image) const override;
    QString name() const override { return "Rotate 90°"; }
};

#endif // ROTATEFILTER_H
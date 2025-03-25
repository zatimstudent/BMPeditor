#ifndef INVERTFILTER_H
#define INVERTFILTER_H

#include "Filter.h"

class InvertFilter : public Filter {
public:
    QImage apply(const QImage& image) const override;
    QString name() const override { return "Invert Colors"; }
};

#endif // INVERTFILTER_H
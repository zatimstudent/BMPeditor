#ifndef FILTER_H
#define FILTER_H

#include <QImage>

class Filter {
public:
    virtual ~Filter() = default;
    virtual QImage apply(const QImage& image) = 0;
    virtual QString name() const = 0;
};

#endif // FILTER_H
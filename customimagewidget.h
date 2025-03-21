#ifndef CUSTOMIMAGEWIDGET_H
#define CUSTOMIMAGEWIDGET_H

#include <QImage>
#include <QPaintEvent>
#include <QWidget>

class CustomImageWidget : public QWidget {
    Q_OBJECT
public:
    CustomImageWidget(QWidget* parent = nullptr);
    void setImage(const QImage& newImage);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QImage image;
};

#endif // CUSTOMIMAGEWIDGET_H
// customimagewidget.h
#ifndef CUSTOMIMAGEWIDGET_H
#define CUSTOMIMAGEWIDGET_H

#include <QImage>
#include <QPaintEvent>
#include <QWidget>
#include <QWheelEvent>

class CustomImageWidget : public QWidget {
    Q_OBJECT
public:
    CustomImageWidget(QWidget* parent = nullptr);
    void setImage(const QImage& newImage);

    // Nové metody pro zoom
    void setZoomFactor(double factor);
    double getZoomFactor() const;
    void zoomIn();
    void zoomOut();
    void resetZoom();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override; // Pro zoom kolečkem myši

private:
    QImage image;
    double zoomFactor;  // Přidána proměnná pro zoom
};

#endif // CUSTOMIMAGEWIDGET_H
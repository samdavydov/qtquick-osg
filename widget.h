#pragma once

#include "OsgWidget.h"

class QTimer;

class QQmlEngine;
class QQuickItem;

class ImageSource;

namespace osg {
    class Texture2D;
    class Image;
}

class Widget : public OsgWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public:
    void updateTexture(QImage&& imageData);

private:
    QTimer* m_timer;

    QQmlEngine* m_qmlEngine;
    QQuickItem* m_quickItem;

    ImageSource* m_imageSource;
    QImage*      m_imageData;

    osg::Texture2D* m_osgTexture;
    osg::Image*     m_osgImage;
};

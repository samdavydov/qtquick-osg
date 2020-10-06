#pragma once

#include "OsgWidget.h"

class QTimer;

class QQmlEngine;
class QQuickItem;

class ImageSource;

namespace osg
{
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
    void updateTexture(osg::Image* osgImage, QImage* imageBuffer, QImage&& imageData);
    void createOsgTextureFromId(osg::Texture2D* texture, int textureId);
    void clearOsgTexture(osg::Texture2D* texture);

protected:
    void initializeGL() override;

private:
    QTimer* m_timer;

    QQmlEngine* m_qmlEngine;

    ImageSource* m_imageSource1;
    ImageSource* m_imageSource2;

    osg::Texture2D* m_osgTexture1;
    osg::Texture2D* m_osgTexture2;

    QImage*     m_imageData1;
    QImage*     m_imageData2;

    osg::Image* m_osgImage1;
    osg::Image* m_osgImage2;
};

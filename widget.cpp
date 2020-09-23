#include "widget.h"

#include "ImageSource.h"

#include <osg/Camera>
#include <osg/Texture2D>
#include <osgGA/TrackballManipulator>

#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QImage>

#include <QTimer>
#include <QDebug>

namespace
{
    const osg::Vec4 BLUE_SKY(osg::Vec4(0.53f, 0.81f, 0.92f, 1.0f));

    QQuickItem* createItem(QQmlEngine* engine, const QUrl& url)
    {
        QQmlComponent component(engine, url, QQmlComponent::PreferSynchronous);
        QQuickItem* item = qobject_cast<QQuickItem*>(component.create());

        if (!item)
            qWarning("failed to load \"%s\": \"%s\"", qPrintable(url.toString()), qPrintable(component.errorString()));

        return item;
    }
}

Widget::Widget(QWidget* parent)
    : OsgWidget(parent)
    , m_timer(new QTimer(this))
    , m_qmlEngine(new QQmlEngine(this))
    , m_quickItem(nullptr)
    , m_imageSource(new ImageSource(this))
    , m_imageData(new QImage)
    , m_osgTexture(new osg::Texture2D)
    , m_osgImage(new osg::Image)
{
    m_osgViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    m_osgViewer->setCameraManipulator(new osgGA::OrbitManipulator());

    osg::Viewport* viewport = new osg::Viewport(0, 0, width(), height());

    osg::Camera* camera = m_osgViewer->getCamera();
    camera->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
    camera->setGraphicsContext(osgContext());
    camera->setClearColor(BLUE_SKY);
    camera->setViewport(viewport);
    camera->setProjectionMatrixAsPerspective(45.f, viewport->aspectRatio(), 0.1, 100000.0);

    m_osgTexture->setImage(m_osgImage);

    osg::ref_ptr<osg::Geode> root = new osg::Geode;
    osg::ref_ptr<osg::Geometry> quad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f, 0.0f, 0.0f),
                                                                       osg::Vec3( 512, 0.0f, 0.0f),
                                                                       osg::Vec3(0.0f, 0.0f,  512));

    quad->getOrCreateStateSet()->setTextureAttributeAndModes(0, m_osgTexture);
    quad->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    root->addChild(quad);
    m_osgViewer->setSceneData(root);

    QOpenGLContext* qmlGLContext = new QOpenGLContext(this);
    qmlGLContext->setShareContext(context());
    qmlGLContext->create();

    m_quickItem = createItem(m_qmlEngine, QUrl::fromLocalFile("test.qml"));
    m_imageSource->init(qmlGLContext, m_quickItem, 256, 256, 0);

    connect(m_imageSource, &ImageSource::newImageAvailable, [this](int id)
    {
        Q_UNUSED(id)
#if 1
        updateTexture(m_imageSource->fbo()->toImage().convertToFormat(QImage::Format_RGBX8888)); // download pixels from QML, upload pixels to OSG (GPU-CPU-GPU)
#else
        // smth that uses FBO to transfer data between QML and OSG
#endif
    });

    connect(m_timer, &QTimer::timeout, [this]
    {
        update();
        m_imageSource->render();
    });

    m_timer->start(16);
}

Widget::~Widget()
{}

void Widget::updateTexture(QImage&& imageData)
{
    m_osgImage->setImage(imageData.width(), imageData.height(), 1, 4, GL_RGBA, GL_UNSIGNED_BYTE,
                         const_cast<uchar*>(imageData.constBits()), osg::Image::NO_DELETE);

    m_imageData->swap(imageData);
    m_osgImage->dirty();
}

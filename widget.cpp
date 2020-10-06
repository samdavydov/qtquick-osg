#include "widget.h"

#include "ImageSource.h"

#include <osg/Camera>
#include <osg/Texture2D>
#include <osgGA/OrbitManipulator>

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
    , m_imageSource1(new ImageSource(this))
    , m_imageSource2(new ImageSource(this))
    , m_osgTexture1(new osg::Texture2D)
    , m_osgTexture2(new osg::Texture2D)
    , m_imageData1(new QImage)
    , m_imageData2(new QImage)
    , m_osgImage1(new osg::Image)
    , m_osgImage2(new osg::Image)
{
#if 1 // FBO (GPU-GPU)
    setWindowTitle("QtQuick-OpenSceneGraph - FBO (GPU -> GPU)");

    connect(m_imageSource1, &ImageSource::fboCreated, [this](QOpenGLFramebufferObject* fbo)
    {
        createOsgTextureFromId(m_osgTexture1, fbo->texture());
    });

    connect(m_imageSource2, &ImageSource::fboCreated, [this](QOpenGLFramebufferObject* fbo)
    {
        createOsgTextureFromId(m_osgTexture2, fbo->texture());
    });

    connect(m_imageSource1, &ImageSource::fboDestroyed, [this]
    {
        clearOsgTexture(m_osgTexture1);
    });

    connect(m_imageSource2, &ImageSource::fboDestroyed, [this]
    {
        clearOsgTexture(m_osgTexture2);
    });
#else // COPY (GPU-CPU-GPU)
    setWindowTitle("QtQuick-OpenSceneGraph - copy texture (GPU -> CPU -> GPU)");

    connect(m_imageSource1, &ImageSource::newImageAvailable, [this]
    {
        updateTexture(m_osgImage1, m_imageData1, m_imageSource1->fbo()->toImage(false)); // download pixels from QML, upload pixels to OSG (GPU-CPU-GPU)
    });

    connect(m_imageSource2, &ImageSource::newImageAvailable, [this]
    {
        updateTexture(m_osgImage2, m_imageData2, m_imageSource2->fbo()->toImage(false)); // download pixels from QML, upload pixels to OSG (GPU-CPU-GPU)
    });

    m_osgTexture1->setImage(m_osgImage1);
    m_osgTexture2->setImage(m_osgImage2);
#endif

    m_osgViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    m_osgViewer->setCameraManipulator(new osgGA::OrbitManipulator());

    osg::Viewport* viewport = new osg::Viewport(0, 0, width(), height());

    osg::Camera* camera = m_osgViewer->getCamera();
    camera->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
    camera->setGraphicsContext(osgContext());
    camera->setClearColor(BLUE_SKY);
    camera->setViewport(viewport);
    camera->setProjectionMatrixAsPerspective(45.f, viewport->aspectRatio(), 0.1, 100000.0);

    osg::ref_ptr<osg::Geometry> quad1 = osg::createTexturedQuadGeometry({  0.f, 0.f, 0.f}, {512.f, 0.f, 0.f}, {0.f, 0.f, 512.f});
    osg::ref_ptr<osg::Geometry> quad2 = osg::createTexturedQuadGeometry({515.f, 0.f, 0.f}, {512.f, 0.f, 0.f}, {0.f, 0.f, 512.f});

    quad1->getOrCreateStateSet()->setTextureAttributeAndModes(0, m_osgTexture1);
    quad1->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    quad2->getOrCreateStateSet()->setTextureAttributeAndModes(0, m_osgTexture2);
    quad2->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    osg::ref_ptr<osg::Geode> root = new osg::Geode;

    root->addChild(quad1);
    root->addChild(quad2);

    m_osgViewer->setSceneData(root);

    connect(m_timer, &QTimer::timeout, [this]
    {
        m_imageSource1->render();
        m_imageSource2->render();

        update();
    });

    m_timer->start(16);
}

Widget::~Widget()
{}

void Widget::updateTexture(osg::Image* osgImage, QImage* imageBuffer, QImage&& imageData)
{
    osgImage->setImage(imageData.width(), imageData.height(), 1, 4, GL_RGBA, GL_UNSIGNED_BYTE,
                       const_cast<uchar*>(imageData.constBits()), osg::Image::NO_DELETE);

    imageBuffer->swap(imageData);
    osgImage->dirty();
}

void Widget::createOsgTextureFromId(osg::Texture2D* texture, int textureId)
{
    osg::Texture::TextureObject* textureObject = new osg::Texture::TextureObject(texture, textureId, GL_TEXTURE_2D);
    textureObject->setAllocated();

    osg::State* state = m_osgGraphicsContext->getState();
    texture->setTextureObject(state->getContextID(), textureObject);

#if 0 // ???
    state->setActiveTextureUnit(0);
    texture->apply(*state);
    state->haveAppliedTextureAttribute(0, texture);
#endif
}

void Widget::clearOsgTexture(osg::Texture2D* texture)
{
    if (m_osgGraphicsContext)
    {
        osg::State* state = m_osgGraphicsContext->getState();
        texture->setTextureObject(state->getContextID(), nullptr);
    }
}

void Widget::initializeGL()
{
    QOpenGLContext* qmlGLContext = new QOpenGLContext(this);
    qmlGLContext->setShareContext(context());
    qmlGLContext->create();

    QQuickItem* item1 = createItem(m_qmlEngine, QUrl::fromLocalFile("test.qml"));
    QQuickItem* item2 = createItem(m_qmlEngine, QUrl::fromLocalFile("test.qml"));

    m_imageSource1->init(qmlGLContext, item1, 1024, 1024);
    m_imageSource2->init(qmlGLContext, item2, 1024, 1024);

    QOpenGLWidget::initializeGL();
}

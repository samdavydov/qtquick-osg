#include "ImageSource.h"

#include <QQuickRenderControl>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QQuickWindow>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QQuickItem>
#include <QTimer>

ImageSource::ImageSource(QObject* parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
    , m_renderControl(new QQuickRenderControl(this))
    , m_quickWindow(new QQuickWindow(m_renderControl))
    , m_id(0)
{
    connect(m_quickWindow  , &QQuickWindow::sceneGraphInitialized , this, &ImageSource::createFbo);
    connect(m_quickWindow  , &QQuickWindow::sceneGraphInvalidated , this, &ImageSource::destroyFbo);
    connect(m_updateTimer  , &QTimer::timeout                     , this, &ImageSource::render);
}

ImageSource::~ImageSource()
{
    clear();
}

void ImageSource::init(QOpenGLContext* glContext, QQuickItem* rootItem, int width, int height, int id)
{
    m_id = id;
    m_glContext = glContext;

    m_offscreenSurface = new QOffscreenSurface;
    m_offscreenSurface->setFormat(m_glContext->format());
    m_offscreenSurface->create();

    m_glContext->makeCurrent(m_offscreenSurface);
    m_renderControl->initialize(m_glContext);

    m_rootItem = rootItem;
    m_rootItem->setParentItem(m_quickWindow->contentItem());

    resize(width, height);
    render();
}

void ImageSource::clear()
{
    stop();

    if (m_glContext)
    {
        m_glContext->makeCurrent(m_offscreenSurface);

        delete m_renderControl;
        delete m_quickWindow;
        delete m_fbo;

        m_glContext->doneCurrent();

        delete m_offscreenSurface;
    }
}

void ImageSource::render()
{
    if (m_glContext->makeCurrent(m_offscreenSurface))
    {
        m_renderControl->polishItems();
        m_renderControl->sync();
        m_renderControl->render();

        m_quickWindow->resetOpenGLState();
        QOpenGLFramebufferObject::bindDefault();

        m_glContext->functions()->glFlush();

        emit newImageAvailable(m_id);
    }
}

void ImageSource::resize(int width, int height)
{
    if (m_rootItem && m_glContext && m_glContext->makeCurrent(m_offscreenSurface))
    {
        m_size = QSize(width, height);

        delete m_fbo;
        createFbo();
        m_glContext->doneCurrent();

        m_rootItem->setSize(m_size);
        m_quickWindow->setGeometry(0, 0, width, height);
    }
    else
        qWarning("failed to resize");
}

void ImageSource::setUpdateInterval(int ms)
{
    m_updateTimer->setInterval(qMax(10, ms));
}

void ImageSource::start()
{
    m_updateTimer->start();
}

void ImageSource::stop()
{
    m_updateTimer->stop();
}

QQuickItem* ImageSource::rootObject() const
{
    return m_rootItem;
}

QOpenGLFramebufferObject* ImageSource::fbo() const
{
    return m_fbo;
}

int ImageSource::updateInterval() const
{
    return m_updateTimer->interval();
}

int ImageSource::width() const
{
    return m_fbo ? m_fbo->width() : -1;
}

int ImageSource::height() const
{
    return m_fbo ? m_fbo->height() : -1;
}

void ImageSource::createFbo()
{
    m_fbo = new QOpenGLFramebufferObject(m_size);
    m_quickWindow->setRenderTarget(m_fbo);
}

void ImageSource::destroyFbo()
{
    delete m_fbo;
    m_fbo = nullptr;
}

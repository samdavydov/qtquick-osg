#include "ImageSource.h"

#include <QQuickRenderControl>
#include <QOffscreenSurface>
#include <QQuickWindow>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QQuickItem>

ImageSource::ImageSource(QObject* parent)
    : QObject(parent)
    , m_renderControl(new QQuickRenderControl(this))
    , m_offscreenSurface(nullptr)
    , m_quickWindow(new QQuickWindow(m_renderControl))
    , m_fbo(nullptr)
    , m_initialized(false)
{
    connect(m_quickWindow, &QQuickWindow::sceneGraphInitialized , [this]
    {
        createFbo(m_quickWindow->size());
    });

    connect(m_quickWindow, &QQuickWindow::sceneGraphInvalidated, this, &ImageSource::destroyFbo);
}

ImageSource::~ImageSource()
{
    clear();
}

void ImageSource::init(QOpenGLContext* glContext, QQuickItem* rootItem, int width, int height)
{
    m_glContext = glContext;

    m_offscreenSurface = new QOffscreenSurface;
    m_offscreenSurface->setFormat(m_glContext->format());
    m_offscreenSurface->create();

    m_glContext->makeCurrent(m_offscreenSurface);
    m_renderControl->initialize(m_glContext);

    m_rootItem = rootItem;
    m_rootItem->setParentItem(m_quickWindow->contentItem());

    m_initialized = true;

    resize(width, height);
    render();
}

void ImageSource::clear()
{
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

#if 0
        m_quickWindow->resetOpenGLState();
        QOpenGLFramebufferObject::bindDefault();
        m_glContext->functions()->glFlush();
#endif

        emit newImageAvailable();
    }
}

void ImageSource::resize(int width, int height)
{
    if (m_rootItem && m_glContext && m_glContext->makeCurrent(m_offscreenSurface))
    {
        QSize size(width, height);

        createFbo(size);

        m_glContext->doneCurrent();

        m_rootItem->setSize(size);
        m_quickWindow->setGeometry(0, 0, width, height);
    }
    else
        qWarning("failed to resize");
}

QQuickWindow* ImageSource::window() const
{
    return m_quickWindow;
}

QQuickItem* ImageSource::rootObject() const
{
    return m_rootItem;
}

QOpenGLFramebufferObject* ImageSource::fbo() const
{
    return m_fbo;
}

int ImageSource::width() const
{
    return m_fbo ? m_fbo->width() : -1;
}

int ImageSource::height() const
{
    return m_fbo ? m_fbo->height() : -1;
}

void ImageSource::createFbo(const QSize& size)
{
    destroyFbo();

    if (m_initialized)
    {
        m_fbo = new QOpenGLFramebufferObject(size);
        m_quickWindow->setRenderTarget(m_fbo);

        emit fboCreated(m_fbo);
    }
}

void ImageSource::destroyFbo()
{
    if (m_fbo)
    {
        delete m_fbo;
        m_fbo = nullptr;

        m_quickWindow->setRenderTarget(nullptr);

        emit fboDestroyed();
    }
}

#pragma once

#include <QObject>
#include <QPointer>

class QQuickRenderControl;
class QOpenGLContext;
class QQuickWindow;
class QOffscreenSurface;
class QOpenGLFramebufferObject;
class QQuickItem;

class ImageSource : public QObject
{
    Q_OBJECT

public:
    explicit ImageSource(QObject* parent = nullptr);
    ~ImageSource() override;

public:
    void init(QOpenGLContext* glContext, QQuickItem* rootItem, int width, int height);
    void clear();

    void render();
    void resize(int width, int height);

public:
    QQuickWindow* window() const;
    QQuickItem* rootObject() const;
    QOpenGLFramebufferObject* fbo() const;

    int width() const;
    int height() const;

private:
    void createFbo(const QSize& size);
    void destroyFbo();

signals:
    void newImageAvailable();
    void fboCreated(QOpenGLFramebufferObject* fbo);
    void fboDestroyed();

private:
    QPointer<QOpenGLContext> m_glContext;
    QPointer<QQuickItem>     m_rootItem;

    QQuickRenderControl*      m_renderControl;
    QOffscreenSurface*        m_offscreenSurface;
    QQuickWindow*             m_quickWindow;
    QOpenGLFramebufferObject* m_fbo;

    bool m_initialized;
};

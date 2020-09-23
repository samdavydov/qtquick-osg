#pragma once

#include <QObject>
#include <QPointer>
#include <QSize>

class QTimer;
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
    explicit ImageSource(QObject *parent = nullptr);
    ~ImageSource() override;

public:
    void init(QOpenGLContext* glContext, QQuickItem* rootItem, int width, int height, int id);
    void clear();

    void render();
    void resize(int width, int height);
    void setUpdateInterval(int ms);

    void start();
    void stop();

public:
    QQuickItem* rootObject() const;
    QOpenGLFramebufferObject* fbo() const;

    int updateInterval() const;
    int width() const;
    int height() const;

private:
    void createFbo();
    void destroyFbo();

signals:
    void newImageAvailable(int id);

private:
    QTimer* m_updateTimer;

    QPointer<QOpenGLContext> m_glContext;
    QPointer<QQuickItem>     m_rootItem;

    QQuickRenderControl*      m_renderControl;
    QOffscreenSurface*        m_offscreenSurface;
    QQuickWindow*             m_quickWindow;
    QOpenGLFramebufferObject* m_fbo;

    QSize m_size;
    int   m_id;
};

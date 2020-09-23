#pragma once

#include <QOpenGLWidget>

#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>

class QInputEvent;

class OsgWidget : public QOpenGLWidget
{
public:
    OsgWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);
    virtual ~OsgWidget();

public:
    osgViewer::Viewer* osgViewer() const;
    osgViewer::GraphicsWindowEmbedded* osgContext() const;

protected: // QOpenGLWdiget
    void paintEvent(QPaintEvent* event) override;
    void paintGL() override;
    void resizeGL(int width, int height ) override;

    void keyPressEvent        (QKeyEvent* event) override;
    void keyReleaseEvent      (QKeyEvent* event) override;

    void mousePressEvent      (QMouseEvent* event) override;
    void mouseReleaseEvent    (QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseMoveEvent       (QMouseEvent* event) override;
    void wheelEvent           (QWheelEvent* event) override;

    bool event(QEvent* event) override;

protected:
    void setKeyboardModifiers(QInputEvent* event);
    osgGA::EventQueue* osgEventQueue() const;

protected:
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_osgGraphicsContext;
    osg::ref_ptr<osgViewer::Viewer> m_osgViewer;

    bool m_firstFrame;
};

#include "OsgWidget.h"

#include <QOpenGLContext>
#include <QInputEvent>
#include <QHash>
#include <QPainter>

#include <osg/DeleteHandler>
#include <osgViewer/ViewerBase>

#include <QDebug>

namespace
{
    using KeyMap = QHash<int, int>;

    KeyMap KEY_MAP = {
        {Qt::Key_Escape   , osgGA::GUIEventAdapter::KEY_Escape},
        {Qt::Key_Delete   , osgGA::GUIEventAdapter::KEY_Delete},
        {Qt::Key_Home     , osgGA::GUIEventAdapter::KEY_Home},
        {Qt::Key_Enter    , osgGA::GUIEventAdapter::KEY_KP_Enter},
        {Qt::Key_End      , osgGA::GUIEventAdapter::KEY_End},
        {Qt::Key_Return   , osgGA::GUIEventAdapter::KEY_Return},
        {Qt::Key_PageUp   , osgGA::GUIEventAdapter::KEY_Page_Up},
        {Qt::Key_PageDown , osgGA::GUIEventAdapter::KEY_Page_Down},
        {Qt::Key_Left     , osgGA::GUIEventAdapter::KEY_Left},
        {Qt::Key_Right    , osgGA::GUIEventAdapter::KEY_Right},
        {Qt::Key_Up       , osgGA::GUIEventAdapter::KEY_Up},
        {Qt::Key_Down     , osgGA::GUIEventAdapter::KEY_Down},
        {Qt::Key_Backspace, osgGA::GUIEventAdapter::KEY_BackSpace},
        {Qt::Key_Tab      , osgGA::GUIEventAdapter::KEY_Tab},
        {Qt::Key_Space    , osgGA::GUIEventAdapter::KEY_Space},
        {Qt::Key_Delete   , osgGA::GUIEventAdapter::KEY_Delete},
        {Qt::Key_Alt      , osgGA::GUIEventAdapter::KEY_Alt_L},
        {Qt::Key_Shift    , osgGA::GUIEventAdapter::KEY_Shift_L},
        {Qt::Key_Control  , osgGA::GUIEventAdapter::KEY_Control_L},
        {Qt::Key_Meta     , osgGA::GUIEventAdapter::KEY_Meta_L},
        {Qt::Key_F1       , osgGA::GUIEventAdapter::KEY_F1},
        {Qt::Key_F2       , osgGA::GUIEventAdapter::KEY_F2},
        {Qt::Key_F3       , osgGA::GUIEventAdapter::KEY_F3},
        {Qt::Key_F4       , osgGA::GUIEventAdapter::KEY_F4},
        {Qt::Key_F5       , osgGA::GUIEventAdapter::KEY_F5},
        {Qt::Key_F6       , osgGA::GUIEventAdapter::KEY_F6},
        {Qt::Key_F7       , osgGA::GUIEventAdapter::KEY_F7},
        {Qt::Key_F8       , osgGA::GUIEventAdapter::KEY_F8},
        {Qt::Key_F9       , osgGA::GUIEventAdapter::KEY_F9},
        {Qt::Key_F10      , osgGA::GUIEventAdapter::KEY_F10},
        {Qt::Key_F11      , osgGA::GUIEventAdapter::KEY_F11},
        {Qt::Key_F12      , osgGA::GUIEventAdapter::KEY_F12},
        {Qt::Key_F13      , osgGA::GUIEventAdapter::KEY_F13},
        {Qt::Key_F14      , osgGA::GUIEventAdapter::KEY_F14},
        {Qt::Key_F15      , osgGA::GUIEventAdapter::KEY_F15},
        {Qt::Key_F16      , osgGA::GUIEventAdapter::KEY_F16},
        {Qt::Key_F17      , osgGA::GUIEventAdapter::KEY_F17},
        {Qt::Key_F18      , osgGA::GUIEventAdapter::KEY_F18},
        {Qt::Key_F19      , osgGA::GUIEventAdapter::KEY_F19},
        {Qt::Key_F20      , osgGA::GUIEventAdapter::KEY_F20},
        {Qt::Key_hyphen   , '-'},
        {Qt::Key_Equal    , '='},
        {Qt::Key_division , osgGA::GUIEventAdapter::KEY_KP_Divide},
        {Qt::Key_multiply , osgGA::GUIEventAdapter::KEY_KP_Multiply},
        {Qt::Key_Minus    , '-'},
        {Qt::Key_Plus     , '+'},
        {Qt::Key_Insert   , osgGA::GUIEventAdapter::KEY_KP_Insert},
        {Qt::Key_Delete   , osgGA::GUIEventAdapter::KEY_KP_Delete},
        {Qt::Key_Home     , osgGA::GUIEventAdapter::KEY_KP_Home},
        {Qt::Key_End      , osgGA::GUIEventAdapter::KEY_KP_End},
        {Qt::Key_PageUp   , osgGA::GUIEventAdapter::KEY_KP_Page_Up},
        {Qt::Key_PageDown , osgGA::GUIEventAdapter::KEY_KP_Page_Down},
    };

    int remapKey(QKeyEvent* event)
    {
        auto i = KEY_MAP.find(event->key());
        return (i != KEY_MAP.end()) ? i.value() : int(*(event->text().toLatin1().data()));
    }

    QSurfaceFormat defaultGraphicsSettings()
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();

        format.setSamples(4);

        return format;
    }
};

OsgWidget::OsgWidget(QWidget* parent, Qt::WindowFlags flags)
    : QOpenGLWidget(parent, flags)
    , m_osgGraphicsContext(new osgViewer::GraphicsWindowEmbedded(x(), y(), width(), height()))
    , m_osgViewer(new osgViewer::Viewer)
    , m_firstFrame(false)
{
    setFormat(defaultGraphicsSettings());

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
}

OsgWidget::~OsgWidget()
{}

osgViewer::Viewer* OsgWidget::osgViewer() const
{
    return m_osgViewer;
}

osgViewer::GraphicsWindowEmbedded* OsgWidget::osgContext() const
{
    return m_osgGraphicsContext;
}

bool OsgWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Hide)
    {
        // workaround "Qt-workaround" that does glFinish before hiding the widget
        // (the Qt workaround was seen at least in Qt 4.6.3 and 4.7.0)
        //
        // Qt makes the context current, performs glFinish, and releases the context.
        // This makes the problem in OSG multithreaded environment as the context
        // is active in another thread, thus it can not be made current for the purpose
        // of glFinish in this thread. We workaround it by skiping QGLWidget::event() code.
        return QWidget::event(event);
    }

    // perform regular event handling
    return QOpenGLWidget::event(event);
}

void OsgWidget::setKeyboardModifiers(QInputEvent* event)
{
    int modkey = event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier);

    unsigned int mask = 0;

    if (modkey & Qt::ShiftModifier  ) mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    if (modkey & Qt::ControlModifier) mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    if (modkey & Qt::AltModifier    ) mask |= osgGA::GUIEventAdapter::MODKEY_ALT;

    osgEventQueue()->getCurrentEventState()->setModKeyMask(mask);
}

osgGA::EventQueue* OsgWidget::osgEventQueue() const
{
    return m_osgGraphicsContext->getEventQueue();
}

void OsgWidget::paintEvent(QPaintEvent* event)
{
    QOpenGLWidget::paintEvent(event);
}

void OsgWidget::paintGL()
{
    if (m_firstFrame)
    {
        m_firstFrame = false;
        osgContext()->setDefaultFboId(defaultFramebufferObject());
    }

    m_osgViewer->frame();
}

void OsgWidget::resizeGL(int width, int height)
{
    osgEventQueue()->windowResize(x(), y(), width, height);
    m_osgGraphicsContext->resized(x(), y(), width, height);
}

void OsgWidget::keyPressEvent(QKeyEvent* event)
{
    setKeyboardModifiers(event);
    osgEventQueue()->keyPress(remapKey(event));

    QOpenGLWidget::keyPressEvent(event);
}

void OsgWidget::keyReleaseEvent( QKeyEvent* event )
{
    setKeyboardModifiers(event);
    osgEventQueue()->keyRelease(remapKey(event));

    QOpenGLWidget::keyReleaseEvent(event);
}

void OsgWidget::mousePressEvent(QMouseEvent* event)
{
    int button = 0;

    switch (event->button())
    {
        case Qt::LeftButton : button = 1; break;
        case Qt::MidButton  : button = 2; break;
        case Qt::RightButton: button = 3; break;

        default:
        case Qt::NoButton: button = 0; break;
    }

    setKeyboardModifiers(event);
    osgEventQueue()->mouseButtonPress(event->x(), event->y(), button);
}

void OsgWidget::mouseReleaseEvent( QMouseEvent* event )
{
    int button = 0;

    switch ( event->button() )
    {
        case Qt::LeftButton : button = 1; break;
        case Qt::MidButton  : button = 2; break;
        case Qt::RightButton: button = 3; break;

        default:
        case Qt::NoButton: button = 0; break;
    }

    setKeyboardModifiers(event);
    osgEventQueue()->mouseButtonRelease(event->x(), event->y(), button);
}

void OsgWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    int button = 0;

    switch (event->button())
    {
        case Qt::LeftButton : button = 1; break;
        case Qt::MidButton  : button = 2; break;
        case Qt::RightButton: button = 3; break;

        default:
        case Qt::NoButton: button = 0; break;
    }

    setKeyboardModifiers(event);
    osgEventQueue()->mouseDoubleButtonPress(event->x(), event->y(), button);
}

void OsgWidget::mouseMoveEvent(QMouseEvent* event)
{
    setKeyboardModifiers(event);
    osgEventQueue()->mouseMotion(event->x(), event->y());
}

void OsgWidget::wheelEvent(QWheelEvent* event)
{
    setKeyboardModifiers(event);

    switch (event->orientation())
    {
        case Qt::Horizontal:
            osgEventQueue()->mouseScroll(event->delta() > 0 ? osgGA::GUIEventAdapter::SCROLL_LEFT
                                                            : osgGA::GUIEventAdapter::SCROLL_RIGHT);
        break;

        case Qt::Vertical:
            osgEventQueue()->mouseScroll(event->delta() > 0 ? osgGA::GUIEventAdapter::SCROLL_UP
                                                            : osgGA::GUIEventAdapter::SCROLL_DOWN);
        break;
    }
}

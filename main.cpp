#include "widget.h"

#include <QApplication>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.show();

#if 0
    QQuickView view(QUrl::fromLocalFile("test.qml"));
    view.setTitle("Qt Quick");
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
#endif

    return a.exec();
}

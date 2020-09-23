#include "widget.h"
#include <QApplication>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Widget w;
    w.setWindowTitle("OpenSceneGraph");
    w.show();

    QQuickView view(QUrl::fromLocalFile("test.qml"));
    view.setTitle("Qt Quick");
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();

    return a.exec();
}

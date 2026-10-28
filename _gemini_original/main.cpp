#include <QGuiApplication>
#include <QSurfaceFormat>
#include <Qt3DQuickExtras/qt3dquickwindow.h>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    Qt3DExtras::Quick::Qt3DQuickWindow view;
    view.setTitle(
        "Arquitetura da Masmorra | [1-6] Camadas | [T] Modo Tocha Shadowdark | [G] Flechas de Barro (Apagar Luz Goblin) | [P] Puzzle de Cristais"
    );
    view.resize(1600, 950);
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
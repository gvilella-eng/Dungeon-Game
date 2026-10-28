// A Chama Primordial - explorador 3D da masmorra (Qt 5.15 ou Qt 6 + OpenGL 3.3 core).
#include "app/CommandLine.h"
#include "app/GameWindow.h"
#include "app/Validator.h"
#include "app/SelfTest.h"
#include "content/Dungeon.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QElapsedTimer>
#include <QDir>
#ifdef _WIN32
#include <windows.h>
extern "C" {
__declspec(dllexport) DWORD NvOptimusEnablement = 1;                    // usa a GPU NVIDIA
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;     // usa a GPU AMD
}
#endif

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    {
        // --bench precisa de swap interval 0 (medir fps sem vsync)
        bool bench = false;
        for (int i = 1; i < argc; ++i) if (QByteArray(argv[i]) == "--bench") bench = true;
        fmt.setSwapInterval(bench ? 0 : 1);
    }
    QSurfaceFormat::setDefaultFormat(fmt);
    QGuiApplication app(argc, argv);

    Options opt = parseCommandLine(app.arguments());
    if (opt.help) { printUsage(); return 0; }

    World world;
    world.testLevelOnly = opt.testLevel;
    QElapsedTimer t; t.start();
    if (opt.stress) buildStressLevel(world); else buildWorld(world, opt.testLevel);
    outLine(QStringLiteral("Conteúdo construído em %1 ms: %2 primitivas SDF, %3 pontos de vista, %4 POIs")
                .arg(t.elapsed()).arg(world.sdf.primCount()).arg(world.viewpoints().size()).arg(world.pois().size()));

    if (opt.selftest) return runSelfTest(world);
    if (opt.validate) return runValidation(world, QCoreApplication::applicationDirPath() + "/validate_report.txt");

    GameWindow win(world, opt);
    if (opt.shots) { win.resize(opt.width > 0 ? std::min(opt.width, 960) : 800, opt.height > 0 ? std::min(opt.height, 540) : 450); win.show(); }
    else if (opt.width > 0 && opt.height > 0) { win.resize(opt.width, opt.height); win.show(); }
    else win.showMaximized();
    return app.exec();
}

#include "CommandLine.h"
#include <cstdio>

void outLine(const QString& s)
{
    QByteArray b = s.toUtf8();
    b.append('\n');
    fwrite(b.constData(), 1, (size_t)b.size(), stdout);
    fflush(stdout);
}

void printUsage()
{
    outLine("Masmorra - uso:");
    outLine("  --shots <dir> [--filter <texto>] [--shots-mode explore|dm|both]   salva capturas dos pontos de vista e sai");
    outLine("  --validate                      verifica a rota (conectividade) e pontos de vista/POIs dentro da rocha");
    outLine("  --start <texto>                 comeca no ponto de vista cujo nome contem o texto");
    outLine("  --test-level                    constroi e comeca na Sala de Testes");
    outLine("  --no-cache                      ignora o cache de malha");
    outLine("  --stress                        carrega geometria sintetica em escala real (medicao)");
    outLine("  --bench <s>                     roda s segundos sem vsync e imprime o fps medio");
    outLine("  --size WxH                      tamanho da janela (e das capturas)");
}

Options parseCommandLine(const QStringList& a)
{
    Options o;
    for (int i = 1; i < a.size(); ++i) {
        const QString& s = a[i];
        auto next = [&]() { return i + 1 < a.size() ? a[++i] : QString(); };
        if (s == "--shots") { o.shots = true; o.shotsDir = next(); }
        else if (s == "--filter") o.filter = next();
        else if (s == "--shots-mode") o.shotsMode = next().toLower();
        else if (s == "--validate") o.validate = true;
        else if (s == "--start") o.start = next();
        else if (s == "--test-level") o.testLevel = true;
        else if (s == "--no-cache") o.noCache = true;
        else if (s == "--stress") o.stress = true;
        else if (s == "--selftest") o.selftest = true;
        else if (s == "--bench") o.bench = next().toFloat();
        else if (s == "--size") {
            QStringList p = next().toLower().split('x');
            if (p.size() == 2) { o.width = p[0].toInt(); o.height = p[1].toInt(); }
        } else if (s == "--help" || s == "-h") o.help = true;
    }
    return o;
}

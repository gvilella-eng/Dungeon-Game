// Opcoes de linha de comando.
#pragma once
#include <QString>
#include <QStringList>

struct Options {
    bool shots = false;
    QString shotsDir;
    QString filter;             // substring (sem diferenciar maiusculas) do nome do ponto de vista
    QString shotsMode = "both"; // explore | dm | both
    bool validate = false;
    QString start;              // ponto de vista inicial (substring)
    bool testLevel = false;     // constroi/spawna so na Sala de Testes
    bool noCache = false;
    int width = 0, height = 0;  // --size WxH (janela; nos shots = resolucao da imagem)
    bool help = false;
    bool selftest = false;
    bool stress = false;        // geometria sintetica em escala real (medir malhagem)
    float bench = 0.f;          // --bench <s>: roda s segundos sem vsync e imprime fps
    QString shotsExtra;
};

Options parseCommandLine(const QStringList& args);
void printUsage();
void outLine(const QString& s);   // stdout em UTF-8

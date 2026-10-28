// Mesher em chunks (Surface Nets) sobre o SdfWorld, multithread, com cache em disco.
#pragma once
#include "Sdf.h"
#include "render/MeshBuilder.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <QString>

struct ChunkMesh {
    int cx = 0, cy = 0, cz = 0;
    bool exterior = false;      // intersecta a caixa do terreno exterior (F9 esconde)
    MeshData mesh;
};

class Mesher {
public:
    static constexpr int kVersion = 3;     // mude ao alterar o algoritmo (invalida caches)

    struct Config {
        float voxel = 0.4f;
        int cells = 32;
        bool useCache = true;
        QString cacheDir;                  // normalmente <exe>/cache
        int threads = 0;                   // 0 = hardware_concurrency - 1
    };

    Mesher(const SdfWorld& world, const Config& cfg);
    ~Mesher();
    void start();                                   // inicia threads (ou leitura do cache)
    // Move os chunks prontos para 'out'; retorna quantos.
    int poll(std::vector<ChunkMesh>& out);
    float progress() const;                         // 0..1
    bool finished() const;                          // tudo meshado E retirado via poll
    bool fromCache() const { return m_fromCache; }
    int totalChunks() const { return (int)m_total; }
    size_t vertexCount() const { return m_verts; }
    double elapsedSeconds() const { return m_elapsed; }
    void waitAll();

    // Meshing sincrono de um unico chunk (testes).
    bool meshChunk(int cx, int cy, int cz, ChunkMesh& out) const;
    Vec3 chunkOrigin(int cx, int cy, int cz) const;
    float chunkSize() const { return m_cfg.voxel * m_cfg.cells; }

private:
    struct Task { int cx, cy, cz; };
    void workerLoop();
    void loaderLoop(const QString& path);
    QString cachePath() const;

    const SdfWorld& m_w;
    Config m_cfg;
    std::vector<Task> m_tasks;
    std::atomic<size_t> m_next{0};
    std::atomic<size_t> m_done{0};
    std::atomic<size_t> m_polled{0};
    std::atomic<size_t> m_written{0};
    size_t m_total = 0;
    std::vector<std::thread> m_threads;
    std::mutex m_mx;
    std::vector<ChunkMesh> m_results;
    bool m_fromCache = false;
    std::atomic<size_t> m_verts{0};
    double m_elapsed = 0.0;
    qint64 m_t0 = 0;
    void* m_cacheFile = nullptr;     // QFile* (escrita)
    std::mutex m_fileMx;
    std::atomic<bool> m_failed{false};
    std::atomic<int> m_alive{0};
    uint64_t m_hash = 0;
};

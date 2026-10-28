#include "World.h"
#include "game/Player.h"
#include "render/GpuMesh.h"
#include <unordered_map>
#include <algorithm>

// ------------------------------------------------------------ Gameplay.h
float Light::intensityAt(float t) const
{
    switch (anim) {
    case LightAnim::None: return intensity;
    case LightAnim::TorchFlicker: {
        float f = 0.5f * std::sin(t * 11.3f + phase) + 0.3f * std::sin(t * 17.9f + 2.1f * phase + 1.f) + 0.2f * std::sin(t * 29.f + 1.3f);
        return intensity * (1.f - animDepth * 0.5f + animDepth * 0.5f * f);
    }
    case LightAnim::Pulse: return intensity * (1.f - animDepth * (0.5f + 0.5f * std::sin(6.2831853f * animFreq * t + phase)));
    case LightAnim::Custom: return custom ? intensity * custom(t) : intensity;
    }
    return intensity;
}

DynamicObject::DynamicObject() = default;
DynamicObject::~DynamicObject() = default;
StaticBatch::StaticBatch() = default;
StaticBatch::~StaticBatch() = default;

void Collider::refresh()
{
    QMatrix4x4 m;
    if (follow) { m.translate(follow->pos); m.rotate(follow->rot); }
    m.translate(center); m.rotate(rot);
    inv = m.inverted();
    Aabb lb;
    switch (type) {
    case Box: lb = Aabb(-half, half); break;
    case Cylinder: lb = Aabb(Vec3(-radius, -halfHeight, -radius), Vec3(radius, halfHeight, radius)); break;
    case Capsule: lb = Aabb(Vec3(-radius, -halfHeight - radius, -radius), Vec3(radius, halfHeight + radius, radius)); break;
    case Sphere: lb = Aabb(Vec3(-radius, -radius, -radius), Vec3(radius, radius, radius)); break;
    }
    bounds = transformAabb(m, lb);
}

float Collider::distance(const Vec3& p) const
{
    Vec3 l = inv.map(p);
    switch (type) {
    case Box: {
        Vec3 q(std::fabs(l.x()) - half.x(), std::fabs(l.y()) - half.y(), std::fabs(l.z()) - half.z());
        Vec3 o(std::max(q.x(), 0.f), std::max(q.y(), 0.f), std::max(q.z(), 0.f));
        return o.length() + std::min(std::max(q.x(), std::max(q.y(), q.z())), 0.f);
    }
    case Cylinder: {
        float qx = std::sqrt(l.x() * l.x() + l.z() * l.z()) - radius, qy = std::fabs(l.y()) - halfHeight;
        float ox = std::max(qx, 0.f), oy = std::max(qy, 0.f);
        return std::sqrt(ox * ox + oy * oy) + std::min(std::max(qx, qy), 0.f);
    }
    case Capsule: {
        float y = clampf(l.y(), -halfHeight, halfHeight);
        return (l - Vec3(0, y, 0)).length() - radius;
    }
    case Sphere: return l.length() - radius;
    }
    return 1e9f;
}

bool ZoneShape::contains(const Vec3& p) const
{
    switch (type) {
    case BoxShape: return box.contains(p);
    case SphereShape: return (p - center).lengthSquared() <= radius * radius;
    case CylinderShape: {
        float dx = p.x() - center.x(), dz = p.z() - center.z();
        return dx * dx + dz * dz <= radius * radius && p.y() >= y0 && p.y() <= y1;
    }
    }
    return false;
}
Aabb ZoneShape::bounds() const
{
    switch (type) {
    case BoxShape: return box;
    case SphereShape: return Aabb(center - Vec3(radius, radius, radius), center + Vec3(radius, radius, radius));
    case CylinderShape: return Aabb(Vec3(center.x() - radius, y0, center.z() - radius), Vec3(center.x() + radius, y1, center.z() + radius));
    }
    return Aabb();
}
float ZoneShape::topY() const { return bounds().mx.y(); }

QString poiCategoryName(PoiCategory c)
{
    switch (c) {
    case PoiCategory::Area: return QStringLiteral("Área");
    case PoiCategory::Trap: return QStringLiteral("Armadilha");
    case PoiCategory::Puzzle: return QStringLiteral("Enigma");
    case PoiCategory::Secret: return QStringLiteral("Segredo");
    case PoiCategory::Creature: return QStringLiteral("Criatura");
    case PoiCategory::Treasure: return QStringLiteral("Tesouro");
    case PoiCategory::Lore: return QStringLiteral("Lore");
    case PoiCategory::Rest: return QStringLiteral("Descanso");
    }
    return QString();
}
QColor poiCategoryColor(PoiCategory c)
{
    switch (c) {
    case PoiCategory::Area: return QColor(100, 175, 255);
    case PoiCategory::Trap: return QColor(255, 90, 80);
    case PoiCategory::Puzzle: return QColor(255, 205, 60);
    case PoiCategory::Secret: return QColor(195, 125, 255);
    case PoiCategory::Creature: return QColor(255, 145, 60);
    case PoiCategory::Treasure: return QColor(255, 230, 100);
    case PoiCategory::Lore: return QColor(130, 225, 175);
    case PoiCategory::Rest: return QColor(115, 235, 115);
    }
    return Qt::white;
}

// ------------------------------------------------------------ World
World::World() { m_scatterRng = 2463534242u; }
World::~World() = default;

void World::carve(const Prim& p) { Prim q = p; q.carve = true; sdf.add(q); }
void World::fill(const Prim& p) { Prim q = p; q.carve = false; sdf.add(q); }

void World::carveTunnel(const std::vector<Vec3>& pts, const std::vector<float>& radii, const TunnelOpts& o)
{
    if (pts.size() < 2 || radii.empty()) return;
    auto rad = [&](size_t i) { return radii.size() == 1 ? radii[0] : radii[std::min(i, radii.size() - 1)]; };
    for (size_t i = 0; i + 1 < pts.size(); ++i) {
        Prim p = Prim::capsule(pts[i], pts[i + 1], rad(i), rad(i + 1));
        p.smooth(o.k).noise(o.noise).mat(o.material).named(o.tag);
        if (o.floorFrac < 0.99f) p.floorRel(o.floorFrac);
        carve(p);
    }
}

void World::carveChamber(const Vec3& c, const Vec3& r, float floorY, int material, float noise, float k, const QString& tag)
{
    Prim p = Prim::ellipsoid(c, r);
    p.smooth(k).noise(noise).mat(material).floorAt(floorY).named(tag);
    carve(p);
}

static float rndf(uint32_t& s) { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s & 0xFFFFFF) / 16777216.f; }

void World::stalactite(const Vec3& cp, float len, float baseR, int material)
{
    Prim p = Prim::cone(cp + Vec3(0, -len, 0), cp + Vec3(0, 0.5f, 0), baseR, 0.04f);
    p.smooth(0.5f).noise(0.08f).mat(material).asFill();
    sdf.add(p);
}
void World::stalagmite(const Vec3& fp, float len, float baseR, int material)
{
    Prim p = Prim::cone(fp + Vec3(0, len, 0), fp + Vec3(0, -0.5f, 0), baseR, 0.05f);
    p.smooth(0.5f).noise(0.08f).mat(material).asFill();
    sdf.add(p);
}

int World::scatterStalactites(const Aabb& r, int count, int seed, float minLen, float maxLen, float minR, float maxR, int material)
{
    uint32_t s = 2463534242u ^ (uint32_t)(seed * 2654435761u); if (!s) s = 1;
    int made = 0;
    for (int tries = 0; tries < count * 8 && made < count; ++tries) {
        float x = lerpf(r.mn.x(), r.mx.x(), rndf(s)), z = lerpf(r.mn.z(), r.mx.z(), rndf(s));
        float len = lerpf(minLen, maxLen, rndf(s)), br = lerpf(minR, maxR, rndf(s));
        Vec3 o(x, r.mn.y(), z);
        if (sdf.distance(o) < 0.3f) continue;
        // sobe ate o teto
        float t = 0.f, maxT = r.size().y();
        bool hit = false;
        while (t < maxT) { float d = sdf.distance(o + Vec3(0, t, 0)); if (d < 0.05f) { hit = true; break; } t += std::max(d * 0.8f, 0.04f); }
        if (!hit) continue;
        Vec3 cp = o + Vec3(0, t, 0);
        if (sdf.gradient(cp - Vec3(0, 0.1f, 0)).y() > -0.6f) continue;
        stalactite(cp, len, br, material);
        ++made;
    }
    return made;
}

int World::scatterStalagmites(const Aabb& r, int count, int seed, float minLen, float maxLen, float minR, float maxR, int material)
{
    uint32_t s = 88172645u ^ (uint32_t)(seed * 2654435761u); if (!s) s = 1;
    int made = 0;
    for (int tries = 0; tries < count * 8 && made < count; ++tries) {
        float x = lerpf(r.mn.x(), r.mx.x(), rndf(s)), z = lerpf(r.mn.z(), r.mx.z(), rndf(s));
        float len = lerpf(minLen, maxLen, rndf(s)), br = lerpf(minR, maxR, rndf(s));
        Vec3 o(x, r.mx.y(), z);
        if (sdf.distance(o) < 0.3f) continue;
        float t = 0.f, maxT = r.size().y();
        bool hit = false;
        while (t < maxT) { float d = sdf.distance(o - Vec3(0, t, 0)); if (d < 0.05f) { hit = true; break; } t += std::max(d * 0.8f, 0.04f); }
        if (!hit) continue;
        Vec3 fp = o - Vec3(0, t, 0);
        if (sdf.gradient(fp + Vec3(0, 0.1f, 0)).y() < 0.7f) continue;
        stalagmite(fp, len, br, material);
        ++made;
    }
    return made;
}

// ---- props ----
static void splitInto(const MeshData& src, std::map<std::tuple<int, int, int>, StaticBatch>& out, bool transparent)
{
    const float B = 24.f;
    std::map<std::tuple<int, int, int>, std::vector<uint32_t>> groups;
    for (size_t t = 0; t + 2 < src.i.size(); t += 3) {
        const Vertex &a = src.v[src.i[t]], &b = src.v[src.i[t + 1]], &c = src.v[src.i[t + 2]];
        float cx = (a.px + b.px + c.px) / 3, cy = (a.py + b.py + c.py) / 3, cz = (a.pz + b.pz + c.pz) / 3;
        groups[{(int)std::floor(cx / B), (int)std::floor(cy / B), (int)std::floor(cz / B)}].push_back((uint32_t)t);
    }
    for (auto& g : groups) {
        StaticBatch& sb = out[g.first];
        MeshData& md = transparent ? sb.transparent : sb.opaque;
        std::unordered_map<uint32_t, uint32_t> remap;
        for (uint32_t t : g.second)
            for (int k = 0; k < 3; ++k) {
                uint32_t oi = src.i[t + k];
                auto it = remap.find(oi);
                if (it == remap.end()) {
                    it = remap.emplace(oi, (uint32_t)md.v.size()).first;
                    md.v.push_back(src.v[oi]);
                    md.bounds.expand(Vec3(src.v[oi].px, src.v[oi].py, src.v[oi].pz));
                }
                md.i.push_back(it->second);
            }
        sb.bounds.expand(md.bounds);
        sb.dirty = true;
    }
}

void World::addStatic(MeshBuilder&& mb)
{
    splitInto(mb.opaque(), m_static, false);
    splitInto(mb.transparent(), m_static, true);
}

DynHandle World::addDynamic(const QString& name, MeshBuilder&& mb, const Vec3& pos, const QQuaternion& rot, const Vec3& scale)
{
    auto d = std::make_unique<DynamicObject>();
    d->name = name; d->opaque = std::move(mb.opaque()); d->transparent = std::move(mb.transparent());
    d->localBounds = d->opaque.bounds; d->localBounds.expand(d->transparent.bounds);
    d->pos = pos; d->rot = rot; d->scl = scale;
    m_dyn.push_back(std::move(d));
    return m_dyn.back().get();
}
DynHandle World::findDynamic(const QString& name) const
{
    for (auto& d : m_dyn) if (d->name == name) return d.get();
    return nullptr;
}

// ---- colisores ----
Collider* World::addBoxCollider(const QString& name, const Vec3& c, const Vec3& half, const QQuaternion& rot, bool openable)
{
    auto k = std::make_unique<Collider>(); k->name = name; k->type = Collider::Box; k->center = c; k->half = half; k->rot = rot; k->openable = openable;
    k->refresh(); m_colliders.push_back(std::move(k)); return m_colliders.back().get();
}
Collider* World::addCylinderCollider(const QString& name, const Vec3& c, float r, float hh, bool openable)
{
    auto k = std::make_unique<Collider>(); k->name = name; k->type = Collider::Cylinder; k->center = c; k->radius = r; k->halfHeight = hh; k->openable = openable;
    k->refresh(); m_colliders.push_back(std::move(k)); return m_colliders.back().get();
}
Collider* World::addCapsuleCollider(const QString& name, const Vec3& c, float r, float hs, const QQuaternion& rot)
{
    auto k = std::make_unique<Collider>(); k->name = name; k->type = Collider::Capsule; k->center = c; k->radius = r; k->halfHeight = hs; k->rot = rot;
    k->refresh(); m_colliders.push_back(std::move(k)); return m_colliders.back().get();
}
Collider* World::addSphereCollider(const QString& name, const Vec3& c, float r)
{
    auto k = std::make_unique<Collider>(); k->name = name; k->type = Collider::Sphere; k->center = c; k->radius = r;
    k->refresh(); m_colliders.push_back(std::move(k)); return m_colliders.back().get();
}
void World::attachCollider(Collider* c, DynHandle obj) { c->follow = obj; c->refresh(); }
Collider* World::findCollider(const QString& name) const
{
    for (auto& c : m_colliders) if (c->name == name) return c.get();
    return nullptr;
}
void World::refreshColliders() { for (auto& c : m_colliders) if (c->follow) c->refresh(); }

// ---- luzes etc ----
Light* World::addLight(const Vec3& pos, const Vec3& color, float intensity, float range, LightAnim anim, const QString& name)
{
    auto l = std::make_unique<Light>(); l->pos = pos; l->color = color; l->intensity = intensity; l->range = range; l->anim = anim; l->name = name;
    l->phase = (float)(m_lights.size() * 1.713);
    if (anim == LightAnim::TorchFlicker) l->animDepth = 0.35f;
    m_lights.push_back(std::move(l)); return m_lights.back().get();
}
EmitterHandle World::addEmitter(ParticlePreset preset, const Vec3& pos, const Vec3& halfExtent, float rate, const QString& name)
{
    EmitterDesc d; d.preset = preset; d.pos = pos; d.name = name;
    ParticleEmitter::applyPreset(d);
    if (halfExtent.x() >= 0.f) d.halfExtent = halfExtent;
    if (rate >= 0.f) d.rate = rate;
    return addEmitter(d);
}
EmitterHandle World::addEmitter(const EmitterDesc& d) { m_emitters.push_back(std::make_unique<ParticleEmitter>(d)); return m_emitters.back().get(); }
Beam* World::addBeam(const QString& name, const Vec3& a, const Vec3& b, const QColor& c, float w)
{
    auto bm = std::make_unique<Beam>(); bm->name = name; bm->a = a; bm->b = b; bm->color = c; bm->width = w;
    m_beams.push_back(std::move(bm)); return m_beams.back().get();
}

// ---- zonas etc ----
Zone* World::addZone(const QString& name, const ZoneShape& shape, const ZoneEffects& fx, int priority)
{
    auto z = std::make_unique<Zone>(); z->name = name; z->shape = shape; z->fx = fx; z->priority = priority;
    m_zones.push_back(std::move(z)); return m_zones.back().get();
}
Zone* World::findZone(const QString& name) const { for (auto& z : m_zones) if (z->name == name) return z.get(); return nullptr; }
Trigger* World::addTrigger(const QString& name, const ZoneShape& shape, std::function<void()> onEnter, std::function<void()> onExit, bool once)
{
    auto t = std::make_unique<Trigger>(); t->name = name; t->shape = shape; t->onEnter = std::move(onEnter); t->onExit = std::move(onExit); t->once = once;
    m_triggers.push_back(std::move(t)); return m_triggers.back().get();
}
Interactable* World::addInteractable(const QString& name, const Vec3& pos, float radius, const QString& prompt, std::function<void()> cb)
{
    auto i = std::make_unique<Interactable>(); i->name = name; i->pos = pos; i->radius = radius; i->prompt = prompt; i->callback = std::move(cb);
    m_inter.push_back(std::move(i)); return m_inter.back().get();
}
Poi* World::addPoi(const Vec3& pos, PoiCategory cat, const QString& title, const QString& text, const QString& session, const QString& area)
{
    auto p = std::make_unique<Poi>(); p->id = m_nextPoi++; p->pos = pos; p->category = cat; p->title = title; p->text = text; p->session = session; p->area = area;
    m_pois.push_back(std::move(p)); return m_pois.back().get();
}
void World::addViewpoint(const QString& name, const Vec3& pos, float yaw, float pitch, bool fly, const QString& group)
{
    Viewpoint v; v.name = name; v.pos = pos; v.yawDeg = yaw; v.pitchDeg = pitch; v.fly = fly; v.group = group;
    m_views.push_back(v);
}
void World::registerEvent(const QString& id, const QString& label, int hotkey, std::function<void()> fn)
{
    m_events.push_back({id, label, hotkey, std::move(fn)});
}
void World::fireEvent(const QString& id)
{
    for (auto& e : m_events) if (e.id == id && e.fn) { e.fn(); return; }
}

// ---- consultas ----
float World::solidDistance(const Vec3& p, bool includeOpenable) const
{
    float d = sdf.distance(p);
    for (auto& c : m_colliders) {
        if (!c->enabled || (!includeOpenable && c->openable)) continue;
        const Aabb& b = c->bounds;
        float dx = std::max(std::max(b.mn.x() - p.x(), p.x() - b.mx.x()), 0.f);
        float dy = std::max(std::max(b.mn.y() - p.y(), p.y() - b.mx.y()), 0.f);
        float dz = std::max(std::max(b.mn.z() - p.z(), p.z() - b.mx.z()), 0.f);
        if (dx * dx + dy * dy + dz * dz >= d * d && d > 0.f) continue;
        d = std::min(d, c->distance(p));
    }
    return d;
}

Vec3 World::solidGradient(const Vec3& p, float e, bool incl) const
{
    static const float sx[4] = {1, -1, -1, 1}, sy[4] = {-1, -1, 1, 1}, sz[4] = {-1, 1, -1, 1};
    Vec3 g;
    for (int i = 0; i < 4; ++i) g += Vec3(sx[i], sy[i], sz[i]) * solidDistance(p + Vec3(sx[i], sy[i], sz[i]) * e, incl);
    float l = g.length();
    return l > 1e-9f ? g / l : Vec3(0, 1, 0);
}

RayHit World::raycast(const Vec3& o, const Vec3& dir, float maxDist, bool incl) const
{
    RayHit r;
    Vec3 d = dir.normalized();
    float t = 0.f;
    for (int i = 0; i < 400 && t <= maxDist; ++i) {
        Vec3 p = o + d * t;
        float s = solidDistance(p, incl);
        if (s < 0.015f) {
            r.hit = true; r.t = t; r.pos = p; r.normal = solidGradient(p - d * 0.03f, 0.05f, incl);
            float best = 0.06f;
            for (auto& c : m_colliders) if (c->enabled && (incl || !c->openable)) { float cd = c->distance(p); if (cd < best) { best = cd; r.collider = c->name; } }
            return r;
        }
        t += std::max(s * 0.85f, 0.015f);
    }
    return r;
}

bool World::lineOfSight(const Vec3& a, const Vec3& b) const
{
    Vec3 d = b - a; float l = d.length();
    if (l < 0.05f) return true;
    RayHit h = raycast(a, d / l, l - 0.1f);
    return !h.hit;
}

bool World::isVisibleToPlayer(const Vec3& p, float maxDist) const
{
    if (!cam.valid) return false;
    if ((p - cam.pos).length() > maxDist) return false;
    if (!cam.frustum.intersects(Aabb(p - Vec3(0.3f, 0.3f, 0.3f), p + Vec3(0.3f, 0.3f, 0.3f)))) return false;
    return lineOfSight(cam.pos, p);
}

Vec3 World::playerPos() const { return m_player ? m_player->pos : Vec3(); }
Vec3 World::playerEye() const { return m_player ? m_player->eyePos() : Vec3(); }
void World::teleportPlayer(const Vec3& f, float yaw, float pitch) { if (m_player) m_player->teleport(f, yaw, pitch); }

QString World::areaAt(const Vec3& p) const
{
    const Zone* best = nullptr;
    for (auto& z : m_zones) if (z->enabled && !z->fx.areaName.isEmpty() && z->shape.contains(p) && (!best || z->priority >= best->priority)) best = z.get();
    return best ? best->fx.areaName : QString();
}

Vec3 World::overviewCenter() const { return m_ovSet ? m_ovCenter : sdf.contentBounds().center(); }
float World::overviewRadius() const { return m_ovSet ? m_ovRadius : std::max(50.f, sdf.contentBounds().size().length() * 0.5f); }

// ---- por quadro ----
static void mergeFx(ZoneEffects& e, const ZoneEffects& z, const Aabb& zb)
{
    if (!z.areaName.isEmpty()) { e.areaName = z.areaName; e.subtitle = z.subtitle; }
    auto M = [](auto& dst, const auto& src) { if (src) dst = src; };
    M(e.temperature, z.temperature); M(e.fogColor, z.fogColor); M(e.fogDensity, z.fogDensity);
    M(e.ambientColor, z.ambientColor); M(e.ambientGround, z.ambientGround); M(e.ambientStrength, z.ambientStrength);
    M(e.gravity, z.gravity); M(e.torchScale, z.torchScale); M(e.hallucination, z.hallucination); M(e.psychic, z.psychic);
    if (z.water) { e.water = true; e.waterSurfaceY = std::isnan(z.waterSurfaceY) ? zb.mx.y() : z.waterSurfaceY; }
    if (z.climb) e.climb = true;
    if (z.wind) { e.wind = true; e.windPeriod = z.windPeriod; e.windDuration = z.windDuration; e.windExtinguishes = z.windExtinguishes; }
    if (z.hazard) { e.hazard = true; e.hazardText = z.hazardText; e.hazardDelay = z.hazardDelay; }
}

void World::updateZones(float dt)
{
    if (!m_player) return;
    Vec3 probe = m_player->bodyCenter();
    ZoneEffects eff = m_defaultFx;
    std::vector<const Zone*> in;
    for (auto& z : m_zones) if (z->enabled && z->shape.contains(probe)) in.push_back(z.get());
    std::stable_sort(in.begin(), in.end(), [](const Zone* a, const Zone* b) { return a->priority < b->priority; });
    for (auto* z : in) mergeFx(eff, z->fx, z->shape.bounds());

    ZoneState tgt;
    ZoneState defs;
    auto V = [](const std::optional<Vec3>& o, const Vec3& d) { return o ? *o : d; };
    auto F = [](const std::optional<float>& o, float d) { return o ? *o : d; };
    tgt.temperature = F(eff.temperature, defs.temperature);
    tgt.fogColor = V(eff.fogColor, defs.fogColor);
    tgt.fogDensity = F(eff.fogDensity, defs.fogDensity);
    tgt.ambientColor = V(eff.ambientColor, defs.ambientColor);
    tgt.ambientGround = V(eff.ambientGround, eff.ambientColor ? *eff.ambientColor * 0.5f : defs.ambientGround);
    tgt.ambientStrength = F(eff.ambientStrength, 1.f);
    tgt.gravity = V(eff.gravity, defs.gravity);
    tgt.torchScale = F(eff.torchScale, 1.f);
    tgt.hallucination = F(eff.hallucination, 0.f);
    tgt.psychic = F(eff.psychic, 0.f);

    float k = m_zoneInit ? 1.f - std::exp(-dt / 0.7f) : 1.f;
    m_zoneInit = true;
    auto L = [&](float& a, float b) { a += (b - a) * k; };
    auto L3 = [&](Vec3& a, const Vec3& b) { a += (b - a) * k; };
    L(m_zs.temperature, tgt.temperature); L3(m_zs.fogColor, tgt.fogColor); L(m_zs.fogDensity, tgt.fogDensity);
    L3(m_zs.ambientColor, tgt.ambientColor); L3(m_zs.ambientGround, tgt.ambientGround); L(m_zs.ambientStrength, tgt.ambientStrength);
    L(m_zs.torchScale, tgt.torchScale); L(m_zs.hallucination, tgt.hallucination); L(m_zs.psychic, tgt.psychic);
    m_zs.gravity = tgt.gravity;
    m_zs.water = eff.water; m_zs.waterSurfaceY = eff.waterSurfaceY;
    m_zs.climb = eff.climb;
    m_zs.hazard = eff.hazard; m_zs.hazardText = eff.hazardText; m_zs.hazardDelay = eff.hazardDelay;
    m_zs.inWindZone = eff.wind;
    bool gust = eff.wind && std::fmod(m_time + eff.windPeriod * 0.5f, eff.windPeriod) < eff.windDuration;
    if (gust && !m_zs.windGust && eff.windExtinguishes && state.torchLit) {
        state.extinguishTorch(QStringLiteral("Uma rajada de vento apagou sua tocha!"));
        state.screenShake(0.02f, 0.5f);
    }
    m_zs.windGust = gust; m_zs.windExtinguishes = eff.windExtinguishes;
    if (!eff.areaName.isEmpty() && eff.areaName != m_lastArea) {
        m_lastArea = eff.areaName;
        state.showBanner(eff.areaName, eff.subtitle);
    }
    if (!eff.areaName.isEmpty()) { m_zs.areaName = eff.areaName; m_zs.subtitle = eff.subtitle; }
}

void World::updateFocus()
{
    m_focusInter = nullptr; m_focusPoi = nullptr;
    if (!m_player || !cam.valid) return;
    Vec3 eye = cam.pos, f = cam.fwd;
    float best = 1e9f;
    for (auto& it : m_inter) {
        if (!it->enabled) continue;
        Vec3 v = it->pos - eye; float dist = v.length();
        if (dist > 3.2f || dist < 0.01f) continue;
        float along = Vec3::dotProduct(v, f);
        if (along < 0.f) continue;
        float perp = (v - f * along).length();
        if (perp > std::max(it->radius, 0.35f)) continue;
        if (!lineOfSight(eye, it->pos)) { RayHit h = raycast(eye, v / dist, dist); if (h.hit && h.t < dist - it->radius - 0.2f) continue; }
        if (perp < best) { best = perp; m_focusInter = it.get(); }
    }
    best = 1e9f;
    for (auto& p : m_pois) {
        Vec3 v = p->pos - eye; float dist = v.length();
        if (dist > 4.5f || dist < 0.01f) continue;
        float along = Vec3::dotProduct(v, f);
        if (along < 0.f) continue;
        float perp = (v - f * along).length();
        if (perp > 1.0f) continue;
        RayHit h = raycast(eye, v / dist, dist);
        if (h.hit && h.t < dist - 0.8f) continue;
        if (perp < best) { best = perp; m_focusPoi = p.get(); }
    }
}

void World::update(float dt)
{
    m_time += dt;
    refreshColliders();
    updateZones(dt);
    if (m_player) {
        Vec3 probe = m_player->bodyCenter();
        for (auto& t : m_triggers) {
            if (!t->enabled) { t->inside = t->shape.contains(probe); continue; }
            bool inside = t->shape.contains(probe);
            if (inside && !t->inside) { if (t->onEnter) t->onEnter(); if (t->once) { t->fired = true; t->enabled = false; } }
            else if (!inside && t->inside) { if (t->onExit) t->onExit(); }
            else if (inside && t->onStay) t->onStay(dt);
            t->inside = inside;
        }
    }
    for (auto& u : m_updaters) u(dt, m_time);
    size_t total = 0;
    for (auto& e : m_emitters) total += e->parts.size();
    size_t budget = total < 20000 ? 20000 - total : 0;
    Vec3 camPos = cam.valid ? cam.pos : playerPos();
    for (auto& e : m_emitters) {
        if (e->d.followPlayer && m_player) e->d.pos = m_player->pos + e->d.followOffset;
        e->update(dt, camPos, budget);
    }
    updateFocus();
}

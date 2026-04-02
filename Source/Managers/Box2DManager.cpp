/**
 * Box2DManager.cpp
 *
 * Implementation of the Box2D v3 physics world manager.
 */

#include "Box2DManager.h"
#include "MOSRotating.h"
#include "MovableObject.h"
#include "AtomGroup.h"
#include "Atom.h"
#include "MovableMan.h"
#include "SceneMan.h"
#include "CameraMan.h"
#include "FrameMan.h"
#include "SLTerrain.h"
#include "TimerMan.h"
#include "TerrainChainBuilder.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace RTE;

void Box2DManager::Clear() {
    m_WorldId = b2_nullWorldId;
    m_TerrainBodyId = b2_nullBodyId;
    m_TerrainChains.clear();
    m_TerrainBuilt = false;
    m_SceneWidthMeters = 0.0f;
    m_BodyMap.clear();
}

void Box2DManager::Initialize() {
    if (b2World_IsValid(m_WorldId)) {
        b2DestroyWorld(m_WorldId);
    }

    b2WorldDef worldDef = b2DefaultWorldDef();
    // Match CC's gravity — GetGlobalAcc() returns acceleration in m/s²
    // CC's Y-axis: positive = down (screen convention)
    // Box2D's Y-axis: positive = up (physics convention)
    // So we negate Y for Box2D
    worldDef.gravity = (b2Vec2){0.0f, -g_SceneMan.GetGlobalAcc().GetY()};
    worldDef.enableSleep = true;
    worldDef.enableContinuous = true;

    m_WorldId = b2CreateWorld(&worldDef);

#ifdef __EMSCRIPTEN__
    EM_ASM({ console.log('[Box2D] World created, gravity=' + $0.toFixed(2) + ' m/s²'); },
           worldDef.gravity.y);
#endif
}

void Box2DManager::Destroy() {
    if (b2World_IsValid(m_WorldId)) {
        b2DestroyWorld(m_WorldId);
    }
    Clear();
}

void Box2DManager::SetSceneWidth(float widthPixels) {
    m_SceneWidthMeters = PixelsToMeters(widthPixels);
}

// ---------------------------------------------------------------------------
// Body management
// ---------------------------------------------------------------------------

b2BodyId Box2DManager::CreateBody(MOSRotating* owner) {
    if (!owner || !b2World_IsValid(m_WorldId)) return b2_nullBodyId;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = ToB2Vec(owner->GetPos().GetX(), -owner->GetPos().GetY()); // Negate Y
    bodyDef.rotation = b2MakeRot(-owner->GetRotAngle()); // Negate for Box2D convention
    bodyDef.linearVelocity = {owner->GetVel().GetX(), -owner->GetVel().GetY()};
    bodyDef.angularVelocity = -owner->GetAngularVel();
    bodyDef.linearDamping = 0.0f;  // CC handles damping in ApplyForces()
    bodyDef.angularDamping = 0.0f;
    bodyDef.gravityScale = 0.0f;   // CC applies gravity in ApplyForces()
    bodyDef.userData = owner;
    bodyDef.enableSleep = true;

    b2BodyId bodyId = b2CreateBody(m_WorldId, &bodyDef);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.enableContactEvents = true;
    bool shapeCreated = false;

    // Try to create a convex hull from AtomGroup atom positions
    AtomGroup* ag = owner->GetAtomGroup();
#ifdef __EMSCRIPTEN__
    {
        static int hullDbg = 0;
        if (++hullDbg <= 10) {
            int atomCount = ag ? (int)ag->GetAtomList().size() : -1;
            EM_ASM({ console.log('[Box2D] CreateBody: atoms=' + $0 + ' radius=' + $1.toFixed(1)); },
                   atomCount, owner->GetIndividualRadius());
        }
    }
#endif
    if (ag) {
        const std::vector<Atom*>& atoms = ag->GetAtomList();
        if (atoms.size() >= 3) {
            // Collect atom offsets as Box2D points.
            // Subsample if too many (b2ComputeHull works best with <=32 points).
            // Also remove near-duplicate points to avoid degenerate hulls.
            std::vector<b2Vec2> points;
            int step = std::max(1, (int)atoms.size() / 24);
            for (size_t i = 0; i < atoms.size(); i += step) {
                Vector offset = atoms[i]->GetOffset();
                b2Vec2 pt = {PixelsToMeters(offset.GetX()),
                             PixelsToMeters(-offset.GetY())};
                // Skip near-duplicates
                bool duplicate = false;
                for (const auto& existing : points) {
                    float dx = pt.x - existing.x;
                    float dy = pt.y - existing.y;
                    if (dx * dx + dy * dy < 0.001f) { duplicate = true; break; }
                }
                if (!duplicate) points.push_back(pt);
            }

            // Need at least 3 unique non-collinear points for a hull.
            // If hull fails (thin objects like rockets), use a bounding box instead.
            b2Hull hull = {0};
            if (points.size() >= 3) {
                hull = b2ComputeHull(points.data(), (int)points.size());
            }

            // Fallback: if hull failed, create a bounding box from min/max extents
            if (hull.count < 3 && points.size() >= 2) {
                float minX = points[0].x, maxX = points[0].x;
                float minY = points[0].y, maxY = points[0].y;
                for (const auto& p : points) {
                    if (p.x < minX) minX = p.x;
                    if (p.x > maxX) maxX = p.x;
                    if (p.y < minY) minY = p.y;
                    if (p.y > maxY) maxY = p.y;
                }
                // Ensure minimum thickness
                float w = maxX - minX;
                float h = maxY - minY;
                if (w < 0.05f) { minX -= 0.025f; maxX += 0.025f; w = 0.05f; }
                if (h < 0.05f) { minY -= 0.025f; maxY += 0.025f; h = 0.05f; }
                float cx = (minX + maxX) * 0.5f;
                float cy = (minY + maxY) * 0.5f;
                b2Polygon box = b2MakeOffsetBox(w * 0.5f, h * 0.5f, (b2Vec2){cx, cy}, b2MakeRot(0));
                shapeDef.density = owner->GetMass() / (w * h);
                b2CreatePolygonShape(bodyId, &shapeDef, &box);
                shapeCreated = true;
#ifdef __EMSCRIPTEN__
                {
                    static int boxDbg = 0;
                    if (++boxDbg <= 10) {
                        EM_ASM({ console.log('[Box2D] BBox fallback: ' + $0.toFixed(2) + 'x' + $1.toFixed(2) + 'm'); },
                               (double)w, (double)h);
                    }
                }
#endif
            }
#ifdef __EMSCRIPTEN__
            {
                static int hullDbg2 = 0;
                if (++hullDbg2 <= 10) {
                    bool valid = hull.count >= 3 && b2ValidateHull(&hull);
                    // Log first few points for debugging
                    if (!valid && points.size() >= 2) {
                        EM_ASM({ console.log('[Box2D] Hull FAILED: pts=' + $0 + ' hullCount=' + $1 +
                                 ' p0=(' + $2.toFixed(3) + ',' + $3.toFixed(3) + ')' +
                                 ' p1=(' + $4.toFixed(3) + ',' + $5.toFixed(3) + ')'); },
                               (int)points.size(), hull.count,
                               (double)points[0].x, (double)points[0].y,
                               (double)points[1].x, (double)points[1].y);
                    } else {
                        EM_ASM({ console.log('[Box2D] Hull OK: pts=' + $0 + ' hullVerts=' + $1); },
                               (int)points.size(), hull.count);
                    }
                }
            }
#endif
            if (hull.count >= 3 && b2ValidateHull(&hull)) {
                b2Polygon poly = b2MakePolygon(&hull, 0.0f);
                // Calculate density from mass and approximate area
                float area = 0.0f;
                for (int i = 0; i < poly.count; i++) {
                    int j = (i + 1) % poly.count;
                    area += poly.vertices[i].x * poly.vertices[j].y;
                    area -= poly.vertices[j].x * poly.vertices[i].y;
                }
                area = fabsf(area) * 0.5f;
                if (area > 0.001f) {
                    shapeDef.density = owner->GetMass() / area;
                }
                b2CreatePolygonShape(bodyId, &shapeDef, &poly);
                shapeCreated = true;
            }
        }
    }

    // Fallback: circle shape if hull failed
    if (!shapeCreated) {
        float radius = PixelsToMeters(owner->GetIndividualRadius() * 0.5f);
        if (radius < 0.05f) radius = 0.05f;
        b2Circle circle = {{0.0f, 0.0f}, radius};
        shapeDef.density = owner->GetMass() / (3.14159f * radius * radius);
        b2CreateCircleShape(bodyId, &shapeDef, &circle);
    }

    m_BodyMap[owner->GetUniqueID()] = bodyId;
    return bodyId;
}

void Box2DManager::DestroyBody(MOSRotating* owner) {
    if (!owner) return;
    auto it = m_BodyMap.find(owner->GetUniqueID());
    if (it != m_BodyMap.end()) {
        if (b2Body_IsValid(it->second)) {
            b2DestroyBody(it->second);
        }
        m_BodyMap.erase(it);
    }
}

b2JointId Box2DManager::CreateWeldJoint(MOSRotating* parent, MOSRotating* child,
                                        const Vector& parentOffset, const Vector& jointOffset,
                                        float stiffness, float breakForce) {
    if (!b2World_IsValid(m_WorldId)) return b2_nullJointId;

    auto parentIt = m_BodyMap.find(parent->GetUniqueID());
    auto childIt = m_BodyMap.find(child->GetUniqueID());
    if (parentIt == m_BodyMap.end() || childIt == m_BodyMap.end()) return b2_nullJointId;
    if (!b2Body_IsValid(parentIt->second) || !b2Body_IsValid(childIt->second)) return b2_nullJointId;

    b2WeldJointDef weldDef = b2DefaultWeldJointDef();
    weldDef.bodyIdA = parentIt->second;
    weldDef.bodyIdB = childIt->second;
    weldDef.localAnchorA = ToB2Vec(parentOffset.GetX(), -parentOffset.GetY());
    weldDef.localAnchorB = ToB2Vec(jointOffset.GetX(), -jointOffset.GetY());

    // Map stiffness to weld joint spring parameters
    if (stiffness < 1.0f) {
        weldDef.linearHertz = 5.0f * stiffness + 0.5f;
        weldDef.angularHertz = 5.0f * stiffness + 0.5f;
        weldDef.linearDampingRatio = 0.7f;
        weldDef.angularDampingRatio = 0.7f;
    }

    b2JointId jointId = b2CreateWeldJoint(m_WorldId, &weldDef);
    return jointId;
}

bool Box2DManager::HasBody(const MOSRotating* owner) const {
    if (!owner) return false;
    return m_BodyMap.count(owner->GetUniqueID()) > 0;
}

// ---------------------------------------------------------------------------
// Per-frame sync and step
// ---------------------------------------------------------------------------

void Box2DManager::PreStep() {
    if (!b2World_IsValid(m_WorldId)) return;

    // NOTE: Terrain chain shapes disabled — the Atom system handles pixel-perfect
    // terrain collision far more accurately than a simplified chain surface.
    // The chain was only the topmost solid pixel per column (missed caves,
    // overhangs, interior terrain). Keeping BuildTerrainChains() for future
    // use but not calling it in the active loop.

    // Clean up bodies whose owners have been removed from the game
    std::vector<long> toRemove;
    for (auto& [uid, bodyId] : m_BodyMap) {
        if (!b2Body_IsValid(bodyId)) {
            toRemove.push_back(uid);
            continue;
        }
        MovableObject* mo = static_cast<MovableObject*>(b2Body_GetUserData(bodyId));
        if (!mo || !g_MovableMan.ValidMO(mo)) {
            b2DestroyBody(bodyId);
            toRemove.push_back(uid);
        }
    }
    for (long uid : toRemove) {
        m_BodyMap.erase(uid);
    }

    SyncToBox2D();
}

void Box2DManager::Step(float deltaTime) {
    if (!b2World_IsValid(m_WorldId)) return;
    if (deltaTime <= 0.0f) return;

    // Cap body count to prevent overload crashes
    b2Counters counters = b2World_GetCounters(m_WorldId);
    if (counters.bodyCount > 200) {
        // Too many bodies — skip step to prevent crash
#ifdef __EMSCRIPTEN__
        static bool warned = false;
        if (!warned) {
            EM_ASM({ console.warn('[Box2D] Too many bodies (' + $0 + '), skipping step'); }, counters.bodyCount);
            warned = true;
        }
#endif
        return;
    }

    // 4 sub-steps for accuracy (Box2D v3 recommendation)
    b2World_Step(m_WorldId, deltaTime, 4);
}

void Box2DManager::PostStep() {
    if (!b2World_IsValid(m_WorldId)) return;

    // Phase 1: just log stats periodically
    static int frameCount = 0;
    if (++frameCount % 300 == 1) {
#ifdef __EMSCRIPTEN__
        b2Counters counters = b2World_GetCounters(m_WorldId);
        EM_ASM({ console.log('[Box2D] bodies=' + $0 + ' contacts=' + $1 + ' shapes=' + $2); },
               counters.bodyCount, counters.contactCount, counters.shapeCount);
#endif
    }

    // Read back Box2D contact events and body positions to CC
    SyncFromBox2D();
}

void Box2DManager::SyncToBox2D() {
    for (auto& [uid, bodyId] : m_BodyMap) {
        if (!b2Body_IsValid(bodyId)) continue;
        MovableObject* mo = static_cast<MovableObject*>(b2Body_GetUserData(bodyId));
        if (!mo) continue;

        // Push CC position/velocity → Box2D body
        b2Vec2 pos = ToB2Vec(mo->GetPos().GetX(), -mo->GetPos().GetY());

        // Handle scene X-wrapping
        if (m_SceneWidthMeters > 0.0f) {
            while (pos.x < 0) pos.x += m_SceneWidthMeters;
            while (pos.x >= m_SceneWidthMeters) pos.x -= m_SceneWidthMeters;
        }

        b2Body_SetTransform(bodyId, pos, b2MakeRot(-mo->GetRotAngle()));
        b2Body_SetLinearVelocity(bodyId, {mo->GetVel().GetX(), -mo->GetVel().GetY()});
        b2Body_SetAngularVelocity(bodyId, -mo->GetAngularVel());
    }
}

// ---------------------------------------------------------------------------
// Terrain chain shapes
// ---------------------------------------------------------------------------

void Box2DManager::BuildTerrainChains() {
    if (!b2World_IsValid(m_WorldId)) return;
    if (!g_SceneMan.GetScene() || !g_SceneMan.GetTerrain()) return;

    // Destroy existing terrain chains
    for (auto& chainId : m_TerrainChains) {
        if (b2Chain_IsValid(chainId)) {
            b2DestroyChain(chainId);
        }
    }
    m_TerrainChains.clear();

    // Create or reuse static terrain body
    if (!b2Body_IsValid(m_TerrainBodyId)) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {0.0f, 0.0f};
        m_TerrainBodyId = b2CreateBody(m_WorldId, &bodyDef);
    }

    // Extract terrain surface contour
    SLTerrain* terrain = g_SceneMan.GetTerrain();
    std::vector<b2Vec2> surface = TerrainChainBuilder::ExtractSurface(terrain, 8);

    if (surface.size() < 4) {
        m_TerrainBuilt = true;
        return;
    }

    // Simplify the contour (reduce vertex count)
    std::vector<b2Vec2> simplified = TerrainChainBuilder::Simplify(surface, 0.15f);

    if (simplified.size() < 4) simplified = surface; // Fallback if over-simplified

    // Create chain shape
    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.points = simplified.data();
    chainDef.count = (int)simplified.size();
    chainDef.isLoop = false;

    b2ChainId chainId = b2CreateChain(m_TerrainBodyId, &chainDef);
    m_TerrainChains.push_back(chainId);

    m_TerrainBuilt = true;
    m_TerrainDebugPoints = simplified; // Cache for debug drawing

#ifdef __EMSCRIPTEN__
    EM_ASM({ console.log('[Box2D] Terrain chain: ' + $0 + ' raw points → ' + $1 + ' simplified'); },
           (int)surface.size(), (int)simplified.size());
#endif
}

void Box2DManager::UpdateDirtyTerrainChains() {
    if (!b2World_IsValid(m_WorldId) || !m_TerrainBuilt) return;
    if (!g_SceneMan.GetScene() || !g_SceneMan.GetTerrain()) return;

    SLTerrain* terrain = g_SceneMan.GetTerrain();
    std::deque<Box>& dirtyAreas = terrain->GetUpdatedMaterialAreas();

    if (dirtyAreas.empty()) return;

    // For now, rebuild the entire terrain if any dirty regions exist.
    // Future optimization: only rebuild chain segments near dirty regions.
    static int dirtyFrameCount = 0;
    dirtyFrameCount++;

    // Batch updates: only rebuild every 30 frames to avoid per-frame rebuilds
    if (dirtyFrameCount % 30 == 0) {
        BuildTerrainChains();
    }
}

// ---------------------------------------------------------------------------
// Debug visualization
// ---------------------------------------------------------------------------

void Box2DManager::DrawDebug() {
    if (!m_DebugDraw || !b2World_IsValid(m_WorldId)) return;

    BITMAP* target = g_FrameMan.GetBackBuffer32();
    if (!target) return;

    Vector cameraOffset = g_CameraMan.GetOffset(0);

    // Helper: draw a line on the 32bpp bitmap
    auto drawLine32 = [&](int x1, int y1, int x2, int y2, uint32_t color) {
        // Bresenham line
        int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
        int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            if (x1 >= 0 && x1 < target->w && y1 >= 0 && y1 < target->h) {
                uint8_t* p = target->line[y1] + x1 * 4;
                p[0] = (color >> 0) & 0xFF;
                p[1] = (color >> 8) & 0xFF;
                p[2] = (color >> 16) & 0xFF;
                p[3] = (color >> 24) & 0xFF;
            }
            if (x1 == x2 && y1 == y2) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    };

    uint32_t green  = 0xC000FF00; // ABGR
    uint32_t yellow = 0xC800FFFF;
    uint32_t red    = 0xC80000FF;
    uint32_t cyan   = 0xA0FFFF00;

    // Draw body outlines (circles) in green
    for (auto& [uid, bodyId] : m_BodyMap) {
        if (!b2Body_IsValid(bodyId)) continue;

        b2Vec2 pos = b2Body_GetPosition(bodyId);
        float sx = MetersToPixels(pos.x) - cameraOffset.GetX();
        float sy = MetersToPixels(-pos.y) - cameraOffset.GetY();

        b2ShapeId shapes[4];
        int shapeCount = b2Body_GetShapes(bodyId, shapes, 4);

        // Use the CC object's rotation for debug draw.
        // Negate because atom offsets were Y-negated when building the hull,
        // which effectively mirrors the shape. Negating the rotation compensates.
        MovableObject* mo = static_cast<MovableObject*>(b2Body_GetUserData(bodyId));
        float ccAngle = mo ? -mo->GetRotAngle() : 0.0f;
        b2Rot rot = b2MakeRot(ccAngle);

        for (int s = 0; s < shapeCount; s++) {
            if (!b2Shape_IsValid(shapes[s])) continue;
            b2ShapeType type = b2Shape_GetType(shapes[s]);

            if (type == b2_circleShape) {
                b2Circle circle = b2Shape_GetCircle(shapes[s]);
                float radius = MetersToPixels(circle.radius);
                int segments = 16;
                for (int i = 0; i < segments; i++) {
                    float a1 = (float)i / segments * 6.2832f;
                    float a2 = (float)(i + 1) / segments * 6.2832f;
                    drawLine32((int)(sx + cosf(a1) * radius), (int)(sy + sinf(a1) * radius),
                               (int)(sx + cosf(a2) * radius), (int)(sy + sinf(a2) * radius), green);
                }
                drawLine32((int)sx, (int)sy, (int)(sx + rot.c * radius), (int)(sy + rot.s * radius), yellow);
            } else if (type == b2_polygonShape) {
                b2Polygon poly = b2Shape_GetPolygon(shapes[s]);
                for (int i = 0; i < poly.count; i++) {
                    int j = (i + 1) % poly.count;
                    // Vertices are in Box2D local space (Y-up).
                    // Negate Y to convert to CC screen space, then rotate by CC angle.
                    float lx1 = MetersToPixels(poly.vertices[i].x);
                    float ly1 = MetersToPixels(-poly.vertices[i].y); // Negate Y for CC
                    float lx2 = MetersToPixels(poly.vertices[j].x);
                    float ly2 = MetersToPixels(-poly.vertices[j].y);
                    // Rotate by CC angle (screen coords, Y-down)
                    float rx1 = lx1 * rot.c - ly1 * rot.s;
                    float ry1 = lx1 * rot.s + ly1 * rot.c;
                    float rx2 = lx2 * rot.c - ly2 * rot.s;
                    float ry2 = lx2 * rot.s + ly2 * rot.c;
                    drawLine32((int)(sx + rx1), (int)(sy + ry1),
                               (int)(sx + rx2), (int)(sy + ry2), green);
                }
                drawLine32((int)sx, (int)sy,
                           (int)(sx + rot.c * 8.0f), (int)(sy + rot.s * 8.0f), yellow);
            }
        }
    }

    // Terrain chain debug drawing disabled — chain shapes not in use
    // (Atom system handles terrain collision pixel-perfectly)

    // Draw joints in cyan
    for (auto& [uid, bodyId] : m_BodyMap) {
        if (!b2Body_IsValid(bodyId)) continue;
        b2JointId joints[8];
        int jointCount = b2Body_GetJoints(bodyId, joints, 8);
        for (int j = 0; j < jointCount; j++) {
            if (!b2Joint_IsValid(joints[j])) continue;
            b2BodyId bodyA = b2Joint_GetBodyA(joints[j]);
            b2BodyId bodyB = b2Joint_GetBodyB(joints[j]);
            if (!b2Body_IsValid(bodyA) || !b2Body_IsValid(bodyB)) continue;
            b2Vec2 posA = b2Body_GetPosition(bodyA);
            b2Vec2 posB = b2Body_GetPosition(bodyB);
            drawLine32((int)(MetersToPixels(posA.x) - cameraOffset.GetX()),
                       (int)(MetersToPixels(-posA.y) - cameraOffset.GetY()),
                       (int)(MetersToPixels(posB.x) - cameraOffset.GetX()),
                       (int)(MetersToPixels(-posB.y) - cameraOffset.GetY()), cyan);
        }
    }
}

void Box2DManager::SyncFromBox2D() {
    // Process contact events — fire CC collision callbacks
    b2ContactEvents events = b2World_GetContactEvents(m_WorldId);

    for (int i = 0; i < events.hitCount; ++i) {
        b2ContactHitEvent* hit = &events.hitEvents[i];

        b2BodyId bodyA = b2Shape_GetBody(hit->shapeIdA);
        b2BodyId bodyB = b2Shape_GetBody(hit->shapeIdB);
        if (!b2Body_IsValid(bodyA) || !b2Body_IsValid(bodyB)) continue;

        MovableObject* moA = static_cast<MovableObject*>(b2Body_GetUserData(bodyA));
        MovableObject* moB = static_cast<MovableObject*>(b2Body_GetUserData(bodyB));
        if (!moA || !moB) continue;

        // Apply collision impulse to both objects
        // hit->normal points from A to B, hit->approachSpeed is the closing speed
        float impulseMag = hit->approachSpeed * 0.5f; // Simplified impulse
        Vector hitNormal(hit->normal.x, -hit->normal.y); // Convert Y back to CC convention
        Vector impulse = hitNormal * impulseMag;

        moA->AddImpulseForce(impulse * -1.0f);
        moB->AddImpulseForce(impulse);
    }

    // Sync body positions/velocities back from Box2D to CC
    b2BodyEvents bodyEvents = b2World_GetBodyEvents(m_WorldId);
    for (int i = 0; i < bodyEvents.moveCount; ++i) {
        b2BodyMoveEvent* event = &bodyEvents.moveEvents[i];
        MovableObject* mo = static_cast<MovableObject*>(event->userData);
        if (!mo || event->fellAsleep) continue;

        // Update CC position from Box2D (convert back to CC Y-down convention)
        b2Vec2 pos = event->transform.p;
        mo->SetPos(Vector(MetersToPixels(pos.x), MetersToPixels(-pos.y)));

        // Update velocity
        b2BodyId bodyId = m_BodyMap[mo->GetUniqueID()];
        if (b2Body_IsValid(bodyId)) {
            b2Vec2 vel = b2Body_GetLinearVelocity(bodyId);
            mo->SetVel(Vector(vel.x, -vel.y));

            float angVel = b2Body_GetAngularVelocity(bodyId);
            // Only update rotation for MOSRotating
            if (MOSRotating* mosr = dynamic_cast<MOSRotating*>(mo)) {
                mosr->SetAngularVel(-angVel);
            }
        }
    }
}

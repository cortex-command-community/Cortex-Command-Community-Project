/**
 * Box2DManager.cpp
 *
 * Implementation of the Box2D v3 physics world manager.
 */

#include "Box2DManager.h"
#include "MOSRotating.h"
#include "SceneMan.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace RTE;

void Box2DManager::Clear() {
    m_WorldId = b2_nullWorldId;
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

    // Create a circle shape approximating the object's collision radius
    float radius = PixelsToMeters(owner->GetIndividualRadius());
    if (radius < 0.1f) radius = 0.1f; // Minimum size

    b2Circle circle = {{0.0f, 0.0f}, radius};
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = owner->GetMass() / (3.14159f * radius * radius);
    shapeDef.enableContactEvents = true;

    b2CreateCircleShape(bodyId, &shapeDef, &circle);

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

bool Box2DManager::HasBody(const MOSRotating* owner) const {
    if (!owner) return false;
    return m_BodyMap.count(owner->GetUniqueID()) > 0;
}

// ---------------------------------------------------------------------------
// Per-frame sync and step
// ---------------------------------------------------------------------------

void Box2DManager::PreStep() {
    if (!b2World_IsValid(m_WorldId)) return;
    SyncToBox2D();
}

void Box2DManager::Step(float deltaTime) {
    if (!b2World_IsValid(m_WorldId)) return;
    if (deltaTime <= 0.0f) return;

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

    // Phase 2+: SyncFromBox2D() will read back positions/velocities
    // SyncFromBox2D();
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

void Box2DManager::SyncFromBox2D() {
    // Phase 2+: Read Box2D contact events and body positions back to CC
    // For now, this is a no-op — Box2D world steps but doesn't affect gameplay
}

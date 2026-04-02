/**
 * Box2DManager.cpp
 *
 * Implementation of the Box2D v3 physics world manager.
 */

#include "Box2DManager.h"
#include "MOSRotating.h"
#include "MovableObject.h"
#include "MovableMan.h"
#include "SceneMan.h"
#include "TimerMan.h"

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

    // Create a circle shape — use half the sprite radius for tighter fit
    float radius = PixelsToMeters(owner->GetIndividualRadius() * 0.5f);
    if (radius < 0.05f) radius = 0.05f; // Minimum size

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

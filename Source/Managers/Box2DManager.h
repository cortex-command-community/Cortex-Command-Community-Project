/**
 * Box2DManager.h
 *
 * Singleton manager for the Box2D v3 physics world.
 * Provides the bridge between Cortex Command's MovableObject system and
 * Box2D's rigid body dynamics.
 *
 * Hybrid approach:
 * - Box2D handles object-to-object collision and joints
 * - The existing Atom/AtomGroup system handles pixel-perfect terrain collision
 * - MOPixel particles stay outside Box2D entirely
 */

#pragma once

#include "Singleton.h"
#include "box2d/box2d.h"
#include <unordered_map>

#define g_Box2DMan Box2DManager::Instance()

namespace RTE {

class Vector;
class MovableObject;
class MOSRotating;

class Box2DManager : public Singleton<Box2DManager> {

public:
    // Pixels per meter conversion (20 pixels = 1 meter)
    static constexpr float PPM = 20.0f;

    static float PixelsToMeters(float px) { return px / PPM; }
    static float MetersToPixels(float m) { return m * PPM; }
    static b2Vec2 ToB2Vec(float pixX, float pixY) { return {pixX / PPM, pixY / PPM}; }

    Box2DManager() { Clear(); }
    ~Box2DManager() { Destroy(); }

    int Create() { return 0; }
    void Initialize();
    void Destroy();
    void Reset() { Clear(); }

    /// Called before MovableMan::Travel() — push CC state to Box2D bodies.
    void PreStep();

    /// Step the Box2D world simulation.
    void Step(float deltaTime);

    /// Called after MovableMan::Travel() — read back Box2D results.
    void PostStep();

    /// Create a Box2D body for a MOSRotating object.
    /// Returns the body ID (stored on the MOSRotating).
    b2BodyId CreateBody(MOSRotating* owner);

    /// Destroy the Box2D body for a MOSRotating object.
    void DestroyBody(MOSRotating* owner);

    /// Check if a MOSRotating has a Box2D body.
    bool HasBody(const MOSRotating* owner) const;

    /// Create a weld joint between parent and child bodies.
    b2JointId CreateWeldJoint(MOSRotating* parent, MOSRotating* child,
                              const Vector& parentOffset, const Vector& jointOffset,
                              float stiffness, float breakForce);

    /// Set scene width for X-wrapping support.
    void SetSceneWidth(float widthPixels);

    /// Get the world ID for direct Box2D API access.
    b2WorldId GetWorldId() const { return m_WorldId; }

    /// Is the Box2D world active?
    bool IsActive() const { return b2World_IsValid(m_WorldId); }

private:
    b2WorldId m_WorldId;
    float m_SceneWidthMeters = 0.0f;

    // Map MO unique IDs → Box2D body IDs
    std::unordered_map<long, b2BodyId> m_BodyMap;

    void Clear();

    // Sync CC object state → Box2D body state
    void SyncToBox2D();

    // Sync Box2D body state → CC object state (Phase 2+)
    void SyncFromBox2D();
};

} // namespace RTE

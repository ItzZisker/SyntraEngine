#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/common.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace syng
{
struct Scene_T;
class Scene;

struct FrustumPlane {
    glm::vec3 normal;
    float distance;

    FrustumPlane();
    FrustumPlane(const glm::vec3& point, const glm::vec3& normalVec);

    float getSignedDistanceToPlane(const glm::vec3& point) const;
};

struct Frustum {
    FrustumPlane topFace, bottomFace, rightFace, leftFace, farFace, nearFace;
};

class AABB {
public:
    glm::vec3 center {0.0f, 0.0f, 0.0f};
    glm::vec3 extents {0.0f, 0.0f, 0.0f};

    AABB(std::vector<glm::vec3> positions);
    AABB(const glm::vec3& min, const glm::vec3& max);
    AABB(const glm::vec3& inCenter, float iI, float iJ, float iK);

    bool isOnOrForwardPlane(const FrustumPlane& plane) const;
};

class Discardable {
public:
    virtual bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) = 0;
};

class FrustumDiscardable : public Discardable {
protected:
    AABB bounding;
public:
    FrustumDiscardable(AABB bounding);
    FrustumDiscardable();
    ~FrustumDiscardable();

    Frustum createFrustum(Scene* scene);
    Frustum createFrustum(Scene_T snapshot);

    bool isInFrustum(const Frustum& frustum, const glm::mat4& transform);
    bool isInView(Scene_T snapshot, const glm::mat4& transform);
    bool isInView(Scene* scene, const glm::mat4& transform);

    bool shouldDiscard(Scene_T snapshot, const glm::mat4& transform) override;
    bool shouldDiscard(Scene* scene, const glm::mat4& transform);

    AABB getBounding();
};

class Coordination2D {
protected:
    glm::mat3 transform = glm::mat3(1.0f);
    glm::vec2 origin = glm::vec2(0.0f);
    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f; // in radians
    glm::vec2 scale = glm::vec2(1.0f);

    void updateTransform() {
        glm::mat3 T = glm::mat3(1.0f);
        T[2] = glm::vec3(position - origin, 1.0f);

        float c = cos(rotation);
        float s = sin(rotation);
        glm::mat3 R = glm::mat3(
            c,  s, 0.0f,
           -s,  c, 0.0f,
            0.0f, 0.0f, 1.0f
        );

        glm::mat3 S = glm::mat3(1.0f);
        S[0][0] = scale.x;
        S[1][1] = scale.y;

        glm::mat3 O = glm::mat3(1.0f);
        O[2] = glm::vec3(-origin, 1.0f);

        this->transform = T * R * S * O;
    }
private:
    void decompose(const glm::mat3& m) {
        this->transform = m;
        this->position = glm::vec2(m[2]);

        glm::vec2 col0 = glm::vec2(m[0]);
        glm::vec2 col1 = glm::vec2(m[1]);

        this->scale.x = glm::length(col0);
        this->scale.y = glm::length(col1);

        if (scale.x != 0) col0 /= scale.x;
        if (scale.y != 0) col1 /= scale.y;

        this->rotation = atan2(col0.y, col0.x);
    }
    void decompose(const glm::mat4& m) {
        glm::mat3 m3;
        m3[0] = glm::vec3(m[0]);
        m3[1] = glm::vec3(m[1]);
        m3[2] = glm::vec3(m[3]);
        decompose(m3);
    }
public:
    Coordination2D(glm::vec2 position = glm::vec2(0.0f), float rotation = 0.0f, glm::vec2 scale = glm::vec2(1.0f)) 
        : position(position), rotation(rotation), scale(scale) {
        updateTransform();
    }

    const glm::mat3& getTransform() const {
        return transform;
    }
    glm::mat4 getTransform4Cpy() const {
        glm::mat4 transform4x4 = glm::mat4(1.0f);
        transform4x4[0] = glm::vec4(transform[0], 0.0f);
        transform4x4[1] = glm::vec4(transform[1], 0.0f);
        transform4x4[2] = glm::vec4(0,0,1,0);
        transform4x4[3] = glm::vec4(transform[2], 1.0f);
        return transform4x4;
    }

    glm::vec2 getPosition() const { return position; }
    glm::vec2 getScale() const { return scale; }
    float getRotation() const { return rotation; }
    glm::vec2 getOrigin() const { return origin; }

    virtual void setTransform(const glm::mat4& transform) { decompose(transform); }
    virtual void setTransform(const glm::mat3& transform) { decompose(transform); }
    virtual void setPosition(const glm::vec2& pos) { position = pos; updateTransform(); }
    virtual void addPosition(const glm::vec2& pos) { position += pos; updateTransform(); }
    virtual void setScale(const glm::vec2& scl) { scale = scl; updateTransform(); }
    virtual void setRotation(float rot) { rotation = rot; updateTransform(); }
    virtual void setOrigin(const glm::vec2& org) { origin = org; updateTransform(); }
};

class Coordination {
public:
    Coordination() { setIdentity(); }
    explicit Coordination(const glm::mat4& transform) { setFromTransform(transform); }
    Coordination(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
        setTRS(position, rotation, scale);
    }

    const glm::mat4& getScaleMatrix() const       { return S_; }
    const glm::mat4& getRotationMatrix() const    { return R_; }
    const glm::mat4& getTranslationMatrix() const { return T_; }
    const glm::mat4& getTransform() const         { return M_; }

    const glm::vec3& getPosition() const  { return position_; }
    const glm::quat& getRotation() const  { return rotation_; }
    const glm::vec3& getScale() const     { return scale_; }

    glm::vec3 getRight()   const { return glm::normalize(glm::vec3(R_[0])); } // +X
    glm::vec3 getUp()      const { return glm::normalize(glm::vec3(R_[1])); } // +Y
    glm::vec3 getForward() const { return glm::normalize(glm::vec3(R_[2])); } // +Z (glTF default)

    virtual void updateTransform() { M_ = T_ * R_ * S_; }

    void setScaleMatrix(const glm::mat4& S) {
        // Expect pure scale matrix (diagonal). We read the diagonal signs too.
        scale_ = glm::vec3(S[0][0], S[1][1], S[2][2]);
        S_ = glm::scale(glm::mat4(1.0f), scale_);
        updateTransform();
    }
    void setRotationMatrix(const glm::mat4& R) {
        // Extract and orthonormalize upper-left 3x3 to ensure a proper rotation (no scale/shear)
        glm::vec3 r = glm::vec3(R[0]);
        glm::vec3 u = glm::vec3(R[1]);
        glm::vec3 f = glm::vec3(R[2]);

        // Gram-Schmidt to get a clean right-handed basis
        r = safeNormalize(r, glm::vec3(1,0,0));
        u = safeNormalize(u - r * glm::dot(u, r), glm::vec3(0,1,0));
        f = glm::normalize(glm::cross(r, u));   // ensure right-handed: r x u = f

        glm::mat4 pure(1.0f);
        pure[0] = glm::vec4(r, 0.0f);
        pure[1] = glm::vec4(u, 0.0f);
        pure[2] = glm::vec4(f, 0.0f);
        R_ = pure;

        rotation_ = glm::normalize(glm::quat_cast(glm::mat3(R_)));
        updateTransform();
    }
    void setTranslationMatrix(const glm::mat4& T) {
        position_ = glm::vec3(T[3]);
        T_ = glm::translate(glm::mat4(1.0f), position_);
        updateTransform();
    }

    // Convenience setters (TRS values)
    void setPosition(const glm::vec3& p) {
        position_ = p;
        T_ = glm::translate(glm::mat4(1.0f), position_);
        updateTransform();
    }
    void addPosition(const glm::vec3& dp) { setPosition(position_ + dp); }

    void setScale(const glm::vec3& s) {
        scale_ = s;
        S_ = glm::scale(glm::mat4(1.0f), scale_);
        updateTransform();
    }
    void setUniformScale(float s) { setScale(glm::vec3(s)); }

    void setRotation(const glm::quat& q) {
        rotation_ = glm::normalize(q);
        R_ = glm::toMat4(rotation_);
        updateTransform();
    }
    // Euler angles in radians, applied X->Y->Z (pitch, yaw, roll)
    // void setRotationEulerXYZ(const glm::vec3& eulerRadians) {
    //     R_ = glm::eulerAngleXYZ(eulerRadians.x, eulerRadians.y, eulerRadians.z);
    //     rotation_ = glm::normalize(glm::quat_cast(glm::mat3(R_)));
    //     updateTransform();
    // }

    // Look/aim the +Z axis towards a direction (glTF-style forward)
    void setRotationTowardsDirection(const glm::vec3& direction, const glm::vec3& upHint = glm::vec3(0,1,0)) {
        glm::vec3 f = safeNormalize(direction, glm::vec3(0,0,1));
        glm::vec3 u = safeNormalize(upHint, glm::vec3(0,1,0));

        // If direction and up are nearly parallel, choose a fallback up
        if (std::abs(glm::dot(f, u)) > 0.999f) {
            u = (std::abs(f.y) > 0.9f) ? glm::vec3(0,0,1) : glm::vec3(0,1,0);
        }

        glm::vec3 r = glm::normalize(glm::cross(u, f)); // right
        glm::vec3 cu = glm::cross(f, r); // corrected up

        glm::mat4 R(1.0f);
        R[0] = glm::vec4(r, 0.0f);
        R[1] = glm::vec4(cu, 0.0f);
        R[2] = glm::vec4(f, 0.0f);

        setRotationMatrix(R); // will update quaternion and M
    }

    void setRotationTowardsDirection_Z(const glm::vec3& dir) {
        if (glm::length2(dir) < 1e-10f) return;
        glm::vec3 f = glm::normalize(dir);
        glm::quat q = glm::rotation(glm::vec3(0,0,1), f); // rotate +Z to f
        setRotation(q);
    }

    void setRotationTowardsDirection_Y(const glm::vec3& dir) {
        if (glm::length2(dir) < 1e-10f) return;
        glm::vec3 f = glm::normalize(dir);
        glm::quat q = glm::rotation(glm::vec3(0,1,0), f); // rotate +Y to f
        setRotation(q);
    }

    void setRotationTowardsDirection_Z_KeepUp(const glm::vec3& dir, const glm::vec3& worldUp = glm::vec3(0,1,0)) {
        glm::vec3 f = glm::normalize(dir);
        if (glm::length2(f) < 1e-12f) return;

        glm::vec3 u = worldUp;
        if (std::abs(glm::dot(f, u)) > 0.999f) u = (std::abs(f.y) > 0.9f) ? glm::vec3(0,0,1) : glm::vec3(0,1,0);

        glm::vec3 r  = glm::normalize(glm::cross(u, f));
        glm::vec3 cu = glm::cross(f, r);

        glm::mat4 R(1.0f);
        R[0] = glm::vec4(r, 0.0f);
        R[1] = glm::vec4(cu, 0.0f);
        R[2] = glm::vec4(f, 0.0f);
        setRotationMatrix(R); // your Coordination setter
    }

    // Combined setters
    void setTRS(const glm::vec3& p, const glm::quat& q, const glm::vec3& s) {
        position_ = p;
        rotation_ = glm::normalize(q);
        scale_    = s;

        T_ = glm::translate(glm::mat4(1.0f), position_);
        R_ = glm::toMat4(rotation_);
        S_ = glm::scale(glm::mat4(1.0f), scale_);

        updateTransform();
    }

    void setMatrices(const glm::mat4& T, const glm::mat4& R, const glm::mat4& S) {
        setTranslationMatrix(T);
        setRotationMatrix(R);
        setScaleMatrix(S);
    }

    // Decompose a combined transform (works for typical glTF TRS, incl. non-uniform and negative scale)
    void setFromTransform(const glm::mat4& M) { decompose(M); }
    void decompose(const glm::mat4& M) {
        // Translation
        position_ = glm::vec3(M[3]);
        T_ = glm::translate(glm::mat4(1.0f), position_);

        // Extract columns (basis with scale)
        glm::vec3 col0 = glm::vec3(M[0]);
        glm::vec3 col1 = glm::vec3(M[1]);
        glm::vec3 col2 = glm::vec3(M[2]);

        float sx = glm::length(col0);
        float sy = glm::length(col1);
        float sz = glm::length(col2);

        // Avoid divide-by-zero; if scale is zero on an axis, pick a fallback basis axis
        if (sx > EPS) col0 /= sx; else { col0 = glm::vec3(1,0,0); sx = 0.0f; }
        if (sy > EPS) col1 /= sy; else { col1 = glm::vec3(0,1,0); sy = 0.0f; }
        if (sz > EPS) col2 /= sz; else { col2 = glm::vec3(0,0,1); sz = 0.0f; }

        // Ensure right-handed rotation (move any reflection into scale)
        glm::mat3 R3(col0, col1, col2);
        if (glm::determinant(R3) < 0.0f) {
            // Flip one axis. we flip Z.
            sz   = -sz;
            col2 = -col2;
            R3   = glm::mat3(col0, col1, col2);
        }

        rotation_ = glm::normalize(glm::quat_cast(R3));
        R_ = glm::toMat4(rotation_);

        scale_ = glm::vec3(sx, sy, sz);
        S_ = glm::scale(glm::mat4(1.0f), scale_);

        updateTransform();
    }
    void setIdentity() {
        position_ = glm::vec3(0.0f);
        rotation_ = glm::quat(1, 0, 0, 0);
        scale_    = glm::vec3(1.0f);

        T_ = glm::mat4(1.0f);
        R_ = glm::mat4(1.0f);
        S_ = glm::mat4(1.0f);
        updateTransform();
    }
private:
    static constexpr float EPS = 1e-7f;

    glm::mat4 S_{1.0f};
    glm::mat4 R_{1.0f};
    glm::mat4 T_{1.0f};

    glm::mat4 M_{1.0f}; // M = T * R * S

    glm::vec3 position_{0.0f};
    glm::quat rotation_{1,0,0,0};
    glm::vec3 scale_{1.0f};

    static glm::vec3 safeNormalize(const glm::vec3& v, const glm::vec3& fallback) {
        float len2 = glm::dot(v, v);
        return (len2 > EPS*EPS) ? v / glm::sqrt(len2) : fallback;
    }
};

}
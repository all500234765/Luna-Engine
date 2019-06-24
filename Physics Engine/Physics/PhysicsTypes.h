#pragma once

#include <algorithm>

#ifndef _PhysicsShapeType_
#define _PhysicsShapeType_
typedef enum: char {
    Undefined,
    Sphere,
    AABB,
    Plane,
    Box,
    Cylinder,
    Complex
} PhysicsShapeType;
#endif

typedef float pFloat;

struct pFloat3 {
    union {
        float x;
        float r;
    };

    union {
        float y;
        float g;
    };

    union {
        float z;
        float b;
    };

    // 
    pFloat3 max(const pFloat3& rhs) const {
        return { std::max(x, rhs.x), std::max(y, rhs.y), std::max(z, rhs.z) };
    }

    pFloat dot(const pFloat3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // 
    pFloat3 operator-(const pFloat3& rhs) const {
        return { x - rhs.x, y - rhs.y, z - rhs.z };
    }

    pFloat3 operator+(const pFloat3& rhs) const {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }

    pFloat3 operator/(const pFloat3& rhs) const {
        return { x / rhs.x, y / rhs.y, z / rhs.z };
    }

    pFloat3 operator*(const pFloat3& rhs) const {
        return { x * rhs.x, y * rhs.y, z * rhs.z };
    }

    // 
    pFloat3& operator+=(const pFloat3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    pFloat3& operator-=(const pFloat3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    pFloat3& operator*=(const pFloat3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }

    pFloat3& operator/=(const pFloat3& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    // 
    template<typename T>
    pFloat3& operator+=(T rhs) {
        x += rhs;
        y += rhs;
        z += rhs;
        return *this;
    }

    template<typename T>
    pFloat3& operator-=(T rhs) {
        x -= rhs;
        y -= rhs;
        z -= rhs;
        return *this;
    }

    template<typename T>
    pFloat3& operator*=(T rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }

    template<typename T>
    pFloat3& operator/=(T rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    // TODO: lhs can be T too, not just pFloat3
    template<typename T>
    pFloat3 operator-(T rhs) const {
        return { x - rhs, y - rhs, z - rhs };
    }

    template<typename T>
    pFloat3 operator*(T rhs) const {
        return { x * rhs, y * rhs, z * rhs };
    }

    template<typename T>
    pFloat3 operator/(T rhs) const {
        return { x / rhs, y / rhs, z / rhs };
    }

    template<typename T>
    pFloat3 operator+(T rhs) const {
        return { x + rhs, y + rhs, z + rhs };
    }

    // 
    pFloat length()  const { return std::sqrtf(length2()); };
    pFloat length2() const { return (x * x + y * y + z * z); };
    pFloat max() const { return std::max(std::max(x, y), z); }
    const pFloat3& normalize() { (pFloat3)*this /= length(); return *this; }
    const pFloat3& normalized() const { return (pFloat3)*this / length(); }

    const pFloat3& Reflected(const pFloat3& n) const {
        return *this - n * this->dot(n) * 2;
    }

    const pFloat3& Reflect(const pFloat3& n) {
        *this -= n * this->dot(n) * 2;
        return *this;
    }
};

struct pQuat {
    union {
        float x;
        float r;
    };

    union {
        float y;
        float g;
    };

    union {
        float z;
        float b;
    };

    union {
        float w;
        float a;
    };
};


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

    pFloat3 max(const pFloat3& rhs) const {
        return { std::max(x, rhs.x), std::max(y, rhs.y), std::max(z, rhs.z) };
    }

    pFloat max() const {
        return std::max(std::max(x, y), z);
    }

    pFloat dot(const pFloat3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

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

    pFloat3 operator-(pFloat rhs) const {
        return { x - rhs, y - rhs, z - rhs };
    }

    pFloat3 operator*(pFloat rhs) const {
        return { x * rhs, y * rhs, z * rhs };
    }

    pFloat3 operator/(pFloat rhs) const {
        return { x / rhs, y / rhs, z / rhs };
    }

    pFloat3 operator+(pFloat rhs) const {
        return { x + rhs, y + rhs, z + rhs };
    }

    pFloat length()  const { return std::sqrtf(length2()); };
    pFloat length2() const { return (x * x + y * y + z * z); };
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


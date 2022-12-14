#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdio>

struct vec3 {
    float vertices[3]{ 0.0f };

    vec3() {}
    vec3(float x, float y, float z) { vertices[0] = x; vertices[1] = y; vertices[2] = z; }

    inline const float& x() { return vertices[0]; }
    inline const float& y() { return vertices[1]; }
    inline const float& z() { return vertices[2]; }
    inline const float& r() const { return vertices[0]; }
    inline const float& g() const { return vertices[1]; }
    inline const float& b() const { return vertices[2]; }
    inline vec3& operator+=(const vec3& o);
    inline vec3& operator-=(const vec3& o);
    inline vec3& operator*=(const vec3& o);
    inline vec3& operator/=(const vec3& o);
    inline vec3& operator*=(const float t);
    inline vec3& operator/=(const float t);

    inline const vec3& operator+() const { return *this; }
    inline vec3 operator-() const { return vec3(-vertices[0], -vertices[1], -vertices[2]); }
    inline const vec3& normalize() { float l = length(); vec3 t(vertices[0] / l, vertices[1] / l, vertices[2] / l); return t; }
    inline float operator[](int i) const { return vertices[i]; }
    inline float& operator[](int i) { return vertices[i]; }
    inline float length() const { return sqrt(vertices[0] * vertices[0] + vertices[1] * vertices[1] + vertices[2] * vertices[2]); }
    inline float squared_length() const { return vertices[0] * vertices[0] + vertices[1] * vertices[1] + vertices[2] * vertices[2]; }

    inline const void print() { printf_s("x%f y%f z%f\n", vertices[0], vertices[1], vertices[2]); };
};

inline vec3 operator+(const vec3& v1, const vec3& v2) {
    return vec3(v1.vertices[0] + v2.vertices[0], v1.vertices[1] + v2.vertices[1], v1.vertices[2] + v2.vertices[2]);
}

inline vec3 operator-(const vec3& v1, const vec3& v2) {
    return vec3(v1.vertices[0] - v2.vertices[0], v1.vertices[1] - v2.vertices[1], v1.vertices[2] - v2.vertices[2]);
}

inline vec3 operator*(const vec3& v1, const vec3& v2) {
    return vec3(v1.vertices[0] * v2.vertices[0], v1.vertices[1] * v2.vertices[1], v1.vertices[2] * v2.vertices[2]);
}

inline vec3 operator/(const vec3& v1, const vec3& v2) {
    return vec3(v1.vertices[0] / v2.vertices[0], v1.vertices[1] / v2.vertices[1], v1.vertices[2] / v2.vertices[2]);
}

inline vec3 operator*(float t, const vec3& v) {
    return vec3(t * v.vertices[0], t * v.vertices[1], t * v.vertices[2]);
}

inline vec3 operator/(const vec3& v, float t) {
    return vec3(v.vertices[0] / t, v.vertices[1] / t, v.vertices[2] / t);
}

inline vec3 operator*(const vec3& v, float t) {
    return vec3(t * v.vertices[0], t * v.vertices[1], t * v.vertices[2]);
}

inline float dot(const vec3& v1, const vec3& v2) { return v1.vertices[0] * v2.vertices[0] + v1.vertices[1] * v2.vertices[1] + v1.vertices[2] * v2.vertices[2]; }

inline vec3 cross(const vec3& v1, const vec3& v2) {
    return vec3(v1.vertices[1] * v2.vertices[2] - v1.vertices[2] * v2.vertices[1],
        v1.vertices[2] * v2.vertices[0] - v1.vertices[0] * v2.vertices[2],
        v1.vertices[0] * v2.vertices[1] - v1.vertices[1] * v2.vertices[0]);
}

inline vec3& vec3::operator+=(const vec3& v) {
    vertices[0] += v.vertices[0];
    vertices[1] += v.vertices[1];
    vertices[2] += v.vertices[2];
    return *this;
}

inline vec3& vec3::operator*=(const vec3& v) {
    vertices[0] *= v.vertices[0];
    vertices[1] *= v.vertices[1];
    vertices[2] *= v.vertices[2];
    return *this;
}

inline vec3& vec3::operator/=(const vec3& v) {
    vertices[0] /= v.vertices[0];
    vertices[1] /= v.vertices[1];
    vertices[2] /= v.vertices[2];
    return *this;
}

inline vec3& vec3::operator-=(const vec3& v) {
    vertices[0] -= v.vertices[0];
    vertices[1] -= v.vertices[1];
    vertices[2] -= v.vertices[2];
    return *this;
}

inline vec3& vec3::operator*=(const float t) {
    vertices[0] *= t;
    vertices[1] *= t;
    vertices[2] *= t;
    return *this;
}

inline vec3& vec3::operator/=(const float t) {
    float k = 1.0f / t;
    vertices[0] *= k;
    vertices[1] *= k;
    vertices[2] *= k;
    return *this;
}

inline vec3 unit_vector(vec3 v) { return v / v.length(); }
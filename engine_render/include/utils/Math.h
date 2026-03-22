#pragma once

#include "core/Types.h"
#include <cmath>

namespace render::math
{
    // 常量
    constexpr float Pi       = 3.14159265358979323846f;
    constexpr float TwoPi    = 2.0f * Pi;
    constexpr float HalfPi   = 0.5f * Pi;
    constexpr float InvPi    = 1.0f / Pi;
    constexpr float InvTwoPi = 1.0f / TwoPi;
    constexpr float DegToRad = Pi / 180.0f;
    constexpr float RadToDeg = 180.0f / Pi;
    constexpr float Epsilon  = 1e-6f;
    constexpr float Infinity = std::numeric_limits<float>::infinity();

    // 辅助函数
    inline float toRadians(float degrees) { return degrees * DegToRad; }
    inline float toDegrees(float radians) { return radians * RadToDeg; }

    template <typename T> T clamp(T value, T min, T max)
    {
        return std::max(min, std::min(value, max));
    }

    template <typename T> T lerp(T a, T b, float t)
    {
        return a + (b - a) * t;
    }

    template <typename T> T smoothStep(T edge0, T edge1, T x)
    {
        T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
        return t * t * (T(3) - T(2) * t);
    }

    inline bool nearZero(float value, float epsilon = Epsilon)
    {
        return std::abs(value) < epsilon;
    }

    inline float saturate(float value)
    {
        return clamp(value, 0.0f, 1.0f);
    }

    // 2D向量
    struct Vec2
    {
        float x, y;

        Vec2() : x(0), y(0)
        {
        }

        Vec2(float v) : x(v), y(v)
        {
        }

        Vec2(float x, float y) : x(x), y(y)
        {
        }

        // 运算符
        Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
        Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
        Vec2 operator*(const Vec2& other) const { return Vec2(x * other.x, y * other.y); }
        Vec2 operator/(const Vec2& other) const { return Vec2(x / other.x, y / other.y); }
        Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
        Vec2 operator/(float s) const { return Vec2(x / s, y / s); }
        Vec2 operator-() const { return Vec2(-x, -y); }

        Vec2& operator+=(const Vec2& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2& operator-=(const Vec2& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2& operator*=(const Vec2& other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        Vec2& operator/=(const Vec2& other)
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        Vec2& operator*=(float s)
        {
            x *= s;
            y *= s;
            return *this;
        }

        Vec2& operator/=(float s)
        {
            x /= s;
            y /= s;
            return *this;
        }

        bool operator==(const Vec2& other) const { return nearZero(x - other.x) && nearZero(y - other.y); }
        bool operator!=(const Vec2& other) const { return !(*this == other); }

        // 方法
        float length() const { return std::sqrt(x * x + y * y); }
        float lengthSq() const { return x * x + y * y; }

        Vec2 normalized() const
        {
            float len = length();
            return len > 0 ? (*this) / len : Vec2(0, 0);
        }

        void  normalize() { *this = normalized(); }
        float dot(const Vec2& other) const { return x * other.x + y * other.y; }
        float cross(const Vec2& other) const { return x * other.y - y * other.x; }

        // 静态方法
        static float dot(const Vec2& a, const Vec2& b) { return a.dot(b); }
        static float cross(const Vec2& a, const Vec2& b) { return a.cross(b); }
        static Vec2  lerp(const Vec2& a, const Vec2& b, float t) { return a + (b - a) * t; }
        static Vec2  min(const Vec2& a, const Vec2& b) { return Vec2(std::min(a.x, b.x), std::min(a.y, b.y)); }
        static Vec2  max(const Vec2& a, const Vec2& b) { return Vec2(std::max(a.x, b.x), std::max(a.y, b.y)); }

        // 常量
        static Vec2 Zero() { return Vec2(0, 0); }
        static Vec2 One() { return Vec2(1, 1); }
        static Vec2 Right() { return Vec2(1, 0); }
        static Vec2 Up() { return Vec2(0, 1); }
    };

    // 3D向量

    struct Vec3
    {
        float x, y, z;

        Vec3() : x(0), y(0), z(0)
        {
        }

        Vec3(float v) : x(v), y(v), z(v)
        {
        }

        Vec3(float x, float y, float z) : x(x), y(y), z(z)
        {
        }

        // 运算符
        Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
        Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
        Vec3 operator*(const Vec3& other) const { return Vec3(x * other.x, y * other.y, z * other.z); }
        Vec3 operator/(const Vec3& other) const { return Vec3(x / other.x, y / other.y, z / other.z); }
        Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
        Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }
        Vec3 operator-() const { return Vec3(-x, -y, -z); }

        Vec3& operator+=(const Vec3& other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vec3& operator-=(const Vec3& other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vec3& operator*=(const Vec3& other)
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        Vec3& operator/=(const Vec3& other)
        {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }

        Vec3& operator*=(float s)
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        Vec3& operator/=(float s)
        {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }

        bool operator==(const Vec3& other) const
        {
            return nearZero(x - other.x) && nearZero(y - other.y) && nearZero(z - other.z);
        }

        bool operator!=(const Vec3& other) const { return !(*this == other); }

        // 方法
        float length() const { return std::sqrt(x * x + y * y + z * z); }
        float lengthSq() const { return x * x + y * y + z * z; }

        Vec3 normalized() const
        {
            float len = length();
            return len > 0 ? (*this) / len : Vec3(0, 0, 0);
        }

        void  normalize() { *this = normalized(); }
        float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }

        Vec3 cross(const Vec3& other) const
        {
            return Vec3(
                        y * other.z - z * other.y,
                        z * other.x - x * other.z,
                        x * other.y - y * other.x
                       );
        }

        // 静态方法
        static float dot(const Vec3& a, const Vec3& b) { return a.dot(b); }
        static Vec3  cross(const Vec3& a, const Vec3& b) { return a.cross(b); }
        static Vec3  lerp(const Vec3& a, const Vec3& b, float t) { return a + (b - a) * t; }

        static Vec3 min(const Vec3& a, const Vec3& b)
        {
            return Vec3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
        }

        static Vec3 max(const Vec3& a, const Vec3& b)
        {
            return Vec3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
        }

        static Vec3 reflect(const Vec3& incident, const Vec3& normal)
        {
            return incident - normal * 2.0f * dot(incident, normal);
        }

        static Vec3 refract(const Vec3& incident, const Vec3& normal, float eta)
        {
            float dotNI = dot(normal, incident);
            float k     = 1.0f - eta * eta * (1.0f - dotNI * dotNI);
            if (k < 0) return Vec3(0, 0, 0);
            return incident * eta - normal * (eta * dotNI + std::sqrt(k));
        }

        // 常量
        static Vec3 Zero() { return Vec3(0, 0, 0); }
        static Vec3 One() { return Vec3(1, 1, 1); }
        static Vec3 Right() { return Vec3(1, 0, 0); }
        static Vec3 Up() { return Vec3(0, 1, 0); }
        static Vec3 Forward() { return Vec3(0, 0, 1); }
    };

    // 别名
    using Vector2 = Vec2;
    using Vector3 = Vec3;

    // 4D向量
    struct Vec4
    {
        float x, y, z, w;

        Vec4() : x(0), y(0), z(0), w(0)
        {
        }

        Vec4(float v) : x(v), y(v), z(v), w(v)
        {
        }

        Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
        {
        }

        Vec4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w)
        {
        }

        // 运算符
        Vec4 operator+(const Vec4& other) const
        {
            return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Vec4 operator-(const Vec4& other) const
        {
            return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        Vec4 operator*(const Vec4& other) const
        {
            return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
        }

        Vec4 operator/(const Vec4& other) const
        {
            return Vec4(x / other.x, y / other.y, z / other.z, w / other.w);
        }

        Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
        Vec4 operator/(float s) const { return Vec4(x / s, y / s, z / s, w / s); }
        Vec4 operator-() const { return Vec4(-x, -y, -z, -w); }

        // 方法
        float dot(const Vec4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
        Vec3  xyz() const { return Vec3(x, y, z); }

        // 静态方法
        static float dot(const Vec4& a, const Vec4& b) { return a.dot(b); }
        static Vec4  lerp(const Vec4& a, const Vec4& b, float t) { return a + (b - a) * t; }
    };

    // 4x4矩阵
    struct Mat4
    {
        float m[4][4];

        Mat4() { *this = identity(); }
        Mat4(const float* data) { std::memcpy(m, data, sizeof(m)); }

        // 访问器
        float*       operator[](int row) { return m[row]; }
        const float* operator[](int row) const { return m[row]; }

        // 运算符
        Mat4 operator*(const Mat4& other) const
        {
            Mat4 result;
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    result.m[i][j] = 0;
                    for (int k = 0; k < 4; ++k)
                    {
                        result.m[i][j] += m[i][k] * other.m[k][j];
                    }
                }
            }
            return result;
        }

        Vec4 operator*(const Vec4& v) const
        {
            return Vec4(
                        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
                        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                        m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
                       );
        }

        Vec3 transformPoint(const Vec3& p) const
        {
            Vec4 v = (*this) * Vec4(p, 1.0f);
            return v.xyz() / v.w;
        }

        Vec3 transformVector(const Vec3& v) const
        {
            return (*this * Vec4(v, 0.0f)).xyz();
        }

        // 静态方法
        static Mat4 identity()
        {
            Mat4 m;
            for (int i = 0; i < 4; ++i)
                for (int j    = 0; j < 4; ++j)
                    m.m[i][j] = (i == j) ? 1.0f : 0.0f;
            return m;
        }

        static Mat4 translate(const Vec3& t)
        {
            Mat4 m    = identity();
            m.m[0][3] = t.x;
            m.m[1][3] = t.y;
            m.m[2][3] = t.z;
            return m;
        }

        static Mat4 scale(const Vec3& s)
        {
            Mat4 m    = identity();
            m.m[0][0] = s.x;
            m.m[1][1] = s.y;
            m.m[2][2] = s.z;
            return m;
        }

        static Mat4 rotateX(float angle)
        {
            Mat4  m   = identity();
            float c   = std::cos(angle);
            float s   = std::sin(angle);
            m.m[1][1] = c;
            m.m[1][2] = -s;
            m.m[2][1] = s;
            m.m[2][2] = c;
            return m;
        }

        static Mat4 rotateY(float angle)
        {
            Mat4  m   = identity();
            float c   = std::cos(angle);
            float s   = std::sin(angle);
            m.m[0][0] = c;
            m.m[0][2] = s;
            m.m[2][0] = -s;
            m.m[2][2] = c;
            return m;
        }

        static Mat4 rotateZ(float angle)
        {
            Mat4  m   = identity();
            float c   = std::cos(angle);
            float s   = std::sin(angle);
            m.m[0][0] = c;
            m.m[0][1] = -s;
            m.m[1][0] = s;
            m.m[1][1] = c;
            return m;
        }

        static Mat4 perspective(float fovY, float aspect, float nearZ, float farZ)
        {
            Mat4  m;
            float tanHalfFovY = std::tan(fovY / 2.0f);
            m.m[0][0]         = 1.0f / (aspect * tanHalfFovY);
            m.m[1][1]         = 1.0f / tanHalfFovY;
            m.m[2][2]         = farZ / (farZ - nearZ);
            m.m[2][3]         = -(farZ * nearZ) / (farZ - nearZ);
            m.m[3][2]         = 1.0f;
            m.m[3][3]         = 0.0f;
            return m;
        }

        static Mat4 ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
        {
            Mat4 m    = identity();
            m.m[0][0] = 2.0f / (right - left);
            m.m[1][1] = 2.0f / (top - bottom);
            m.m[2][2] = 1.0f / (farZ - nearZ);
            m.m[0][3] = -(right + left) / (right - left);
            m.m[1][3] = -(top + bottom) / (top - bottom);
            m.m[2][3] = -nearZ / (farZ - nearZ);
            return m;
        }

        static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
        {
            Vec3 z    = (eye - target).normalized();
            Vec3 x    = Vec3::cross(up, z).normalized();
            Vec3 y    = Vec3::cross(z, x);
            Mat4 m    = identity();
            m.m[0][0] = x.x;
            m.m[0][1] = x.y;
            m.m[0][2] = x.z;
            m.m[0][3] = -Vec3::dot(x, eye);
            m.m[1][0] = y.x;
            m.m[1][1] = y.y;
            m.m[1][2] = y.z;
            m.m[1][3] = -Vec3::dot(y, eye);
            m.m[2][0] = z.x;
            m.m[2][1] = z.y;
            m.m[2][2] = z.z;
            m.m[2][3] = -Vec3::dot(z, eye);
            return m;
        }

        Mat4 transposed() const
        {
            Mat4 result;
            for (int i = 0; i < 4; ++i)
                for (int j         = 0; j < 4; ++j)
                    result.m[i][j] = m[j][i];
            return result;
        }
    };

    // 四元数
    struct Quat
    {
        float x, y, z, w;

        Quat() : x(0), y(0), z(0), w(1)
        {
        }

        Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
        {
        }

        Quat(const Vec3& axis, float angle)
        {
            float halfAngle = angle / 2.0f;
            float s         = std::sin(halfAngle);
            x               = axis.x * s;
            y               = axis.y * s;
            z               = axis.z * s;
            w               = std::cos(halfAngle);
        }

        // 运算符
        Quat operator*(const Quat& other) const
        {
            return Quat(
                        w * other.x + x * other.w + y * other.z - z * other.y,
                        w * other.y - x * other.z + y * other.w + z * other.x,
                        w * other.z + x * other.y - y * other.x + z * other.w,
                        w * other.w - x * other.x - y * other.y - z * other.z
                       );
        }

        Quat operator*(float s) const { return Quat(x * s, y * s, z * s, w * s); }
        Quat operator/(float s) const { return Quat(x / s, y / s, z / s, w / s); }
        Quat operator+(const Quat& other) const { return Quat(x + other.x, y + other.y, z + other.z, w + other.w); }
        Quat operator-() const { return Quat(-x, -y, -z, -w); }

        // 方法
        float length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

        Quat normalized() const
        {
            float len = length();
            return len > 0 ? (*this) * (1.0f / len) : Quat(0, 0, 0, 1);
        }

        void normalize() { *this = normalized(); }
        Quat conjugated() const { return Quat(-x, -y, -z, w); }
        Quat inversed() const { return conjugated() / length(); }

        Vec3 rotate(const Vec3& v) const
        {
            Quat q = (*this) * Quat(v.x, v.y, v.z, 0) * inversed();
            return Vec3(q.x, q.y, q.z);
        }

        Mat4 toMatrix() const
        {
            Mat4  m;
            float xx  = x * x, yy = y * y, zz = z * z;
            float xy  = x * y, xz = x * z, yz = y * z;
            float wx  = w * x, wy = w * y, wz = w * z;
            m.m[0][0] = 1 - 2 * (yy + zz);
            m.m[0][1] = 2 * (xy + wz);
            m.m[0][2] = 2 * (xz - wy);
            m.m[1][0] = 2 * (xy - wz);
            m[1][1]   = 1 - 2 * (xx + zz);
            m[1][2]   = 2 * (yz + wx);
            m[2][0]   = 2 * (xz + wy);
            m[2][1]   = 2 * (yz - wx);
            m[2][2]   = 1 - 2 * (xx + yy);
            m.m[3][3] = 1;
            return m;
        }

        // 静态方法
        static Quat identity() { return Quat(0, 0, 0, 1); }

        static Quat lerp(const Quat& a, const Quat& b, float t)
        {
            return (a * (1 - t) + b * t).normalized();
        }

        static Quat slerp(const Quat& a, const Quat& b, float t)
        {
            float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
            Quat  end = b;
            if (dot < 0)
            {
                dot = -dot;
                end = -b;
            }
            if (dot > 0.9995f)
            {
                return lerp(a, end, t);
            }
            float theta    = std::acos(dot);
            float sinTheta = std::sin(theta);
            float w1       = std::sin((1 - t) * theta) / sinTheta;
            float w2       = std::sin(t * theta) / sinTheta;
            return a * w1 + end * w2;
        }

        static Quat fromEuler(const Vec3& euler)
        {
            float cx = std::cos(euler.x * 0.5f);
            float sx = std::sin(euler.x * 0.5f);
            float cy = std::cos(euler.y * 0.5f);
            float sy = std::sin(euler.y * 0.5f);
            float cz = std::cos(euler.z * 0.5f);
            float sz = std::sin(euler.z * 0.5f);
            return Quat(
                        sx * cy * cz - cx * sy * sz,
                        cx * sy * cz + sx * cy * sz,
                        cx * cy * sz - sx * sy * cz,
                        cx * cy * cz + sx * sy * sz
                       );
        }
    };

    // 颜色转换
    inline Vec3 rgbToHsv(const Vec3& rgb)
    {
        float max   = std::max({rgb.x, rgb.y, rgb.z});
        float min   = std::min({rgb.x, rgb.y, rgb.z});
        float delta = max - min;
        Vec3  hsv;
        hsv.z = max;
        if (delta < Epsilon)
        {
            hsv.x = 0;
            hsv.y = 0;
        }
        else
        {
            hsv.y = delta / max;
            if (max == rgb.x)
            {
                hsv.x = (rgb.y - rgb.z) / delta + (rgb.y < rgb.z ? 6 : 0);
            }
            else if (max == rgb.y)
            {
                hsv.x = (rgb.z - rgb.x) / delta + 2;
            }
            else
            {
                hsv.x = (rgb.x - rgb.y) / delta + 4;
            }
            hsv.x /= 6.0f;
        }
        return hsv;
    }

    inline Vec3 hsvToRgb(const Vec3& hsv)
    {
        float h = hsv.x * 6.0f;
        float s = hsv.y;
        float v = hsv.z;
        int   i = static_cast<int>(h);
        float f = h - i;
        float p = v * (1 - s);
        float q = v * (1 - f * s);
        float t = v * (1 - (1 - f) * s);
        switch (i % 6)
        {
            case 0: return Vec3(v, t, p);
            case 1: return Vec3(q, v, p);
            case 2: return Vec3(p, v, t);
            case 3: return Vec3(p, q, v);
            case 4: return Vec3(t, p, v);
            case 5: return Vec3(v, p, q);
        }
        return Vec3(0, 0, 0);
    }
}

#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <string>

// 3D向量类
class Vec3 {
public:
    float x, y, z;
    
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    // 向量加法
    Vec3 operator+(const Vec3& v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }
    
    // 向量减法
    Vec3 operator-(const Vec3& v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }
    
    // 向量乘以标量
    Vec3 operator*(float s) const {
        return Vec3(x * s, y * s, z * s);
    }
    
    // 向量除以标量
    Vec3 operator/(float s) const {
        if (s == 0.0f) return *this;
        float inv = 1.0f / s;
        return Vec3(x * inv, y * inv, z * inv);
    }
    
    // 向量点乘
    float dot(const Vec3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    // 向量叉乘
    Vec3 cross(const Vec3& v) const {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
    
    // 向量长度的平方
    float lengthSquared() const {
        return x * x + y * y + z * z;
    }
    
    // 向量长度
    float length() const {
        return std::sqrt(lengthSquared());
    }
    
    // 归一化向量
    Vec3 normalize() const {
        float len = length();
        if (len == 0.0f) return *this;
        return *this / len;
    }
};

// 4x4矩阵类
class Mat4 {
public:
    float m[4][4];
    
    Mat4() {
        // 初始化为单位矩阵
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
    
    // 矩阵乘法
    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
    
    // 矩阵乘以向量
    Vec3 operator*(const Vec3& v) const {
        float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
        float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
        float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
        float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];
        
        if (w != 0.0f) {
            return Vec3(x / w, y / w, z / w);
        }
        return Vec3(x, y, z);
    }
    
    // 创建平移矩阵
    static Mat4 translate(const Vec3& v) {
        Mat4 result;
        result.m[0][3] = v.x;
        result.m[1][3] = v.y;
        result.m[2][3] = v.z;
        return result;
    }
    
    // 创建缩放矩阵
    static Mat4 scale(const Vec3& v) {
        Mat4 result;
        result.m[0][0] = v.x;
        result.m[1][1] = v.y;
        result.m[2][2] = v.z;
        return result;
    }
    
    // 创建绕X轴旋转的矩阵
    static Mat4 rotateX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        Mat4 result;
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }
    
    // 创建绕Y轴旋转的矩阵
    static Mat4 rotateY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        Mat4 result;
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }
    
    // 创建绕Z轴旋转的矩阵
    static Mat4 rotateZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        Mat4 result;
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }
    
    // 创建视图矩阵
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalize();
        Vec3 r = f.cross(up).normalize();
        Vec3 u = r.cross(f);
        
        Mat4 result;
        result.m[0][0] = r.x;
        result.m[0][1] = r.y;
        result.m[0][2] = r.z;
        result.m[0][3] = -r.dot(eye);
        
        result.m[1][0] = u.x;
        result.m[1][1] = u.y;
        result.m[1][2] = u.z;
        result.m[1][3] = -u.dot(eye);
        
        result.m[2][0] = -f.x;
        result.m[2][1] = -f.y;
        result.m[2][2] = -f.z;
        result.m[2][3] = f.dot(eye);
        
        return result;
    }
    
    // 创建透视投影矩阵 (DirectX风格)
    static Mat4 perspective(float fov, float aspect, float nearPlane, float farPlane) {
        float yScale = 1.0f / std::tan(fov / 2.0f);
        float xScale = yScale / aspect;
        
        Mat4 result;
        // 按照DirectX的透视投影矩阵格式
        result.m[0][0] = xScale;
        result.m[0][1] = 0.0f;
        result.m[0][2] = 0.0f;
        result.m[0][3] = 0.0f;
        
        result.m[1][0] = 0.0f;
        result.m[1][1] = yScale;
        result.m[1][2] = 0.0f;
        result.m[1][3] = 0.0f;
        
        result.m[2][0] = 0.0f;
        result.m[2][1] = 0.0f;
        result.m[2][2] = farPlane / (farPlane - nearPlane);
        result.m[2][3] = 1.0f;
        
        result.m[3][0] = 0.0f;
        result.m[3][1] = 0.0f;
        result.m[3][2] = -nearPlane * farPlane / (farPlane - nearPlane);
        result.m[3][3] = 0.0f;
        
        return result;
    }
    
    // DirectX风格的正交投影矩阵
    static Mat4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        Mat4 result;
        result.m[0][0] = 2.0f / (right - left);
        result.m[1][1] = 2.0f / (top - bottom);
        result.m[2][2] = 1.0f / (farPlane - nearPlane);
        result.m[0][3] = -(right + left) / (right - left);
        result.m[1][3] = -(top + bottom) / (top - bottom);
        result.m[2][3] = nearPlane / (nearPlane - farPlane);
        result.m[3][3] = 1.0f;
        
        return result;
    }
};

// 颜色类
class Color {
public:
    uint8_t r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    
    // 将颜色转换为32位整数
    uint32_t toUint32() const {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    
    // 颜色混合
    Color blend(const Color& other, float factor) const {
        factor = std::clamp(factor, 0.0f, 1.0f);
        float invFactor = 1.0f - factor;
        
        return Color(
            static_cast<uint8_t>(r * invFactor + other.r * factor),
            static_cast<uint8_t>(g * invFactor + other.g * factor),
            static_cast<uint8_t>(b * invFactor + other.b * factor),
            static_cast<uint8_t>(a * invFactor + other.a * factor)
        );
    }
    
    // 颜色乘以强度因子
    Color multiply(float factor) const {
        factor = std::clamp(factor, 0.0f, 1.0f);
        
        return Color(
            static_cast<uint8_t>(r * factor),
            static_cast<uint8_t>(g * factor),
            static_cast<uint8_t>(b * factor),
            a
        );
    }
};

#endif // MATH3D_H
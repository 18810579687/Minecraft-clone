#ifndef CAMERA_H
#define CAMERA_H

#include "math3d.h"
#include <iostream>  // 为std::cout

class Camera {
public:
    // 移动方向枚举
    enum Direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    
    // 相机属性
    Vec3 position;    // 相机位置
    Vec3 front;       // 相机前方向
    Vec3 up;          // 相机上方向
    Vec3 right;       // 相机右方向
    Vec3 worldUp;     // 世界上方向
    
    // 欧拉角
    float yaw;        // 偏航角
    float pitch;      // 俯仰角
    
    // 视场和宽高比
    float fov;        // 视场角（弧度）
    float aspectRatio; // 宽高比
    float nearPlane;   // 近平面
    float farPlane;    // 远平面
    
    // 构造函数
    Camera() : 
        position(Vec3(0.0f, 0.0f, 0.0f)),
        front(Vec3(0.0f, 0.0f, -1.0f)),
        worldUp(Vec3(0.0f, 1.0f, 0.0f)),
        yaw(-90.0f),
        pitch(0.0f),
        fov(45.0f * 3.14159f / 180.0f),
        aspectRatio(4.0f / 3.0f),
        nearPlane(0.1f),
        farPlane(100.0f) {
        updateCameraVectors();
    }
    
    // 初始化相机
    void init(const Vec3& position, const Vec3& front, const Vec3& worldUp) {
        this->position = position;
        this->front = front.normalize();
        this->worldUp = worldUp.normalize();
        this->yaw = -90.0f;
        this->pitch = 0.0f;
        updateCameraVectors();
    }
    
    // 获取视图矩阵 - DirectX风格
    Mat4 getViewMatrix() const {
        // 对于DirectX，视图矩阵是相机空间到世界空间变换的逆矩阵
        Vec3 zaxis = front.normalize();
        Vec3 xaxis = up.cross(zaxis).normalize();
        Vec3 yaxis = zaxis.cross(xaxis);
        
        Mat4 view;
        
        // 填充旋转部分
        view.m[0][0] = xaxis.x;
        view.m[0][1] = yaxis.x;
        view.m[0][2] = zaxis.x;
        view.m[0][3] = 0.0f;
        
        view.m[1][0] = xaxis.y;
        view.m[1][1] = yaxis.y;
        view.m[1][2] = zaxis.y;
        view.m[1][3] = 0.0f;
        
        view.m[2][0] = xaxis.z;
        view.m[2][1] = yaxis.z;
        view.m[2][2] = zaxis.z;
        view.m[2][3] = 0.0f;
        
        // 填充平移部分（注意负号）
        view.m[3][0] = -xaxis.dot(position);
        view.m[3][1] = -yaxis.dot(position);
        view.m[3][2] = -zaxis.dot(position);
        view.m[3][3] = 1.0f;
        
        return view;
    }
    
    // 获取DirectX风格投影矩阵
    Mat4 getProjectionMatrix() const {
        // 直接使用修改后的Mat4::perspective函数，现在它返回DirectX风格的投影矩阵
        return Mat4::perspective(fov, aspectRatio, nearPlane, farPlane);
    }
    
    // 设置视场角（FOV）
    void setFOV(float degrees) {
        // 将角度转换为弧度，并限制在合理范围内（60-120度）
        float radians = std::max(60.0f, std::min(degrees, 120.0f)) * 3.14159f / 180.0f;
        fov = radians;
    }
    
    // 获取视图投影矩阵
    Mat4 getViewProjectionMatrix() const {
        return getProjectionMatrix() * getViewMatrix();
    }
    
    // 移动相机
    void move(Direction direction, float distance) {
        switch (direction) {
            case FORWARD:
                position = position + front * distance;
                break;
            case BACKWARD:
                position = position - front * distance;
                break;
            case LEFT:
                position = position - right * distance;
                break;
            case RIGHT:
                position = position + right * distance;
                break;
            case UP:
                position = position + worldUp * distance;
                break;
            case DOWN:
                position = position - worldUp * distance;
                break;
        }
    }
    
    // 旋转相机
    void rotate(float yawOffset, float pitchOffset) {
        // 反转yaw的偏移量，使得鼠标左移对应视角左转，鼠标右移对应视角右转
        yaw -= yawOffset;
        // 反转pitch的偏移量，使得鼠标上移对应视角上抬，鼠标下移对应视角下垂
        pitch -= pitchOffset;
        
        // 限制俯仰角，防止万向节锁，但不限制偏航角
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
        // 不再限制yaw，允许无限制旋转
        
        updateCameraVectors();
    }
    
    // 设置视场角
    void setFov(float degrees) {
        fov = degrees * 3.14159f / 180.0f;
    }
    
    // 设置宽高比
    void setAspectRatio(float ratio) {
        aspectRatio = ratio;
    }
    
    // 设置透视参数
    void setPerspective(float fovDegrees, float aspect, float nearVal, float farVal) {
        fov = fovDegrees * 3.14159f / 180.0f;
        aspectRatio = aspect;
        nearPlane = nearVal;
        farPlane = farVal;
    }
    
private:
    // 更新相机向量
    void updateCameraVectors() {
        // 计算新的前向量
        Vec3 newFront;
        newFront.x = cos(yaw * 3.14159f / 180.0f) * cos(pitch * 3.14159f / 180.0f);
        newFront.y = sin(pitch * 3.14159f / 180.0f);
        newFront.z = sin(yaw * 3.14159f / 180.0f) * cos(pitch * 3.14159f / 180.0f);
        front = newFront.normalize();
        
        // 重新计算右向量和上向量
        right = front.cross(worldUp).normalize();
        up = right.cross(front).normalize();
    }
};

#endif // CAMERA_H
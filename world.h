#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <random>
#include <cmath>
#include <map>
#include <string>
#include <Windows.h>
#include "math3d.h"
#include "camera.h"
#include "renderer.h"
#include "ui_manager.h" // 添加UI管理器头文件

// 前向声明
class Renderer;

// 世界类
class World {
private:
    int width;  // X轴方向的大小
    int height; // Y轴方向的大小
    int depth;  // Z轴方向的大小
    std::vector<Block> blocks; // 方块数组
    unsigned int worldSeed; // 存储世界种子
    bool isSuperFlat; // 是否为超平坦世界
    BlockType superFlatBlockType; // 超平坦世界的方块类型
    
    // 出生点坐标
    int spawnX;
    int spawnY;
    int spawnZ;
    
    // X-ray模式
    bool xrayMode = false;
    
    // 噪声生成器（用于地形生成）
    std::mt19937 rng;
    
    // 获取方块索引
    int getIndex(int x, int y, int z) const {
        return (z * width * height) + (y * width) + x;
    }
    
    // 检查坐标是否在世界范围内
    public:
    bool isInBounds(int x, int y, int z) const {
        return x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth;
    }
    
    // 获取世界尺寸
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getDepth() const { return depth; }
    
    // 获取世界种子
    unsigned int getSeed() const { return worldSeed; }
    
    // 获取方块（常量引用版本，用于渲染）
    const Block& getBlockConst(int x, int y, int z) const {
        if (isInBounds(x, y, z)) {
            return blocks[getIndex(x, y, z)];
        }
        static Block airBlock(BLOCK_AIR);
        return airBlock;
    }
    
    // 简化版柏林噪声函数
    float perlinNoise(float x, float z) {
        // 使用简化的柏林噪声实现
        x = x * 0.01f;
        z = z * 0.01f;
        
        // 获取整数部分和小数部分
        int xi = static_cast<int>(x);
        int zi = static_cast<int>(z);
        float xf = x - xi;
        float zf = z - zi;
        
        // 平滑插值函数
        auto fade = [](float t) -> float {
            return t * t * t * (t * (t * 6 - 15) + 10); // 平滑曲线
        };
        
        // 应用平滑插值
        float u = fade(xf);
        float v = fade(zf);
        
        // 使用伪随机函数生成四个角的梯度
        auto hash = [this](int x, int z) -> float {
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            std::mt19937 localRng(x * 1664525 + z * 1013904223 + 1013904223);
            return dist(localRng) * 2.0f - 1.0f;
        };
        
        // 获取四个角的值
        float v00 = hash(xi, zi);
        float v10 = hash(xi + 1, zi);
        float v01 = hash(xi, zi + 1);
        float v11 = hash(xi + 1, zi + 1);
        
        // 双线性插值
        float x1 = v00 + u * (v10 - v00);
        float x2 = v01 + u * (v11 - v01);
        float y = x1 + v * (x2 - x1);
        
        return y;
    }
    
    // 生成地形高度图（使用改进的柏林噪声）
    std::vector<int> generateHeightMap() {
        std::vector<int> heightMap(width * depth);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // 基础高度和振幅参数
        float baseHeight = height * 0.5f;
        float amplitude1 = height * 0.25f; // 大尺度地形
        // 使用这些变量来计算不同尺度的地形影响
        float amplitude2 = height * 0.15f; // 中尺度地形
        float amplitude3 = height * 0.05f; // 小尺度地形
        
        // 生成几个特殊地形特征（山脉、峡谷等）
        const int featurePoints = 5;
        std::vector<Vec3> features;
        std::vector<float> featureHeights;
        std::vector<float> featureRadii;
        
        for (int i = 0; i < featurePoints; i++) {
            features.push_back(Vec3(
                dist(rng) * width,
                0,
                dist(rng) * depth
            ));
            // 随机高度修饰符（正值为山脉，负值为峡谷）
            featureHeights.push_back((dist(rng) * 2.0f - 0.5f) * height * 0.3f);
            // 随机影响半径
            featureRadii.push_back(width * (0.15f + dist(rng) * 0.25f));
        }
        
        // 为每个x,z坐标计算高度
        for (int z = 0; z < depth; z++) {
            for (int x = 0; x < width; x++) {
                // 使用多层柏林噪声生成基础地形
                float noise1 = perlinNoise(x * 1.0f, z * 1.0f);       // 大尺度地形
                float noise2 = perlinNoise(x * 2.0f, z * 2.0f) * 0.5f; // 中尺度地形
                float noise3 = perlinNoise(x * 4.0f, z * 4.0f) * 0.25f; // 小尺度地形
                
                // 组合不同尺度的噪声，使用所有振幅变量
                float combinedNoise = noise1 + (noise2 * amplitude2 / amplitude1) + (noise3 * amplitude3 / amplitude1);
                
                // 计算基础高度
                float baseTerrainHeight = baseHeight + combinedNoise * amplitude1;
                
                // 应用特殊地形特征
                float featureInfluence = 0.0f;
                for (size_t i = 0; i < features.size(); i++) {
                    float dx = x - features[i].x;
                    float dz = z - features[i].z;
                    float distance = std::sqrt(dx * dx + dz * dz);
                    float radius = featureRadii[i];
                    
                    // 如果在影响半径内，应用高度修饰
                    if (distance < radius) {
                        float factor = 1.0f - (distance / radius); // 0到1之间的值
                        factor = factor * factor * (3 - 2 * factor); // 平滑过渡函数
                        featureInfluence += featureHeights[i] * factor;
                    }
                }
                
                // 应用特征影响并转换为整数高度
                int h = static_cast<int>(baseTerrainHeight + featureInfluence);
                
                // 限制高度范围
                h = std::max(height / 4, std::min(h, height - 4));
                
                heightMap[z * width + x] = h;
            }
        }
        
        // 应用平滑滤波器
        std::vector<int> smoothedHeightMap = heightMap;
        for (int z = 1; z < depth - 1; z++) {
            for (int x = 1; x < width - 1; x++) {
                // 3x3平均平滑
                int sum = 0;
                for (int dz = -1; dz <= 1; dz++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        sum += heightMap[(z + dz) * width + (x + dx)];
                    }
                }
                smoothedHeightMap[z * width + x] = sum / 9;
            }
        }
        
        return smoothedHeightMap;
    }
    
    // 生成矿洞
    void generateCaves() {
        std::cout << "Generating caves..." << std::endl;
        
        // 使用世界种子初始化随机数生成器
        std::mt19937 caveRng(worldSeed + 12345);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // 如果世界太小，跳过矿洞生成
        if (width <= 1 || height <= 1 || depth <= 1) {
            std::cout << "World too small, skipping cave generation." << std::endl;
            return;
        }
        
        std::uniform_int_distribution<int> radiusDist(2, 4);
        std::uniform_int_distribution<int> lengthDist(5, std::max(5, 30)); // 增加矿洞长度
        std::uniform_int_distribution<int> branchDist(0, std::max(0, 4));  // 增加分支数量
        
        // 矿洞数量与世界大小成正比
        int caveCount = (width * depth) / 400; // 增加矿洞数量
        // 确保至少有几个矿洞
        caveCount = std::max(1, caveCount);
        
        std::cout << "Creating " << caveCount << " cave systems..." << std::endl;
        
        // 存储所有矿洞的起点和终点，用于后续连接
        std::vector<Vec3> caveStartPoints;
        std::vector<Vec3> caveEndPoints;
        
        // 预先计算地表高度图和水面高度图，用于防止破坏地表结构
        std::vector<int> surfaceHeightMap(width * depth, 0);
        std::vector<bool> isWaterOrSand(width * depth, false);
        
        std::cout << "Calculating surface map for cave protection..." << std::endl;
        for (int z = 0; z < depth; z++) {
            for (int x = 0; x < width; x++) {
                // 查找地表高度
                for (int y = height - 1; y >= 0; y--) {
                    if (isInBounds(x, y, z)) {
                        BlockType blockType = blocks[getIndex(x, y, z)].type;
                        if (blockType != BLOCK_AIR) {
                            surfaceHeightMap[z * width + x] = y;
                            
                            // 检查是否为水面或沙滩
                            if (blockType == BLOCK_WATER || blockType == BLOCK_SAND) {
                                isWaterOrSand[z * width + x] = true;
                            }
                            break;
                        }
                    }
                }
            }
        }
        
        // 生成多个矿洞系统
        for (int cave = 0; cave < caveCount; cave++) {
            // 随机选择起点，避免太靠近地表
            int startX = std::uniform_int_distribution<int>(5, std::max(5, width - 6))(caveRng);
            int startY = std::uniform_int_distribution<int>(5, std::max(5, height / 2))(caveRng);
            int startZ = std::uniform_int_distribution<int>(5, std::max(5, depth - 6))(caveRng);
            
            // 检查起点是否在水面或沙滩下方
            int surfaceHeight = surfaceHeightMap[startZ * width + startX];
            bool isSurfaceWaterOrSand = isWaterOrSand[startZ * width + startX];
            
            // 如果在水面或沙滩下方，降低起点高度以避免破坏结构
            if (isSurfaceWaterOrSand) {
                // 确保矿洞起点至少在水面/沙滩下方8个方块
                startY = std::min(startY, surfaceHeight - 8);
            } else {
                // 对于普通地形，确保矿洞起点至少在地表下方5个方块
                startY = std::min(startY, surfaceHeight - 5);
            }
            
            // 确保起点不会太低
            startY = std::max(5, startY);
            
            // 记录起点
            caveStartPoints.push_back(Vec3(startX, startY, startZ));
            
            // 生成矿洞路径，传递地表高度图和水面/沙滩标记
            Vec3 endPoint = generateCavePath(startX, startY, startZ, caveRng, surfaceHeightMap, isWaterOrSand);
            
            // 记录终点
            caveEndPoints.push_back(endPoint);
            
            // 显示进度
            if (cave % 10 == 0 || cave == caveCount - 1) {
                int percent = static_cast<int>((cave + 1) * 100 / caveCount);
                std::cout << "Cave generation: " << percent << "% (" << (cave + 1) << "/" << caveCount << ")" << std::endl;
            }
        }
        
        // 连接部分矿洞系统，创建更大的网络
        std::cout << "Connecting cave systems..." << std::endl;
        int connectionCount = caveCount / 3; // 连接约1/3的矿洞
        
        for (int i = 0; i < connectionCount; i++) {
            // 随机选择两个矿洞
            int cave1 = std::uniform_int_distribution<int>(0, std::max(0, caveCount - 1))(caveRng);
            int cave2 = std::uniform_int_distribution<int>(0, std::max(0, caveCount - 1))(caveRng);
            
            // 确保选择不同的矿洞
            if (cave1 == cave2) {
                cave2 = (cave2 + 1) % caveCount;
            }
            
            // 连接一个矿洞的终点到另一个的起点
            Vec3 start = caveEndPoints[cave1];
            Vec3 end = caveStartPoints[cave2];
            
            // 计算方向向量
            Vec3 dir = end - start;
            float distance = dir.length();
            dir = dir.normalize();
            
            // 如果距离太远，跳过连接
            if (distance > 40) {
                continue;
            }
            
            // 创建连接隧道
            float x = start.x;
            float y = start.y;
            float z = start.z;
            
            int radius = std::uniform_int_distribution<int>(2, std::max(2, 3))(caveRng);
            int steps = static_cast<int>(distance);
            
            for (int step = 0; step < steps; step++) {
                // 检查当前位置是否在水面或沙滩下方
                int ix = static_cast<int>(x);
                int iy = static_cast<int>(y);
                int iz = static_cast<int>(z);
                
                if (isInBounds(ix, 0, iz)) {
                    int surfaceHeight = surfaceHeightMap[iz * width + ix];
                    bool isSurfaceWaterOrSand = isWaterOrSand[iz * width + ix];
                    
                    // 如果接近水面或沙滩，调整方向向下
                    int minDepth = isSurfaceWaterOrSand ? 8 : 5;
                    if (iy > surfaceHeight - minDepth) {
                        // 跳过此位置的挖掘，继续前进
                        x += dir.x;
                        y += dir.y;
                        z += dir.z;
                        continue;
                    }
                }
                
                // 挖掉当前位置周围的方块形成隧道
                carveSphere(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), radius);
                
                // 移动到下一个位置
                x += dir.x;
                y += dir.y;
                z += dir.z;
                
                // 如果超出世界范围，停止生成
                if (x < 5 || x >= width - 5 || y < 5 || y >= height - 5 || z < 5 || z >= depth - 5) {
                    break;
                }
                
                // 随机改变方向（轻微）
                if (step % 5 == 0) {
                    dir.x = dir.x * 0.9f + (dist(caveRng) * 0.2f - 0.1f);
                    dir.y = dir.y * 0.9f + (dist(caveRng) * 0.2f - 0.1f);
                    dir.z = dir.z * 0.9f + (dist(caveRng) * 0.2f - 0.1f);
                    dir = dir.normalize();
                }
            }
        }
        
        // 修复水泄漏问题
        fixWaterLeaks();
        
        // 修复沙滩结构
        fixSandStructures();
        
        std::cout << "Cave generation complete!" << std::endl;
    }
    
    // 生成单个矿洞路径，返回终点坐标
    Vec3 generateCavePath(int startX, int startY, int startZ, std::mt19937& caveRng,
                          const std::vector<int>& surfaceHeightMap, const std::vector<bool>& isWaterOrSand) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        std::uniform_int_distribution<int> radiusDist(2, 4);
        std::uniform_int_distribution<int> lengthDist(5, std::max(5, 30));
        std::uniform_int_distribution<int> branchDist(0, std::max(0, 4));
        
        // 初始方向
        float dirX = dist(caveRng) * 2.0f - 1.0f;
        float dirY = dist(caveRng) * 0.5f - 0.25f; // 垂直方向变化较小
        float dirZ = dist(caveRng) * 2.0f - 1.0f;
        
        // 标准化方向向量
        float length = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
        dirX /= length;
        dirY /= length;
        dirZ /= length;
        
        // 路径长度
        int pathLength = lengthDist(caveRng);
        
        // 矿洞半径
        int radius = radiusDist(caveRng);
        
        // 生成矿洞路径
        float x = static_cast<float>(startX);
        float y = static_cast<float>(startY);
        float z = static_cast<float>(startZ);
        
        // 记录终点位置
        float endX = x;
        float endY = y;
        float endZ = z;
        
        for (int step = 0; step < pathLength; step++) {
            // 检查当前位置是否在水面或沙滩下方
            int ix = static_cast<int>(x);
            int iy = static_cast<int>(y);
            int iz = static_cast<int>(z);
            
            bool skipCarving = false;
            if (isInBounds(ix, 0, iz)) {
                int surfaceHeight = surfaceHeightMap[iz * width + ix];
                bool isSurfaceWaterOrSand = isWaterOrSand[iz * width + ix];
                
                // 如果接近水面或沙滩，调整方向向下
                int minDepth = isSurfaceWaterOrSand ? 8 : 5;
                if (iy > surfaceHeight - minDepth) {
                    // 对于水面或沙滩，强制向下调整方向
                    if (isSurfaceWaterOrSand) {
                        dirY = -std::abs(dirY) * 1.8f; // 更强的向下力
                    } else {
                        dirY = -std::abs(dirY) * 1.5f; // 强制向下
                    }
                    
                    // 标准化方向向量
                    float len = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
                    dirX /= len;
                    dirY /= len;
                    dirZ /= len;
                    
                    // 如果太接近水面或沙滩，跳过此位置的挖掘
                    if (iy > surfaceHeight - 3) {
                        skipCarving = true;
                    }
                }
            }
            
            // 挖掉当前位置周围的方块形成矿洞
            if (!skipCarving) {
                carveSphere(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), radius);
            }
            
            // 移动到下一个位置
            x += dirX;
            y += dirY;
            z += dirZ;
            
            // 更新终点位置
            endX = x;
            endY = y;
            endZ = z;
            
            // 如果超出世界范围或接近地表，停止生成
            if (x < 5 || x >= width - 5 || y < 5 || y >= height - 5 || z < 5 || z >= depth - 5) {
                break;
            }
            
            // 随机改变方向
            if (step % 5 == 0) {
                dirX = dirX * 0.8f + (dist(caveRng) * 2.0f - 1.0f) * 0.2f;
                dirY = dirY * 0.8f + (dist(caveRng) * 0.5f - 0.25f) * 0.2f;
                dirZ = dirZ * 0.8f + (dist(caveRng) * 2.0f - 1.0f) * 0.2f;
                
                // 标准化方向向量
                length = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
                dirX /= length;
                dirY /= length;
                dirZ /= length;
                
                // 随机改变半径
                radius = radiusDist(caveRng);
            }
            
            // 随机生成分支
            if (step > 3 && dist(caveRng) < 0.15f) { // 增加分支概率
                int branches = branchDist(caveRng);
                for (int b = 0; b < branches; b++) {
                    // 生成分支
                    float branchDirX = dirX * 0.5f + (dist(caveRng) * 2.0f - 1.0f) * 0.5f;
                    float branchDirY = dirY * 0.5f + (dist(caveRng) * 0.5f - 0.25f) * 0.5f;
                    float branchDirZ = dirZ * 0.5f + (dist(caveRng) * 2.0f - 1.0f) * 0.5f;
                    
                    // 标准化分支方向向量
                    float branchLength = std::sqrt(branchDirX * branchDirX + branchDirY * branchDirY + branchDirZ * branchDirZ);
                    branchDirX /= branchLength;
                    branchDirY /= branchLength;
                    branchDirZ /= branchLength;
                    
                    // 分支起点稍微偏离主路径
                    float branchX = x + branchDirX * 2.0f;
                    float branchY = y + branchDirY * 2.0f;
                    float branchZ = z + branchDirZ * 2.0f;
                    
                    // 递归生成分支路径（较短）
                    int branchRadius = radius - 1;
                    if (branchRadius >= 2) {
                        int branchPathLength = lengthDist(caveRng) / 2;
                        
                        for (int branchStep = 0; branchStep < branchPathLength; branchStep++) {
                            // 检查当前分支位置是否在水面或沙滩下方
                            int ibx = static_cast<int>(branchX);
                            int iby = static_cast<int>(branchY);
                            int ibz = static_cast<int>(branchZ);
                            
                            bool skipBranchCarving = false;
                            if (isInBounds(ibx, 0, ibz)) {
                                int surfaceHeight = surfaceHeightMap[ibz * width + ibx];
                                bool isSurfaceWaterOrSand = isWaterOrSand[ibz * width + ibx];
                                
                                // 如果接近水面或沙滩，调整方向向下
                                int minDepth = isSurfaceWaterOrSand ? 8 : 5;
                                if (iby > surfaceHeight - minDepth) {
                                    // 对于水面或沙滩，强制向下调整方向
                                    if (isSurfaceWaterOrSand) {
                                        branchDirY = -std::abs(branchDirY) * 1.8f; // 更强的向下力
                                    } else {
                                        branchDirY = -std::abs(branchDirY) * 1.5f; // 强制向下
                                    }
                                    
                                    // 标准化方向向量
                                    float len = std::sqrt(branchDirX * branchDirX + branchDirY * branchDirY + branchDirZ * branchDirZ);
                                    branchDirX /= len;
                                    branchDirY /= len;
                                    branchDirZ /= len;
                                    
                                    // 如果太接近水面或沙滩，跳过此位置的挖掘
                                    if (iby > surfaceHeight - 3) {
                                        skipBranchCarving = true;
                                    }
                                }
                            }
                            
                            // 挖掉当前位置周围的方块形成矿洞
                            if (!skipBranchCarving) {
                                carveSphere(static_cast<int>(branchX), static_cast<int>(branchY), static_cast<int>(branchZ), branchRadius);
                            }
                            
                            // 移动到下一个位置
                            branchX += branchDirX;
                            branchY += branchDirY;
                            branchZ += branchDirZ;
                            
                            // 如果超出世界范围，停止生成
                            if (branchX < 5 || branchX >= width - 5 || branchY < 5 || branchY >= height - 5 || branchZ < 5 || branchZ >= depth - 5) {
                                break;
                            }
                            
                            // 随机改变方向
                            if (branchStep % 3 == 0) {
                                branchDirX = branchDirX * 0.8f + (dist(caveRng) * 2.0f - 1.0f) * 0.2f;
                                branchDirY = branchDirY * 0.8f + (dist(caveRng) * 0.5f - 0.25f) * 0.2f;
                                branchDirZ = branchDirZ * 0.8f + (dist(caveRng) * 2.0f - 1.0f) * 0.2f;
                                
                                // 标准化方向向量
                                branchLength = std::sqrt(branchDirX * branchDirX + branchDirY * branchDirY + branchDirZ * branchDirZ);
                                branchDirX /= branchLength;
                                branchDirY /= branchLength;
                                branchDirZ /= branchLength;
                            }
                        }
                    }
                }
            }
        }
        
        return Vec3(endX, endY, endZ);
    }
    
    // 挖掉球形区域内的方块形成矿洞
    void carveSphere(int centerX, int centerY, int centerZ, int radius) {
        // 遍历球体范围内的所有方块
        for (int x = centerX - radius; x <= centerX + radius; x++) {
            for (int y = centerY - radius; y <= centerY + radius; y++) {
                for (int z = centerZ - radius; z <= centerZ + radius; z++) {
                    // 检查是否在世界范围内
                    if (!isInBounds(x, y, z)) {
                        continue;
                    }
                    
                    // 计算到中心的距离平方
                    int dx = x - centerX;
                    int dy = y - centerY;
                    int dz = z - centerZ;
                    int distSq = dx * dx + dy * dy + dz * dz;
                    
                    // 如果在球体内部，挖掉方块（设置为空气）
                    if (distSq <= radius * radius) {
                        // 不要挖掉基岩
                        if (y > 0 && blocks[getIndex(x, y, z)].type != BLOCK_BEDROCK) {
                            // 标记此位置为矿洞
                            blocks[getIndex(x, y, z)] = Block(BLOCK_AIR);
                        }
                    }
                }
            }
        }
    }
    
    // 修复水泄漏问题
    void fixWaterLeaks() {
        std::cout << "Fixing water leaks..." << std::endl;
        
        // 遍历所有水方块
        for (int z = 0; z < depth; z++) {
            for (int x = 0; x < width; x++) {
                bool isWaterSurface = false;
                int waterSurfaceY = -1;
                
                // 从上往下找到水面
                for (int y = height - 1; y >= 0; y--) {
                    if (isInBounds(x, y, z) && blocks[getIndex(x, y, z)].type == BLOCK_WATER) {
                        isWaterSurface = true;
                        waterSurfaceY = y;
                        break;
                    }
                }
                
                // 如果找到水面，检查下方是否有空气（泄漏）
                if (isWaterSurface && waterSurfaceY > 0) {
                    // 从水面向下检查
                    for (int y = waterSurfaceY; y >= 0; y--) {
                        // 如果遇到实体方块，停止检查
                        if (blocks[getIndex(x, y, z)].type != BLOCK_AIR && blocks[getIndex(x, y, z)].type != BLOCK_WATER) {
                            break;
                        }
                        
                        // 如果是空气，填充水
                        if (blocks[getIndex(x, y, z)].type == BLOCK_AIR) {
                            blocks[getIndex(x, y, z)] = Block(BLOCK_WATER);
                        }
                    }
                }
            }
        }
        
        std::cout << "Water leaks fixed!" << std::endl;
    }
    
    // 修复沙滩结构
    void fixSandStructures() {
        std::cout << "Fixing sand structures..." << std::endl;
        
        // 遍历所有方块
        for (int z = 0; z < depth; z++) {
            for (int x = 0; x < width; x++) {
                bool isSandSurface = false;
                int sandSurfaceY = -1;
                
                // 从上往下找到沙滩表面
                for (int y = height - 1; y >= 0; y--) {
                    if (isInBounds(x, y, z) && blocks[getIndex(x, y, z)].type == BLOCK_SAND) {
                        isSandSurface = true;
                        sandSurfaceY = y;
                        break;
                    }
                }
                
                // 如果找到沙滩表面，检查下方是否有空气（被矿洞破坏）
                if (isSandSurface && sandSurfaceY > 0) {
                    // 检查下方是否有空气
                    bool hasAirGap = false;
                    int airGapY = -1;
                    
                    // 从沙滩表面向下检查
                    for (int y = sandSurfaceY - 1; y >= std::max(0, sandSurfaceY - 5); y--) {
                        if (isInBounds(x, y, z) && blocks[getIndex(x, y, z)].type == BLOCK_AIR) {
                            hasAirGap = true;
                            airGapY = y;
                            break;
                        }
                    }
                    
                    // 如果发现空气间隙，填充沙子
                    if (hasAirGap) {
                        for (int y = sandSurfaceY - 1; y >= airGapY; y--) {
                            if (blocks[getIndex(x, y, z)].type == BLOCK_AIR) {
                                blocks[getIndex(x, y, z)] = Block(BLOCK_SAND);
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << "Sand structures fixed!" << std::endl;
    }
    
    // 生成矿物
    void generateOres() {
        std::cout << "Generating ores..." << std::endl;
        
        // 使用世界种子初始化随机数生成器
        std::mt19937 oreRng(worldSeed + 54321);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // 如果世界太小，跳过矿物生成
        if (width <= 1 || height <= 1 || depth <= 1) {
            std::cout << "World too small, skipping ore generation." << std::endl;
            return;
        }
        
        // 定义矿物生成参数
        struct OreDefinition {
            BlockType oreType;
            int minHeight;
            int maxHeight;
            float frequency;   // 生成频率（0-1）
            int minSize;       // 最小矿脉大小
            int maxSize;       // 最大矿脉大小
            const char* name;  // 矿物名称
            bool preferCaves;  // 是否偏好在矿洞附近生成
            float rarity;      // 稀有度 (0-1)，值越高越稀有
            float caveBonus;   // 在洞穴附近生成的概率加成
        };
        
        // 定义不同矿物的生成参数
        OreDefinition ores[] = {
            // 常见矿物 - 分布广泛
            {BLOCK_COAL_ORE,     5, height - 15, 0.18f, 5, 16, "Coal", false, 0.1f, 1.2f},
            {BLOCK_IRON_ORE,     2, height / 2,  0.15f, 3, 10, "Iron", false, 0.3f, 1.3f},
            
            // 中等稀有度矿物 - 特定高度分布
            {BLOCK_GOLD_ORE,     2, height / 3,  0.08f, 2, 8,  "Gold", true,  0.5f, 1.5f},
            {BLOCK_REDSTONE_ORE, 2, height / 4,  0.12f, 3, 9,  "Redstone", true, 0.4f, 1.4f},
            
            // 稀有矿物 - 低高度或洞穴附近
            {BLOCK_DIAMOND_ORE,  2, height / 6,  0.05f, 2, 6,  "Diamond", true, 0.8f, 2.0f},
            {BLOCK_EMERALD_ORE,  2, height / 3,  0.04f, 1, 3,  "Emerald", true, 0.9f, 2.2f},
            
            // 特殊矿物 - 极其稀有，只在特定条件下生成
            {BLOCK_LAVA,         2, height / 8,  0.03f, 3, 7,  "Lava Pocket", true, 0.7f, 1.8f}
        };
        
        // 遍历每种矿物类型
        for (const auto& ore : ores) {
            // 矿脉数量与世界大小、频率和稀有度成反比
            int veinCount = static_cast<int>((width * depth * ore.frequency) / (100 * (0.5f + ore.rarity * 0.5f)));
            // 确保至少有几个矿脉
            veinCount = std::max(3, veinCount);
            
            std::cout << "Generating " << veinCount << " veins of " << ore.name << "..." << std::endl;
            
            // 生成多个矿脉
            for (int vein = 0; vein < veinCount; vein++) {
                // 根据高度分布计算生成概率
                // 使用正态分布使矿物在理想高度附近更常见
                float idealHeight = (ore.minHeight + ore.maxHeight) / 2.0f;
                float heightRange = (ore.maxHeight - ore.minHeight) / 2.0f;
                
                // 随机选择起点
                int startX = std::uniform_int_distribution<int>(0, std::max(0, width - 1))(oreRng);
                int startZ = std::uniform_int_distribution<int>(0, std::max(0, depth - 1))(oreRng);
                
                // 使用高斯分布确定Y坐标，使矿物在理想高度附近更集中
                std::normal_distribution<float> heightDist(idealHeight, heightRange / 2.0f);
                int startY = static_cast<int>(std::clamp(heightDist(oreRng), 
                                                        static_cast<float>(ore.minHeight), 
                                                        static_cast<float>(ore.maxHeight)));
                
                // 如果矿物偏好在矿洞附近生成，尝试找到合适的位置
                bool nearCave = false;
                if (ore.preferCaves) {
                    // 尝试几次找到靠近矿洞的位置
                    for (int attempt = 0; attempt < 8; attempt++) {
                        int testX = std::uniform_int_distribution<int>(0, std::max(0, width - 1))(oreRng);
                        int testZ = std::uniform_int_distribution<int>(0, std::max(0, depth - 1))(oreRng);
                        int testY = static_cast<int>(std::clamp(heightDist(oreRng), 
                                                               static_cast<float>(ore.minHeight), 
                                                               static_cast<float>(ore.maxHeight)));
                        
                        // 检查周围是否有矿洞
                        int searchRadius = 5; // 增加搜索半径
                        for (int dy = -searchRadius; dy <= searchRadius && !nearCave; dy++) {
                            for (int dx = -searchRadius; dx <= searchRadius && !nearCave; dx++) {
                                for (int dz = -searchRadius; dz <= searchRadius && !nearCave; dz++) {
                                    if (isInBounds(testX + dx, testY + dy, testZ + dz) && 
                                        blocks[getIndex(testX + dx, testY + dy, testZ + dz)].type == BLOCK_AIR) {
                                        nearCave = true;
                                        startX = testX;
                                        startY = testY;
                                        startZ = testZ;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        if (nearCave) break;
                    }
                }
                
                // 根据是否靠近洞穴调整矿脉大小
                float sizeMultiplier = nearCave ? ore.caveBonus : 1.0f;
                
                // 随机矿脉大小，受洞穴加成影响
                int veinBaseSize = std::uniform_int_distribution<int>(ore.minSize, std::max(ore.minSize, ore.maxSize))(oreRng);
                int veinSize = static_cast<int>(veinBaseSize * sizeMultiplier);
                
                // 应用稀有度影响 - 稀有矿物矿脉更小
                veinSize = static_cast<int>(veinSize * (1.0f - ore.rarity * 0.3f));
                veinSize = std::max(1, veinSize);
                
                // 生成矿脉
                generateOreVein(startX, startY, startZ, ore.oreType, veinSize, oreRng, ore.rarity);
            }
            
            std::cout << ore.name << " ore generation complete!" << std::endl;
        }
        
        // 生成随机分散的小型矿物点
        generateScatteredOres(oreRng);
        
        std::cout << "Ore generation complete!" << std::endl;
    }
    
    // 生成随机分散的小型矿物点
    void generateScatteredOres(std::mt19937& oreRng) {
        std::cout << "Generating scattered ore deposits..." << std::endl;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // 定义散点矿物生成参数
        struct ScatteredOreDefinition {
            BlockType oreType;
            int minHeight;
            int maxHeight;
            float chance;      // 每个位置生成的概率
            const char* name;  // 矿物名称
        };
        
        // 定义不同散点矿物的生成参数
        ScatteredOreDefinition scatteredOres[] = {
            {BLOCK_COAL_ORE,     height / 4, height - 10, 0.006f, "Scattered Coal"},
            {BLOCK_IRON_ORE,     5, height / 2, 0.004f, "Scattered Iron"},
            {BLOCK_GOLD_ORE,     5, height / 3, 0.002f, "Scattered Gold"},
            {BLOCK_REDSTONE_ORE, 5, height / 4, 0.003f, "Scattered Redstone"},
            {BLOCK_DIAMOND_ORE,  5, height / 6, 0.001f, "Scattered Diamond"}
        };
        
        // 遍历一部分世界方块
        int checkInterval = 6; // 每隔几个方块检查一次，减少计算量
        int totalScattered = 0;
        
        for (int z = 0; z < depth; z += checkInterval) {
            for (int y = 0; y < height; y += checkInterval) {
                for (int x = 0; x < width; x += checkInterval) {
                    // 只在石头方块中生成
                    if (isInBounds(x, y, z) && blocks[getIndex(x, y, z)].type == BLOCK_STONE) {
                        // 检查是否靠近洞穴
                        bool nearCave = false;
                        for (int dy = -3; dy <= 3 && !nearCave; dy++) {
                            for (int dx = -3; dx <= 3 && !nearCave; dx++) {
                                for (int dz = -3; dz <= 3 && !nearCave; dz++) {
                                    if (isInBounds(x + dx, y + dy, z + dz) && 
                                        blocks[getIndex(x + dx, y + dy, z + dz)].type == BLOCK_AIR) {
                                        nearCave = true;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        // 遍历每种散点矿物
                        for (const auto& ore : scatteredOres) {
                            // 检查高度范围
                            if (y >= ore.minHeight && y <= ore.maxHeight) {
                                // 计算生成概率，靠近洞穴时提高概率
                                float chance = ore.chance * (nearCave ? 2.0f : 1.0f);
                                
                                // 根据高度调整概率 - 在理想高度处概率最大
                                float idealHeight = (ore.minHeight + ore.maxHeight) / 2.0f;
                                float heightFactor = 1.0f - std::abs(y - idealHeight) / (ore.maxHeight - ore.minHeight);
                                chance *= (0.5f + heightFactor * 0.5f);
                                
                                // 随机决定是否生成
                                if (dist(oreRng) < chance) {
                                    // 在当前位置生成矿物
                                    blocks[getIndex(x, y, z)] = Block(ore.oreType);
                                    totalScattered++;
                                    
                                    // 有小概率在周围也生成同类矿物
                                    if (dist(oreRng) < 0.3f) {
                                        for (int dy2 = -1; dy2 <= 1; dy2++) {
                                            for (int dx2 = -1; dx2 <= 1; dx2++) {
                                                for (int dz2 = -1; dz2 <= 1; dz2++) {
                                                    // 跳过中心方块（已经设置了）
                                                    if (dx2 == 0 && dy2 == 0 && dz2 == 0) continue;
                                                    
                                                    // 随机选择是否在此位置放置矿石
                                                    if (dist(oreRng) < 0.15f && 
                                                        isInBounds(x + dx2, y + dy2, z + dz2) && 
                                                        blocks[getIndex(x + dx2, y + dy2, z + dz2)].type == BLOCK_STONE) {
                                                        blocks[getIndex(x + dx2, y + dy2, z + dz2)] = Block(ore.oreType);
                                                        totalScattered++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    
                                    // 一个位置只生成一种矿物
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        std::cout << "Generated " << totalScattered << " scattered ore blocks." << std::endl;
    }
    
    // 生成单个矿脉
    void generateOreVein(int startX, int startY, int startZ, BlockType oreType, int veinSize, std::mt19937& oreRng, float rarity) {
        std::uniform_int_distribution<int> dirDist(-1, 1);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // 从起点开始生成矿脉
        int x = startX;
        int y = startY;
        int z = startZ;
        
        // 放置第一个矿石方块
        if (isInBounds(x, y, z) && blocks[getIndex(x, y, z)].type == BLOCK_STONE) {
            blocks[getIndex(x, y, z)] = Block(oreType);
        }
        
        // 创建更自然的矿脉形状
        float branchChance = 0.4f - rarity * 0.2f; // 稀有矿物分支更少
        float turnChance = 0.6f - rarity * 0.2f;   // 稀有矿物路径更直
        
        // 用于记录当前路径的栈
        std::vector<std::tuple<int, int, int>> pathStack;
        pathStack.push_back(std::make_tuple(x, y, z));
        
        // 已放置的矿石数量
        int placedOres = 1;
        
        // 继续生成直到达到目标大小或无法继续
        while (placedOres < veinSize && !pathStack.empty()) {
            // 获取当前位置
            std::tie(x, y, z) = pathStack.back();
            pathStack.pop_back();
            
            // 尝试向多个方向扩展
            int directions = 1 + static_cast<int>(dist(oreRng) * 3); // 1-3个方向
            
            for (int d = 0; d < directions; d++) {
                // 随机选择相邻位置，使用加权随机以创建更自然的矿脉形状
                float dirBias = dist(oreRng);
                int dx, dy, dz;
                
                if (dirBias < turnChance) {
                    // 随机方向，但偏向于水平扩展
                    dx = dirDist(oreRng);
                    dy = dirDist(oreRng) / 2; // 垂直方向变化较小
                    dz = dirDist(oreRng);
                } else {
                    // 完全随机方向
                    dx = dirDist(oreRng);
                    dy = dirDist(oreRng);
                    dz = dirDist(oreRng);
                }
                
                // 确保至少有一个方向有变化
                if (dx == 0 && dy == 0 && dz == 0) {
                    dx = dirDist(oreRng);
                    if (dx == 0) dx = 1;
                }
                
                // 移动到新位置
                int newX = x + dx;
                int newY = y + dy;
                int newZ = z + dz;
                
                // 检查是否在世界范围内
                if (!isInBounds(newX, newY, newZ)) {
                    continue;
                }
                
                // 只替换石头方块
                if (blocks[getIndex(newX, newY, newZ)].type == BLOCK_STONE) {
                    blocks[getIndex(newX, newY, newZ)] = Block(oreType);
                    placedOres++;
                    
                    // 将新位置添加到路径栈中，以便继续扩展
                    pathStack.push_back(std::make_tuple(newX, newY, newZ));
                    
                    // 创建矿簇 - 为更稀有的矿物创建更小的簇
                    float clusterChance = 0.35f - rarity * 0.2f;
                    if (dist(oreRng) < clusterChance) {
                        for (int cy = -1; cy <= 1; cy++) {
                            for (int cx = -1; cx <= 1; cx++) {
                                for (int cz = -1; cz <= 1; cz++) {
                                    // 跳过中心方块（已经设置了）
                                    if (cx == 0 && cy == 0 && cz == 0) continue;
                                    
                                    // 随机选择是否在此位置放置矿石
                                    if (dist(oreRng) < 0.25f - rarity * 0.1f && 
                                        isInBounds(newX + cx, newY + cy, newZ + cz) && 
                                        blocks[getIndex(newX + cx, newY + cy, newZ + cz)].type == BLOCK_STONE) {
                                        blocks[getIndex(newX + cx, newY + cy, newZ + cz)] = Block(oreType);
                                        placedOres++;
                                        
                                        // 检查是否已达到目标大小
                                        if (placedOres >= veinSize) {
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // 检查是否已达到目标大小
                    if (placedOres >= veinSize) {
                        return;
                    }
                    
                    // 有概率创建分支
                    if (dist(oreRng) < branchChance) {
                        // 创建分支 - 从当前位置开始新的路径
                        int branchX = newX + dirDist(oreRng);
                        int branchY = newY + dirDist(oreRng);
                        int branchZ = newZ + dirDist(oreRng);
                        
                        if (isInBounds(branchX, branchY, branchZ) && 
                            blocks[getIndex(branchX, branchY, branchZ)].type == BLOCK_STONE) {
                            blocks[getIndex(branchX, branchY, branchZ)] = Block(oreType);
                            placedOres++;
                            
                            // 将分支位置添加到路径栈中
                            pathStack.push_back(std::make_tuple(branchX, branchY, branchZ));
                            
                            // 检查是否已达到目标大小
                            if (placedOres >= veinSize) {
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 生成世界
    void generateWorld() {
        // 确保世界尺寸至少为2，防止uniform_int_distribution断言失败
        if (width < 2) width = 2;
        if (height < 2) height = 2;
        if (depth < 2) depth = 2;
        
        // 计算总方块数
        long long totalBlocks = static_cast<long long>(width) * height * depth;
        long long processedBlocks = 0;
        
        std::cout << "Generating world..." << std::endl;
        std::cout << "World size: " << width << "x" << height << "x" << depth << std::endl;
        std::cout << "Total blocks: " << totalBlocks << std::endl;
        
        // 初始化所有方块为空气
        blocks.resize(width * height * depth, Block(BLOCK_AIR));
        processedBlocks = totalBlocks; // 初始化完成
        
        // 显示进度
        std::cout << "Initialized air blocks: 100%" << std::endl;
        
        if (isSuperFlat) {
            // 生成超平坦世界 - 只有一层方块
            std::cout << "Generating superflat world..." << std::endl;
            
            long long flatBlocks = 0;
            long long totalFlatBlocks = static_cast<long long>(width) * depth;
            
            for (int z = 0; z < depth; z++) {
                for (int x = 0; x < width; x++) {
                    // 在Y=0处放置超平坦世界的方块
                    blocks[getIndex(x, 0, z)] = Block(superFlatBlockType);
                    
                    // 更新进度
                    flatBlocks++;
                    if (flatBlocks % 10000 == 0 || flatBlocks == totalFlatBlocks) {
                        int percent = static_cast<int>((flatBlocks * 100) / totalFlatBlocks);
                        std::cout << "Generating superflat surface: " << percent << "% (" 
                                  << flatBlocks << "/" << totalFlatBlocks << " blocks)" << std::endl;
                    }
                }
            }
            
            std::cout << "Updating block visibility..." << std::endl;
            // 更新所有方块的可见性
            updateBlockVisibility();
            std::cout << "World generation complete!" << std::endl;
            return;
        }
        
        // 以下是普通世界生成逻辑
        std::cout << "Generating terrain heightmap..." << std::endl;
        // 生成高度图
        auto heightMap = generateHeightMap();
        std::cout << "Heightmap generated!" << std::endl;
        
        // 计算平均高度和最高点
        int totalHeight = 0;
        int maxHeight = 0;
        for (int i = 0; i < width * depth; i++) {
            totalHeight += heightMap[i];
            maxHeight = std::max(maxHeight, heightMap[i]);
        }
        int avgHeight = totalHeight / (width * depth);
        int waterLevel = height / 3;
        int snowLevel = maxHeight - (maxHeight - avgHeight) / 3;
        
        std::cout << "Average terrain height: " << avgHeight << std::endl;
        std::cout << "Water level: " << waterLevel << std::endl;
        std::cout << "Snow level: " << snowLevel << std::endl;
        
        // 生成地形
        std::cout << "Generating terrain blocks..." << std::endl;
        long long terrainBlocks = 0;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        for (int z = 0; z < depth; z++) {
            for (int x = 0; x < width; x++) {
                int terrainHeight = heightMap[z * width + x];
                
                // 生成基岩层
                blocks[getIndex(x, 0, z)] = Block(BLOCK_BEDROCK);
                terrainBlocks++;
                
                // 生成矿石和石头层
                for (int y = 1; y < terrainHeight - 3; y++) {
                    // 默认为石头
                    BlockType blockType = BLOCK_STONE;
                    blocks[getIndex(x, y, z)] = Block(blockType);
                    terrainBlocks++;
                }
                
                // 生成土壤和草地层
                for (int y = terrainHeight - 3; y < terrainHeight; y++) {
                    BlockType blockType;
                    
                    // 根据高度和生物群系选择方块类型
                    if (y >= snowLevel) {
                        // 雪地生物群系
                        blockType = BLOCK_SNOW;
                    } else if (terrainHeight <= waterLevel + 1) {
                        // 沙地生物群系
                        blockType = BLOCK_SAND;
                        } else {
                        // 默认为泥土
                        blockType = BLOCK_DIRT;
                    }
                    
                    // 设置方块
                    blocks[getIndex(x, y, z)] = Block(blockType);
                    terrainBlocks++;
                }
                
                // 顶层方块特殊处理
                if (terrainHeight > 0) {
                    BlockType surfaceType;
                    
                    if (terrainHeight >= snowLevel) {
                        // 雪地表面
                        surfaceType = BLOCK_SNOW;
                    } else if (terrainHeight <= waterLevel + 1) {
                        // 沙地表面
                        surfaceType = BLOCK_SAND;
                    } else {
                        // 一般地表使用草方块
                        surfaceType = BLOCK_GRASS;
                        
                        // 随机生成树木
                        if (dist(rng) < 0.01f && terrainHeight < height - 10) {
                            generateTree(x, terrainHeight, z);
                        }
                    }
                    
                    blocks[getIndex(x, terrainHeight - 1, z)] = Block(surfaceType);
                    terrainBlocks++;
                }
                
                // 生成水体
                int waterBlocks = 0;
                if (terrainHeight < waterLevel) {
                    for (int y = terrainHeight; y <= waterLevel; y++) {
                        // 水面下是水方块
                        blocks[getIndex(x, y, z)] = Block(BLOCK_WATER);
                        terrainBlocks++;
                        waterBlocks++;
                    }
                }
                
                // 显示进度
                if ((x + z * width) % 1000 == 0 || (x == width - 1 && z == depth - 1)) {
                    int percent = static_cast<int>(((x + z * width) * 100) / (width * depth));
                    std::cout << "Generating terrain: " << percent << "% (" 
                              << terrainBlocks << " blocks, including " << waterBlocks << " water blocks)" << std::endl;
                }
            }
        }
        
        std::cout << "Terrain generation complete! Total blocks modified: " << terrainBlocks << std::endl;
        
        // 生成矿物
        generateOres();
        
        // 生成矿洞
        generateCaves();
        
        std::cout << "Updating block visibility..." << std::endl;
        
        // 更新所有方块的可见性
        updateBlockVisibility();
        
        // 统计矿物数量
        countOres();
        
        std::cout << "World generation complete!" << std::endl;
    }
    
    // 生成树
    void generateTree(int x, int y, int z) {
        // 检查是否有足够的空间生成树
        if (y + 4 >= height || x <= 1 || x >= width - 2 || z <= 1 || z >= depth - 2) {
            return;
        }
        
        // 生成树干
        for (int treeY = y; treeY < y + 4; treeY++) {
            blocks[getIndex(x, treeY, z)] = Block(BLOCK_WOOD);
        }
        
        // 生成树叶
        for (int leafY = y + 2; leafY < y + 5; leafY++) {
            for (int leafX = x - 1; leafX <= x + 1; leafX++) {
                for (int leafZ = z - 1; leafZ <= z + 1; leafZ++) {
                    if (isInBounds(leafX, leafY, leafZ) && 
                        blocks[getIndex(leafX, leafY, leafZ)].type == BLOCK_AIR) {
                        blocks[getIndex(leafX, leafY, leafZ)] = Block(BLOCK_LEAVES);
                    }
                }
            }
        }
    }
    
    // 检查方块是否为透明或半透明
    bool isTransparent(BlockType type) const {
        // 首先检查基础透明方块类型
        bool baseTransparent = type == BLOCK_AIR || 
               type == BLOCK_WATER || 
               type == BLOCK_LEAVES || 
               type == BLOCK_ICE ||
               type == BLOCK_LAVA ||
               type == BLOCK_SLIME;
               
        // 如果是基本透明类型，直接返回true
        if (baseTransparent) {
            return true;
        }
        
        // 对于CHANGE_BLOCK，需要检查其透明度设置
        if (type == BLOCK_CHANGE_BLOCK) {
            // 由于这里不能直接访问方块实例，我们返回true
            // 实际透明度将在渲染时根据具体方块实例进行处理
            return true;
        }
        
        return false;
    }
    
    // 更新特定方块及其周围方块的可见性
    void updateBlockVisibilityAt(int x, int y, int z) {
        // 检查坐标是否有效
        if (!isInBounds(x, y, z)) {
            return;
        }
        
        // 获取当前方块
        int index = getIndex(x, y, z);
        Block& block = blocks[index];
        
        // 如果是空气方块，它本身不可见，但需要更新周围方块的可见性
        if (block.type == BLOCK_AIR) {
            block.isVisible = false;
        } else {
            // 检查六个面是否有相邻的透明方块
            bool hasTransparentNeighbor = false;
            
            // 检查前面 (z+1)
            if (z == depth - 1 || isTransparent(blocks[getIndex(x, y, z + 1)].type)) {
                hasTransparentNeighbor = true;
            }
            
            // 检查后面 (z-1)
            if (z == 0 || isTransparent(blocks[getIndex(x, y, z - 1)].type)) {
                hasTransparentNeighbor = true;
            }
            
            // 检查左面 (x-1)
            if (x == 0 || isTransparent(blocks[getIndex(x - 1, y, z)].type)) {
                hasTransparentNeighbor = true;
            }
            
            // 检查右面 (x+1)
            if (x == width - 1 || isTransparent(blocks[getIndex(x + 1, y, z)].type)) {
                hasTransparentNeighbor = true;
            }
            
            // 检查上面 (y+1)
            if (y == height - 1 || isTransparent(blocks[getIndex(x, y + 1, z)].type)) {
                hasTransparentNeighbor = true;
            }
            
            // 检查下面 (y-1)
            if (y == 0 || isTransparent(blocks[getIndex(x, y - 1, z)].type)) {
                hasTransparentNeighbor = true;
            }
            
            // 如果方块是半透明的，始终渲染它
            if (block.type == BLOCK_WATER || block.type == BLOCK_LEAVES || block.type == BLOCK_LAVA) {
                hasTransparentNeighbor = true;
            }
            
            // 修复剔除过度问题：如果方块在玩家视野范围内，强制设置为可见
            // 这将在渲染时进一步判断，这里只是确保方块不会被错误地剔除
            
            // 设置方块可见性
            block.isVisible = hasTransparentNeighbor;
        }
        
        // 更新周围六个方块的可见性
        // 前面 (z+1)
        if (z < depth - 1) {
            updateSingleBlockVisibility(x, y, z + 1);
        }
        
        // 后面 (z-1)
        if (z > 0) {
            updateSingleBlockVisibility(x, y, z - 1);
        }
        
        // 左面 (x-1)
        if (x > 0) {
            updateSingleBlockVisibility(x - 1, y, z);
        }
        
        // 右面 (x+1)
        if (x < width - 1) {
            updateSingleBlockVisibility(x + 1, y, z);
        }
        
        // 上面 (y+1)
        if (y < height - 1) {
            updateSingleBlockVisibility(x, y + 1, z);
        }
        
        // 下面 (y-1)
        if (y > 0) {
            updateSingleBlockVisibility(x, y - 1, z);
        }
    }
    
    // 更新单个方块的可见性（不递归更新周围方块）
    void updateSingleBlockVisibility(int x, int y, int z) {
        // 检查坐标是否有效
        if (!isInBounds(x, y, z)) {
            return;
        }
        
        // 获取当前方块
        int index = getIndex(x, y, z);
        Block& block = blocks[index];
        
        // 空气方块始终不可见
        if (block.type == BLOCK_AIR) {
            block.isVisible = false;
            return;
        }
        
        // 检查六个面是否有相邻的透明方块
        bool hasTransparentNeighbor = false;
        
        // 检查前面 (z+1)
        if (z == depth - 1 || isTransparent(blocks[getIndex(x, y, z + 1)].type)) {
            hasTransparentNeighbor = true;
        }
        
        // 检查后面 (z-1)
        if (z == 0 || isTransparent(blocks[getIndex(x, y, z - 1)].type)) {
            hasTransparentNeighbor = true;
        }
        
        // 检查左面 (x-1)
        if (x == 0 || isTransparent(blocks[getIndex(x - 1, y, z)].type)) {
            hasTransparentNeighbor = true;
        }
        
        // 检查右面 (x+1)
        if (x == width - 1 || isTransparent(blocks[getIndex(x + 1, y, z)].type)) {
            hasTransparentNeighbor = true;
        }
        
        // 检查上面 (y+1)
        if (y == height - 1 || isTransparent(blocks[getIndex(x, y + 1, z)].type)) {
            hasTransparentNeighbor = true;
        }
        
        // 检查下面 (y-1)
        if (y == 0 || isTransparent(blocks[getIndex(x, y - 1, z)].type)) {
            hasTransparentNeighbor = true;
        }
        
        // 如果方块是半透明的，始终渲染它
        if (block.type == BLOCK_WATER || block.type == BLOCK_LEAVES || block.type == BLOCK_LAVA) {
            hasTransparentNeighbor = true;
        }
        
        // 设置方块可见性
        block.isVisible = hasTransparentNeighbor;
    }
    
    // 更新特定坐标方块的可见性
    void updateBlockVisibility(int x, int y, int z) {
        updateBlockVisibilityAt(x, y, z);
        
        // 确保更新下方方块的可见性（修复挖掉方块后下方方块不可见的问题）
        if (y > 0) {
            updateBlockVisibilityAt(x, y-1, z);
        }
    }
    
    // 更新所有方块可见性（只渲染可见的方块面）
    void updateBlockVisibility() {
        // 获取地表高度图（用于地下渲染优化）
        std::vector<int> surfaceHeightMap(width * depth, 0);
        for (int z = 0; z < depth; z++) {
            for (int x = 0; x < width; x++) {
                for (int y = height - 1; y >= 0; y--) {
                    BlockType blockType = blocks[getIndex(x, y, z)].type;
                    if (blockType != BLOCK_AIR && blockType != BLOCK_WATER) {
                        surfaceHeightMap[z * width + x] = y;
                        break;
                    }
                }
            }
        }
        
        for (int z = 0; z < depth; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int index = getIndex(x, y, z);
                    Block& block = blocks[index];
                    
                    // 空气方块始终不可见
                    if (block.type == BLOCK_AIR) {
                        block.isVisible = false;
                        continue;
                    }
                    
                    // 地下渲染优化：如果方块完全被其他方块包围且在地表以下，则不渲染
                    int surfaceHeight = surfaceHeightMap[z * width + x];
                    bool isUnderground = y < surfaceHeight - 1; // 在地表以下至少2个方块
                    
                    if (isUnderground) {
                        // 检查是否完全被不透明方块包围
                        bool completelyHidden = true;
                        
                        // 检查六个面是否有相邻的透明方块
                        // 前面 (z+1)
                        if (z == depth - 1 || isTransparent(blocks[getIndex(x, y, z + 1)].type)) {
                            completelyHidden = false;
                        }
                        
                        // 后面 (z-1)
                        if (completelyHidden && (z == 0 || isTransparent(blocks[getIndex(x, y, z - 1)].type))) {
                            completelyHidden = false;
                        }
                        
                        // 左面 (x-1)
                        if (completelyHidden && (x == 0 || isTransparent(blocks[getIndex(x - 1, y, z)].type))) {
                            completelyHidden = false;
                        }
                        
                        // 右面 (x+1)
                        if (completelyHidden && (x == width - 1 || isTransparent(blocks[getIndex(x + 1, y, z)].type))) {
                            completelyHidden = false;
                        }
                        
                        // 上面 (y+1)
                        if (completelyHidden && (y == height - 1 || isTransparent(blocks[getIndex(x, y + 1, z)].type))) {
                            completelyHidden = false;
                        }
                        
                        // 下面 (y-1)
                        if (completelyHidden && (y == 0 || isTransparent(blocks[getIndex(x, y - 1, z)].type))) {
                            completelyHidden = false;
                        }
                        
                        // 如果完全被包围，则不可见
                        if (completelyHidden) {
                            block.isVisible = false;
                            continue;
                        }
                    }
                    
                    // 检查六个面是否有相邻的透明方块
                    bool hasTransparentNeighbor = false;
                    
                    // 检查前面 (z+1)
                    if (z == depth - 1 || isTransparent(blocks[getIndex(x, y, z + 1)].type)) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 检查后面 (z-1)
                    if (z == 0 || isTransparent(blocks[getIndex(x, y, z - 1)].type)) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 检查左面 (x-1)
                    if (x == 0 || isTransparent(blocks[getIndex(x - 1, y, z)].type)) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 检查右面 (x+1)
                    if (x == width - 1 || isTransparent(blocks[getIndex(x + 1, y, z)].type)) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 检查上面 (y+1)
                    if (y == height - 1 || isTransparent(blocks[getIndex(x, y + 1, z)].type)) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 检查下面 (y-1)
                    if (y == 0 || isTransparent(blocks[getIndex(x, y - 1, z)].type)) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 如果方块是半透明的，始终渲染它
                    if (block.type == BLOCK_WATER || block.type == BLOCK_LEAVES || block.type == BLOCK_LAVA) {
                        hasTransparentNeighbor = true;
                    }
                    
                    // 设置方块可见性
                    block.isVisible = hasTransparentNeighbor;
                }
            }
        }
    }
    
    // 统计矿物数量
    void countOres() {
        std::cout << "========== Ore Statistics ==========" << std::endl;
        
        // 用于存储每种矿物的计数
        std::map<BlockType, int> oreCount;
        std::map<BlockType, std::string> oreNames = {
            {BLOCK_COAL_ORE, "Coal Ore"},
            {BLOCK_IRON_ORE, "Iron Ore"},
            {BLOCK_GOLD_ORE, "Gold Ore"},
            {BLOCK_DIAMOND_ORE, "Diamond Ore"},
            {BLOCK_REDSTONE_ORE, "Redstone Ore"},
            {BLOCK_EMERALD_ORE, "Emerald Ore"},
            {BLOCK_LAVA, "Lava Source"}
        };
        
        // 遍历所有方块
        for (int z = 0; z < depth; z++) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    BlockType type = blocks[getIndex(x, y, z)].type;
                    
                    // 检查是否是矿物
                    if (oreNames.find(type) != oreNames.end()) {
                        oreCount[type]++;
                    }
                }
            }
        }
        
        // 输出统计结果
        std::cout << "World Size: " << width << "x" << height << "x" << depth << std::endl;
        std::cout << "World Seed: " << worldSeed << std::endl;
        
        int totalOres = 0;
        for (const auto& ore : oreNames) {
            int count = oreCount[ore.first];
            totalOres += count;
            std::cout << ore.second << ": " << count << " blocks" << std::endl;
        }
        
        std::cout << "Total Ores: " << totalOres << " blocks" << std::endl;
        std::cout << "==============================" << std::endl;
    }
    
public:
    World() : width(0), height(0), depth(0), isSuperFlat(false), superFlatBlockType(BLOCK_GRASS), 
              spawnX(0), spawnY(0), spawnZ(0) {
        // 初始化随机数生成器
        std::random_device rd;
        rng = std::mt19937(rd());
    }
    
    // 初始化世界
    void init(int width, int height, int depth) {
        this->width = width;
        this->height = height;
        this->depth = depth;
        
        // 初始化随机种子
        std::random_device rd;
        worldSeed = rd();
        rng = std::mt19937(worldSeed);
        
        generateWorld();
        
        // 设置出生点
        findSpawnPoint();
        
        std::cout << "World initialized: Size=" << width << "x" << height << "x" << depth 
                  << ", Seed=" << worldSeed << std::endl;
    }
    
    // 初始化世界（带种子）
    void init(int width, int height, int depth, unsigned int seed) {
        this->width = width;
        this->height = height;
        this->depth = depth;
        this->worldSeed = seed;
        
        // 使用提供的种子初始化随机数生成器
        rng = std::mt19937(seed);
        
        generateWorld();
        
        // 设置出生点
        findSpawnPoint();
    }
    
    // 初始化世界（带超平坦选项）
    void init(int width, int height, int depth, unsigned int seed, bool superFlat, BlockType flatBlockType) {
        this->width = width;
        this->height = height;
        this->depth = depth;
        this->worldSeed = seed;
        this->isSuperFlat = superFlat;
        this->superFlatBlockType = flatBlockType;
        
        // 使用提供的种子初始化随机数生成器
        rng = std::mt19937(seed);
        
        generateWorld();
        
        // 设置出生点
        findSpawnPoint();
        
        std::cout << "World initialized: Size=" << width << "x" << height << "x" << depth 
                  << ", Seed=" << seed 
                  << ", SuperFlat=" << (superFlat ? "true" : "false")
                  << std::endl;
    }
    
    // 重新生成世界（可以用于更改世界大小）
    void regenerateWorld(int newWidth, int newHeight, int newDepth) {
        this->width = newWidth;
        this->height = newHeight;
        this->depth = newDepth;
        
        generateWorld();
    }
    
    // 获取指定位置的方块
    Block& getBlock(int x, int y, int z) {
        static Block airBlock(BLOCK_AIR);
        if (!isInBounds(x, y, z)) {
            return airBlock;
        }
        return blocks[getIndex(x, y, z)];
    }
    
    // 获取指定位置的方块（const版本）
    const Block& getBlock(int x, int y, int z) const {
        static Block airBlock(BLOCK_AIR);
        if (!isInBounds(x, y, z)) {
            return airBlock;
        }
        return blocks[getIndex(x, y, z)];
    }
    
    // 设置指定位置的方块
    void setBlock(int x, int y, int z, BlockType type) {
        if (!isInBounds(x, y, z)) {
            return;
        }
        
        blocks[getIndex(x, y, z)] = Block(type);
        
        // 更新该方块及其相邻方块的可见性
        updateBlockVisibilityAt(x, y, z);
    }
    
    // 渲染世界（在renderer.cpp中实现）
    friend void Renderer::renderWorld(const World& world, const Camera& camera);
    
    // 获取出生点坐标
    Vec3 getSpawnPoint() const {
        return Vec3(spawnX + 0.5f, spawnY + 1.7f, spawnZ + 0.5f);
    }
    
    // 设置出生点坐标
    void setSpawnPoint(int x, int y, int z) {
        spawnX = x;
        spawnY = y;
        spawnZ = z;
    }
    
    // 切换X-ray模式
    void toggleXrayMode() {
        xrayMode = !xrayMode;
        std::cout << "X-ray mode: " << (xrayMode ? "enabled" : "disabled") << std::endl;
    }
    
    // 获取X-ray模式状态
    bool isXrayMode() const {
        return xrayMode;
    }
    
    // 查找合适的出生点
    void findSpawnPoint() {
        // 默认出生在世界中心
        spawnX = width / 2;
        spawnZ = depth / 2;
        spawnY = 0;
        
        // 从顶部向下搜索第一个非空气方块
        for (int y = height - 1; y >= 0; y--) {
            if (getBlock(spawnX, y, spawnZ).type != BLOCK_AIR) {
                spawnY = y + 1; // 设置为方块上方一格
                break;
            }
        }
        
        // 确保玩家不会出生在地下
        spawnY = std::max(spawnY, height / 2);
        
        std::cout << "Spawn point set to: X=" << spawnX << ", Y=" << spawnY << ", Z=" << spawnZ << std::endl;
    }
};

// 实现Renderer::renderWorld方法（需要在World类定义后）
void Renderer::renderWorld(const World& world, const Camera& camera) {
    // 开始渲染前检查渲染器是否已初始化
    if (frameBuffer == nullptr) return;

    // 计算视图投影矩阵
    Mat4 view = camera.getViewMatrix();
    Mat4 projection = camera.getProjectionMatrix();
    
    // 将矩阵传递给GPU渲染器
    gpuRenderer.SetTransform(view, projection);
    
    // 计算渲染距离的平方(用于方块级别的距离检查)
    float maxDistanceSq = static_cast<float>(renderDistance * renderDistance);
    
    // 获取玩家位置的整数坐标
    int playerX = static_cast<int>(camera.position.x);
    int playerY = static_cast<int>(camera.position.y);
    int playerZ = static_cast<int>(camera.position.z);
    
    // 区块大小定义（每个区块包含16x16x16个方块）
    const int CHUNK_SIZE = 16;
    
    // 计算需要渲染的区域范围(区块级别的预筛选)
    int minX = std::max(0, playerX - renderDistance);
    int maxX = std::min(world.width - 1, playerX + renderDistance);
    int minY = std::max(0, playerY - renderDistance);
    int maxY = std::min(world.height - 1, playerY + renderDistance);
    int minZ = std::max(0, playerZ - renderDistance);
    int maxZ = std::min(world.depth - 1, playerZ + renderDistance);
    
    // 转换为区块坐标
    int minChunkX = minX / CHUNK_SIZE;
    int maxChunkX = maxX / CHUNK_SIZE;
    int minChunkY = minY / CHUNK_SIZE;
    int maxChunkY = maxY / CHUNK_SIZE;
    int minChunkZ = minZ / CHUNK_SIZE;
    int maxChunkZ = maxZ / CHUNK_SIZE;
    
    // 清理过期的区块缓存
    cleanupChunkCache();
    
    // 调试信息
    int renderedBlocks = 0;
    int renderedFaces = 0;
    
    // 首先绘制不透明方块，然后绘制半透明方块（如水、树叶）
    for (int pass = 0; pass < 2; pass++) {
        // 先处理区块级别的渲染
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
            for (int chunkY = minChunkY; chunkY <= maxChunkY; chunkY++) {
                for (int chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
                    // 计算区块中心
                    float chunkCenterX = (chunkX * CHUNK_SIZE) + CHUNK_SIZE * 0.5f;
                    float chunkCenterY = (chunkY * CHUNK_SIZE) + CHUNK_SIZE * 0.5f;
                    float chunkCenterZ = (chunkZ * CHUNK_SIZE) + CHUNK_SIZE * 0.5f;
                    
                    // 计算区块中心到相机的距离
                    Vec3 chunkCenter(chunkCenterX, chunkCenterY, chunkCenterZ);
                    float chunkDistSq = (chunkCenter - camera.position).lengthSquared();
                    
                    // 计算区块对角线长度的一半，用于包围球检测
                    float chunkRadius = CHUNK_SIZE * 0.866f; // 约等于CHUNK_SIZE * sqrt(3)/2
                    float chunkBoundingSphereRadiusSq = chunkRadius * chunkRadius;
                    
                    // 检查区块是否在渲染距离内或区块是否在缓存中
                    // 增加区块边界容错，扩大判定范围
                    float distanceMargin = 1.2f; // 增加20%的边界容错
                    bool inRenderDistance = chunkDistSq <= (maxDistanceSq + chunkBoundingSphereRadiusSq) * distanceMargin;
                    bool inCache = isChunkLoaded(chunkX, chunkY, chunkZ);
                    bool shouldRender = inRenderDistance || inCache;
                    
                    // 检查区块是否在视锥体内 - 使用更宽松的检测
                    bool inViewFrustum = isBlockInFrustum(chunkCenter, camera);
                    
                    // 更新区块缓存
                    if (inRenderDistance) {
                        updateChunkCache(chunkX, chunkY, chunkZ, inViewFrustum);
                    }
                    
                    // 如果区块不需要渲染，跳过
                    if (!shouldRender) continue;
                    
                    // 渲染区块内的方块
                    int startX = chunkX * CHUNK_SIZE;
                    int startY = chunkY * CHUNK_SIZE;
                    int startZ = chunkZ * CHUNK_SIZE;
                    
                    int endX = std::min(startX + CHUNK_SIZE, world.width);
                    int endY = std::min(startY + CHUNK_SIZE, world.height);
                    int endZ = std::min(startZ + CHUNK_SIZE, world.depth);
                    
                    // 仅遍历当前区块内的方块
                    for (int z = startZ; z < endZ; z++) {
                        for (int y = startY; y < endY; y++) {
                            for (int x = startX; x < endX; x++) {
                    // 获取方块信息
                    const Block& block = world.getBlockConst(x, y, z);
                
                    // 跳过空气方块
                    if (block.type == BLOCK_AIR) continue;
                                
                                // 特殊处理可变方块(BLOCK_CHANGE_BLOCK)
                                bool isCustomTransparent = false;
                                if (block.type == BLOCK_CHANGE_BLOCK && block.hasCustomColors) {
                                    // 检查任何一个面是否半透明(alpha < 255)
                                    for (int i = 0; i < FACE_COUNT; i++) {
                                        if (block.customColors[i].a < 255) {
                                            isCustomTransparent = true;
                                            break;
                                        }
                                    }
                                }
                                
                                // 判断是否为透明方块
                                bool isTransparent = world.isTransparent(block.type) || isCustomTransparent;
                    
                    // 第一遍绘制不透明方块，第二遍绘制透明方块
                    if ((pass == 0 && isTransparent) || (pass == 1 && !isTransparent)) {
                        continue;
                    }
                    
                                // 精确的视距检查
                    Vec3 blockCenter(x + 0.5f, y + 0.5f, z + 0.5f);
                    float distSq = (blockCenter - camera.position).lengthSquared();
                    if (distSq > maxDistanceSq) continue;
                
                                // 如果启用了地下优化且相机在地下，检查是否需要渲染
                                if (optimizeUndergroundRendering && camera.position.y < world.height - 5) {
                                    // 使用更严格的距离限制，地下渲染距离约为正常渲染距离的1/3
                                    float undergroundMaxDistSq = 1024.0f; // 32*32 = 1024
                                    
                                    // 检查是否超出地下渲染距离
                                    if (distSq > undergroundMaxDistSq) {
                                        // 检查方块是否在玩家视线方向上
                                        Vec3 blockDir = (blockCenter - camera.position).normalize();
                                        float viewAlignment = blockDir.dot(camera.front);
                                        
                                        // 如果方块不在玩家视线方向前方，则跳过
                                        if (viewAlignment < 0.7f || distSq > maxDistanceSq * 0.3f) {
                                            continue;
                                        }
                                    }
                                }
                                
                                // 视锥体剔除 - 对于远距离方块执行严格的视锥体检测
                                if (distSq > 100.0f && !isBlockInFrustum(blockCenter, camera)) {
                                    continue;
                                }
                                
                                // X-ray模式的特殊处理
                    if (world.isXrayMode()) {
                                    // 检查是否是矿物
                                    bool isOre = (block.type == BLOCK_COAL_ORE || 
                                                block.type == BLOCK_IRON_ORE || 
                                                block.type == BLOCK_GOLD_ORE || 
                                                block.type == BLOCK_DIAMOND_ORE || 
                                                block.type == BLOCK_REDSTONE_ORE || 
                                                block.type == BLOCK_EMERALD_ORE || 
                                                block.type == BLOCK_LAVA);
                                    
                                    // 计算与玩家的距离
                                    float distToPlayer = std::sqrt(distSq);
                                    
                                    // 如果不是矿物且不在玩家附近，则不渲染
                                    if (!isOre && distToPlayer > 5.0f) continue;
                                    
                                    // 如果是石头且不在玩家附近，则只渲染少量以提高性能
                                    if (block.type == BLOCK_STONE && distToPlayer > 3.0f) {
                                        // 跳过大部分石头方块，只渲染少量以提高性能
                                        if ((x + y + z) % 10 != 0) continue;
                                    }
                                    
                                    // 对于矿物，应用特殊高亮效果
                                    if (isOre) {
                                        // 获取基础颜色
                                        Color baseColor;
                        switch (block.type) {
                            case BLOCK_COAL_ORE:
                                                baseColor = Color(50, 50, 50);
                                                break;
                            case BLOCK_IRON_ORE:
                                                baseColor = Color(150, 120, 100);
                                                break;
                            case BLOCK_GOLD_ORE:
                                                baseColor = Color(200, 170, 60);
                                                break;
                            case BLOCK_DIAMOND_ORE:
                                                baseColor = Color(80, 220, 220);
                                                break;
                            case BLOCK_REDSTONE_ORE:
                                                baseColor = Color(180, 50, 50);
                                                break;
                                            case BLOCK_EMERALD_ORE:
                                                baseColor = Color(30, 180, 70);
                                                break;
                                            case BLOCK_LAVA:
                                                baseColor = Color(200, 80, 20);
                                break;
                            default:
                                                baseColor = Color(200, 200, 200);
                                break;
                                        }
                                        
                                        // 增强颜色亮度
                                        baseColor.r = std::min(255, baseColor.r + 80);
                                        baseColor.g = std::min(255, baseColor.g + 80);
                                        baseColor.b = std::min(255, baseColor.b + 80);
                                        
                                        // 应用脉冲效果
                                        int pulseValue = (int)(50 * sin(GetTickCount() * 0.003f) + 50);
                                        baseColor.r = std::min(255, baseColor.r + pulseValue);
                                        baseColor.g = std::min(255, baseColor.g + pulseValue);
                                        baseColor.b = std::min(255, baseColor.b + pulseValue);
                    
                    // 创建世界矩阵，设置方块位置
                    Mat4 worldMatrix = Mat4::translate(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
                    gpuRenderer.SetWorldMatrix(worldMatrix);
                    
                                        // 渲染所有面，使用高亮颜色
                                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_FRONT, baseColor);
                                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_BACK, baseColor);
                                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_LEFT, baseColor);
                                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_RIGHT, baseColor);
                                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_TOP, baseColor);
                                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_BOTTOM, baseColor);
                                        
                                        renderedFaces += 6;
                                        renderedBlocks++;
                                        continue;
                                    }
                                }
                                
                                // 创建世界矩阵，设置方块位置
                                Mat4 worldMatrix = Mat4::translate(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
                                gpuRenderer.SetWorldMatrix(worldMatrix);
                                
                                // 判断每个面是否应该被渲染 - 优化的面剔除
                    bool anyFaceRendered = false;
                    
                                // 检查前面 (z+1)
                                if (z + 1 >= world.depth || world.isTransparent(world.getBlockConst(x, y, z + 1).type)) {
                        Color faceColor = block.getFaceColor(FACE_FRONT);
                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_FRONT, faceColor);
                        anyFaceRendered = true;
                                    renderedFaces++;
                    }
                    
                                // 检查后面 (z-1)
                                if (z - 1 < 0 || world.isTransparent(world.getBlockConst(x, y, z - 1).type)) {
                        Color faceColor = block.getFaceColor(FACE_BACK);
                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_BACK, faceColor);
                        anyFaceRendered = true;
                                    renderedFaces++;
                    }
                    
                                // 检查左面 (x-1)
                                if (x - 1 < 0 || world.isTransparent(world.getBlockConst(x - 1, y, z).type)) {
                        Color faceColor = block.getFaceColor(FACE_LEFT);
                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_LEFT, faceColor);
                        anyFaceRendered = true;
                                    renderedFaces++;
                    }
                
                                // 检查右面 (x+1)
                                if (x + 1 >= world.width || world.isTransparent(world.getBlockConst(x + 1, y, z).type)) {
                        Color faceColor = block.getFaceColor(FACE_RIGHT);
                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_RIGHT, faceColor);
                        anyFaceRendered = true;
                                    renderedFaces++;
                    }
                
                                // 检查上面 (y+1)
                                if (y + 1 >= world.height || world.isTransparent(world.getBlockConst(x, y + 1, z).type)) {
                        Color faceColor = block.getFaceColor(FACE_TOP);
                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_TOP, faceColor);
                        anyFaceRendered = true;
                                    renderedFaces++;
                    }
                
                                // 检查下面 (y-1)
                                if (y - 1 < 0 || world.isTransparent(world.getBlockConst(x, y - 1, z).type)) {
                        Color faceColor = block.getFaceColor(FACE_BOTTOM);
                        gpuRenderer.DrawBlockFace(Vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)), FACE_BOTTOM, faceColor);
                        anyFaceRendered = true;
                                    renderedFaces++;
                    }
                    
                    // 如果渲染了任何面，计数增加
                    if (anyFaceRendered) {
                        renderedBlocks++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 调试输出
    static int frameCount = 0;
    static int lastBlockCount = 0;
    static int lastFaceCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) {  // 每60帧输出一次
        if (renderedBlocks != lastBlockCount || renderedFaces != lastFaceCount) {
            std::cout << "Rendered blocks: " << renderedBlocks << ", faces: " << renderedFaces << std::endl;
            lastBlockCount = renderedBlocks;
            lastFaceCount = renderedFaces;
                }
        frameCount = 0;
    }
    
    // 在方块渲染完成后，渲染太阳和云
    // 检查游戏状态，如果游戏暂停，则不更新云的位置
    if (uiManager && uiManager->getGameState() != GAME_PAUSED) {
        // 渲染太阳
        renderSun(camera);
        
        // 渲染云
        renderClouds(camera);
    } else {
        // 游戏暂停时，仍然渲染太阳和云，但不更新位置
        renderSun(camera);
        renderClouds(camera);
    }
}

#endif // WORLD_H
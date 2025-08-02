#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <windows.h>
#include "math3d.h"
#include "camera.h"
#include "render_gpu.h"  // 添加GPU渲染器头文件

// 前向声明
class World;
class UIManager;  // 添加UIManager的前向声明

// 方块面枚举
enum Face {
    FACE_FRONT,
    FACE_BACK,
    FACE_LEFT,
    FACE_RIGHT,
    FACE_TOP,
    FACE_BOTTOM,
    FACE_COUNT
};

// 方块类型枚举
enum BlockType {
    BLOCK_AIR,  // 空气方块
    BLOCK_DIRT,  // 泥土方块
    BLOCK_GRASS,   // 草方块
    BLOCK_STONE,    // 石方块
    BLOCK_SAND,    // 沙方块
    BLOCK_WATER,    // 水方块
    BLOCK_WOOD,        // 木头方块
    BLOCK_LEAVES,     // 树叶方块
    BLOCK_SNOW,       // 雪方块
    BLOCK_ICE,        // 冰方块
    BLOCK_GRAVEL,     // 砂砾方块
    BLOCK_CLAY,       // 粘土方块
    BLOCK_COAL_ORE,   // 煤矿石
    BLOCK_IRON_ORE,   // 铁矿石
    BLOCK_GOLD_ORE,   // 金矿石
    BLOCK_BEDROCK,    // 基岩
    BLOCK_OBSIDIAN,   // 黑曜石
    BLOCK_LAVA,       // 岩浆
    BLOCK_DIAMOND_ORE,// 钻石矿石
    BLOCK_EMERALD_ORE,// 绿宝石矿石
    BLOCK_REDSTONE_ORE,// 红石矿石
    BLOCK_MOSSY_STONE,// 苔石
    BLOCK_SANDSTONE,  // 砂岩
    BLOCK_CACTUS,     // 仙人掌
    BLOCK_PUMPKIN,    // 南瓜
    BLOCK_NETHERRACK, // 下界岩
    BLOCK_SOUL_SAND,  // 灵魂沙
    BLOCK_GLOWSTONE,  // 荧石
    BLOCK_BRICK,      // 砖块
    BLOCK_BOOKSHELF,  // 书架
    BLOCK_QUARTZ,     // 石英
    BLOCK_MYCELIUM,   // 菌丝
    BLOCK_END_STONE,  // 末地石
    BLOCK_PRISMARINE, // 海晶石
    BLOCK_MAGMA,      // 岩浆块
    BLOCK_NETHER_WART,// 下界疣块
    BLOCK_SLIME,      // 粘液块
    BLOCK_CHANGE_BLOCK, // 可变方块 - 新增
    BLOCK_COUNT // 方块类型数量
};

// 方块类
class Block {
public:
    BlockType type;
    bool isVisible;
    Color customColors[FACE_COUNT]; // 添加自定义颜色数组，为六个面各存储一种颜色
    bool hasCustomColors; // 标记是否使用自定义颜色
    
    Block() : type(BLOCK_AIR), isVisible(false), hasCustomColors(false) {
        // 初始化自定义颜色为默认值（例如灰色）
        for (int i = 0; i < FACE_COUNT; i++) {
            customColors[i] = Color(127, 127, 127);
        }
    }
    
    Block(BlockType type) : type(type), isVisible(type != BLOCK_AIR), hasCustomColors(false) {
        // 初始化自定义颜色为默认值
        for (int i = 0; i < FACE_COUNT; i++) {
            customColors[i] = Color(127, 127, 127);
        }
    }
    
    // 设置特定面的自定义颜色
    void setCustomColor(Face face, const Color& color) {
        customColors[face] = color;
        hasCustomColors = true;
    }
    
    // 获取特定面的自定义颜色
    const Color& getCustomColor(Face face) const {
        return customColors[face];
    }
    
    // 复制自定义颜色设置
    void copyCustomColorsFrom(const Block& other) {
        hasCustomColors = other.hasCustomColors;
        for (int i = 0; i < FACE_COUNT; i++) {
            customColors[i] = other.customColors[i];
        }
    }
    
    // 从颜色信息推断方块类型的静态方法
    static BlockType getBlockTypeFromColor(const Color& color) {
        // 根据颜色的RGB值来判断方块类型
        // 注意：这里使用近似匹配，因为光照会影响颜色
        
        // 泥土 - 棕色
        if (color.r > 100 && color.r < 140 && color.g > 70 && color.g < 100 && color.b > 40 && color.b < 70) {
            return BLOCK_DIRT;
        }
        // 草地 - 绿色顶部或棕色侧面
        else if ((color.r < 110 && color.g > 140 && color.b < 70) || 
                 (color.r > 90 && color.r < 120 && color.g > 80 && color.g < 110 && color.b > 50 && color.b < 70)) {
            return BLOCK_GRASS;
        }
        // 石头 - 灰色
        else if (color.r > 100 && color.r < 150 && color.g > 100 && color.g < 150 && color.b > 100 && color.b < 150 && 
                 std::abs(color.r - color.g) < 20 && std::abs(color.r - color.b) < 20) {
            return BLOCK_STONE;
        }
        // 沙子 - 黄褐色
        else if (color.r > 170 && color.r < 220 && color.g > 150 && color.g < 200 && color.b > 100 && color.b < 150) {
            return BLOCK_SAND;
        }
        // 水 - 蓝色且半透明
        else if (color.r < 70 && color.g > 70 && color.g < 110 && color.b > 130 && color.a < 230) {
            return BLOCK_WATER;
        }
        // 木头 - 棕色
        else if (color.r > 90 && color.r < 140 && color.g > 70 && color.g < 100 && color.b > 40 && color.b < 70 && 
                 color.r > color.g && color.g > color.b) {
            return BLOCK_WOOD;
        }
        // 树叶 - 绿色且半透明
        else if (color.r < 80 && color.g > 120 && color.g < 160 && color.b < 90 && color.a < 240) {
            return BLOCK_LEAVES;
        }
        // 雪 - 白色
        else if (color.r > 230 && color.g > 230 && color.b > 230) {
            return BLOCK_SNOW;
        }
        // 冰 - 淡蓝色且半透明
        else if (color.r > 130 && color.r < 170 && color.g > 160 && color.g < 200 && color.b > 220 && color.a < 240) {
            return BLOCK_ICE;
        }
        // 砂砾 - 灰色
        else if (color.r > 120 && color.r < 160 && color.g > 120 && color.g < 160 && color.b > 120 && color.b < 160 && 
                 std::abs(color.r - color.g) < 15 && std::abs(color.r - color.b) < 15) {
            return BLOCK_GRAVEL;
        }
        // 粘土 - 淡灰色
        else if (color.r > 140 && color.r < 180 && color.g > 150 && color.g < 190 && color.b > 160 && color.b < 200) {
            return BLOCK_CLAY;
        }
        // 煤矿石 - 深灰色
        else if (color.r > 30 && color.r < 90 && color.g > 30 && color.g < 90 && color.b > 30 && color.b < 90 && 
                 std::abs(color.r - color.g) < 15 && std::abs(color.r - color.b) < 15) {
            return BLOCK_COAL_ORE;
        }
        // 铁矿石 - 棕灰色
        else if (color.r > 120 && color.r < 180 && color.g > 100 && color.g < 140 && color.b > 80 && color.b < 120) {
            return BLOCK_IRON_ORE;
        }
        // 金矿石 - 金黄色
        else if (color.r > 170 && color.r < 230 && color.g > 140 && color.g < 190 && color.b > 40 && color.b < 80) {
            return BLOCK_GOLD_ORE;
        }
        // 基岩 - 黑色
        else if (color.r < 60 && color.g < 60 && color.b < 60) {
            return BLOCK_BEDROCK;
        }
        // 黑曜石 - 深紫色
        else if (color.r < 30 && color.g < 30 && color.b > 20 && color.b < 50) {
            return BLOCK_OBSIDIAN;
        }
        // 岩浆 - 橙红色
        else if (color.r > 180 && color.g > 50 && color.g < 100 && color.b < 30 && color.a < 240) {
            return BLOCK_LAVA;
        }
        // 钻石矿石 - 青色
        else if (color.r > 50 && color.r < 100 && color.g > 200 && color.b > 180) {
            return BLOCK_DIAMOND_ORE;
        }
        // 绿宝石矿石 - 绿色
        else if (color.r < 40 && color.g > 150 && color.g < 200 && color.b > 50 && color.b < 100) {
            return BLOCK_EMERALD_ORE;
        }
        // 红石矿石 - 红色
        else if (color.r > 130 && color.g < 60 && color.b < 60) {
            return BLOCK_REDSTONE_ORE;
        }
        // 苔石 - 灰绿色
        else if (color.r > 80 && color.r < 110 && color.g > 100 && color.g < 130 && color.b > 80 && color.b < 110 && color.g > color.r) {
            return BLOCK_MOSSY_STONE;
        }
        
        // 默认返回石头类型
        return BLOCK_STONE;
    }
    
    // 获取方块特定面的颜色
    Color getFaceColor(Face face) const {
        // 如果是自定义方块且有自定义颜色，则返回自定义颜色
        if (type == BLOCK_CHANGE_BLOCK && hasCustomColors) {
            // 确保透明度不会导致透视问题
            Color customColor = customColors[face];
            // 如果透明度低于128（半透明），则设置为128，以防止过度透明导致的穿透问题
            if (customColor.a < 128) {
                customColor.a = 128;
            }
            return customColor;
        }
        
        if (type == BLOCK_AIR) return Color(0, 0, 0, 0);
        
        // 预先声明所有可能需要的变量
        int variation = 0;
        int waveEffect = 0;
        int leafVariation = 0;
        int gravelVar = 0;
        int bedrockVar = 0;
        int lavaVar = 0;
        
        // 获取基础颜色
        Color baseColor;
        
        // 使用坐标和面信息生成固定的纹理模式，而不是随机数
        // 这样可以确保每次渲染相同位置的方块时，纹理都是一致的
        int texturePattern = ((int)(face) * 1234 + 5678) % 16;
        
        switch (type) {
            case BLOCK_DIRT:
                baseColor = Color(121, 85, 58);
                break;
                
            case BLOCK_GRASS:
                if (face == FACE_TOP) {
                    baseColor = Color(95, 159, 53);
                } else if (face == FACE_BOTTOM) {
                    baseColor = Color(121, 85, 58);
                } else {
                    baseColor = Color(108, 96, 60);
                }
                break;
                
            case BLOCK_STONE:
                baseColor = Color(127, 127, 127);
                break;
                
            case BLOCK_SAND:
                baseColor = Color(194, 178, 128);
                break;
                
            case BLOCK_WATER:
                baseColor = Color(52, 86, 155, 200);
                break;
                
            case BLOCK_WOOD:
                if (face == FACE_TOP || face == FACE_BOTTOM) {
                    baseColor = Color(96, 76, 50);
                } else {
                    baseColor = Color(119, 89, 55);
                }
                break;
                
            case BLOCK_LEAVES:
                baseColor = Color(60, 143, 72, 230);
                break;
                
            case BLOCK_SNOW:
                baseColor = Color(240, 240, 245);
                break;
                
            case BLOCK_ICE:
                baseColor = Color(160, 188, 255, 220);
                break;
                
            case BLOCK_GRAVEL:
                baseColor = Color(136, 126, 126);
                break;
                
            case BLOCK_CLAY:
                baseColor = Color(159, 164, 177);
                break;
                
            case BLOCK_COAL_ORE:
                baseColor = Color(50, 50, 50);
                break;
                
            case BLOCK_IRON_ORE:
                baseColor = Color(180, 180, 180);
                break;
                
            case BLOCK_GOLD_ORE:
                baseColor = Color(255, 215, 0);
                break;
                
            case BLOCK_BEDROCK:
                baseColor = Color(40, 40, 40);
                break;
                
            case BLOCK_OBSIDIAN:
                baseColor = Color(20, 18, 29);
                break;
                
            case BLOCK_LAVA:
                baseColor = Color(207, 16, 32, 230);
                break;
                
            case BLOCK_DIAMOND_ORE:
                baseColor = Color(0, 191, 255);
                break;
                
            case BLOCK_EMERALD_ORE:
                baseColor = Color(0, 217, 58);
                break;
                
            case BLOCK_REDSTONE_ORE:
                baseColor = Color(255, 0, 0);
                break;
                
            case BLOCK_MOSSY_STONE:
                baseColor = Color(90, 108, 90);
                break;
                
            case BLOCK_SANDSTONE:
                baseColor = Color(219, 207, 163);
                break;
                
            case BLOCK_CACTUS:
                if (face == FACE_TOP) {
                    baseColor = Color(12, 156, 51);
                } else {
                    baseColor = Color(27, 122, 69);
                }
                break;
                
            case BLOCK_PUMPKIN:
                if (face == FACE_FRONT) {
                    baseColor = Color(212, 126, 3);
                } else {
                    baseColor = Color(202, 118, 0);
                }
                break;
                
            case BLOCK_NETHERRACK:
                baseColor = Color(100, 50, 50);
                break;
                
            case BLOCK_SOUL_SAND:
                baseColor = Color(90, 70, 55);
                break;
                
            case BLOCK_GLOWSTONE:
                baseColor = Color(247, 215, 100);
                break;
                
            case BLOCK_BRICK:
                baseColor = Color(150, 75, 75);
                break;
                
            case BLOCK_BOOKSHELF:
                if (face == FACE_TOP || face == FACE_BOTTOM) {
                    baseColor = Color(96, 76, 50); // 木质
                } else {
                    baseColor = Color(180, 150, 100); // 书籍
                }
                break;
                
            case BLOCK_QUARTZ:
                baseColor = Color(236, 233, 226);
                break;
                
            case BLOCK_MYCELIUM:
                if (face == FACE_TOP) {
                    baseColor = Color(114, 88, 110); // 菌丝顶部紫色
                } else {
                    baseColor = Color(121, 85, 58); // 底部与泥土相似
                }
                break;
                
            case BLOCK_END_STONE:
                baseColor = Color(221, 223, 165);
                break;
                
            case BLOCK_PRISMARINE:
                baseColor = Color(99, 156, 151);
                break;
                
            case BLOCK_MAGMA:
                baseColor = Color(155, 57, 9);
                break;
                
            case BLOCK_NETHER_WART:
                baseColor = Color(153, 42, 42);
                break;
                
            case BLOCK_SLIME:
                baseColor = Color(121, 200, 101, 230); // 半透明
                break;
                
            case BLOCK_CHANGE_BLOCK:
                // 如果没有自定义颜色，则使用默认颜色
                baseColor = Color(200, 200, 200); // 默认为亮灰色
                break;
                
            default:
                baseColor = Color(255, 0, 255); // 默认为亮紫色（易于识别的错误颜色）
        }
        
        return baseColor;
    }
};

// 云朵结构体
struct Cloud {
    Vec3 position;    // 云的位置
    float width;      // 云的宽度
    float depth;      // 云的深度
    float opacity;    // 云的不透明度
};

// Renderer class
class Renderer {
private:
    int width;
    int height;
    uint32_t* frameBuffer;
    
    // 使用GPU渲染器替代CPU渲染
    GPURenderer gpuRenderer;
    HWND hwnd; // 窗口句柄，用于初始化GPU渲染器
    
    // Whether to enable underground rendering optimization
    bool optimizeUndergroundRendering = true;
    
    // 渲染距离（方块数）
    int renderDistance = 96;  // 从64增加到96
    
    // 区块缓存系统 - 保存最近加载的区块信息
    struct ChunkInfo {
        int x, y, z;          // 区块坐标
        float lastSeenTime;   // 最后一次被看到的时间
        bool inViewFrustum;   // 当前是否在视锥体内
    };
    
    // 已加载区块的缓存，使用队列管理
    std::vector<ChunkInfo> loadedChunks;
    
    // 区块缓存的最大大小
    const int MAX_CACHED_CHUNKS = 256;
    
    // 区块缓存的过期时间（秒）
    const float CHUNK_CACHE_LIFETIME = 3.0f; // 3秒缓存时间
    
    // 当前游戏时间
    float gameTime = 0.0f;
    
    // Crosshair properties
    int crosshairSize = 10;
    int crosshairThickness = 2;
    Color crosshairColor = Color(255, 255, 255, 200);
    
    // Arm rendering properties
    bool showArm = true;
    float armSwingAngle = 0.0f;
    float armSwingSpeed = 3.0f;
    float armSwingAmount = 0.0f;
    
    // UI管理器指针
    UIManager* uiManager;
    
    // 云和太阳相关成员
    std::vector<Cloud> clouds;       // 云朵列表
    Vec3 sunPosition;                // 太阳位置
    float cloudMovementSpeed;        // 云移动速度
    float cloudHeight;               // 云层高度
    bool cloudsEnabled;              // 是否启用云
    bool sunEnabled;                 // 是否启用太阳

    // 初始化云
    void initClouds() {
        // 清空现有云
        clouds.clear();
        
        // 设置云层高度
        cloudHeight = 60.0f;
        
        // 设置云移动速度
        cloudMovementSpeed = 0.5f;
        
        // 生成随机云朵
        std::mt19937 rng(42); // 使用固定种子以确保云的生成一致
        std::uniform_real_distribution<float> widthDist(5.0f, 15.0f);
        std::uniform_real_distribution<float> depthDist(5.0f, 15.0f);
        std::uniform_real_distribution<float> opacityDist(0.5f, 0.8f);
        std::uniform_real_distribution<float> xDist(-100.0f, 100.0f);
        std::uniform_real_distribution<float> zDist(-100.0f, 100.0f);
        
        // 生成足够多的云
        for (int i = 0; i < 100; i++) {
            Cloud cloud;
            cloud.position = Vec3(xDist(rng), cloudHeight, zDist(rng));
            cloud.width = widthDist(rng);
            cloud.depth = depthDist(rng);
            cloud.opacity = opacityDist(rng);
            clouds.push_back(cloud);
        }
        
        // 初始化太阳位置（放在远处的高空）
        sunPosition = Vec3(500.0f, 200.0f, 500.0f);
        
        // 默认启用云和太阳
        cloudsEnabled = true;
        sunEnabled = true;
        
        // 初始化游戏时间
        gameTime = 0.0f;
    }
    
    // 渲染云
    void renderClouds(const Camera& camera) {
        if (!cloudsEnabled) return;
        
        // 遍历所有云朵
        for (const auto& cloud : clouds) {
            // 计算云朵的边界
            float halfWidth = cloud.width / 2.0f;
            float halfDepth = cloud.depth / 2.0f;
            
            // 创建云朵的顶部面
            Vec3 cloudTopVerts[4] = {
                Vec3(cloud.position.x - halfWidth, cloud.position.y, cloud.position.z - halfDepth),
                Vec3(cloud.position.x + halfWidth, cloud.position.y, cloud.position.z - halfDepth),
                Vec3(cloud.position.x + halfWidth, cloud.position.y, cloud.position.z + halfDepth),
                Vec3(cloud.position.x - halfWidth, cloud.position.y, cloud.position.z + halfDepth)
            };
            
            // 创建云朵的底部面（稍微低一些）
            Vec3 cloudBottomVerts[4] = {
                Vec3(cloud.position.x - halfWidth, cloud.position.y - 1.0f, cloud.position.z - halfDepth),
                Vec3(cloud.position.x + halfWidth, cloud.position.y - 1.0f, cloud.position.z - halfDepth),
                Vec3(cloud.position.x + halfWidth, cloud.position.y - 1.0f, cloud.position.z + halfDepth),
                Vec3(cloud.position.x - halfWidth, cloud.position.y - 1.0f, cloud.position.z + halfDepth)
            };
            
            // 创建云的颜色（白色半透明）
            Color cloudColor(255, 255, 255, static_cast<uint8_t>(cloud.opacity * 255));
            
            // 渲染云的顶部和底部面
            gpuRenderer.DrawCloudFace(cloudTopVerts, cloudColor);
            gpuRenderer.DrawCloudFace(cloudBottomVerts, cloudColor);
            
            // 渲染云的侧面
            for (int i = 0; i < 4; i++) {
                int nextI = (i + 1) % 4;
                Vec3 sideVerts[4] = {
                    cloudTopVerts[i],
                    cloudTopVerts[nextI],
                    cloudBottomVerts[nextI],
                    cloudBottomVerts[i]
                };
                gpuRenderer.DrawCloudFace(sideVerts, cloudColor);
            }
        }
    }
    
    // 更新云的位置
    void updateClouds(float deltaTime) {
        if (!cloudsEnabled) return;
        
        // 更新游戏时间
        gameTime += deltaTime;
        
        // 移动所有云朵
        for (auto& cloud : clouds) {
            // 沿X轴移动云（可以根据需要调整方向）
            cloud.position.x += cloudMovementSpeed * deltaTime;
            
            // 如果云移出了视野范围，将其移到另一侧
            if (cloud.position.x > 150.0f) {
                cloud.position.x = -150.0f;
            }
        }
    }
    
    // 渲染太阳
    void renderSun(const Camera& camera) {
        if (!sunEnabled) return;
        
        // 太阳大小
        float sunSize = 50.0f;
        
        // 太阳位置（相对于玩家位置，始终保持在同一方向）
        Vec3 sunDir = Vec3(1.0f, 0.5f, 1.0f).normalize(); // 太阳方向
        Vec3 sunPos = camera.position + sunDir * 500.0f; // 太阳位置（远离玩家）
        
        // 创建太阳的顶点（一个始终面向玩家的正方形）
        Vec3 right = camera.right * sunSize;
        Vec3 up = camera.up * sunSize;
        
        Vec3 sunVerts[4] = {
            sunPos - right - up,
            sunPos + right - up,
            sunPos + right + up,
            sunPos - right + up
        };
        
        // 太阳颜色（纯白色，略带透明）
        Color sunColor(255, 255, 220, 220);
        
        // 渲染太阳
        gpuRenderer.DrawSun(sunVerts, sunColor);
    }
    
public:
    Renderer() : width(0), height(0), frameBuffer(nullptr), hwnd(NULL), uiManager(nullptr) {}
    
    // 设置窗口句柄（需要在初始化前调用）
    void setHWND(HWND hwnd) {
        this->hwnd = hwnd;
    }
    
    // 绘制线段
    void drawLine(int x1, int y1, int x2, int y2, const Color& color) {
        gpuRenderer.DrawLine(x1, y1, x2, y2, color);
    }
    
    // 初始化渲染器
    void init(int width, int height, uint32_t* frameBuffer) {
        this->width = width;
        this->height = height;
        this->frameBuffer = frameBuffer;
        
        // 初始化GPU渲染器
        if (hwnd != NULL) {
            gpuRenderer.Initialize(hwnd, width, height);
        }
        else {
            MessageBox(NULL, "Window handle not set! GPU rendering will not work.", "Error", MB_OK);
        }
        
        // 初始化云和太阳
        initClouds();
    }
    
    // 调整渲染器大小
    void resize(int newWidth, int newHeight, uint32_t* newFrameBuffer) {
        this->width = newWidth;
        this->height = newHeight;
        this->frameBuffer = newFrameBuffer;
        
        // 调整GPU渲染器大小
        gpuRenderer.Resize(newWidth, newHeight);
    }
    
    // Set underground rendering optimization
    void setOptimizeUndergroundRendering(bool optimize) {
        optimizeUndergroundRendering = optimize;
    }
    
    // Get underground rendering optimization state
    bool getOptimizeUndergroundRendering() const {
        return optimizeUndergroundRendering;
    }
    
    // 设置渲染距离
    void setRenderDistance(int distance) {
        // 限制渲染距离在合理范围内
        const int MIN_RENDER_DISTANCE = 3;
        const int MAX_RENDER_DISTANCE = 128;  // 将最大渲染距离从64增大到128
        renderDistance = std::max(MIN_RENDER_DISTANCE, std::min(distance, MAX_RENDER_DISTANCE));
    }
    
    // 获取当前渲染距离
    int getRenderDistance() const {
        return renderDistance;
    }
    
    // Draw crosshair in the center of the screen
    void drawCrosshair() {
        int centerX = width / 2;
        int centerY = height / 2;
        
        // Draw horizontal line
        drawLine(centerX - crosshairSize, centerY, centerX - crosshairThickness, centerY, crosshairColor);
        drawLine(centerX + crosshairThickness, centerY, centerX + crosshairSize, centerY, crosshairColor);
        
        // Draw vertical line
        drawLine(centerX, centerY - crosshairSize, centerX, centerY - crosshairThickness, crosshairColor);
        drawLine(centerX, centerY + crosshairThickness, centerX, centerY + crosshairSize, crosshairColor);
    }
    
    // Update arm swing animation
    void updateArmSwing(float deltaTime, bool isMoving, bool isBreaking) {
        if (isMoving || isBreaking) {
            // Increase swing amount when moving or breaking blocks
            armSwingAmount += deltaTime * armSwingSpeed * (isBreaking ? 2.0f : 1.0f);
            if (armSwingAmount > 1.0f) {
                armSwingAmount = 0.0f;
            }
        } else {
            // Reset swing when not moving
            armSwingAmount = std::max(0.0f, armSwingAmount - deltaTime * armSwingSpeed * 0.5f);
        }
        
        // Calculate swing angle based on swing amount
        armSwingAngle = sin(armSwingAmount * 3.14159f * 2.0f) * 15.0f;
    }
    
    // Draw player arm in 3D - 已移除
    void drawArm(const Camera& camera [[maybe_unused]]) {
        // 手臂渲染功能已被移除
        // 不执行任何操作
    }
    
    // Get width
    int getWidth() const {
        return width;
    }
    
    // Get height
    int getHeight() const {
        return height;
    }
    
    // 绘制矩形边框
    void drawRectOutline(int x, int y, int width, int height, const Color& color) {
        gpuRenderer.DrawRectOutline(x, y, width, height, color);
    }
    
    // 绘制矩形
    void drawRect(int x, int y, int width, int height, const Color& color) {
        gpuRenderer.DrawRect(x, y, width, height, color);
    }
    
    // 绘制文本 - 使用GPU渲染
    void drawText(int x, int y, const std::string& text, const Color& color) {
        gpuRenderer.DrawText(x, y, text, color);
    }
    
    // Clear screen
    void clear(const Color& color = Color(135, 206, 235)) { // 默认天空蓝色
        // 开始场景渲染，传入背景颜色
        gpuRenderer.BeginScene(color);
    }
    
    // 渲染世界 - GPU渲染版本
    void renderWorld(const World& world, const Camera& camera);
    
    // 渲染结束
    void endFrame() {
        // 在GPU渲染器上显示统计信息
        const AdapterInfo& adapterInfo = gpuRenderer.GetAdapterInfo();
        
        // 显示GPU信息
        std::string gpuInfo = "===== GPU Information =====\n";
        gpuInfo += "Selected GPU: " + std::string(adapterInfo.identifier.Description) + "\n";
        gpuInfo += "Vendor ID: 0x" + std::to_string(adapterInfo.identifier.VendorId) + "\n";
        gpuInfo += "Device ID: 0x" + std::to_string(adapterInfo.identifier.DeviceId) + "\n";
        gpuInfo += "VRAM: " + std::to_string(adapterInfo.vramMB) + " MB\n";
        gpuInfo += "GPU Type: " + std::string(adapterInfo.isDiscrete ? "Discrete" : "Integrated") + "\n";
        
        std::string vendorName = "Unknown";
        if (adapterInfo.isNVIDIA) vendorName = "NVIDIA";
        else if (adapterInfo.isAMD) vendorName = "AMD";
        else if (adapterInfo.isIntel) vendorName = "Intel";
        
        gpuInfo += "Vendor: " + vendorName + "\n";
        gpuInfo += "Driver Version: " + std::to_string(adapterInfo.identifier.DriverVersion.QuadPart) + "\n";
        gpuInfo += "==========================\n";
        
        // 在调试输出中打印GPU信息
        OutputDebugStringA(gpuInfo.c_str());
        
        // 获取渲染统计信息并计算帧率
        static DWORD lastFrameTime = GetTickCount();
        DWORD currentTime = GetTickCount();
        static int frameCount = 0;
        
        frameCount++;
        if (currentTime - lastFrameTime >= 1000) {
            float fps = static_cast<float>(frameCount) * 1000.0f / static_cast<float>(currentTime - lastFrameTime);
            
            // 打印帧率和统计信息
            std::string stats = "FPS: " + std::to_string(static_cast<int>(fps)) + 
                              " | Triangles: " + std::to_string(gpuRenderer.GetTrianglesRendered()) + 
                              " | Draw Calls: " + std::to_string(gpuRenderer.GetDrawCalls());
            
            OutputDebugStringA(stats.c_str());
            
            lastFrameTime = currentTime;
            frameCount = 0;
        }
        
        // 结束场景渲染
        gpuRenderer.EndScene();
    }
    
    // 获取GPU适配器信息
    const AdapterInfo& getGPUAdapterInfo() const {
        return gpuRenderer.GetAdapterInfo();
    }
    
    // 检查方块是否在视锥体内
    bool isBlockInFrustum(const Vec3& blockPos, const Camera& camera) {
        // 计算方块到相机的向量
        Vec3 dirToBlock = blockPos - camera.position;
        float distance = dirToBlock.length();
        
        // 距离检测 - 使用renderDistance而不是硬编码的值
        if (distance > static_cast<float>(renderDistance)) return false;
        
        // 归一化方向向量，用于视锥体检测
        Vec3 normalizedDir = dirToBlock.normalize();
        
        // 计算方块中心点在相机前向方向上的投影
        float dotProduct = normalizedDir.dot(camera.front);
        
        // 如果方块在相机后方且不是很近，则不可见（宽松检测，增加容错）
        // 距离很近的方块总是可见，防止近距离物体被错误剔除
        if (dotProduct < -0.1f && distance > 3.0f) return false;
        
        // 计算视场角的余弦值 (FOV/2)
        float fovRad = camera.fov * 0.5f;
        float fovCos = cosf(fovRad);
        
        // 考虑方块大小，增加一定的容错
        // 立方体对角线长度约为1.732，我们增加一点容错边界
        float blockRadius = 1.0f; // 更宽松的方块半径
        
        // 调整视锥体角度，增加边界容错
        float angleFactor = 1.1f; // 扩大视锥体角度10%
        float adjustedFovCos = cos(fovRad * angleFactor);
        
        // 如果方向向量与相机前向量的夹角大于FOV/2，且考虑了方块半径，则不在视锥体内
        // 使用余弦比较避免计算反余弦
        float angleCos = dotProduct;
        
        // 根据距离动态调整视锥体边界，远处物体使用更宽松的判断
        float distanceFactor = std::min(1.0f, distance / 32.0f); // 0-1之间的系数
        float boundaryExtension = 0.1f + distanceFactor * 0.2f;  // 0.1-0.3之间的边界扩展
        float adjustedCos = adjustedFovCos - boundaryExtension;
        
        // 如果方块距离很近，始终认为它可见（防止近处方块被错误剔除）
        if (distance < 5.0f) return true;
        
        // 对于边缘情况，额外宽松处理
        if (angleCos > adjustedCos - blockRadius / distance) return true;
        
        // 最后的判断，更宽松的条件
        return angleCos > adjustedCos;
    }
    
    // 更新时间，添加云的更新
    void updateTime(float deltaTime) {
        // 更新游戏时间
        gameTime += deltaTime;
        
        // 更新云的位置
        updateClouds(deltaTime);
    }
    
    // 清理过期的区块缓存
    void cleanupChunkCache() {
        // 删除过期的区块
        loadedChunks.erase(
            std::remove_if(loadedChunks.begin(), loadedChunks.end(),
                [this](const ChunkInfo& chunk) {
                    return gameTime - chunk.lastSeenTime > CHUNK_CACHE_LIFETIME;
                }
            ),
            loadedChunks.end()
        );
    }
    
    // 检查区块是否在缓存中
    bool isChunkLoaded(int chunkX, int chunkY, int chunkZ) const {
        for (const auto& chunk : loadedChunks) {
            if (chunk.x == chunkX && chunk.y == chunkY && chunk.z == chunkZ) {
                return true;
            }
        }
        return false;
    }
    
    // 更新区块缓存
    void updateChunkCache(int chunkX, int chunkY, int chunkZ, bool inView) {
        // 尝试更新现有的区块记录
        for (auto& chunk : loadedChunks) {
            if (chunk.x == chunkX && chunk.y == chunkY && chunk.z == chunkZ) {
                chunk.lastSeenTime = gameTime;
                chunk.inViewFrustum = inView;
                return;
            }
        }
        
        // 如果不存在，添加新记录
        if (loadedChunks.size() < MAX_CACHED_CHUNKS) {
            loadedChunks.push_back({chunkX, chunkY, chunkZ, gameTime, inView});
        }
        else {
            // 找到最旧的且不在视野中的区块替换
            auto oldestIt = std::min_element(loadedChunks.begin(), loadedChunks.end(),
                [this](const ChunkInfo& a, const ChunkInfo& b) {
                    // 优先考虑不在视野中的区块
                    if (a.inViewFrustum != b.inViewFrustum) {
                        return !a.inViewFrustum;
                    }
                    return a.lastSeenTime < b.lastSeenTime;
                });
                
            if (oldestIt != loadedChunks.end()) {
                *oldestIt = {chunkX, chunkY, chunkZ, gameTime, inView};
            }
        }
    }
    
    // 设置UI管理器
    void setUIManager(UIManager* manager) {
        uiManager = manager;
    }
};

#endif // RENDERER_H
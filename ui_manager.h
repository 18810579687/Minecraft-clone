#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>  // 添加线程支持
#include <windows.h>
#include "renderer.h"
#include "camera.h"
#include "world.h"
#include "music.h" // 添加音乐头文件

// 用于跟踪自定义音乐播放准备状态的全局变量
static bool isPreparingCustomMusic = false;

// 游戏状态枚举
enum GameState {
    GAME_PLAYING,    // Game in progress
    GAME_PAUSED,     // Game paused (menu open)
    GAME_INVENTORY   // Inventory open
};

// 物品类型枚举
enum ItemType {
    ITEM_DIRT,
    ITEM_GRASS,
    ITEM_STONE,
    ITEM_SAND,
    ITEM_WOOD,
    ITEM_LEAVES,
    ITEM_COAL,
    ITEM_IRON,
    ITEM_GOLD,
    ITEM_DIAMOND,
    ITEM_BEDROCK,
    ITEM_OBSIDIAN,
    ITEM_LAVA,
    ITEM_REDSTONE_ORE,
    ITEM_EMERALD_ORE,
    ITEM_MOSSY_STONE,
    ITEM_GRAVEL,
    ITEM_CLAY,
    ITEM_ICE,
    ITEM_SNOW,
    ITEM_WATER,
    ITEM_SANDSTONE,
    ITEM_CACTUS,
    ITEM_PUMPKIN,
    ITEM_NETHERRACK,
    ITEM_SOUL_SAND,
    ITEM_QUARTZ,
    ITEM_GLOWSTONE,
    ITEM_MYCELIUM,
    ITEM_END_STONE,
    ITEM_PRISMARINE,
    ITEM_MAGMA,
    ITEM_NETHER_WART,
    ITEM_SLIME,
    ITEM_BRICK,      // 添加砖块物品类型
    ITEM_BOOKSHELF,  // 添加书架物品类型
    ITEM_CHANGE_BLOCK, // 可自定义方块
    ITEM_COUNT
};

// 物品类
class Item {
public:
    ItemType type;
    int count;
    bool isEmpty;  // 新增标志表示物品是否为空
    
    Item(ItemType type, int count = 1) : type(type), count(count), isEmpty(false) {}
    
    // 添加默认构造函数，创建空物品
    Item() : type(ITEM_DIRT), count(0), isEmpty(true) {}
    
    // 获取物品颜色
    Color getColor() const {
        if (isEmpty) return Color(0, 0, 0, 0); // 空物品返回完全透明的颜色
        
        switch (type) {
            case ITEM_DIRT: return Color(121, 85, 58);
            case ITEM_GRASS: return Color(95, 159, 53);
            case ITEM_STONE: return Color(127, 127, 127);
            case ITEM_SAND: return Color(194, 178, 128);
            case ITEM_WOOD: return Color(119, 89, 55);
            case ITEM_LEAVES: return Color(60, 143, 72);
            case ITEM_COAL: return Color(50, 50, 50);
            case ITEM_IRON: return Color(180, 180, 180);
            case ITEM_GOLD: return Color(255, 215, 0);
            case ITEM_DIAMOND: return Color(0, 191, 255);
            case ITEM_BEDROCK: return Color(40, 40, 40);
            case ITEM_OBSIDIAN: return Color(20, 18, 29);
            case ITEM_LAVA: return Color(207, 16, 32);
            case ITEM_REDSTONE_ORE: return Color(255, 0, 0);
            case ITEM_EMERALD_ORE: return Color(0, 217, 58);
            case ITEM_MOSSY_STONE: return Color(90, 108, 90);
            case ITEM_GRAVEL: return Color(136, 126, 126);
            case ITEM_CLAY: return Color(159, 164, 177);
            case ITEM_ICE: return Color(160, 188, 255);
            case ITEM_SNOW: return Color(250, 250, 250);
            case ITEM_WATER: return Color(39, 112, 145);
            case ITEM_SANDSTONE: return Color(219, 211, 160);
            case ITEM_CACTUS: return Color(76, 120, 45);
            case ITEM_PUMPKIN: return Color(227, 144, 29);
            case ITEM_NETHERRACK: return Color(111, 54, 52);
            case ITEM_SOUL_SAND: return Color(84, 64, 51);
            case ITEM_QUARTZ: return Color(236, 233, 226);
            case ITEM_GLOWSTONE: return Color(249, 212, 77);
            case ITEM_MYCELIUM: return Color(114, 88, 110);
            case ITEM_END_STONE: return Color(221, 223, 165);
            case ITEM_PRISMARINE: return Color(99, 156, 151);
            case ITEM_MAGMA: return Color(155, 57, 9);
            case ITEM_NETHER_WART: return Color(153, 42, 42);
            case ITEM_SLIME: return Color(121, 200, 101);
            case ITEM_BRICK: return Color(150, 75, 75);       // 添加砖块颜色
            case ITEM_BOOKSHELF: return Color(162, 118, 74);  // 添加书架颜色
            case ITEM_CHANGE_BLOCK: return Color(200, 200, 200); // 可变方块默认为亮灰色
            default: return Color(255, 0, 255);
        }
    }
    
    // 获取物品名称
    std::string getName() const {
        if (isEmpty) return ""; // 空物品不返回名称
        
        switch (type) {
            case ITEM_DIRT: return "Dirt";
            case ITEM_GRASS: return "Grass Block";
            case ITEM_STONE: return "Stone";
            case ITEM_SAND: return "Sand";
            case ITEM_WOOD: return "Wood";
            case ITEM_LEAVES: return "Leaves";
            case ITEM_COAL: return "Coal";
            case ITEM_IRON: return "Iron Ingot";
            case ITEM_GOLD: return "Gold Ingot";
            case ITEM_DIAMOND: return "Diamond";
            case ITEM_BEDROCK: return "Bedrock";
            case ITEM_OBSIDIAN: return "Obsidian";
            case ITEM_LAVA: return "Lava";
            case ITEM_REDSTONE_ORE: return "Redstone Ore";
            case ITEM_EMERALD_ORE: return "Emerald Ore";
            case ITEM_MOSSY_STONE: return "Mossy Stone";
            case ITEM_GRAVEL: return "Gravel";
            case ITEM_CLAY: return "Clay";
            case ITEM_ICE: return "Ice";
            case ITEM_SNOW: return "Snow";
            case ITEM_WATER: return "Water";
            case ITEM_SANDSTONE: return "Sandstone";
            case ITEM_CACTUS: return "Cactus";
            case ITEM_PUMPKIN: return "Pumpkin";
            case ITEM_NETHERRACK: return "Netherrack";
            case ITEM_SOUL_SAND: return "Soul Sand";
            case ITEM_QUARTZ: return "Nether Quartz";
            case ITEM_GLOWSTONE: return "Glowstone";
            case ITEM_MYCELIUM: return "Mycelium";
            case ITEM_END_STONE: return "End Stone";
            case ITEM_PRISMARINE: return "Prismarine";
            case ITEM_MAGMA: return "Magma Block";
            case ITEM_NETHER_WART: return "Nether Wart Block";
            case ITEM_SLIME: return "Slime Block";
            case ITEM_BRICK: return "Brick Block";      // 添加砖块名称
            case ITEM_BOOKSHELF: return "Bookshelf";    // 添加书架名称
            case ITEM_CHANGE_BLOCK: return "Change Block";  // 添加可变方块名称
            default: return "Unknown Item";
        }
    }
};

// UI管理器类，负责游戏中所有UI元素的渲染
class UIManager {
private:
    int screenWidth;
    int screenHeight;
    
    // 游戏状态
    GameState gameState = GAME_PLAYING;
    
    // 方块编辑状态
    bool showBlockEditor = false;
    Face selectedBlockFace = FACE_FRONT; // 当前选择的方块面
    Color faceColors[FACE_COUNT]; // 六个面的自定义颜色
    Color savedFaceColors[FACE_COUNT]; // 保存之前保存的颜色配置
    Block templateChangeBlock; // 保存当前编辑中的方块模板
    
    // 3D立方体预览相关
    float cubeRotationY = 0.0f; // Y轴旋转角度
    float cubeRotationX = 0.0f; // X轴旋转角度
    std::chrono::steady_clock::time_point lastCubeUpdateTime = std::chrono::steady_clock::now(); // 上一次更新时间
    
    // 菜单操作结果
    int menuActionResult = 1; // 默认值1表示继续游戏
    
    // 物品栏
    std::vector<Item> inventory;
    int selectedSlot = 0;
    const int MAX_HOTBAR_SLOTS = 9;
    
    // 菜单选项
    std::vector<std::string> menuOptions = {
        "Continue Game",
        "Control Tips",
        "Options",
        "Reset World",
        "Exit Game"
    };
    int selectedMenuOption = 0;
    
    // 当前手持方块类型
    BlockType currentBlockType = BLOCK_DIRT;
    
    // 方块名称提示
    std::string currentBlockName = "";
    float blockNameDisplayTime = 0.0f;
    const float MAX_BLOCK_NAME_DISPLAY_TIME = 2.0f; // 方块名称显示时间
    
    // 鼠标悬停的物品提示
    std::string hoverItemName = "";
    bool showItemTooltip = false;
    int tooltipX = 0;
    int tooltipY = 0;
    
    // 音乐相关
    float musicVolumeValue = 0.5f; // 音乐音量值 (0.0-1.0)
    std::unique_ptr<BGM> backgroundMusic = nullptr; // 背景音乐
    bool musicEnabled = true; // 音乐是否启用
    std::chrono::steady_clock::time_point lastMusicPlayTime; // 上次播放音乐的时间
    const std::chrono::seconds musicPlayInterval{180}; // 音乐播放间隔(秒)
    bool musicInitialized = false; // 音乐是否已初始化
    bool shouldPlayMusic = false; // 是否应该播放音乐
    int randomMusicDelay = 0; // 随机延迟时间(秒)
    bool isPlayingCustomMusic = false; // 标记是否正在播放自定义音乐
    bool isMusicActuallyStopped = true; // 新增标志：音乐是否真正被停止
    
    // 音乐选择相关
    enum MusicType {
        MUSIC_HAGGSTROM,
        MUSIC_HAGGSTROM_VARIANT,
        MUSIC_COUNT
    };
    
    MusicType currentMusicType = MUSIC_HAGGSTROM; // 当前音乐类型
    
    // 物品栏替换提示
    bool showReplacePrompt = false;
    BlockType blockToReplace = BLOCK_AIR;
    int replacePromptX = 0;
    int replacePromptY = 0;
    
    // UI显示控制
    bool showUI = true;
    
    // 种子相关
    std::string worldSeed = "";
    bool isSeedInputActive = false;
    bool m_isSystemGeneratedSeed = true;
    int cursorBlinkTimer = 0;
    bool showCursor = true;
    bool showSeedSavedMessage = false;  // 是否显示保存种子确认消息
    int seedSavedMessageTimer = 0;      // 确认消息显示倒计时
    int seedInputY = 0;                 // 种子输入框的Y坐标
    
    // 存储世界信息的成员变量
    int worldWidth = 0;
    int worldHeight = 0;
    int worldDepth = 0;
    unsigned int worldSeedValue = 0;
    
    // 保存提示相关
    bool showSavePrompt = false; // 是否显示保存提示
    std::string savePromptText = ""; // 保存提示文本
    Color savePromptColor = Color(255, 255, 255); // 保存提示颜色
    int savePromptTimer = 0; // 保存提示计时器
    const int MAX_SAVE_PROMPT_TIME = 60; // 提示显示时间 (约1秒)
    
    // 绘制文本（使用系统字体库的简化版本）
    void drawText(Renderer& renderer, const std::string& text, int x, int y, const Color& color) {
        // 直接调用renderer的drawText方法，不要交换参数顺序或做额外处理
        // 修正坐标顺序，确保x和y参数被正确传递
        renderer.drawText(x, y, text, color);
    }
    
    // 绘制3D立方体的辅助方法
    void drawCubeFace(Renderer& renderer, const Vec3 vertices[4], const Color& color, bool outline = true) {
        // 使用三角形填充方式绘制面
        // 将四边形分为两个三角形
        
        // 计算三角形的顶点坐标
        int x1 = static_cast<int>(vertices[0].x);
        int y1 = static_cast<int>(vertices[0].y);
        int x2 = static_cast<int>(vertices[1].x);
        int y2 = static_cast<int>(vertices[1].y);
        int x3 = static_cast<int>(vertices[2].x);
        int y3 = static_cast<int>(vertices[2].y);
        int x4 = static_cast<int>(vertices[3].x);
        int y4 = static_cast<int>(vertices[3].y);
        
        // 绘制填充的三角形
        fillTriangle(renderer, x1, y1, x2, y2, x3, y3, color);
        fillTriangle(renderer, x1, y1, x3, y3, x4, y4, color);
        
        // 绘制边框
        if (outline) {
            for (int i = 0; i < 4; i++) {
                int next = (i + 1) % 4;
                renderer.drawLine(
                    static_cast<int>(vertices[i].x),
                    static_cast<int>(vertices[i].y),
                    static_cast<int>(vertices[next].x),
                    static_cast<int>(vertices[next].y),
                    Color(0, 0, 0)
                );
            }
        }
    }
    
    // 辅助方法：填充三角形
    void fillTriangle(Renderer& renderer, int x1, int y1, int x2, int y2, int x3, int y3, const Color& color) {
        // 按y坐标排序顶点
        if (y1 > y2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        if (y1 > y3) {
            std::swap(x1, x3);
            std::swap(y1, y3);
        }
        if (y2 > y3) {
            std::swap(x2, x3);
            std::swap(y2, y3);
        }
        
        // 如果三角形退化为一条线或一个点，直接返回
        if (y1 == y3) return;
        
        // 计算边的斜率
        float dx1 = 0, dx2 = 0, dx3 = 0;
        if (y2 - y1 > 0) dx1 = static_cast<float>(x2 - x1) / (y2 - y1);
        if (y3 - y1 > 0) dx2 = static_cast<float>(x3 - x1) / (y3 - y1);
        if (y3 - y2 > 0) dx3 = static_cast<float>(x3 - x2) / (y3 - y2);
        
        // 起始点
        float sx = static_cast<float>(x1);
        float ex = static_cast<float>(x1);
        
        // 填充上半部分三角形
        if (y2 > y1) {
            for (int y = y1; y < y2; y++) {
                renderer.drawLine(static_cast<int>(sx), y, static_cast<int>(ex), y, color);
                sx += dx1;
                ex += dx2;
            }
        }
        
        // 填充下半部分三角形
        if (y3 > y2) {
            // 更新起始点
            sx = static_cast<float>(x2);
            for (int y = y2; y <= y3; y++) {
                renderer.drawLine(static_cast<int>(sx), y, static_cast<int>(ex), y, color);
                sx += dx3;
                ex += dx2;
            }
        }
    }
    
    // 绘制3D旋转立方体预览
    void draw3DRotatingCube(Renderer& renderer, int centerX, int centerY, int size, const Color faceColors[FACE_COUNT]) {
        // 定义一个立方体的8个顶点（3D空间中）
        Vec3 vertices[8] = {
            Vec3(-1.0f, -1.0f, -1.0f), // 0 前左下
            Vec3( 1.0f, -1.0f, -1.0f), // 1 前右下
            Vec3( 1.0f,  1.0f, -1.0f), // 2 前右上
            Vec3(-1.0f,  1.0f, -1.0f), // 3 前左上
            Vec3(-1.0f, -1.0f,  1.0f), // 4 后左下
            Vec3( 1.0f, -1.0f,  1.0f), // 5 后右下
            Vec3( 1.0f,  1.0f,  1.0f), // 6 后右上
            Vec3(-1.0f,  1.0f,  1.0f)  // 7 后左上
        };
        
        // 应用旋转矩阵
        float radX = cubeRotationX * 3.14159f / 180.0f;
        float radY = cubeRotationY * 3.14159f / 180.0f;
        
        // 先创建旋转后的顶点副本
        Vec3 rotatedVertices[8];
        
        // 应用Y轴旋转
        for (int i = 0; i < 8; i++) {
            rotatedVertices[i].x = vertices[i].x * cosf(radY) - vertices[i].z * sinf(radY);
            rotatedVertices[i].y = vertices[i].y;
            rotatedVertices[i].z = vertices[i].x * sinf(radY) + vertices[i].z * cosf(radY);
        }
        
        // 应用X轴旋转
        for (int i = 0; i < 8; i++) {
            vertices[i].x = rotatedVertices[i].x;
            vertices[i].y = rotatedVertices[i].y * cosf(radX) - rotatedVertices[i].z * sinf(radX);
            vertices[i].z = rotatedVertices[i].y * sinf(radX) + rotatedVertices[i].z * cosf(radX);
        }
        
        // 应用缩放和平移（将立方体放在预览位置）
        for (int i = 0; i < 8; i++) {
            vertices[i].x = vertices[i].x * size / 2 + centerX;
            vertices[i].y = vertices[i].y * size / 2 + centerY;
        }
        
        // 计算每个面的中心点Z坐标，用于Z-sorting
        float faceZ[6];
        faceZ[FACE_FRONT] = (vertices[0].z + vertices[1].z + vertices[2].z + vertices[3].z) / 4.0f;
        faceZ[FACE_BACK] = (vertices[4].z + vertices[5].z + vertices[6].z + vertices[7].z) / 4.0f;
        faceZ[FACE_LEFT] = (vertices[0].z + vertices[3].z + vertices[7].z + vertices[4].z) / 4.0f;
        faceZ[FACE_RIGHT] = (vertices[1].z + vertices[2].z + vertices[6].z + vertices[5].z) / 4.0f;
        faceZ[FACE_TOP] = (vertices[3].z + vertices[2].z + vertices[6].z + vertices[7].z) / 4.0f;
        faceZ[FACE_BOTTOM] = (vertices[0].z + vertices[1].z + vertices[5].z + vertices[4].z) / 4.0f;
        
        // 创建面索引数组，用于按深度排序
        int faceIndices[6] = {FACE_FRONT, FACE_BACK, FACE_LEFT, FACE_RIGHT, FACE_TOP, FACE_BOTTOM};
        
        // 简单的冒泡排序，按Z值从小到大排序（绘制从后往前）
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5 - i; j++) {
                if (faceZ[faceIndices[j]] > faceZ[faceIndices[j + 1]]) {
                    std::swap(faceIndices[j], faceIndices[j + 1]);
                }
            }
        }
        
        // 按排序后的顺序绘制面（从后往前）
        for (int i = 0; i < 6; i++) {
            int faceIndex = faceIndices[i];
            Vec3 faceVertices[4];
            
            // 根据面索引选择对应的顶点
            switch (faceIndex) {
                case FACE_FRONT: // 前面
                    faceVertices[0] = vertices[0];
                    faceVertices[1] = vertices[1];
                    faceVertices[2] = vertices[2];
                    faceVertices[3] = vertices[3];
                    break;
                case FACE_BACK: // 后面
                    faceVertices[0] = vertices[5];
                    faceVertices[1] = vertices[4];
                    faceVertices[2] = vertices[7];
                    faceVertices[3] = vertices[6];
                    break;
                case FACE_LEFT: // 左面
                    faceVertices[0] = vertices[4];
                    faceVertices[1] = vertices[0];
                    faceVertices[2] = vertices[3];
                    faceVertices[3] = vertices[7];
                    break;
                case FACE_RIGHT: // 右面
                    faceVertices[0] = vertices[1];
                    faceVertices[1] = vertices[5];
                    faceVertices[2] = vertices[6];
                    faceVertices[3] = vertices[2];
                    break;
                case FACE_TOP: // 顶面
                    faceVertices[0] = vertices[3];
                    faceVertices[1] = vertices[2];
                    faceVertices[2] = vertices[6];
                    faceVertices[3] = vertices[7];
                    break;
                case FACE_BOTTOM: // 底面
                    faceVertices[0] = vertices[4];
                    faceVertices[1] = vertices[5];
                    faceVertices[2] = vertices[1];
                    faceVertices[3] = vertices[0];
                    break;
            }
            
            // 绘制面
            Color faceColor = faceColors[faceIndex];
            // 根据面的朝向添加一点阴影效果
            if (faceIndex == FACE_LEFT || faceIndex == FACE_RIGHT) {
                faceColor.r = static_cast<uint8_t>(faceColor.r * 0.85f);
                faceColor.g = static_cast<uint8_t>(faceColor.g * 0.85f);
                faceColor.b = static_cast<uint8_t>(faceColor.b * 0.85f);
            } else if (faceIndex == FACE_TOP) {
                faceColor.r = static_cast<uint8_t>(faceColor.r * 1.15f);
                faceColor.g = static_cast<uint8_t>(faceColor.g * 1.15f);
                faceColor.b = static_cast<uint8_t>(faceColor.b * 1.15f);
            } else if (faceIndex == FACE_BOTTOM) {
                faceColor.r = static_cast<uint8_t>(faceColor.r * 0.75f);
                faceColor.g = static_cast<uint8_t>(faceColor.g * 0.75f);
                faceColor.b = static_cast<uint8_t>(faceColor.b * 0.75f);
            }
            
            drawCubeFace(renderer, faceVertices, faceColor);
        }
    }
    
    // 绘制物品栏
    void drawInventory(Renderer& renderer) {
        // 调整物品栏尺寸以适应屏幕
        int slotSize = screenHeight / 15; // 根据屏幕高度调整槽位大小
        int padding = slotSize / 10;
        int totalWidth = (slotSize + padding) * MAX_HOTBAR_SLOTS - padding;
        int startX = (screenWidth - totalWidth) / 2;
        int startY = screenHeight - slotSize - 10;
        
        // 绘制物品栏背景
        renderer.drawRect(startX - 5, startY - 5, totalWidth + 10, slotSize + 10, Color(0, 0, 0, 150));
        
        // 绘制物品槽
        for (int i = 0; i < MAX_HOTBAR_SLOTS; i++) {
            int slotX = startX + i * (slotSize + padding);
            int slotY = startY;
            
            // 绘制槽背景
            Color bgColor = (i == selectedSlot) ? Color(255, 255, 255, 100) : Color(80, 80, 80, 100);
            renderer.drawRect(slotX, slotY, slotSize, slotSize, bgColor);
            
            // 如果槽中有物品，绘制物品
            if (i < static_cast<int>(inventory.size())) {
                // 只有非空物品才绘制
                if (!inventory[i].isEmpty && inventory[i].count > 0) {
                // 绘制物品图标
                renderer.drawRect(slotX + 4, slotY + 4, slotSize - 8, slotSize - 8, inventory[i].getColor());
                
                // 绘制物品数量
                if (inventory[i].count > 1) {
                    std::string countText = std::to_string(inventory[i].count);
                    drawText(renderer, countText, slotX + slotSize - 15, slotY + slotSize - 15, Color(255, 255, 255));
                    }
                }
            }
            
            // 绘制槽边框
            Color borderColor = (i == selectedSlot) ? Color(255, 255, 255) : Color(120, 120, 120);
            renderer.drawLine(slotX, slotY, slotX + slotSize, slotY, borderColor);
            renderer.drawLine(slotX + slotSize, slotY, slotX + slotSize, slotY + slotSize, borderColor);
            renderer.drawLine(slotX, slotY + slotSize, slotX + slotSize, slotY + slotSize, borderColor);
            renderer.drawLine(slotX, slotY, slotX, slotY + slotSize, borderColor);
        }
        
        // 如果是物品栏状态，绘制完整物品栏
        if (gameState == GAME_INVENTORY) {
            // 绘制完整物品栏背景
            int inventoryWidth = totalWidth;
            int inventoryHeight = slotSize * 4;
            int invStartX = (screenWidth - inventoryWidth) / 2;
            int invStartY = (screenHeight - inventoryHeight) / 2;
            
            // 绘制半透明背景
            renderer.drawRect(0, 0, screenWidth, screenHeight, Color(0, 0, 0, 100));
            
            // 绘制物品栏背景
            renderer.drawRect(invStartX - 10, invStartY - 10, 
                             inventoryWidth + 20, inventoryHeight + 20, 
                             Color(50, 50, 50, 200));
            
            // 绘制物品栏边框
            renderer.drawRectOutline(invStartX - 10, invStartY - 10, 
                                    inventoryWidth + 20, inventoryHeight + 20, 
                                    Color(200, 200, 200));
            
            // 绘制物品栏标题
            std::string title = "Inventory";
            drawText(renderer, title, invStartX + (inventoryWidth - title.length() * 10) / 2, 
                    invStartY - 30, Color(255, 255, 255));
        }
    }
    
        // 按钮结构体
    struct Button {
        int x, y, width, height;
        std::string text;
        bool hovered;
        
        Button(int x, int y, int width, int height, const std::string& text)
            : x(x), y(y), width(width), height(height), text(text), hovered(false) {}
        
        bool isMouseOver(int mouseX, int mouseY) const {
            return mouseX >= x && mouseX < x + width && mouseY >= y && mouseY < y + height;
        }
        
        void draw(Renderer& renderer, bool selected = false) {
            // 渐变背景色
            Color topColor = selected ? Color(80, 120, 255, 220) : 
                             hovered ? Color(70, 100, 220, 220) : Color(50, 70, 180, 220);
            Color bottomColor = selected ? Color(50, 90, 220, 220) : 
                                hovered ? Color(40, 70, 180, 220) : Color(30, 50, 150, 220);
            
            // 绘制渐变背景
            int segments = 10;
            int segmentHeight = height / segments;
            for (int i = 0; i < segments; i++) {
                float t = static_cast<float>(i) / (segments - 1);
                Color segmentColor(
                    topColor.r * (1 - t) + bottomColor.r * t,
                    topColor.g * (1 - t) + bottomColor.g * t,
                    topColor.b * (1 - t) + bottomColor.b * t,
                    topColor.a * (1 - t) + bottomColor.a * t
                );
                renderer.drawRect(x, y + i * segmentHeight, width, segmentHeight, segmentColor);
            }
            
            // 绘制漂亮的边框
            Color borderColor = selected ? Color(180, 200, 255) : 
                               hovered ? Color(150, 170, 255) : Color(100, 130, 200);
            
            // 顶部边框（亮一些）
            renderer.drawRect(x, y, width, 2, Color(borderColor.r + 30, borderColor.g + 30, borderColor.b + 30));
            // 左侧边框
            renderer.drawRect(x, y, 2, height, borderColor);
            // 底部边框（暗一些）
            renderer.drawRect(x, y + height - 2, width, 2, Color(borderColor.r - 30, borderColor.g - 30, borderColor.b - 30, 255));
            // 右侧边框
            renderer.drawRect(x + width - 2, y, 2, height, borderColor);
            
            // 绘制文本阴影（微微偏移）
            int textX = x + (width - text.length() * 8) / 2;
            int textY = y + (height - 15) / 2;
            renderer.drawText(textX + 1, textY + 1, text, Color(20, 20, 20, 180));
            
            // 绘制文本
            renderer.drawText(textX, textY, text, Color(255, 255, 255));
            
            // 如果被选中或悬停，添加轻微发光效果
            if (selected || hovered) {
                // 顶部微光
                renderer.drawRect(x + 2, y, width - 4, 1, Color(255, 255, 255, 80));
            }
        }
    };
    
    // 滑块结构体
    struct Slider {
        int x, y, width, height;
        std::string text;
        float value; // Value between 0.0f and 1.0f
        bool dragging;
        
        Slider(int x, int y, int width, int height, const std::string& text, float initialValue = 0.5f)
            : x(x), y(y), width(width), height(height), text(text), value(initialValue), dragging(false) {}
        
        bool isMouseOver(int mouseX, int mouseY) const {
            // 扩大滑块的可点击区域
            int handleX = x + 5 + static_cast<int>((width - 10) * value);
            int handleWidth = 20; // 扩大手柄宽度
            int handleExtendedArea = 10; // 额外的可点击区域
            
            // 检查鼠标是否在滑块轨道上
            bool onTrack = mouseX >= x && mouseX < x + width && 
                         mouseY >= y && mouseY < y + height;
                         
            // 检查鼠标是否在滑块手柄上（扩大判定区域）
            bool onHandle = mouseX >= handleX - handleWidth/2 - handleExtendedArea && 
                          mouseX < handleX + handleWidth/2 + handleExtendedArea && 
                          mouseY >= y - handleExtendedArea && 
                          mouseY < y + height + handleExtendedArea;
                          
            return onTrack || onHandle;
        }
        
        void updateValue(int mouseX) {
            if (dragging) {
                // 计算新值，限制在0-1范围内
                float newValue = static_cast<float>(mouseX - x - 5) / (width - 10);
                value = std::max(0.0f, std::min(1.0f, newValue));
            }
        }
        
        void draw(Renderer& renderer) {
            // 绘制滑块文本 - 位于滑块上方，左对齐
            renderer.drawText(x, y - 25, text, Color(220, 220, 220));
            
            // 绘制滑块背景 - 圆角矩形效果
            // 顶部和底部的阴影
            renderer.drawRect(x + 3, y - 1, width - 6, 1, Color(20, 20, 20, 100));
            renderer.drawRect(x + 3, y + height, width - 6, 1, Color(120, 120, 120, 100));
            
            // 主轨道 - 渐变背景
            Color trackStartColor(50, 50, 60, 200);
            Color trackEndColor(40, 40, 50, 200);
            
            int segments = 5;
            int segmentHeight = height / segments;
            for (int i = 0; i < segments; i++) {
                float t = static_cast<float>(i) / (segments - 1);
                Color segmentColor(
                    trackStartColor.r * (1 - t) + trackEndColor.r * t,
                    trackStartColor.g * (1 - t) + trackEndColor.g * t,
                    trackStartColor.b * (1 - t) + trackEndColor.b * t,
                    trackStartColor.a * (1 - t) + trackEndColor.a * t
                );
                renderer.drawRect(x + 5, y + i * segmentHeight, width - 10, segmentHeight, segmentColor);
            }
            
            // 左侧填充部分 - 已选部分的轨道
            int fillWidth = static_cast<int>((width - 10) * value);
            if (fillWidth > 0) {
                Color fillStartColor(80, 120, 255, 220);
                Color fillEndColor(50, 90, 220, 220);
                
                for (int i = 0; i < segments; i++) {
                    float t = static_cast<float>(i) / (segments - 1);
                    Color segmentColor(
                        fillStartColor.r * (1 - t) + fillEndColor.r * t,
                        fillStartColor.g * (1 - t) + fillEndColor.g * t,
                        fillStartColor.b * (1 - t) + fillEndColor.b * t,
                        fillStartColor.a * (1 - t) + fillEndColor.a * t
                    );
                    renderer.drawRect(x + 5, y + i * segmentHeight, fillWidth, segmentHeight, segmentColor);
                }
            }
            
            // 绘制滑块手柄 - 高亮的立体手柄
            int handleX = x + 5 + static_cast<int>((width - 10) * value);
            int handleWidth = 20;
            int handleHeight = height + 6;
            
            // 手柄背景 - 渐变效果
            Color handleTopColor(180, 200, 255, 255);
            Color handleBottomColor(100, 130, 220, 255);
            
            // 手柄阴影
            renderer.drawRect(handleX - handleWidth/2 + 2, y + height + 1, handleWidth - 4, 2, Color(0, 0, 0, 80));
            
            int handleSegments = 8;
            int handleSegmentHeight = handleHeight / handleSegments;
            for (int i = 0; i < handleSegments; i++) {
                float t = static_cast<float>(i) / (handleSegments - 1);
                Color segmentColor(
                    handleTopColor.r * (1 - t) + handleBottomColor.r * t,
                    handleTopColor.g * (1 - t) + handleBottomColor.g * t,
                    handleTopColor.b * (1 - t) + handleBottomColor.b * t,
                    handleTopColor.a * (1 - t) + handleBottomColor.a * t
                );
                renderer.drawRect(
                    handleX - handleWidth/2, 
                    y - 3 + i * handleSegmentHeight, 
                    handleWidth, 
                    handleSegmentHeight, 
                    segmentColor
                );
            }
            
            // 手柄边框
            renderer.drawRectOutline(handleX - handleWidth/2, y - 3, handleWidth, handleHeight, Color(60, 80, 150));
            
            // 手柄高光
            renderer.drawRect(handleX - handleWidth/2 + 2, y - 3, handleWidth - 4, 2, Color(255, 255, 255, 100));
            
            // 值文本显示 - 移到右侧单独显示
            // 音乐音量滑块将在drawOptionsMenu中特殊处理
            if (text != "Music Volume") {
                std::string valueText = std::to_string(static_cast<int>(value * 100)) + "%";
                // 绘制数值文本，放在滑块右侧
                renderer.drawText(x + width + 15, y + (height - 15) / 2, valueText, Color(220, 220, 220));
            }
        }
    };

    // 绘制暂停菜单
    void drawPauseMenu(Renderer& renderer) {
        // 绘制渐变半透明背景
        for (int y = 0; y < screenHeight; y += 4) {
            float t = static_cast<float>(y) / screenHeight;
            Color bgColor(0, 0, 0, 120 + static_cast<int>(t * 80));
            renderer.drawRect(0, y, screenWidth, 4, bgColor);
        }
        
        // 添加模糊效果模拟 - 绘制一些半透明的矩形
        for (int i = 0; i < 10; i++) {
            int x = i * screenWidth / 10;
            int width = screenWidth / 10;
            renderer.drawRect(x, 0, width, screenHeight, Color(0, 0, 0, 20));
        }
        
        // 绘制菜单标题
        std::string title = "Game Menu";
        int titleY = screenHeight / 6;
        
        // 绘制标题阴影
        drawText(renderer, title, (screenWidth - title.length() * 12) / 2 + 2, titleY + 2, Color(0, 0, 0, 180));
        
        // 绘制标题
        drawText(renderer, title, (screenWidth - title.length() * 12) / 2, titleY, Color(255, 255, 255));
        
        // 绘制标题下方的装饰线
        int lineWidth = std::min(400, screenWidth - 80);
        int lineX = (screenWidth - lineWidth) / 2;
        int lineY = titleY + 30;
        
        // 渐变装饰线
        for (int x = 0; x < lineWidth; x++) {
            float t = static_cast<float>(x) / lineWidth;
            float alpha = sin(t * 3.14159f) * 255.0f;
            Color lineColor(100, 150, 255, static_cast<uint8_t>(alpha));
            renderer.drawRect(lineX + x, lineY, 1, 2, lineColor);
        }
        
        // 获取鼠标位置
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(GetActiveWindow(), &mousePos);
        
        // 计算按钮尺寸和位置 - 根据屏幕大小调整
        int buttonWidth = std::min(350, screenWidth - 100);
        int buttonHeight = 45;
        int buttonSpacing = std::max(20, screenHeight / 20);
        int startY = screenHeight / 3 + 20;
        int startX = (screenWidth - buttonWidth) / 2;
        
        // 检查鼠标点击状态（使用静态变量检测单次点击）
        static bool lastMouseDown = false;
        bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        bool mouseClicked = mouseDown && !lastMouseDown;
        
        // 创建并绘制按钮
        for (size_t i = 0; i < menuOptions.size(); i++) {
            int buttonY = startY + i * (buttonHeight + buttonSpacing);
            Button button(startX, buttonY, buttonWidth, buttonHeight, menuOptions[i]);
            
            // 检查鼠标悬停
            button.hovered = button.isMouseOver(mousePos.x, mousePos.y);
            
            // 如果鼠标悬停在按钮上，自动选中该按钮
            if (button.hovered) {
                selectedMenuOption = static_cast<int>(i);
                
                // 检查鼠标点击，立即执行该选项
                if (mouseClicked) {
                    lastMouseDown = mouseDown;
                menuActionResult = executeSelectedMenuOption();
                    return; // 执行后立即返回，防止在同一帧内执行多个选项
                }
            }
            
            // 绘制按钮
            button.draw(renderer, static_cast<int>(i) == selectedMenuOption);
        }
        
        // 更新鼠标状态
        lastMouseDown = mouseDown;
        
        // 确保鼠标指针可见
        ShowCursor(TRUE);
        
        // 绘制版本信息和版权声明
        std::string versionText = "Version 1.0.0";
        std::string copyrightText = "Complete By Limuran";
        int versionY = screenHeight - 40;
        int copyrightY = screenHeight - 20;
        
        // 小字体版本信息
        renderer.drawText(10, versionY, versionText, Color(150, 150, 150));
        renderer.drawText(10, copyrightY, copyrightText, Color(150, 150, 150));
    }
    
public:
    UIManager(int width, int height) : screenWidth(width), screenHeight(height) {
        // 初始化物品栏 - 按照用户指定的顺序初始化
        inventory.push_back(Item(ITEM_DIRT, 64));      // 第一格：泥土
        inventory.push_back(Item(ITEM_GRASS, 64));     // 第二格：草方块
        inventory.push_back(Item(ITEM_STONE, 64));     // 第三格：石头
        inventory.push_back(Item(ITEM_SAND, 64));      // 第四格：沙子
        inventory.push_back(Item(ITEM_WOOD, 64));      // 第五格：木头
        inventory.push_back(Item(ITEM_LEAVES, 64));    // 第六格：树叶
        inventory.push_back(Item(ITEM_COAL, 64));      // 第七格：煤矿
        inventory.push_back(Item(ITEM_IRON, 64));      // 第八格：铁矿
        inventory.push_back(Item(ITEM_GOLD, 64));      // 第九格：金矿
        
        // 初始化当前选中的方块类型
        updateCurrentBlockType();
        
        // 初始化模板方块
        templateChangeBlock = Block(BLOCK_CHANGE_BLOCK);
        
        // 初始化方块编辑器颜色数组
        for (int i = 0; i < FACE_COUNT; i++) {
            faceColors[i] = Color(200, 200, 200); // 默认为亮灰色
            savedFaceColors[i] = Color(200, 200, 200); // 初始化保存的颜色
        }
        
        // 更新当前方块类型
        updateCurrentBlockType();
        
        // 初始化控制菜单
        initControlsMenu();
        
        // 初始化选项菜单
        initOptionsMenu();
        
        // 初始化音乐系统
        lastMusicPlayTime = std::chrono::steady_clock::now();
        
        // 设置随机延迟时间(30-120秒)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(30, 120);
        randomMusicDelay = distr(gen);
        
        std::cout << "[Music] System initialized. Random delay: " << randomMusicDelay << " seconds" << std::endl;
        
        // 游戏开始时不立即播放音乐,而是等待随机延迟
        shouldPlayMusic = false;
        musicInitialized = false;
        
        // 初始化聊天系统
        chatHistory.clear();
        chatHistoryIsSystem.clear();
        chatMessageTimers.clear();
        chatHistoryIndex = 0;
        showChatBox = false;
        chatInputText = "";
        chatCommandMode = false;
        ignoreNextChar = false;
        
        // 添加欢迎消息
        addSystemMessage("Welcome to Minecraft Clone!");
        addSystemMessage("Press T to chat, / to enter commands");
    }
    
    // 析构函数 - 清理动态分配的资源
    ~UIManager() {
        delete backButton;
        delete controlsBackButton;
    }
    
    // 获取游戏状态
    GameState getGameState() const {
        return gameState;
    }
    
    // 设置游戏状态
    void setGameState(GameState state) {
        gameState = state;
    }
    
    // 切换菜单状态
    void toggleMenu() {
        if (gameState == GAME_PLAYING) {
            // 从游戏状态切换到暂停状态
            gameState = GAME_PAUSED;
            
            // 暂停音乐(如果正在播放)
            if (musicEnabled && backgroundMusic && !isMusicActuallyStopped) {
                std::cout << "[Music] Pausing music due to game pause" << std::endl;
                backgroundMusic->player.pauseMusic = true;
            }
            
            std::cout << "Game paused" << std::endl;
        } else if (gameState == GAME_PAUSED) {
            // 从暂停状态返回游戏
            gameState = GAME_PLAYING;
            
            // 恢复音乐(如果之前在播放)
            if (musicEnabled && backgroundMusic && !isMusicActuallyStopped) {
                std::cout << "[Music] Resuming music after game unpause" << std::endl;
                backgroundMusic->player.pauseMusic = false;
            }
            
            std::cout << "Game resumed" << std::endl;
        } else if (gameState == GAME_INVENTORY) {
            // 从物品栏返回游戏
            gameState = GAME_PLAYING;
            std::cout << "Inventory closed" << std::endl;
        }
    }
    
    // 切换物品栏
    void toggleInventory() {
        if (gameState == GAME_PLAYING) {
            gameState = GAME_INVENTORY;
            std::cout << "Inventory opened" << std::endl;
        } else if (gameState == GAME_INVENTORY) {
            gameState = GAME_PLAYING;
            std::cout << "Inventory closed" << std::endl;
        }
    }
    
    // 选择下一个菜单选项
    void selectNextMenuOption() {
        selectedMenuOption = (selectedMenuOption + 1) % menuOptions.size();
    }
    
    // 选择上一个菜单选项
    void selectPrevMenuOption() {
        selectedMenuOption = (selectedMenuOption - 1 + menuOptions.size()) % menuOptions.size();
    }
    
    // 游戏状态 - 添加控制提示状态和选项状态
    enum ControlsState {
        CONTROLS_NONE,
        CONTROLS_SHOWING
    };
    
    enum OptionsState {
        OPTIONS_NONE,
        OPTIONS_SHOWING
    };
    
    // 当前控制提示状态和选项状态
    ControlsState controlsState = CONTROLS_NONE;
    OptionsState optionsState = OPTIONS_NONE;
    
    // 执行当前选中的菜单选项
    int executeSelectedMenuOption() {
        switch (selectedMenuOption) {
            case 0: // 继续游戏
                gameState = GAME_PLAYING;
                controlsState = CONTROLS_NONE;
                optionsState = OPTIONS_NONE;
                // 锁定鼠标到准心
                lockMouseToCrosshair();
                return 1;
            case 1: // 控制按键提示
                controlsState = CONTROLS_SHOWING;
                optionsState = OPTIONS_NONE;
                // 初始化控制菜单（确保按钮位置正确）
                initControlsMenu();
                return 1;
            case 2: // 选项
                optionsState = OPTIONS_SHOWING;
                controlsState = CONTROLS_NONE;
                // 初始化选项菜单（确保滑块位置正确）
                initOptionsMenu();
                return 1;
            case 3: // 重置世界
                // 返回特殊值2表示需要重置世界
                return 2;
            case 4: // 退出游戏
                return 0; // 返回0表示退出游戏
            default:
                return 1;
        }
    }
    
    // 获取控制提示状态
    ControlsState getControlsState() const {
        return controlsState;
    }
    
    // 设置控制提示状态
    void setControlsState(ControlsState state) {
        controlsState = state;
    }
    
    // 获取选项状态
    OptionsState getOptionsState() const {
        return optionsState;
    }
    
    // 设置选项状态
    void setOptionsState(OptionsState state) {
        optionsState = state;
    }
    
    // 检查是否可以在指定位置放置方块（防止玩家被卡住）
    bool canPlaceBlockAt(const Vec3& blockPos, const Vec3& playerPos, bool flyingMode) {
        // 如果是飞行模式，允许放置
        if (flyingMode) return true;
        
        // 计算方块与玩家的距离
        float distX = std::abs(blockPos.x + 0.5f - playerPos.x);
        float distY = std::abs(blockPos.y + 0.5f - playerPos.y - 0.9f); // 考虑玩家眼睛位置
        float distZ = std::abs(blockPos.z + 0.5f - playerPos.z);
        
        // 玩家碰撞箱大小
        const float PLAYER_RADIUS = 0.3f;
        const float PLAYER_HEIGHT = 1.8f;
        
        // 检查方块是否与玩家碰撞箱重叠或距离过近
        if (distX < PLAYER_RADIUS + 0.5f && 
            distY < PLAYER_HEIGHT/2 + 0.5f && 
            distZ < PLAYER_RADIUS + 0.5f) {
            return false; // 不允许放置，防止玩家被卡住
        }
        
        return true;
    }
    
    // 选择下一个物品栏槽位
    void selectNextSlot() {
        selectedSlot = (selectedSlot + 1) % MAX_HOTBAR_SLOTS;
        if (selectedSlot < static_cast<int>(inventory.size())) {
            // 更新当前手持方块类型
            updateCurrentBlockType();
            
            // 显示当前方块名称（只显示非空物品）
            if (!inventory[selectedSlot].isEmpty && inventory[selectedSlot].count > 0) {
                setCurrentBlockName(inventory[selectedSlot].getName());
            } else {
                setCurrentBlockName("");
            }
        }
    }
    
    // 选择上一个物品栏槽位
    void selectPrevSlot() {
        selectedSlot = (selectedSlot - 1 + MAX_HOTBAR_SLOTS) % MAX_HOTBAR_SLOTS;
        if (selectedSlot < static_cast<int>(inventory.size())) {
            // 更新当前手持方块类型
            updateCurrentBlockType();
            
            // 显示当前方块名称（只显示非空物品）
            if (!inventory[selectedSlot].isEmpty && inventory[selectedSlot].count > 0) {
                setCurrentBlockName(inventory[selectedSlot].getName());
            } else {
                setCurrentBlockName("");
            }
        }
    }
    
    // 更新当前手持方块类型
    void updateCurrentBlockType() {
        if (selectedSlot < static_cast<int>(inventory.size())) {
            // 如果物品是空的，设置为空气方块（不放置任何方块）
            if (inventory[selectedSlot].isEmpty || inventory[selectedSlot].count <= 0) {
                currentBlockType = BLOCK_AIR;
                return;
            }
            
            // 确保物品类型和方块类型正确对应
            switch (inventory[selectedSlot].type) {
                case ITEM_DIRT: currentBlockType = BLOCK_DIRT; break;
                case ITEM_GRASS: currentBlockType = BLOCK_GRASS; break;
                case ITEM_STONE: currentBlockType = BLOCK_STONE; break;
                case ITEM_SAND: currentBlockType = BLOCK_SAND; break;
                case ITEM_WOOD: currentBlockType = BLOCK_WOOD; break;
                case ITEM_LEAVES: currentBlockType = BLOCK_LEAVES; break;
                case ITEM_COAL: currentBlockType = BLOCK_COAL_ORE; break;
                case ITEM_IRON: currentBlockType = BLOCK_IRON_ORE; break;
                case ITEM_GOLD: currentBlockType = BLOCK_GOLD_ORE; break;
                case ITEM_DIAMOND: currentBlockType = BLOCK_DIAMOND_ORE; break;
                case ITEM_BEDROCK: currentBlockType = BLOCK_BEDROCK; break;
                case ITEM_OBSIDIAN: currentBlockType = BLOCK_OBSIDIAN; break;
                case ITEM_LAVA: currentBlockType = BLOCK_LAVA; break;
                case ITEM_REDSTONE_ORE: currentBlockType = BLOCK_REDSTONE_ORE; break;
                case ITEM_EMERALD_ORE: currentBlockType = BLOCK_EMERALD_ORE; break;
                case ITEM_MOSSY_STONE: currentBlockType = BLOCK_MOSSY_STONE; break;
                case ITEM_GRAVEL: currentBlockType = BLOCK_GRAVEL; break;
                case ITEM_CLAY: currentBlockType = BLOCK_CLAY; break;
                case ITEM_ICE: currentBlockType = BLOCK_ICE; break;
                case ITEM_SNOW: currentBlockType = BLOCK_SNOW; break;
                case ITEM_WATER: currentBlockType = BLOCK_WATER; break;
                case ITEM_SANDSTONE: currentBlockType = BLOCK_SANDSTONE; break;
                case ITEM_CACTUS: currentBlockType = BLOCK_CACTUS; break;
                case ITEM_PUMPKIN: currentBlockType = BLOCK_PUMPKIN; break;
                case ITEM_NETHERRACK: currentBlockType = BLOCK_NETHERRACK; break;
                case ITEM_SOUL_SAND: currentBlockType = BLOCK_SOUL_SAND; break;
                case ITEM_QUARTZ: currentBlockType = BLOCK_QUARTZ; break;
                case ITEM_GLOWSTONE: currentBlockType = BLOCK_GLOWSTONE; break;
                case ITEM_MYCELIUM: currentBlockType = BLOCK_MYCELIUM; break;
                case ITEM_END_STONE: currentBlockType = BLOCK_END_STONE; break;
                case ITEM_PRISMARINE: currentBlockType = BLOCK_PRISMARINE; break;
                case ITEM_MAGMA: currentBlockType = BLOCK_MAGMA; break;
                case ITEM_NETHER_WART: currentBlockType = BLOCK_NETHER_WART; break;
                case ITEM_SLIME: currentBlockType = BLOCK_SLIME; break;
                case ITEM_BRICK: currentBlockType = BLOCK_BRICK; break;            // 添加砖块映射
                case ITEM_BOOKSHELF: currentBlockType = BLOCK_BOOKSHELF; break;    // 添加书架映射
                case ITEM_CHANGE_BLOCK: currentBlockType = BLOCK_CHANGE_BLOCK; break;
                default: 
                    std::cout << "WARNING: Unknown item type " << static_cast<int>(inventory[selectedSlot].type) << std::endl;
                    currentBlockType = BLOCK_AIR; 
                    break;
            }
            
            // 输出调试信息
            std::cout << "Selected item: " << inventory[selectedSlot].getName() 
                      << ", Corresponding block type: " << static_cast<int>(currentBlockType) << std::endl;
        } else {
            // 如果选中的槽位没有物品，设置为空气方块（不放置任何方块）
            currentBlockType = BLOCK_AIR;
        }
    }
    
    // 获取当前手持方块类型
    BlockType getCurrentBlockType() const {
        return currentBlockType;
    }
    
    // 获取物品栏大小
    size_t getInventorySize() const {
        return inventory.size();
    }
    
    // 获取当前方块名称
    std::string getCurrentBlockName() const {
        return currentBlockName;
    }
    
    // 获取世界种子
    std::string getWorldSeed() const {
        return worldSeed;
    }
    
    // 检查是否使用系统生成的种子
    bool getSystemGeneratedSeed() const {
        return m_isSystemGeneratedSeed; // 返回成员变量
    }
    
    // 获取渲染距离值 (0.0-1.0)
    float getRenderDistanceValue() const {
        return renderDistanceValue;
    }
    
    // 获取实际渲染距离（方块数）
    int getActualRenderDistance() const {
        const int MIN_RENDER_DISTANCE = 16;
        const int MAX_RENDER_DISTANCE = 128;  // 将最大渲染距离从16增加到128
        return MIN_RENDER_DISTANCE + static_cast<int>((MAX_RENDER_DISTANCE - MIN_RENDER_DISTANCE) * renderDistanceValue);
    }
    
    // 获取FOV值 (0.0-1.0)
    float getFOVValue() const {
        return fovValue;
    }
    
    // 获取行走速度值 (0.0-1.0)
    float getWalkSpeedValue() const {
        return walkSpeedValue;
    }
    
    // 获取飞行速度值 (0.0-1.0)
    float getFlySpeedValue() const {
        return flySpeedValue;
    }
    
    // 获取实际行走速度
    float getActualWalkSpeed() const {
        return 5.0f + walkSpeedValue * 15.0f; // 5-20范围
    }
    
    // 获取实际飞行速度
    float getActualFlySpeed() const {
        return 10.0f + flySpeedValue * 40.0f; // 10-50范围
    }
    
    // 切换调试信息显示
    void toggleDebugInfo() {
        debugInfoEnabled = !debugInfoEnabled;
    }
    
    // 检查调试信息是否启用
    bool isDebugInfoEnabled() const {
        return debugInfoEnabled;
    }
    
    // 获取多线程渲染状态
    bool getMultiThreadingEnabled() const {
        return false;
    }
    
    // 绘制完整物品栏（显示所有方块）
    void drawFullInventory(Renderer& renderer) {
        // 计算需要显示的物品总数（ITEM_COUNT）
        int totalItems = ITEM_COUNT;
        
        // 调整物品栏尺寸以适应屏幕和物品数量
        int slotSize = screenHeight / 15;
        int padding = slotSize / 10;
        int cols = 8; // 每行8个物品
        
        // 根据物品数量计算需要的行数
        int rows = (totalItems + cols - 1) / cols; // 向上取整
        // 限制最大行数，防止超出屏幕
        rows = std::min(rows, 6); // 最多显示6行
        
        int inventoryWidth = (slotSize + padding) * cols - padding;
        int inventoryHeight = (slotSize + padding) * rows - padding;
        int invStartX = (screenWidth - inventoryWidth) / 2;
        int invStartY = (screenHeight - inventoryHeight) / 2;
        
        // 绘制半透明背景
        renderer.drawRect(0, 0, screenWidth, screenHeight, Color(0, 0, 0, 150));
        
        // 绘制物品栏背景
        renderer.drawRect(invStartX - 10, invStartY - 10, 
                         inventoryWidth + 20, inventoryHeight + 20, 
                         Color(50, 50, 50, 200));
        
        // 绘制物品栏边框
        renderer.drawRectOutline(invStartX - 10, invStartY - 10, 
                                inventoryWidth + 20, inventoryHeight + 20, 
                                Color(200, 200, 200));
        
        // 绘制物品栏标题
        std::string title = "Inventory";
        drawText(renderer, title, invStartX + (inventoryWidth - title.length() * 10) / 2, 
                invStartY - 30, Color(255, 255, 255));
        
        // 绘制所有物品类型
        int index = 0;
        for (int i = 0; i < ITEM_COUNT; i++) {
            ItemType itemType = static_cast<ItemType>(i);
            
            int row = index / cols;
            int col = index % cols;
            
            if (row >= rows) {
                // 如果超出显示区域，添加翻页提示
                std::string moreItemsText = "...and more items";
                drawText(renderer, moreItemsText, invStartX, invStartY + inventoryHeight + 10, Color(255, 255, 255));
                break;
            }
            
            int slotX = invStartX + col * (slotSize + padding);
            int slotY = invStartY + row * (slotSize + padding);
            
            // 绘制槽背景
            renderer.drawRect(slotX, slotY, slotSize, slotSize, Color(80, 80, 80, 100));
            
            // 创建临时物品并绘制
            Item tempItem(itemType);
            renderer.drawRect(slotX + 4, slotY + 4, slotSize - 8, slotSize - 8, tempItem.getColor());
            
            // 绘制槽边框
            renderer.drawRectOutline(slotX, slotY, slotSize, slotSize, Color(120, 120, 120));
            
            // 检查鼠标是否悬停在此物品上
            if (gameState == GAME_INVENTORY) {
                POINT mousePos;
                GetCursorPos(&mousePos);
                ScreenToClient(GetActiveWindow(), &mousePos);
                
                if (mousePos.x >= slotX && mousePos.x < slotX + slotSize &&
                    mousePos.y >= slotY && mousePos.y < slotY + slotSize) {
                    // 高亮显示选中的物品
                    renderer.drawRectOutline(slotX, slotY, slotSize, slotSize, Color(255, 255, 255));
                    
                    // 设置物品提示
                    showItemTooltip = true;
                    tooltipX = mousePos.x + 15;
                    tooltipY = mousePos.y;
                    
                    // 获取物品名称，直接使用Item类的getName方法
                    Item tempItem(static_cast<ItemType>(i));
                    hoverItemName = tempItem.getName();
                    
                    // 检测鼠标点击，显示替换提示
                    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                        showReplacePrompt = true;
                        // 将ItemType转换为对应的BlockType
                        switch (static_cast<ItemType>(i)) {
                            case ITEM_DIRT: blockToReplace = BLOCK_DIRT; break;
                            case ITEM_GRASS: blockToReplace = BLOCK_GRASS; break;
                            case ITEM_STONE: blockToReplace = BLOCK_STONE; break;
                            case ITEM_SAND: blockToReplace = BLOCK_SAND; break;
                            case ITEM_WOOD: blockToReplace = BLOCK_WOOD; break;
                            case ITEM_LEAVES: blockToReplace = BLOCK_LEAVES; break;
                            case ITEM_COAL: blockToReplace = BLOCK_COAL_ORE; break;
                            case ITEM_IRON: blockToReplace = BLOCK_IRON_ORE; break;
                            case ITEM_GOLD: blockToReplace = BLOCK_GOLD_ORE; break;
                            case ITEM_DIAMOND: blockToReplace = BLOCK_DIAMOND_ORE; break;
                            case ITEM_BEDROCK: blockToReplace = BLOCK_BEDROCK; break;
                            case ITEM_OBSIDIAN: blockToReplace = BLOCK_OBSIDIAN; break;
                            case ITEM_LAVA: blockToReplace = BLOCK_LAVA; break;
                            case ITEM_REDSTONE_ORE: blockToReplace = BLOCK_REDSTONE_ORE; break;
                            case ITEM_EMERALD_ORE: blockToReplace = BLOCK_EMERALD_ORE; break;
                            case ITEM_MOSSY_STONE: blockToReplace = BLOCK_MOSSY_STONE; break;
                            case ITEM_GRAVEL: blockToReplace = BLOCK_GRAVEL; break;
                            case ITEM_CLAY: blockToReplace = BLOCK_CLAY; break;
                            case ITEM_ICE: blockToReplace = BLOCK_ICE; break;
                            case ITEM_SNOW: blockToReplace = BLOCK_SNOW; break;
                            case ITEM_WATER: blockToReplace = BLOCK_WATER; break;
                            case ITEM_SANDSTONE: blockToReplace = BLOCK_SANDSTONE; break;
                            case ITEM_CACTUS: blockToReplace = BLOCK_CACTUS; break;
                            case ITEM_PUMPKIN: blockToReplace = BLOCK_PUMPKIN; break;
                            case ITEM_NETHERRACK: blockToReplace = BLOCK_NETHERRACK; break;
                            case ITEM_SOUL_SAND: blockToReplace = BLOCK_SOUL_SAND; break;
                            case ITEM_QUARTZ: blockToReplace = BLOCK_QUARTZ; break;
                            case ITEM_GLOWSTONE: blockToReplace = BLOCK_GLOWSTONE; break;
                            case ITEM_MYCELIUM: blockToReplace = BLOCK_MYCELIUM; break;
                            case ITEM_END_STONE: blockToReplace = BLOCK_END_STONE; break;
                            case ITEM_PRISMARINE: blockToReplace = BLOCK_PRISMARINE; break;
                            case ITEM_MAGMA: blockToReplace = BLOCK_MAGMA; break;
                            case ITEM_NETHER_WART: blockToReplace = BLOCK_NETHER_WART; break;
                            case ITEM_SLIME: blockToReplace = BLOCK_SLIME; break;
                            case ITEM_BRICK: blockToReplace = BLOCK_BRICK; break;            // 添加砖块映射
                            case ITEM_BOOKSHELF: blockToReplace = BLOCK_BOOKSHELF; break;    // 添加书架映射
                            case ITEM_CHANGE_BLOCK: blockToReplace = BLOCK_CHANGE_BLOCK; break;
                            default: blockToReplace = BLOCK_DIRT; break;
                        }
                        replacePromptX = mousePos.x;
                        replacePromptY = mousePos.y + 30;
                    }
                }
            }
            
            index++;
        }
        
        // 绘制删除按钮
        int deleteButtonX = invStartX + inventoryWidth - slotSize;
        int deleteButtonY = invStartY + inventoryHeight + 10;
        renderer.drawRect(deleteButtonX, deleteButtonY, slotSize, slotSize, Color(200, 50, 50, 200));
        renderer.drawRectOutline(deleteButtonX, deleteButtonY, slotSize, slotSize, Color(255, 255, 255));
        drawText(renderer, "X", deleteButtonX + slotSize/2 - 5, deleteButtonY + slotSize/2 - 5, Color(255, 255, 255));
        
        // 检查鼠标是否悬停在删除按钮上
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(GetActiveWindow(), &mousePos);
        if (mousePos.x >= deleteButtonX && mousePos.x < deleteButtonX + slotSize &&
            mousePos.y >= deleteButtonY && mousePos.y < deleteButtonY + slotSize) {
            // 高亮显示删除按钮
            renderer.drawRectOutline(deleteButtonX, deleteButtonY, slotSize, slotSize, Color(255, 255, 0));
            
            // 设置物品提示
            showItemTooltip = true;
            tooltipX = mousePos.x + 15;
            tooltipY = mousePos.y;
            hoverItemName = "Clear Inventory (Press 0)";
            
            // 检测鼠标点击，显示清空提示
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                showReplacePrompt = true;
                blockToReplace = BLOCK_AIR; // 特殊标记，表示清空物品栏
                replacePromptX = mousePos.x;
                replacePromptY = mousePos.y + 30;
            }
        }
        
        // 绘制物品提示
        if (showItemTooltip && !hoverItemName.empty()) {
            int textWidth = hoverItemName.length() * 8;
            int textHeight = 20;
            
            // 绘制提示背景
            renderer.drawRect(tooltipX, tooltipY, textWidth + 10, textHeight, Color(0, 0, 0, 200));
            renderer.drawRectOutline(tooltipX, tooltipY, textWidth + 10, textHeight, Color(200, 200, 200));
            
            // 绘制提示文本
            drawText(renderer, hoverItemName, tooltipX + 5, tooltipY + 5, Color(255, 255, 255));
        }
        
        // 绘制替换提示
        if (showReplacePrompt) {
            std::string promptText;
            if (blockToReplace == BLOCK_AIR) {
                promptText = "Press 0 to clear inventory";
            } else {
                promptText = "Press 1-9 to replace slot";
            }
            
            int textWidth = promptText.length() * 8;
            int textHeight = 20;
            
            // 绘制提示背景
            renderer.drawRect(replacePromptX, replacePromptY, textWidth + 10, textHeight, Color(0, 0, 0, 200));
            renderer.drawRectOutline(replacePromptX, replacePromptY, textWidth + 10, textHeight, Color(255, 255, 0));
            
            // 绘制提示文本
            drawText(renderer, promptText, replacePromptX + 5, replacePromptY + 5, Color(255, 255, 0));
        }
    }
    
        // 控制菜单设置
    Button* controlsBackButton = nullptr;
    
    // 初始化控制菜单
    void initControlsMenu() {
        // 删除旧的返回按钮
        delete controlsBackButton;
        
        // 创建新的返回按钮 - 设计与其他菜单一致
        int buttonWidth = std::min(250, screenWidth / 4);
        int buttonHeight = std::max(45, screenHeight / 20);
        int buttonY = screenHeight - buttonHeight - 30; // 底部固定位置
        
        controlsBackButton = new Button(
            (screenWidth - buttonWidth) / 2, 
            buttonY, 
            buttonWidth, 
            buttonHeight, 
            "Back"
        );
    }
    
    // 处理控制菜单的鼠标输入
    void handleControlsMouseInput(int mouseX, int mouseY, bool mouseDown) {
        // 检查返回按钮
        if (controlsBackButton) {
            controlsBackButton->hovered = controlsBackButton->isMouseOver(mouseX, mouseY);
            if (controlsBackButton->hovered && mouseDown) {
                // 返回到暂停菜单
                controlsState = CONTROLS_NONE;
                gameState = GAME_PAUSED;
            }
        }
    }
    
    // 绘制按键提示界面
    void drawControlsMenu(Renderer& renderer) {
        // 绘制渐变半透明背景
        for (int y = 0; y < screenHeight; y += 4) {
            float t = static_cast<float>(y) / screenHeight;
            Color bgColor(0, 0, 0, 120 + static_cast<int>(t * 80));
            renderer.drawRect(0, y, screenWidth, 4, bgColor);
        }
        
        // 添加模糊效果模拟
        for (int i = 0; i < 10; i++) {
            int x = i * screenWidth / 10;
            int width = screenWidth / 10;
            renderer.drawRect(x, 0, width, screenHeight, Color(0, 0, 0, 20));
        }
        
        // 绘制菜单标题
        std::string title = "Controls";
        int titleY = screenHeight / 10;
        
        // 绘制标题阴影
        drawText(renderer, title, (screenWidth - title.length() * 12) / 2 + 2, titleY + 2, Color(0, 0, 0, 180));
        
        // 绘制标题
        drawText(renderer, title, (screenWidth - title.length() * 12) / 2, titleY, Color(255, 255, 255));
        
        // 绘制标题下方的装饰线
        int lineWidth = std::min(300, screenWidth - 80);
        int lineX = (screenWidth - lineWidth) / 2;
        int lineY = titleY + 30;
        
        // 渐变装饰线
        for (int x = 0; x < lineWidth; x++) {
            float t = static_cast<float>(x) / lineWidth;
            float alpha = sin(t * 3.14159f) * 255.0f;
            Color lineColor(100, 150, 255, static_cast<uint8_t>(alpha));
            renderer.drawRect(lineX + x, lineY, 1, 2, lineColor);
        }
        
        // 控制信息面板 - 基于屏幕大小动态调整
        int panelStartY = titleY + 60;
        int panelWidth = std::min(600, screenWidth - 80);
        int panelHeight = screenHeight - panelStartY - 100;
        int panelX = (screenWidth - panelWidth) / 2;
        
        // 检查是否需要使用双列布局 (基于屏幕宽度)
        bool useDoubleColumn = screenWidth >= 900;
        
        // 绘制半透明控制面板背景
        renderer.drawRect(panelX, panelStartY, panelWidth, panelHeight, Color(30, 30, 50, 180));
        
        // 绘制面板边框
        Color borderColor(80, 100, 200, 200);
        // 顶部边框（亮一些）
        renderer.drawRect(panelX, panelStartY, panelWidth, 2, Color(borderColor.r + 30, borderColor.g + 30, borderColor.b + 30));
        // 左侧边框
        renderer.drawRect(panelX, panelStartY, 2, panelHeight, borderColor);
        // 底部边框（暗一些）
        renderer.drawRect(panelX, panelStartY + panelHeight - 2, panelWidth, 2, Color(borderColor.r - 30, borderColor.g - 30, borderColor.b - 30, 255));
        // 右侧边框
        renderer.drawRect(panelX + panelWidth - 2, panelStartY, 2, panelHeight, borderColor);
        
        // 控制说明文本
        std::vector<std::string> controlsText = {
            "Movement Controls:",
            "W, A, S, D - Move forward, left, backward, right",
            "Space - Jump / Fly up",
            "Shift - Sneak / Fly down",
            "F - Toggle flying mode",
            "C - Toggle No-Clip Mode (in Flying Mode)",
            "R - Return to Spawn Point",
            "",
            "Game Controls:",
            "ESC - Menu / Close inventory /Close other UI",
            "E - Open inventory",
            "F3 - Toggle debug info",
            "F1 - Toggle UI visibility",
            "",
            "Block Interaction:",
            "Left Mouse - Break block",
            "Right Mouse - Place block",
            "Mouse Wheel - Select block in hotbar",
            "1-9 - Select hotbar slot",
            "Middle Mouse - Pick block (copy block type)",
            "",
            "Other Controls:",
            "O - Open block editor (for custom blocks)",
            "X - Toggle X-ray mode (show ores)",
            "Q - Quit game"
        };
        
        // 计算最佳文本间距 - 根据内容和面板高度动态调整
        int totalLines = controlsText.size();
        // 对于空行，减少计数
        int emptyLines = 0;
        for (const auto& line : controlsText) {
            if (line.empty()) emptyLines++;
        }
        int textLines = totalLines - emptyLines;
        
        // 计算可用的文本显示高度
        int availableTextHeight = panelHeight - 40; // 面板高度减去上下边距
        
        // 计算最佳行间距
        int textSpacing;
        if (useDoubleColumn) {
            // 双列布局
            textSpacing = std::max(20, std::min(25, availableTextHeight / ((textLines / 2) + 2)));
        } else {
            // 单列布局
            textSpacing = std::max(18, std::min(25, availableTextHeight / (textLines + 2)));
        }
        
        // 绘制控制说明文本
        int textStartX = panelX + 20;
        int textStartY = panelStartY + 20;
        
        if (useDoubleColumn) {
            // 双列布局
            int columnWidth = (panelWidth - 60) / 2;
            int column2X = textStartX + columnWidth + 20;
            
            // 确定每列的项目数
            int itemsPerColumn = (totalLines + 1) / 2;
            
            // 遍历控制文本列表
            for (size_t i = 0; i < controlsText.size(); i++) {
                // 确定当前项目应该在哪一列
                bool isColumn1 = i < itemsPerColumn;
                int currentX = isColumn1 ? textStartX : column2X;
                int currentY = textStartY + (isColumn1 ? i : i - itemsPerColumn) * textSpacing;
                
                std::string& line = controlsText[i];
        
                // 跳过空行的绘制，但仍计入位置
                if (line.empty()) continue;
                
                // 标题行使用不同的颜色和样式
                if (line.find(':') != std::string::npos && line.back() == ':') {
                    // 这是一个标题行
                    // 绘制文本阴影
                    drawText(renderer, line, currentX + 1, currentY + 1, Color(0, 0, 0, 180));
                    // 绘制文本
                    drawText(renderer, line, currentX, currentY, Color(150, 200, 255));
                    
                    // 添加标题下的短分割线
                    if (i < controlsText.size() - 1 && !controlsText[i + 1].empty()) {
                        int titleLineWidth = 100;
                        renderer.drawRect(currentX, currentY + 20, titleLineWidth, 1, Color(100, 130, 200, 150));
                    }
                }
                else {
                    // 普通控制行
                    // 查找第一个 "-" 的位置，分隔按键和功能说明
                    size_t dashPos = line.find(" - ");
                    if (dashPos != std::string::npos) {
                        // 分割按键和功能说明
                        std::string keys = line.substr(0, dashPos);
                        std::string description = line.substr(dashPos + 3);
                        
                        // 绘制按键部分（高亮）
                        drawText(renderer, keys, currentX, currentY, Color(255, 255, 150));
                        
                        // 绘制分隔符
                        drawText(renderer, " - ", currentX + keys.length() * 8, currentY, Color(150, 150, 150));
                        
                        // 绘制功能说明
                        drawText(renderer, description, currentX + (keys.length() + 3) * 8, currentY, Color(220, 220, 220));
                    }
                    else {
                        // 普通文本行
                        drawText(renderer, line, currentX, currentY, Color(220, 220, 220));
                    }
                }
            }
        } else {
            // 单列布局
            for (size_t i = 0; i < controlsText.size(); i++) {
                std::string& line = controlsText[i];
                
                // 跳过空行的绘制，但仍计入位置
                if (line.empty()) continue;
                
                int currentY = textStartY + i * textSpacing;
                
                // 标题行使用不同的颜色和样式
                if (line.find(':') != std::string::npos && line.back() == ':') {
                    // 这是一个标题行
                    // 绘制文本阴影
                    drawText(renderer, line, textStartX + 1, currentY + 1, Color(0, 0, 0, 180));
                    // 绘制文本
                    drawText(renderer, line, textStartX, currentY, Color(150, 200, 255));
                    
                    // 添加标题下的短分割线
                    if (i < controlsText.size() - 1 && !controlsText[i + 1].empty()) {
                        int titleLineWidth = 100;
                        renderer.drawRect(textStartX, currentY + 20, titleLineWidth, 1, Color(100, 130, 200, 150));
                    }
                }
                else {
                    // 普通控制行
                    // 查找第一个 "-" 的位置，分隔按键和功能说明
                    size_t dashPos = line.find(" - ");
                    if (dashPos != std::string::npos) {
                        // 分割按键和功能说明
                        std::string keys = line.substr(0, dashPos);
                        std::string description = line.substr(dashPos + 3);
                        
                        // 绘制按键部分（高亮）
                        drawText(renderer, keys, textStartX, currentY, Color(255, 255, 150));
                        
                        // 绘制分隔符
                        drawText(renderer, " - ", textStartX + keys.length() * 8, currentY, Color(150, 150, 150));
                        
                        // 绘制功能说明
                        drawText(renderer, description, textStartX + (keys.length() + 3) * 8, currentY, Color(220, 220, 220));
                    }
                    else {
                        // 普通文本行
                        drawText(renderer, line, textStartX, currentY, Color(220, 220, 220));
                    }
                }
            }
        }
        
        // 获取鼠标位置
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(GetActiveWindow(), &mousePos);
        
        // 绘制返回按钮
        if (controlsBackButton) {
            controlsBackButton->hovered = controlsBackButton->isMouseOver(mousePos.x, mousePos.y);
            controlsBackButton->draw(renderer);
            
            // 检查鼠标点击
            static bool lastMouseDown = false;
            bool mouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool mouseClicked = mouseDown && !lastMouseDown;
            
            if (controlsBackButton->hovered && mouseClicked) {
                // 返回主菜单
                controlsState = CONTROLS_NONE;
                gameState = GAME_PAUSED;
            }
            
            lastMouseDown = mouseDown;
        }
    }
    
        // 选项菜单相关变量
    std::vector<Slider> optionSliders;
    Button* backButton = nullptr;
    Button* saveSeedButton = nullptr;
    
    // 渲染距离、FOV和移动速度设置
    float renderDistanceValue = 0.5f; // 默认值
    float fovValue = 0.5f; // 默认值
    float walkSpeedValue = 0.85f; // 行走速度默认值
    float flySpeedValue = 0.5f; // 飞行速度默认值
    float worldSizeValue = 0.5f; // 世界大小默认值
    bool superFlatWorld = false; // 超平坦世界设置
    
    // 调试信息显示
    bool debugInfoEnabled = false;
    
    // 初始化选项菜单 - 动态适应窗口大小
    void initOptionsMenu() {
        // 清除旧的滑块
        optionSliders.clear();
        
        // 删除旧的按钮
        delete backButton;
        delete saveSeedButton;
        
        // 计算基于屏幕大小的滑块尺寸和位置 - 更好地适应不同屏幕尺寸
        int sliderWidth = std::min(300, screenWidth - 150);
        int sliderHeight = std::max(20, screenHeight / 40); // 减小滑块高度
        
        // 计算可用内容区域高度
        int titleHeight = screenHeight / 10;
        int titleSpace = 60; // 标题下方预留空间
        int contentHeight = screenHeight - titleHeight - titleSpace - 150; // 可用内容高度
        
        // 动态计算滑块间距，确保所有元素都能显示
        int totalSliders = 6; // 滑块总数
        int sliderSpacing = std::max(45, contentHeight / (totalSliders + 2)); // 动态间距，增加额外空间
        
        // 确保开始位置足够高，但不会太高
        int startY = titleHeight + titleSpace; // 从标题下方开始
        int startX = (screenWidth - sliderWidth) / 2;
        
        // 计算最后一个滑块的Y位置
        int lastSliderY = startY + 5 * sliderSpacing;
        
        // 计算种子输入框和相关元素的位置 - 确保足够的间距
        int additionalSpace = 20; // 添加额外间距
        seedInputY = lastSliderY + sliderHeight + 60 + additionalSpace; 
        
        // 创建滑块 - 使用计算好的间距
        optionSliders.push_back(Slider(startX, startY, sliderWidth, sliderHeight, "Music Volume", musicVolumeValue));
        optionSliders.push_back(Slider(startX, startY + sliderSpacing, sliderWidth, sliderHeight, "Render Distance", renderDistanceValue));
        optionSliders.push_back(Slider(startX, startY + 2 * sliderSpacing, sliderWidth, sliderHeight, "FOV", fovValue));
        optionSliders.push_back(Slider(startX, startY + 3 * sliderSpacing, sliderWidth, sliderHeight, "Walk Speed", walkSpeedValue));
        optionSliders.push_back(Slider(startX, startY + 4 * sliderSpacing, sliderWidth, sliderHeight, "Fly Speed", flySpeedValue));
        optionSliders.push_back(Slider(startX, startY + 5 * sliderSpacing, sliderWidth, sliderHeight, "World Size", worldSizeValue));
        
        // 检查种子输入框是否会超出屏幕底部，如果是，调整所有元素的位置
        int seedInputHeight = 30;
        int seedLabelHeight = 25;
        int checkboxHeight = 30;
        int checkboxSpacing = 40; // 增加复选框之间的间距
        int saveButtonHeight = std::max(35, screenHeight / 25);
        int backButtonHeight = std::max(45, screenHeight / 20);
        int bottomMargin = 30; // 底部边距
        
        int totalBottomHeight = seedInputHeight + seedLabelHeight + checkboxHeight * 2 + checkboxSpacing + saveButtonHeight + backButtonHeight + bottomMargin;
        
        if (seedInputY + totalBottomHeight > screenHeight) {
            // 如果超出屏幕底部，减小滑块间距
            sliderSpacing = std::max(35, (contentHeight - sliderHeight * totalSliders) / (totalSliders + 2));
            
            // 重新计算滑块位置
            startY = titleHeight + titleSpace;
            
            // 重新创建滑块
            optionSliders.clear();
            optionSliders.push_back(Slider(startX, startY, sliderWidth, sliderHeight, "Music Volume", musicVolumeValue));
            optionSliders.push_back(Slider(startX, startY + sliderSpacing, sliderWidth, sliderHeight, "Render Distance", renderDistanceValue));
            optionSliders.push_back(Slider(startX, startY + 2 * sliderSpacing, sliderWidth, sliderHeight, "FOV", fovValue));
            optionSliders.push_back(Slider(startX, startY + 3 * sliderSpacing, sliderWidth, sliderHeight, "Walk Speed", walkSpeedValue));
            optionSliders.push_back(Slider(startX, startY + 4 * sliderSpacing, sliderWidth, sliderHeight, "Fly Speed", flySpeedValue));
            optionSliders.push_back(Slider(startX, startY + 5 * sliderSpacing, sliderWidth, sliderHeight, "World Size", worldSizeValue));
            
            // 重新计算种子输入框位置
            seedInputY = startY + 5 * sliderSpacing + sliderHeight + 50;
        }
        
        // 创建保存种子按钮 - 调整大小以适应屏幕
        int seedButtonWidth = std::min(150, screenWidth / 6);
        int seedButtonHeight = std::max(35, screenHeight / 25);
        
        // 位置调整，确保不会与其他元素重叠
        int seedButtonX = startX + sliderWidth - seedButtonWidth;
        int seedButtonY = seedInputY + seedInputHeight + 20;
        
        saveSeedButton = new Button(seedButtonX, seedButtonY, seedButtonWidth, seedButtonHeight, "Save Seed");
        
        // 创建返回按钮 - 调整大小以适应屏幕
        int buttonWidth = std::min(250, screenWidth / 4);
        int buttonHeight = std::max(45, screenHeight / 20);
        int buttonY = seedButtonY + seedButtonHeight + 30;
        buttonY = std::min(buttonY, screenHeight - buttonHeight - bottomMargin); // 确保按钮不会超出屏幕
        backButton = new Button((screenWidth - buttonWidth) / 2, buttonY, buttonWidth, buttonHeight, "Back");
    }
    
    // 处理选项菜单的鼠标输入
    void handleOptionsMouseInput(int mouseX, int mouseY, bool mouseDown) {
        // 更新滑块状态
        for (size_t i = 0; i < optionSliders.size(); i++) {
            auto& slider = optionSliders[i];
            
            // 检查鼠标是否在滑块上
            if (slider.isMouseOver(mouseX, mouseY)) {
                if (mouseDown) {
                    slider.dragging = true;
                }
            }
            
            // 如果正在拖动，更新滑块值
            if (slider.dragging) {
                slider.updateValue(mouseX);
                if (!mouseDown) {
                    slider.dragging = false;
                    
                    // 保存滑块值
                    if (i == 0) { // 音乐音量
                        musicVolumeValue = slider.value;
                        updateMusicVolume();
                    } else if (i == 1) { // 视距
                        renderDistanceValue = slider.value;
                    } else if (i == 2) { // FOV
                        fovValue = slider.value;
                    } else if (i == 3) { // 行走速度
                        walkSpeedValue = slider.value;
                    } else if (i == 4) { // 飞行速度
                        flySpeedValue = slider.value;
                    } else if (i == 5) { // 世界大小
                        worldSizeValue = slider.value;
                    }
                }
            }
        }
        
        // 检查种子输入框
        int seedInputWidth = optionSliders[0].width;
        int seedInputHeight = 30;
        if (mouseX >= optionSliders[0].x && mouseX < optionSliders[0].x + seedInputWidth &&
            mouseY >= seedInputY && mouseY < seedInputY + seedInputHeight) {
            
            if (mouseDown) {
                isSeedInputActive = true;
            }
        } else if (mouseDown) {
            isSeedInputActive = false;
        }
        
        // 处理种子输入
        if (isSeedInputActive) {
            // 处理数字键输入
            for (int key = '0'; key <= '9'; key++) {
                if (GetAsyncKeyState(key) & 1) { // 按键刚刚被按下
                    worldSeed += (char)key;
                    m_isSystemGeneratedSeed = false;
                }
            }
            
            // 处理退格键
            if ((GetAsyncKeyState(VK_BACK) & 1) && !worldSeed.empty()) {
                worldSeed.pop_back();
                m_isSystemGeneratedSeed = worldSeed.empty();
        }
        
        // 更新光标闪烁
        cursorBlinkTimer++;
        if (cursorBlinkTimer > 30) {
            showCursor = !showCursor;
            cursorBlinkTimer = 0;
            }
        }
        
        // 检查保存种子按钮
        if (saveSeedButton) {
            saveSeedButton->hovered = saveSeedButton->isMouseOver(mouseX, mouseY);
            if (saveSeedButton->hovered && mouseDown) {
                // 保存种子设置
                m_isSystemGeneratedSeed = worldSeed.empty();
                
                // 显示保存确认消息
                showSeedSavedMessage = true;
                seedSavedMessageTimer = 120; // 约2秒
            }
        }
        
        // 检查返回按钮
        if (backButton) {
            backButton->hovered = backButton->isMouseOver(mouseX, mouseY);
            if (backButton->hovered && mouseDown) {
                // 返回到主菜单
                optionsState = OPTIONS_NONE;
                // 保持在暂停菜单状态
                gameState = GAME_PAUSED;
            }
        }
        
        // 检查系统生成种子复选框
        std::string systemSeedText = "Use system-generated seed: ";
        int checkboxSize = 20;
        int checkboxX = optionSliders[0].x + systemSeedText.length() * 6 - checkboxSize - 5;
        int checkboxY = seedInputY + seedInputHeight + 10;
        
        static bool wasPressed = false;
        if (mouseX >= checkboxX && mouseX < checkboxX + checkboxSize &&
            mouseY >= checkboxY && mouseY < checkboxY + checkboxSize) {
            if (mouseDown) {
                if (!wasPressed) {
                    m_isSystemGeneratedSeed = !m_isSystemGeneratedSeed;
                    wasPressed = true;
                }
            } else {
                wasPressed = false;
            }
        } else {
            if (!mouseDown) {
                wasPressed = false;
            }
        }
        
        // 检查超平坦世界复选框 - 位置调整，确保不重叠
        std::string superFlatText = "Super Flat World: ";
        int superFlatCheckboxX = optionSliders[0].x + superFlatText.length() * 6 - checkboxSize - 5;
        int superFlatCheckboxY = seedInputY - 40;
        
        static bool superFlatWasPressed = false;
        if (mouseX >= superFlatCheckboxX && mouseX < superFlatCheckboxX + checkboxSize &&
            mouseY >= superFlatCheckboxY && mouseY < superFlatCheckboxY + checkboxSize) {
            if (mouseDown) {
                if (!superFlatWasPressed) {
                    superFlatWorld = !superFlatWorld;
                    superFlatWasPressed = true;
                }
            } else {
                superFlatWasPressed = false;
            }
        } else {
            if (!mouseDown) {
                superFlatWasPressed = false;
            }
        }
    }
    
    // 绘制选项菜单
    void drawOptionsMenu(Renderer& renderer) {
        // 绘制渐变半透明背景
        for (int y = 0; y < screenHeight; y += 4) {
            float t = static_cast<float>(y) / screenHeight;
            Color bgColor(0, 0, 0, 120 + static_cast<int>(t * 80));
            renderer.drawRect(0, y, screenWidth, 4, bgColor);
        }
        
        // 添加模糊效果模拟
        for (int i = 0; i < 10; i++) {
            int x = i * screenWidth / 10;
            int width = screenWidth / 10;
            renderer.drawRect(x, 0, width, screenHeight, Color(0, 0, 0, 20));
        }
        
        // 绘制菜单标题
        std::string title = "Options";
        int titleY = screenHeight / 10;
        
        // 绘制标题阴影
        drawText(renderer, title, (screenWidth - title.length() * 12) / 2 + 2, titleY + 2, Color(0, 0, 0, 180));
        
        // 绘制标题
        drawText(renderer, title, (screenWidth - title.length() * 12) / 2, titleY, Color(255, 255, 255));
        
        // 绘制标题下方的装饰线
        int lineWidth = std::min(300, screenWidth - 80);
        int lineX = (screenWidth - lineWidth) / 2;
        int lineY = titleY + 30;
        
        // 渐变装饰线
        for (int x = 0; x < lineWidth; x++) {
            float t = static_cast<float>(x) / lineWidth;
            float alpha = sin(t * 3.14159f) * 255.0f;
            Color lineColor(100, 150, 255, static_cast<uint8_t>(alpha));
            renderer.drawRect(lineX + x, lineY, 1, 2, lineColor);
        }
        
        // 获取鼠标位置
        POINT mousePos;
        GetCursorPos(&mousePos);
        ScreenToClient(GetActiveWindow(), &mousePos);
        
        // 处理鼠标输入
        handleOptionsMouseInput(mousePos.x, mousePos.y, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
        
        // 绘制所有滑块
        for (auto& slider : optionSliders) {
            slider.draw(renderer);
            
            // 为音量滑块添加特殊标签
            if (slider.text == "Music Volume") {
                std::string volumeText;
                if (slider.value <= 0.01f) {
                    volumeText = "Off";
                } else if (slider.value <= 0.3f) {
                    volumeText = "Low";
                } else if (slider.value <= 0.7f) {
                    volumeText = "Medium";
                } else {
                    volumeText = "High";
                }
                
                // 显示百分比
                std::string percentText = std::to_string(static_cast<int>(slider.value * 100)) + "%";
                
                // 计算标签位置
                int labelX = slider.x + slider.width + 15;
                int labelY = slider.y + (slider.height - 15) / 2;
                
                // 绘制文本和百分比
                renderer.drawText(labelX, labelY, volumeText, Color(220, 220, 220));
                renderer.drawText(labelX + volumeText.length() * 8 + 10, labelY, percentText, Color(180, 180, 180));
            }
        }
        
        // 种子输入框相关元素
        int seedInputWidth = optionSliders[0].width;
        int seedInputHeight = 30;
        
        // 检查是否有足够的空间显示种子输入框及下面的元素
        bool canShowSeedInput = (seedInputY + seedInputHeight + 120 <= screenHeight);
        
        // 绘制超平坦世界选项 - 放在最后一个滑块下方，与其他元素保持一定距离
        std::string superFlatText = "Super Flat World";
        int superFlatY = optionSliders.back().y + optionSliders.back().height + 25;
        int superFlatX = optionSliders[0].x;
        
        // 绘制超平坦世界标签
        drawText(renderer, superFlatText, superFlatX, superFlatY, Color(220, 220, 220));
        
        // 绘制美化的复选框 - 与标签保持水平对齐
        int checkboxSize = 24;
        int checkboxX = superFlatX + 200;
        int checkboxY = superFlatY - 5;
        
        // 绘制复选框背景
        Color checkboxBg = superFlatWorld ? Color(80, 120, 255, 200) : Color(60, 60, 80, 200);
        renderer.drawRect(checkboxX, checkboxY, checkboxSize, checkboxSize, checkboxBg);
        
        // 绘制复选框边框
        renderer.drawRectOutline(checkboxX, checkboxY, checkboxSize, checkboxSize, Color(150, 150, 180));
        
        // 如果选中，绘制勾选标记
        if (superFlatWorld) {
            // 绘制白色勾选符号
            int checkmarkX1 = checkboxX + 5;
            int checkmarkY1 = checkboxY + checkboxSize/2;
            int checkmarkX2 = checkboxX + checkboxSize/2 - 2;
            int checkmarkY2 = checkboxY + checkboxSize - 7;
            int checkmarkX3 = checkboxX + checkboxSize - 5;
            int checkmarkY3 = checkboxY + 5;
            
            renderer.drawLine(checkmarkX1, checkmarkY1, checkmarkX2, checkmarkY2, Color(255, 255, 255));
            renderer.drawLine(checkmarkX2, checkmarkY2, checkmarkX3, checkmarkY3, Color(255, 255, 255));
        }
        
        // 检查超平坦复选框点击
        if (mousePos.x >= checkboxX && mousePos.x < checkboxX + checkboxSize &&
            mousePos.y >= checkboxY && mousePos.y < checkboxY + checkboxSize) {
            // 高亮复选框边框
            renderer.drawRectOutline(checkboxX, checkboxY, checkboxSize, checkboxSize, Color(200, 200, 255));
            
            // 检测点击
            static bool lastCheckboxMouseDown = false;
            bool checkboxMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            if (checkboxMouseDown && !lastCheckboxMouseDown) {
                superFlatWorld = !superFlatWorld;
            }
            lastCheckboxMouseDown = checkboxMouseDown;
        }
        
        if (canShowSeedInput) {
            // 绘制种子输入标签 - 放在超平坦复选框下方，保持一定间距
            int seedLabelY = superFlatY + 40; // 增加间距
            std::string seedLabel = "World Seed (leave empty for random seed):";
            drawText(renderer, seedLabel, optionSliders[0].x, seedLabelY, Color(220, 220, 220));
            
            // 重新计算种子输入框位置，保持与标签对齐
            seedInputY = seedLabelY + 25;
            
            // 绘制美化的种子输入框
            // 输入框阴影
            renderer.drawRect(optionSliders[0].x + 2, seedInputY + 2, seedInputWidth, seedInputHeight, Color(20, 20, 20, 150));
            
            // 输入框背景
            Color inputBgColor = isSeedInputActive ? Color(50, 50, 60, 220) : Color(40, 40, 50, 200);
            renderer.drawRect(optionSliders[0].x, seedInputY, seedInputWidth, seedInputHeight, inputBgColor);
            
            // 输入框边框
            Color borderColor = isSeedInputActive ? Color(100, 150, 255, 200) : Color(100, 100, 130, 200);
            renderer.drawRectOutline(optionSliders[0].x, seedInputY, seedInputWidth, seedInputHeight, borderColor);
            
            // 输入框高光
            if (isSeedInputActive) {
                renderer.drawRect(optionSliders[0].x + 1, seedInputY + 1, seedInputWidth - 2, 1, Color(150, 180, 255, 150));
            }
            
            // 绘制当前种子文本
            Color textColor = isSeedInputActive ? Color(255, 255, 255) : Color(200, 200, 200);
            drawText(renderer, worldSeed + (isSeedInputActive && showCursor ? "|" : ""), 
                    optionSliders[0].x + 8, seedInputY + 6, textColor);
            
            // 绘制系统生成种子选项 - 美化为按钮样式，位置调整以避免重叠
            std::string systemSeedText = "Use system-generated seed";
            int sysCheckboxY = seedInputY + seedInputHeight + 25;
            
            // 绘制标签
            drawText(renderer, systemSeedText, optionSliders[0].x, sysCheckboxY, Color(220, 220, 220));
            
            // 绘制美化的复选框 - 确保位置合适
            int sysCheckboxSize = 24;
            int sysCheckboxX = optionSliders[0].x + 250; // 放在文本右侧适当位置
            int sysCheckboxPosY = sysCheckboxY - 5;
            
            // 绘制复选框背景
            Color sysCheckboxBg = m_isSystemGeneratedSeed ? Color(80, 120, 255, 200) : Color(60, 60, 80, 200);
            renderer.drawRect(sysCheckboxX, sysCheckboxPosY, sysCheckboxSize, sysCheckboxSize, sysCheckboxBg);
            
            // 绘制复选框边框
            renderer.drawRectOutline(sysCheckboxX, sysCheckboxPosY, sysCheckboxSize, sysCheckboxSize, Color(150, 150, 180));
            
            // 如果选中，绘制勾选标记
            if (m_isSystemGeneratedSeed) {
                // 绘制白色勾选符号
                int checkmarkX1 = sysCheckboxX + 5;
                int checkmarkY1 = sysCheckboxPosY + sysCheckboxSize/2;
                int checkmarkX2 = sysCheckboxX + sysCheckboxSize/2 - 2;
                int checkmarkY2 = sysCheckboxPosY + sysCheckboxSize - 7;
                int checkmarkX3 = sysCheckboxX + sysCheckboxSize - 5;
                int checkmarkY3 = sysCheckboxPosY + 5;
                
                renderer.drawLine(checkmarkX1, checkmarkY1, checkmarkX2, checkmarkY2, Color(255, 255, 255));
                renderer.drawLine(checkmarkX2, checkmarkY2, checkmarkX3, checkmarkY3, Color(255, 255, 255));
            }
            
            // 检查系统生成种子复选框点击
            if (mousePos.x >= sysCheckboxX && mousePos.x < sysCheckboxX + sysCheckboxSize &&
                mousePos.y >= sysCheckboxPosY && mousePos.y < sysCheckboxPosY + sysCheckboxSize) {
                // 高亮复选框边框
                renderer.drawRectOutline(sysCheckboxX, sysCheckboxPosY, sysCheckboxSize, sysCheckboxSize, Color(200, 200, 255));
                
                // 检测点击
                static bool lastSysCheckboxMouseDown = false;
                bool sysCheckboxMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
                if (sysCheckboxMouseDown && !lastSysCheckboxMouseDown) {
                    m_isSystemGeneratedSeed = !m_isSystemGeneratedSeed;
                }
                lastSysCheckboxMouseDown = sysCheckboxMouseDown;
            }
            
            // 保存种子按钮 - 位置调整，放在系统生成种子复选框下方
            int saveButtonY = sysCheckboxY + 40; // 增加间距
            
            if (saveSeedButton) {
                // 更新按钮位置
                saveSeedButton->y = saveButtonY;
                
                saveSeedButton->hovered = saveSeedButton->isMouseOver(mousePos.x, mousePos.y);
                saveSeedButton->draw(renderer);
            }
            
            // 显示保存确认消息
            if (showSeedSavedMessage) {
                std::string message = "Seed settings saved!";
                drawText(renderer, message, 
                        (screenWidth - message.length() * 8) / 2, 
                        saveSeedButton->y + saveSeedButton->height + 10, 
                        Color(100, 255, 100));
            }
        }
        
        // 返回按钮 - 确保总是在底部，且不会被其他元素遮挡
        if (backButton) {
            // 计算最佳位置
            int idealY = screenHeight - backButton->height - 30;
            
            // 如果有保存种子按钮，确保不会重叠
            if (saveSeedButton && canShowSeedInput) {
                idealY = std::max(idealY, saveSeedButton->y + saveSeedButton->height + 30);
            }
            
            // 更新按钮位置
            backButton->y = idealY;
            
            backButton->hovered = backButton->isMouseOver(mousePos.x, mousePos.y);
            backButton->draw(renderer);
        }
    }
    
    // 更新方块名称提示
    void updateBlockNameDisplay(float deltaTime) {
        if (blockNameDisplayTime > 0) {
            blockNameDisplayTime -= deltaTime;
        }
    }
    
    // 设置当前方块名称
    void setCurrentBlockName(const std::string& name) {
        currentBlockName = name;
        blockNameDisplayTime = MAX_BLOCK_NAME_DISPLAY_TIME;
    }
    
    // 替换物品栏中的物品
    void replaceInventoryItem(int slot, BlockType blockType) {
        if (slot >= 0 && slot < MAX_HOTBAR_SLOTS) {
            // 确保物品栏大小足够
            while (inventory.size() <= slot) {
                inventory.push_back(Item()); // 使用空物品填充
            }
            
                // 根据方块类型设置对应的物品类型
                ItemType itemType;
                switch (blockType) {
                    case BLOCK_DIRT: itemType = ITEM_DIRT; break;
                    case BLOCK_GRASS: itemType = ITEM_GRASS; break;
                    case BLOCK_STONE: itemType = ITEM_STONE; break;
                    case BLOCK_SAND: itemType = ITEM_SAND; break;
                    case BLOCK_WOOD: itemType = ITEM_WOOD; break;
                    case BLOCK_LEAVES: itemType = ITEM_LEAVES; break;
                    case BLOCK_COAL_ORE: itemType = ITEM_COAL; break;
                    case BLOCK_IRON_ORE: itemType = ITEM_IRON; break;
                    case BLOCK_GOLD_ORE: itemType = ITEM_GOLD; break;
                    case BLOCK_DIAMOND_ORE: itemType = ITEM_DIAMOND; break;
                    case BLOCK_BEDROCK: itemType = ITEM_BEDROCK; break;
                    case BLOCK_OBSIDIAN: itemType = ITEM_OBSIDIAN; break;
                    case BLOCK_LAVA: itemType = ITEM_LAVA; break;
                    case BLOCK_REDSTONE_ORE: itemType = ITEM_REDSTONE_ORE; break;
                    case BLOCK_EMERALD_ORE: itemType = ITEM_EMERALD_ORE; break;
                    case BLOCK_MOSSY_STONE: itemType = ITEM_MOSSY_STONE; break;
                    case BLOCK_GRAVEL: itemType = ITEM_GRAVEL; break;
                    case BLOCK_CLAY: itemType = ITEM_CLAY; break;
                    case BLOCK_ICE: itemType = ITEM_ICE; break;
                    case BLOCK_SNOW: itemType = ITEM_SNOW; break;
                    case BLOCK_WATER: itemType = ITEM_WATER; break;
                    case BLOCK_SANDSTONE: itemType = ITEM_SANDSTONE; break;
                    case BLOCK_CACTUS: itemType = ITEM_CACTUS; break;
                    case BLOCK_PUMPKIN: itemType = ITEM_PUMPKIN; break;
                    case BLOCK_NETHERRACK: itemType = ITEM_NETHERRACK; break;
                    case BLOCK_SOUL_SAND: itemType = ITEM_SOUL_SAND; break;
                    case BLOCK_QUARTZ: itemType = ITEM_QUARTZ; break;
                    case BLOCK_GLOWSTONE: itemType = ITEM_GLOWSTONE; break;
                    case BLOCK_MYCELIUM: itemType = ITEM_MYCELIUM; break;
                    case BLOCK_END_STONE: itemType = ITEM_END_STONE; break;
                    case BLOCK_PRISMARINE: itemType = ITEM_PRISMARINE; break;
                    case BLOCK_MAGMA: itemType = ITEM_MAGMA; break;
                    case BLOCK_NETHER_WART: itemType = ITEM_NETHER_WART; break;
                    case BLOCK_SLIME: itemType = ITEM_SLIME; break;
                    case BLOCK_BRICK: itemType = ITEM_BRICK; break;            // 添加砖块映射
                    case BLOCK_BOOKSHELF: itemType = ITEM_BOOKSHELF; break;    // 添加书架映射
                case BLOCK_CHANGE_BLOCK: itemType = ITEM_CHANGE_BLOCK; break;
                default: 
                    if (blockType == BLOCK_AIR) {
                        // 设置为空物品
                        inventory[slot] = Item();
                        std::cout << "Set slot " << slot << " to empty" << std::endl;
                    } else {
                        itemType = ITEM_DIRT; // 默认为泥土
                        inventory[slot] = Item(itemType, 64);
                    }
                        break;
                }
            
            // 如果不是空气方块，创建对应的物品
            if (blockType != BLOCK_AIR) {
                inventory[slot] = Item(itemType, 64);
            }
            
            // 更新当前手持方块类型
            if (slot == selectedSlot) {
                updateCurrentBlockType();
                if (!inventory[selectedSlot].isEmpty) {
                setCurrentBlockName(inventory[selectedSlot].getName());
                } else {
                    // 清除方块名称显示
                    setCurrentBlockName("");
                }
            }
        }
    }
    
    // 清空物品栏
    void clearInventory() {
        // 将所有物品设置为空状态，而不是完全清空数组
        for (int i = 0; i < inventory.size(); i++) {
            inventory[i] = Item(); // 使用默认构造函数创建空物品
        }
        
        // 重置当前选中的槽位
        selectedSlot = 0;
        // 重置当前手持方块类型
        currentBlockType = BLOCK_AIR;
    }
    
    // 直接选择指定槽位
    void selectSlot(int slot) {
        if (slot >= 0 && slot < MAX_HOTBAR_SLOTS) {
            selectedSlot = slot;
            if (selectedSlot < static_cast<int>(inventory.size())) {
                // 更新当前手持方块类型
                updateCurrentBlockType();
                
                // 显示当前方块名称（只有非空物品才显示名称）
                if (!inventory[selectedSlot].isEmpty && inventory[selectedSlot].count > 0) {
                setCurrentBlockName(inventory[selectedSlot].getName());
                } else {
                    // 清除当前方块名称显示
                    setCurrentBlockName("");
                }
            }
        }
    }
    
    // 处理数字键输入（用于替换物品栏或直接选择物品栏）
    void handleNumberKeyInput(int key) {
        // 如果在暂停菜单或选项菜单中,不处理数字键输入
        if (gameState == GAME_PAUSED || optionsState == OPTIONS_SHOWING || controlsState == CONTROLS_SHOWING) {
            return;
        }
        
        // 如果有替换提示，处理替换逻辑
        if (showReplacePrompt) {
            if (key >= '1' && key <= '9') {
                int slot = key - '1';
                if (blockToReplace != BLOCK_AIR) {
                    // 替换指定槽位的物品
                    replaceInventoryItem(slot, blockToReplace);
                }
            } else if (key == '0') {
                // 清空物品栏
                clearInventory();
            }
            
            // 关闭替换提示
            showReplacePrompt = false;
        } else if (gameState == GAME_PLAYING) {
            // 在游戏进行中，数字键1-9直接选择物品栏槽位
            if (key >= '1' && key <= '9') {
                int slot = key - '1';
                
                // 确保物品栏大小足够容纳选定的槽位
                while (inventory.size() <= slot) {
                    inventory.push_back(Item()); // 使用空物品填充
                }
                
                selectSlot(slot);
            }
        }
    }
    
    // 切换UI显示状态
    void toggleUI() {
        showUI = !showUI;
    }
    
    // 获取UI显示状态
    bool isUIVisible() const {
        return showUI;
    }
    
    // 更新屏幕尺寸（窗口大小变化时调用）
    void updateScreenSize(int width, int height) {
        screenWidth = width;
        screenHeight = height;
        
        // 重新初始化UI元素以适应新的屏幕尺寸
        initOptionsMenu();
        initControlsMenu();
    }
    
    // 处理ESC键按下
    void handleEscKeyPress() {
        // 如果聊天框开着，先关闭聊天框
        if (showChatBox) {
            closeChatBox();
        } 
        // 如果方块编辑器打开，先关闭方块编辑器
        else if (showBlockEditor) {
            showBlockEditor = false;
            // 显示未保存提示
            showSavePrompt = true;
            savePromptText = "EXIT WITHOUT SAVED!";
            savePromptColor = Color(255, 0, 0); // 红色
            savePromptTimer = MAX_SAVE_PROMPT_TIME;
            return; // 不执行其他操作
        }
        // 如果控制说明开着，先关闭控制说明
        else if (controlsState == CONTROLS_SHOWING) {
            setControlsState(CONTROLS_NONE);
            }
        // 如果选项菜单开着，先关闭选项菜单
        else if (optionsState == OPTIONS_SHOWING) {
            setOptionsState(OPTIONS_NONE);
        } 
        // 如果物品栏打开，关闭物品栏并返回游戏状态
        else if (gameState == GAME_INVENTORY) {
            setGameState(GAME_PLAYING);
            std::cout << "Inventory closed by ESC" << std::endl;
            // 锁定鼠标到准心
            lockMouseToCrosshair();
        } 
        // 如果游戏进行中，则打开菜单
        else if (gameState == GAME_PLAYING) {
            setGameState(GAME_PAUSED);
            
            // 只有当音乐真正在播放时才暂停
            if (musicEnabled && backgroundMusic && !isMusicActuallyStopped) {
                std::cout << "[Music] Pausing music due to game pause (ESC)" << std::endl;
                backgroundMusic->player.pauseMusic = true;
            }
            
            // 显示鼠标指针
            ShowCursor(TRUE);
        } 
        // 如果菜单打开，则关闭菜单
        else if (gameState == GAME_PAUSED) {
            setGameState(GAME_PLAYING);
            
            // 只有当音乐真正在播放时才恢复
            if (musicEnabled && backgroundMusic && !isMusicActuallyStopped) {
                std::cout << "[Music] Resuming music after closing pause menu (ESC)" << std::endl;
                backgroundMusic->player.pauseMusic = false;
            }
        }
    }
    
    // 锁定鼠标到屏幕中心（准心位置）
    void lockMouseToCrosshair() {
        // 隐藏鼠标指针
        ShowCursor(FALSE);
        
        // 获取窗口句柄
        HWND hwnd = GetActiveWindow();
        if (hwnd) {
            // 获取窗口客户区域大小
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // 将鼠标移动到窗口中心
            POINT center;
            center.x = (rect.right - rect.left) / 2;
            center.y = (rect.bottom - rect.top) / 2;
            
            // 转换为屏幕坐标
            ClientToScreen(hwnd, &center);
            
            // 设置鼠标位置
            SetCursorPos(center.x, center.y);
        }
    }
    
    // 获取菜单操作结果
    int getMenuActionResult() {
        int result = menuActionResult;
        // 重置结果为默认值
        if (result != 1) {
            menuActionResult = 1;
        }
        return result;
    }
    
    // 渲染UI - 支持多线程
    void render(Renderer& renderer, const Camera& camera, const World& world [[maybe_unused]], int fps) {
        // 更新音乐状态
        updateMusic();
        
        // 更新3D立方体预览的旋转角度
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<float>(currentTime - lastCubeUpdateTime).count();
        lastCubeUpdateTime = currentTime;
        
        // 更新聊天消息计时器
        updateChatMessageTimers(elapsed);
        
        // 缓慢旋转立方体 - 提高旋转速度
        cubeRotationY += 6.0f * elapsed;
        cubeRotationX += 2.0f * elapsed;
        
        // 保持角度在0-360度之间
        while (cubeRotationY >= 360.0f) cubeRotationY -= 360.0f;
        while (cubeRotationX >= 360.0f) cubeRotationX -= 360.0f;
        
        // 重置物品提示状态
        showItemTooltip = false;
        
        // 只有在UI显示且游戏进行中时才绘制准心
        if (gameState == GAME_PLAYING && showUI && !showBlockEditor) {
            renderer.drawCrosshair();
        }
        
        // 如果UI被隐藏且不在菜单或物品栏状态，直接返回
        if (!showUI && gameState == GAME_PLAYING && controlsState != CONTROLS_SHOWING && 
            optionsState != OPTIONS_SHOWING && !debugInfoEnabled && !showBlockEditor) {
            return;
        }
        
        // 绘制物品栏（始终显示，除非在控制提示界面或选项界面或UI被隐藏）
        if (showUI && controlsState != CONTROLS_SHOWING && optionsState != OPTIONS_SHOWING && !showBlockEditor) {
            drawInventory(renderer);
        }
        
        // 如果方块编辑器打开，则渲染它（优先级最高）
        if (showBlockEditor && gameState == GAME_PLAYING) {
            drawBlockEditor(renderer);
            return;  // 不绘制其他UI
        }
        
        // 根据游戏状态绘制不同UI
        if (controlsState == CONTROLS_SHOWING) {
            // 显示控制提示界面
            drawControlsMenu(renderer);
        } else if (optionsState == OPTIONS_SHOWING) {
            // 显示选项界面
            drawOptionsMenu(renderer);
        } else if (gameState == GAME_PAUSED) {
            drawPauseMenu(renderer);
        } else if (gameState == GAME_INVENTORY) {
            drawFullInventory(renderer);
        } else if (showUI || debugInfoEnabled) {
            // 如果启用了调试信息，显示详细调试信息
            if (debugInfoEnabled) {
                // 设置基础间距和起始位置
                const int baseY = 10;           // 起始Y坐标
                const int lineHeight = 20;      // 每行文本高度
                int currentY = baseY;           // 当前Y坐标
                
                // 显示FPS计数器
                std::string fpsText = "FPS: " + std::to_string(fps);
                drawText(renderer, fpsText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示玩家位置信息
                Vec3 playerPos = camera.position;
                std::string posText = "Pos: X:" + std::to_string(static_cast<int>(playerPos.x)) + 
                                     " Y:" + std::to_string(static_cast<int>(playerPos.y)) + 
                                     " Z:" + std::to_string(static_cast<int>(playerPos.z));
                drawText(renderer, posText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示更详细的位置信息（包括小数点）
                std::string detailPosText = "Exact Position: X:" + std::to_string(playerPos.x).substr(0, 6) + 
                                         " Y:" + std::to_string(playerPos.y).substr(0, 6) + 
                                         " Z:" + std::to_string(playerPos.z).substr(0, 6);
                drawText(renderer, detailPosText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示视角信息
                std::string lookText = "Look: Pitch:" + std::to_string(camera.pitch).substr(0, 6) + 
                                     " Yaw:" + std::to_string(camera.yaw).substr(0, 6);
                drawText(renderer, lookText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示FOV信息
                float actualFOV = 60.0f + fovValue * 60.0f;
                std::string fovText = "FOV: " + std::to_string(actualFOV).substr(0, 6);
                drawText(renderer, fovText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示移动速度信息
                float actualWalkSpeed = 5.0f + walkSpeedValue * 15.0f;
                float actualFlySpeed = 10.0f + flySpeedValue * 40.0f;
                std::string speedText = "Speed: Walk=" + std::to_string(actualWalkSpeed).substr(0, 5) + 
                                      " Fly=" + std::to_string(actualFlySpeed).substr(0, 5);
                drawText(renderer, speedText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示世界信息
                std::string worldText = "World Size: " + std::to_string(worldWidth) + "x" + 
                                      std::to_string(worldHeight) + "x" + 
                                      std::to_string(worldDepth);
                drawText(renderer, worldText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 添加世界种子信息
                std::string seedText = "World Seed: " + std::to_string(worldSeedValue);
                drawText(renderer, seedText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 添加当前方块类型信息
                std::string blockText;
                // 正确转换BlockType到ItemType
                BlockType currentType = getCurrentBlockType();
                ItemType itemType;
                switch (currentType) {
                    case BLOCK_DIRT: itemType = ITEM_DIRT; break;
                    case BLOCK_GRASS: itemType = ITEM_GRASS; break;
                    case BLOCK_STONE: itemType = ITEM_STONE; break;
                    case BLOCK_SAND: itemType = ITEM_SAND; break;
                    case BLOCK_WOOD: itemType = ITEM_WOOD; break;
                    case BLOCK_LEAVES: itemType = ITEM_LEAVES; break;
                    case BLOCK_COAL_ORE: itemType = ITEM_COAL; break;
                    case BLOCK_IRON_ORE: itemType = ITEM_IRON; break;
                    case BLOCK_GOLD_ORE: itemType = ITEM_GOLD; break;
                    case BLOCK_DIAMOND_ORE: itemType = ITEM_DIAMOND; break;
                    case BLOCK_BEDROCK: itemType = ITEM_BEDROCK; break;
                    case BLOCK_OBSIDIAN: itemType = ITEM_OBSIDIAN; break;
                    case BLOCK_LAVA: itemType = ITEM_LAVA; break;
                    case BLOCK_REDSTONE_ORE: itemType = ITEM_REDSTONE_ORE; break;
                    case BLOCK_EMERALD_ORE: itemType = ITEM_EMERALD_ORE; break;
                    case BLOCK_MOSSY_STONE: itemType = ITEM_MOSSY_STONE; break;
                    case BLOCK_GRAVEL: itemType = ITEM_GRAVEL; break;
                    case BLOCK_CLAY: itemType = ITEM_CLAY; break;
                    case BLOCK_ICE: itemType = ITEM_ICE; break;
                    case BLOCK_SNOW: itemType = ITEM_SNOW; break;
                    case BLOCK_WATER: itemType = ITEM_WATER; break;
                    case BLOCK_SANDSTONE: itemType = ITEM_SANDSTONE; break;
                    case BLOCK_CACTUS: itemType = ITEM_CACTUS; break;
                    case BLOCK_PUMPKIN: itemType = ITEM_PUMPKIN; break;
                    case BLOCK_NETHERRACK: itemType = ITEM_NETHERRACK; break;
                    case BLOCK_SOUL_SAND: itemType = ITEM_SOUL_SAND; break;
                    case BLOCK_QUARTZ: itemType = ITEM_QUARTZ; break;
                    case BLOCK_GLOWSTONE: itemType = ITEM_GLOWSTONE; break;
                    case BLOCK_MYCELIUM: itemType = ITEM_MYCELIUM; break;
                    case BLOCK_END_STONE: itemType = ITEM_END_STONE; break;
                    case BLOCK_PRISMARINE: itemType = ITEM_PRISMARINE; break;
                    case BLOCK_MAGMA: itemType = ITEM_MAGMA; break;
                    case BLOCK_NETHER_WART: itemType = ITEM_NETHER_WART; break;
                    case BLOCK_SLIME: itemType = ITEM_SLIME; break;
                    case BLOCK_BRICK: itemType = ITEM_BRICK; break;
                    case BLOCK_BOOKSHELF: itemType = ITEM_BOOKSHELF; break;
                    case BLOCK_AIR: 
                    default: 
                        blockText = "Current Block: None";
                        itemType = ITEM_DIRT;
                        break;
                }
                
                if (currentType != BLOCK_AIR) {
                    blockText = "Current Block: " + Item(itemType).getName();
                }
                
                drawText(renderer, blockText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 渲染距离信息
                std::string renderDistText = "Render Distance: " + std::to_string(getActualRenderDistance());
                drawText(renderer, renderDistText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 窗口分辨率
                std::string resolutionText = "Resolution: " + std::to_string(screenWidth) + "x" + std::to_string(screenHeight);
                drawText(renderer, resolutionText, 10, currentY, Color(255, 255, 255));
                currentY += lineHeight;
                
                // 显示游戏版本
                std::string versionText = "Minecraft Clone v1.0";
                drawText(renderer, versionText, 10, currentY, Color(255, 255, 255));
            } else if (showUI) {
                // 只在非调试模式下显示简单信息
                // 显示FPS计数器
                std::string fpsText = "FPS: " + std::to_string(fps);
                drawText(renderer, fpsText, 10, 10, Color(255, 255, 255));
            }
            
            // 绘制当前方块名称提示（如果有）
            if (blockNameDisplayTime > 0 && !currentBlockName.empty()) {
                int textWidth = currentBlockName.length() * 8;
                int textX = (screenWidth - textWidth) / 2;
                int textY = screenHeight / 2 + 30;
                
                // 计算透明度（淡出效果）
                float alpha = std::min(1.0f, blockNameDisplayTime / (MAX_BLOCK_NAME_DISPLAY_TIME / 2));
                int alphaValue = static_cast<int>(alpha * 255);
                
                // 绘制文本背景
                renderer.drawRect(textX - 5, textY - 5, textWidth + 10, 30, Color(0, 0, 0, alphaValue * 0.7f));
                
                // 绘制文本
                drawText(renderer, currentBlockName, textX, textY, Color(255, 255, 255, alphaValue));
            }
        }
        
        // 更新保存提示计时器
        if (showSavePrompt && savePromptTimer > 0) {
            savePromptTimer--;
            if (savePromptTimer <= 0) {
                showSavePrompt = false;
            }
        }
        
        // 如果有保存提示，绘制提示文本
        if (showSavePrompt && savePromptTimer > 0) {
            int textWidth = savePromptText.length() * 8;
            int textX = (screenWidth - textWidth) / 2;
            int textY = screenHeight / 2 - 50; // 在屏幕中上方显示
            
            renderer.drawText(textX, textY, savePromptText, savePromptColor);
        }

        // 在UI最顶层绘制聊天框
        if (showUI || showChatBox) {
            drawChatBox(renderer);
        }
    }
    
    // 处理菜单鼠标点击，返回执行的选项索引（与executeSelectedMenuOption相同）
    int handleMenuMouseClick(int mouseX, int mouseY) {
        // 计算菜单选项的位置
        int buttonWidth = 300;
        int buttonHeight = 40;
        int buttonSpacing = 10;
        int startX = (screenWidth - buttonWidth) / 2;
        int startY = screenHeight / 3;
        
        // 检查每个选项是否被点击
        for (size_t i = 0; i < menuOptions.size(); i++) {
            int buttonY = startY + (buttonHeight + buttonSpacing) * i;
            
            // 简单的矩形碰撞检测
            if (mouseX >= startX && mouseX < startX + buttonWidth &&
                mouseY >= buttonY && mouseY < buttonY + buttonHeight) {
                // 选项被点击
                selectedMenuOption = i;
                return executeSelectedMenuOption();
            }
        }
        
        return -1; // 没有选项被点击
    }
    
    // 获取超平坦世界设置
    bool getSuperFlatWorld() const {
        return superFlatWorld;
    }
    
    // 获取世界大小值
    float getWorldSizeValue() const {
        return worldSizeValue;
    }
    
    // 获取实际世界大小
    int getActualWorldSize() const {
        // 根据世界大小滑块值计算实际世界大小
        // 最小为8，最大为360
        return static_cast<int>(6 + worldSizeValue * 355);
    }
    
    // 获取方块并添加到物品栏
    void pickBlock(BlockType blockType) {
        // 如果是空气方块，不做任何处理
        if (blockType == BLOCK_AIR) {
            return;
        }
        
        // 将方块类型转换为对应的物品类型
        ItemType correspondingItemType;
        switch (blockType) {
            case BLOCK_DIRT: correspondingItemType = ITEM_DIRT; break;
            case BLOCK_GRASS: correspondingItemType = ITEM_GRASS; break;
            case BLOCK_STONE: correspondingItemType = ITEM_STONE; break;
            case BLOCK_SAND: correspondingItemType = ITEM_SAND; break;
            case BLOCK_WOOD: correspondingItemType = ITEM_WOOD; break;
            case BLOCK_LEAVES: correspondingItemType = ITEM_LEAVES; break;
            case BLOCK_COAL_ORE: correspondingItemType = ITEM_COAL; break;
            case BLOCK_IRON_ORE: correspondingItemType = ITEM_IRON; break;
            case BLOCK_GOLD_ORE: correspondingItemType = ITEM_GOLD; break;
            case BLOCK_DIAMOND_ORE: correspondingItemType = ITEM_DIAMOND; break;
            case BLOCK_BEDROCK: correspondingItemType = ITEM_BEDROCK; break;
            case BLOCK_OBSIDIAN: correspondingItemType = ITEM_OBSIDIAN; break;
            case BLOCK_LAVA: correspondingItemType = ITEM_LAVA; break;
            case BLOCK_REDSTONE_ORE: correspondingItemType = ITEM_REDSTONE_ORE; break;
            case BLOCK_EMERALD_ORE: correspondingItemType = ITEM_EMERALD_ORE; break;
            case BLOCK_MOSSY_STONE: correspondingItemType = ITEM_MOSSY_STONE; break;
            case BLOCK_GRAVEL: correspondingItemType = ITEM_GRAVEL; break;
            case BLOCK_CLAY: correspondingItemType = ITEM_CLAY; break;
            case BLOCK_ICE: correspondingItemType = ITEM_ICE; break;
            case BLOCK_SNOW: correspondingItemType = ITEM_SNOW; break;
            case BLOCK_WATER: correspondingItemType = ITEM_WATER; break;
            case BLOCK_SANDSTONE: correspondingItemType = ITEM_SANDSTONE; break;
            case BLOCK_CACTUS: correspondingItemType = ITEM_CACTUS; break;
            case BLOCK_PUMPKIN: correspondingItemType = ITEM_PUMPKIN; break;
            case BLOCK_NETHERRACK: correspondingItemType = ITEM_NETHERRACK; break;
            case BLOCK_SOUL_SAND: correspondingItemType = ITEM_SOUL_SAND; break;
            case BLOCK_QUARTZ: correspondingItemType = ITEM_QUARTZ; break;
            case BLOCK_GLOWSTONE: correspondingItemType = ITEM_GLOWSTONE; break;
            case BLOCK_MYCELIUM: correspondingItemType = ITEM_MYCELIUM; break;
            case BLOCK_END_STONE: correspondingItemType = ITEM_END_STONE; break;
            case BLOCK_PRISMARINE: correspondingItemType = ITEM_PRISMARINE; break;
            case BLOCK_MAGMA: correspondingItemType = ITEM_MAGMA; break;
            case BLOCK_NETHER_WART: correspondingItemType = ITEM_NETHER_WART; break;
            case BLOCK_SLIME: correspondingItemType = ITEM_SLIME; break;
            case BLOCK_BRICK: correspondingItemType = ITEM_BRICK; break;
            case BLOCK_BOOKSHELF: correspondingItemType = ITEM_BOOKSHELF; break;
            case BLOCK_CHANGE_BLOCK: correspondingItemType = ITEM_CHANGE_BLOCK; break;
            default: correspondingItemType = ITEM_DIRT; break;
        }
        
        // 首先检查物品栏中是否已经有这个方块
        for (int i = 0; i < static_cast<int>(inventory.size()) && i < MAX_HOTBAR_SLOTS; i++) {
            // 如果找到相同类型的物品，直接选择该槽位
            if (!inventory[i].isEmpty && inventory[i].type == correspondingItemType && inventory[i].count > 0) {
                selectedSlot = i;
                updateCurrentBlockType();
                setCurrentBlockName(inventory[i].getName());
                std::cout << "Found existing block in slot " << i + 1 << ", switching to it" << std::endl;
                return;
            }
        }
        
        // 确保物品栏可以容纳当前选择的槽位
        while (inventory.size() <= selectedSlot) {
            // 添加空物品到物品栏直到达到所需大小
            inventory.push_back(Item());
        }
        
        // 在选定的槽位放置新物品
        inventory[selectedSlot] = Item(correspondingItemType, 64);
        inventory[selectedSlot].isEmpty = false; // 确保isEmpty标志设置正确
        
        // 更新当前手持方块类型
        updateCurrentBlockType();
        setCurrentBlockName(inventory[selectedSlot].getName());
        
        std::cout << "Picked block and placed in selected slot " << selectedSlot + 1 << std::endl;
    }
    
    // 更新音乐音量
    void updateMusicVolume() {
        // 记录之前的状态
        bool wasDisabled = !musicEnabled;
        
        // 如果音量为0，停止音乐
        if (musicVolumeValue <= 0.01f) {
            stopMusic();
            musicEnabled = false;
            std::cout << "[Music] Volume set to 0, music disabled" << std::endl;
        } else {
            // 设置音量并启用音乐
            musicEnabled = true;
            
            // 设置音量 (0-127)
            int volume = static_cast<int>(musicVolumeValue * 127);
            std::cout << "[Music] Volume set to " << volume << "/127 (" << static_cast<int>(musicVolumeValue * 100) << "%)" << std::endl;
            
            // 如果之前禁用了音乐,现在重新启用,重置随机延迟
            if (wasDisabled) {
                // 重置随机延迟
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distr(10, 60); // 较短的延迟(10-60秒)
                randomMusicDelay = distr(gen);
                
                // 重置计时器
                lastMusicPlayTime = std::chrono::steady_clock::now();
                
                // 重置状态
                shouldPlayMusic = false;
                musicInitialized = false;
                
                std::cout << "[Music] Music re-enabled, new random delay: " << randomMusicDelay << " seconds" << std::endl;
            } else if (backgroundMusic) {
                // 如果音乐已初始化,只更新音量(无论是游戏内置音乐还是自定义音乐)
                backgroundMusic->player.setVolume(volume);
                std::cout << "[Music] Volume applied to current music" << std::endl;
            } else {
                // 不要在这里初始化音乐,只更新音量值
                std::cout << "[Music] No active music to apply volume to" << std::endl;
            }
        }
    }
    
    // 初始化音乐
    void initMusic() {
        if (!musicEnabled) {
            std::cout << "[Music] Initialization skipped - music is disabled" << std::endl;
            return;
        }
        
        try {
            // 选择要播放的音乐
            std::string musicName;
            std::string musicData;
            
            // 随机选择音乐类型
            std::random_device rd;
            std::mt19937 gen(rd());
            
            // 90%的几率播放变种,10%的几率播放原版
            std::uniform_int_distribution<> musicDist(1, 100);
            int musicRoll = musicDist(gen);
            
            if (musicRoll <= 90) {
                // 90%的几率播放变种
                currentMusicType = MUSIC_HAGGSTROM_VARIANT;
                musicName = "Haggstrom Variant";
                musicData = MinecraftMusic::HAGGSTROM_VARIANT;
                std::cout << "[Music] Selected Haggstrom Variant (90% chance)" << std::endl;
            } else {
                // 10%的几率播放原版
                currentMusicType = MUSIC_HAGGSTROM;
                musicName = "Haggstrom";
                musicData = MinecraftMusic::HAGGSTROM_MUSIC;
                std::cout << "[Music] Selected original Haggstrom (10% chance)" << std::endl;
            }
            
            // 尝试从文件加载变种音乐
            bool loadedFromFile = false;
            if (currentMusicType == MUSIC_HAGGSTROM_VARIANT) {
                // 创建背景音乐对象(如果不存在)
                if (!backgroundMusic) {
                    int volume = static_cast<int>(musicVolumeValue * 127);
                    backgroundMusic = std::make_unique<BGM>("", volume);
                }
                
                // 尝试从文件加载
                loadedFromFile = backgroundMusic->loadMusicFromFile("Minecraft.txt");
                // 删除成功加载提示信息
            }
            
            // 如果没有从文件加载成功,使用内置音乐数据
            if (!loadedFromFile) {
                std::cout << "[Music] Using built-in " << musicName << " music data" << std::endl;
                
                // 设置音量
                int volume = static_cast<int>(musicVolumeValue * 127);
                
                // 创建背景音乐
                if (!backgroundMusic) {
                    backgroundMusic = std::make_unique<BGM>("", volume);
                } else {
                    backgroundMusic->player.setVolume(volume);
                }
                
                // 设置音乐数据
                backgroundMusic->setMusicFromString(musicData, musicName);
            }
            
            std::cout << "[Music] Music initialized with volume " << static_cast<int>(musicVolumeValue * 127) << "/127" << std::endl;
            musicInitialized = true;
            
            // 记录播放时间
            lastMusicPlayTime = std::chrono::steady_clock::now();
        } catch (const std::exception& e) {
            std::cerr << "[Music] Failed to initialize music: " << e.what() << std::endl;
            musicInitialized = false;
        } catch (...) {
            std::cerr << "[Music] Failed to initialize music: Unknown error" << std::endl;
            musicInitialized = false;
        }
    }
    
    // 播放音乐
    void playMusic() {
        // 确保有音乐对象
        if (!musicInitialized) {
            std::cout << "[Music] Play skipped - music not initialized" << std::endl;
            return;
        }
        
        if (!backgroundMusic) {
            std::cout << "[Music] Play failed - backgroundMusic is null" << std::endl;
            return;
        }
        
        try {
            std::cout << "[Music] Playing Haggstrom..." << std::endl;
            backgroundMusic->play();
            lastMusicPlayTime = std::chrono::steady_clock::now();
            std::cout << "[Music] Music started playing" << std::endl;
            
            // 设置音乐非停止状态
            isMusicActuallyStopped = false;
        } catch (const std::exception& e) {
            std::cerr << "[Music] Failed to play music: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Music] Failed to play music: Unknown error" << std::endl;
        }
    }
    
    // 停止音乐
    void stopMusic() {
        // 检查是否正在准备播放自定义音乐
        if (backgroundMusic) {
            try {
                std::cout << "[Music] Stopping music..." << std::endl;
                backgroundMusic->stop();
                std::cout << "[Music] Music stopped" << std::endl;
                
                // 设置音乐真正停止的标志
                isMusicActuallyStopped = true;
                
                // 只有在不处于准备状态时才重置自定义音乐标志
                if (!isPreparingCustomMusic) {
                    isPlayingCustomMusic = false;
                    std::cout << "[Music] Reset isPlayingCustomMusic flag" << std::endl;
                } else {
                    std::cout << "[Music] Maintaining isPlayingCustomMusic flag due to pending playback" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "[Music] Failed to stop music: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "[Music] Failed to stop music: Unknown error" << std::endl;
            }
        } else {
            std::cout << "[Music] Stop skipped - no active music" << std::endl;
        }
    }
    
    // 更新音乐状态
    void updateMusic() {
        if (!musicEnabled) return;
        
        // 如果正在准备播放自定义音乐，暂时不处理其他音乐逻辑
        if (isPreparingCustomMusic) {
            if (DEBUG) std::cout << "[Music] Custom music preparation in progress, skipping updateMusic logic" << std::endl;
            return;
        }
        
        auto currentTime = std::chrono::steady_clock::now();
        
        // 检查音乐是否播放完成
        if (musicInitialized && backgroundMusic) {
            if (backgroundMusic->isFinished()) {
                std::cout << "[Music] Detected music has finished playing" << std::endl;
                
                // 停止当前音乐
                stopMusic();
                
                // 如果是自定义音乐，重置标志
                if (isPlayingCustomMusic) {
                    isPlayingCustomMusic = false;
                    std::cout << "[Music] Custom music playback ended" << std::endl;
                }
                
                // 设置新的随机延迟(120-300秒,即2-5分钟)
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> distr(120, 300);
                randomMusicDelay = distr(gen);
                
                std::cout << "[Music] Setting new delay of " << randomMusicDelay << " seconds before next play" << std::endl;
                
                // 重置状态
                lastMusicPlayTime = currentTime;
                shouldPlayMusic = false;
                musicInitialized = false;
                isMusicActuallyStopped = true; // 确保设置为停止状态
            }
            
            // 如果正在播放自定义音乐，不进行其他处理
            if (isPlayingCustomMusic) {
                return;
            }
        } else if (isPlayingCustomMusic) {
            // 如果标记为正在播放自定义音乐，但音乐对象不存在或未初始化
            // 这可能是因为在延迟播放线程执行前检查，此时不应该继续游戏音乐的处理
            return;
        }
        
        // 如果音乐尚未初始化且应该播放,先等待随机延迟
        if (!musicInitialized && !shouldPlayMusic && !isPlayingCustomMusic) {
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(
                currentTime - lastMusicPlayTime).count();
                
            if (elapsedTime >= randomMusicDelay) {
                std::cout << "[Music] Random delay of " << randomMusicDelay << " seconds elapsed, preparing to play music" << std::endl;
                shouldPlayMusic = true;
            }
        }
        
        // 如果应该播放音乐但尚未初始化,初始化并播放
        if (shouldPlayMusic && !musicInitialized && !isPlayingCustomMusic) {
            std::cout << "[Music] Initializing music after delay..." << std::endl;
            initMusic();
            if (musicInitialized) {
                playMusic();
            }
            return;
        }
    }
    
    // 获取音乐音量值
    float getMusicVolumeValue() const {
        return musicVolumeValue;
    }
    
    // 判断音乐是否启用
    bool isMusicEnabled() const {
        return musicEnabled;
    }
    
    // 设置世界大小信息（由外部调用）
    void setWorldInfo(int width, int height, int depth, unsigned int seed) {
        worldWidth = width;
        worldHeight = height;
        worldDepth = depth;
        worldSeedValue = seed;
    }
    
    // 绘制方块编辑器UI
    void drawBlockEditor(Renderer& renderer) {
        // 计算编辑器窗口尺寸和位置
        int editorWidth = screenWidth / 2;
        int editorHeight = screenHeight / 2;
        int startX = (screenWidth - editorWidth) / 2;
        int startY = (screenHeight - editorHeight) / 2;
        
        // 绘制背景
        renderer.drawRect(startX, startY, editorWidth, editorHeight, Color(30, 30, 30, 200));
        renderer.drawRectOutline(startX, startY, editorWidth, editorHeight, Color(200, 200, 200));
        
        // 绘制标题
        std::string title = "Change Block Editor";
        renderer.drawText(startX + (editorWidth - title.length() * 8) / 2, startY + 20, title, Color(255, 255, 255));
        
        // 计算布局参数 - 动态分配位置
        int padding = 20;
        int buttonWidth = 80;
        int buttonHeight = 30;
        int sliderWidth = 150;
        int sliderHeight = 20;
        int sliderGap = 35;
        
        // 计算左侧区域宽度
        int leftSectionWidth = editorWidth / 2 - padding;
        
        // 绘制当前选择的面
        std::string faceNames[FACE_COUNT] = {"Front", "Back", "Left", "Right", "Top", "Bottom"};
        std::string currentFaceText = "Selected face: " + faceNames[selectedBlockFace];
        renderer.drawText(startX + padding, startY + 60, currentFaceText, Color(255, 255, 255));
        
        // 绘制面选择按钮
        int buttonsStartX = startX + padding;
        int buttonsStartY = startY + 90;
        int buttonsPerRow = 3;
        
        for (int i = 0; i < FACE_COUNT; i++) {
            int row = i / buttonsPerRow;
            int col = i % buttonsPerRow;
            int buttonX = buttonsStartX + col * (buttonWidth + 10);
            int buttonY = buttonsStartY + row * (buttonHeight + 10);
            
            // 绘制按钮背景
            Color buttonColor = (i == selectedBlockFace) ? Color(100, 100, 255, 200) : Color(70, 70, 70, 200);
            renderer.drawRect(buttonX, buttonY, buttonWidth, buttonHeight, buttonColor);
            renderer.drawRectOutline(buttonX, buttonY, buttonWidth, buttonHeight, Color(150, 150, 150));
            
            // 绘制按钮文本
            renderer.drawText(buttonX + (buttonWidth - faceNames[i].length() * 8) / 2, 
                             buttonY + (buttonHeight - 16) / 2, faceNames[i], Color(255, 255, 255));
        }
        
        // 绘制颜色调整滑块 - 确保在UI面板内
        int slidersStartX = startX + padding + 30; // 增加左边距，确保标签在面板内
        int slidersStartY = startY + 180;
        
        // 获取当前面的颜色
        Color currentColor = faceColors[selectedBlockFace];
        
        // 红色滑块
        renderer.drawText(slidersStartX, slidersStartY, "R:", Color(255, 100, 100));
        renderer.drawRect(slidersStartX + 25, slidersStartY, sliderWidth, sliderHeight, Color(50, 50, 50, 200));
        renderer.drawRect(slidersStartX + 25, slidersStartY, sliderWidth * currentColor.r / 255, sliderHeight, Color(255, 0, 0, 200));
        renderer.drawRectOutline(slidersStartX + 25, slidersStartY, sliderWidth, sliderHeight, Color(150, 150, 150));
        renderer.drawText(slidersStartX + 25 + sliderWidth + 10, slidersStartY, std::to_string(currentColor.r), Color(255, 255, 255));
        
        // 绿色滑块
        renderer.drawText(slidersStartX, slidersStartY + sliderGap, "G:", Color(100, 255, 100));
        renderer.drawRect(slidersStartX + 25, slidersStartY + sliderGap, sliderWidth, sliderHeight, Color(50, 50, 50, 200));
        renderer.drawRect(slidersStartX + 25, slidersStartY + sliderGap, sliderWidth * currentColor.g / 255, sliderHeight, Color(0, 255, 0, 200));
        renderer.drawRectOutline(slidersStartX + 25, slidersStartY + sliderGap, sliderWidth, sliderHeight, Color(150, 150, 150));
        renderer.drawText(slidersStartX + 25 + sliderWidth + 10, slidersStartY + sliderGap, std::to_string(currentColor.g), Color(255, 255, 255));
        
        // 蓝色滑块
        renderer.drawText(slidersStartX, slidersStartY + sliderGap * 2, "B:", Color(100, 100, 255));
        renderer.drawRect(slidersStartX + 25, slidersStartY + sliderGap * 2, sliderWidth, sliderHeight, Color(50, 50, 50, 200));
        renderer.drawRect(slidersStartX + 25, slidersStartY + sliderGap * 2, sliderWidth * currentColor.b / 255, sliderHeight, Color(0, 0, 255, 200));
        renderer.drawRectOutline(slidersStartX + 25, slidersStartY + sliderGap * 2, sliderWidth, sliderHeight, Color(150, 150, 150));
        renderer.drawText(slidersStartX + 25 + sliderWidth + 10, slidersStartY + sliderGap * 2, std::to_string(currentColor.b), Color(255, 255, 255));
        
        // 透明度滑块
        renderer.drawText(slidersStartX, slidersStartY + sliderGap * 3, "A:", Color(200, 200, 200));
        renderer.drawRect(slidersStartX + 25, slidersStartY + sliderGap * 3, sliderWidth, sliderHeight, Color(50, 50, 50, 200));
        renderer.drawRect(slidersStartX + 25, slidersStartY + sliderGap * 3, sliderWidth * currentColor.a / 255, sliderHeight, Color(200, 200, 200, 200));
        renderer.drawRectOutline(slidersStartX + 25, slidersStartY + sliderGap * 3, sliderWidth, sliderHeight, Color(150, 150, 150));
        renderer.drawText(slidersStartX + 25 + sliderWidth + 10, slidersStartY + sliderGap * 3, std::to_string(currentColor.a), Color(255, 255, 255));
        
        // 绘制预览方块
        int previewSize = 100; // 减小预览尺寸
        int previewX = startX + editorWidth - previewSize/2 - padding*2; // 靠右放置预览
        int previewY = startY + editorHeight / 2 - 20; // 向上移动预览窗口
        
        // 绘制预览标题 - 向上移动更多，避免被立方体遮挡
        std::string previewTitle = "Block Preview";
        renderer.drawText(previewX - previewTitle.length() * 4, previewY - previewSize/2 - 50, previewTitle, Color(255, 255, 255));
        
        // 绘制3D旋转立方体预览 - 不再绘制周围的边框
        draw3DRotatingCube(renderer, previewX, previewY, previewSize, faceColors);
        
        // 计算按钮区域位置 - 与绘制函数保持一致
        int buttonAreaY = startY + editorHeight - buttonHeight - padding*2;
        int buttonSpacing = 20;
        int totalButtonsWidth = 3 * buttonWidth + 2 * buttonSpacing;
        int buttonsStartPosX = startX + (editorWidth - totalButtonsWidth) / 2;
        
        // 绘制保存按钮
        int saveButtonWidth = buttonWidth;
        int saveButtonHeight = buttonHeight;
        int saveButtonX = buttonsStartPosX;
        int saveButtonY = buttonAreaY;
        
        renderer.drawRect(saveButtonX, saveButtonY, saveButtonWidth, saveButtonHeight, Color(0, 150, 0, 200));
        renderer.drawRectOutline(saveButtonX, saveButtonY, saveButtonWidth, saveButtonHeight, Color(100, 200, 100));
        // 调整文字位置，确保居中
        renderer.drawText(saveButtonX + (saveButtonWidth - 32) / 2, saveButtonY + (saveButtonHeight - 16) / 2, "Save", Color(255, 255, 255));
        
        // 绘制重置颜色按钮
        int resetButtonWidth = buttonWidth;
        int resetButtonHeight = buttonHeight;
        int resetButtonX = buttonsStartPosX + buttonWidth + buttonSpacing;
        int resetButtonY = buttonAreaY;
        
        renderer.drawRect(resetButtonX, resetButtonY, resetButtonWidth, resetButtonHeight, Color(150, 150, 0, 200));
        renderer.drawRectOutline(resetButtonX, resetButtonY, resetButtonWidth, resetButtonHeight, Color(200, 200, 100));
        // 调整文字位置，确保居中且不重叠
        renderer.drawText(resetButtonX + 10, resetButtonY + (resetButtonHeight - 16) / 2, "Reset", Color(255, 255, 255));
        
        // 绘制重置所有面颜色按钮
        int resetAllButtonWidth = buttonWidth + 20;
        int resetAllButtonHeight = buttonHeight;
        int resetAllButtonX = buttonsStartPosX + 2 * buttonWidth + 2 * buttonSpacing;
        int resetAllButtonY = buttonAreaY;
        
        renderer.drawRect(resetAllButtonX, resetAllButtonY, resetAllButtonWidth, resetAllButtonHeight, Color(150, 100, 0, 200));
        renderer.drawRectOutline(resetAllButtonX, resetAllButtonY, resetAllButtonWidth, resetAllButtonHeight, Color(200, 150, 50));
        // 调整文字位置，确保居中且不重叠
        renderer.drawText(resetAllButtonX + 10, resetAllButtonY + (resetAllButtonHeight - 16) / 2, "Reset All", Color(255, 255, 255));
        
        // 绘制关闭按钮
        int closeButtonWidth = 20;
        int closeButtonHeight = 20;
        int closeButtonX = startX + editorWidth - closeButtonWidth - 10;
        int closeButtonY = startY + 10;
        
        renderer.drawRect(closeButtonX, closeButtonY, closeButtonWidth, closeButtonHeight, Color(150, 0, 0, 200));
        renderer.drawRectOutline(closeButtonX, closeButtonY, closeButtonWidth, closeButtonHeight, Color(200, 100, 100));
        renderer.drawText(closeButtonX + 6, closeButtonY + 2, "X", Color(255, 255, 255));
        
        // 如果有保存提示，绘制提示文本
        if (showSavePrompt && savePromptTimer > 0) {
            int textWidth = savePromptText.length() * 8;
            int textX = (screenWidth - textWidth) / 2;
            int textY = screenHeight / 2 - 50; // 在屏幕中上方显示
            
            renderer.drawText(textX, textY, savePromptText, savePromptColor);
        }
    }
    
    // 处理方块编辑器的鼠标输入
    void handleBlockEditorMouseInput(int mouseX, int mouseY, bool mouseDown) {
        if (!showBlockEditor) return;
        
        // 计算编辑器窗口尺寸和位置
        int editorWidth = screenWidth / 2;
        int editorHeight = screenHeight / 2;
        int startX = (screenWidth - editorWidth) / 2;
        int startY = (screenHeight - editorHeight) / 2;
        
        // 计算布局参数 - 与绘制函数保持一致
        int padding = 20;
        int buttonWidth = 80;
        int buttonHeight = 30;
        int sliderWidth = 150;
        int sliderHeight = 20;
        int sliderGap = 35;
        
        // 处理关闭按钮
        int closeButtonWidth = 20;
        int closeButtonHeight = 20;
        int closeButtonX = startX + editorWidth - closeButtonWidth - 10;
        int closeButtonY = startY + 10;
        
        if (mouseX >= closeButtonX && mouseX < closeButtonX + closeButtonWidth &&
            mouseY >= closeButtonY && mouseY < closeButtonY + closeButtonHeight && mouseDown) {
            showBlockEditor = false;
            return;
        }
        
        // 计算按钮区域位置 - 与绘制函数保持一致
        int buttonAreaY = startY + editorHeight - buttonHeight - padding*2;
        int buttonSpacing = 20;
        int totalButtonsWidth = 3 * buttonWidth + 2 * buttonSpacing;
        int buttonsStartPosX = startX + (editorWidth - totalButtonsWidth) / 2;
        
        // 处理保存按钮
        int saveButtonWidth = buttonWidth;
        int saveButtonHeight = buttonHeight;
        int saveButtonX = buttonsStartPosX;
        int saveButtonY = buttonAreaY;
        
        if (mouseX >= saveButtonX && mouseX < saveButtonX + saveButtonWidth &&
            mouseY >= saveButtonY && mouseY < saveButtonY + saveButtonHeight && mouseDown) {
            // 将自定义颜色应用到模板方块
            templateChangeBlock.type = BLOCK_CHANGE_BLOCK;
            templateChangeBlock.hasCustomColors = true;
            for (int i = 0; i < FACE_COUNT; i++) {
                templateChangeBlock.setCustomColor(static_cast<Face>(i), faceColors[i]);
                savedFaceColors[i] = faceColors[i]; // 保存当前配置
            }
            
            // 更新物品栏中的方块
            for (auto& item : inventory) {
                if (item.type == ITEM_CHANGE_BLOCK) {
                    // 可以在这里添加额外的逻辑，例如对物品进行特殊标记
                }
            }
            
            // 显示保存提示
            showSavePrompt = true;
            savePromptText = "SAVED!";
            savePromptColor = Color(0, 255, 0); // 绿色
            savePromptTimer = MAX_SAVE_PROMPT_TIME;
            
            showBlockEditor = false;
            return;
        }
        
        // 处理重置颜色按钮
        int resetButtonWidth = buttonWidth;
        int resetButtonHeight = buttonHeight;
        int resetButtonX = buttonsStartPosX + buttonWidth + buttonSpacing;
        int resetButtonY = buttonAreaY;
        
        if (mouseX >= resetButtonX && mouseX < resetButtonX + resetButtonWidth &&
            mouseY >= resetButtonY && mouseY < resetButtonY + resetButtonHeight && mouseDown) {
            // 重置当前选中面的颜色为默认颜色
            faceColors[selectedBlockFace] = Color(200, 200, 200);
            return;
        }
        
        // 处理重置所有面颜色按钮
        int resetAllButtonWidth = buttonWidth + 20;
        int resetAllButtonHeight = buttonHeight;
        int resetAllButtonX = buttonsStartPosX + 2 * buttonWidth + 2 * buttonSpacing;
        int resetAllButtonY = buttonAreaY;
        
        if (mouseX >= resetAllButtonX && mouseX < resetAllButtonX + resetAllButtonWidth &&
            mouseY >= resetAllButtonY && mouseY < resetAllButtonY + resetAllButtonHeight && mouseDown) {
            // 重置所有面的颜色为默认颜色
            for (int i = 0; i < FACE_COUNT; i++) {
                faceColors[i] = Color(200, 200, 200);
            }
            return;
        }
        
        // 处理面选择按钮
        int buttonsStartX = startX + padding;
        int buttonsStartY = startY + 90;
        int buttonsPerRow = 3;
        
        for (int i = 0; i < FACE_COUNT; i++) {
            int row = i / buttonsPerRow;
            int col = i % buttonsPerRow;
            int buttonX = buttonsStartX + col * (buttonWidth + 10);
            int buttonY = buttonsStartY + row * (buttonHeight + 10);
            
            if (mouseX >= buttonX && mouseX < buttonX + buttonWidth &&
                mouseY >= buttonY && mouseY < buttonY + buttonHeight && mouseDown) {
                selectedBlockFace = static_cast<Face>(i);
                return;
            }
        }
        
        // 处理滑块
        int slidersStartX = startX + padding + 30;
        int slidersStartY = startY + 180;
        
        // 红色滑块
        if (mouseY >= slidersStartY && mouseY < slidersStartY + sliderHeight &&
            mouseX >= slidersStartX + 25 && mouseX < slidersStartX + 25 + sliderWidth && mouseDown) {
            int value = static_cast<int>((mouseX - (slidersStartX + 25)) * 255 / sliderWidth);
            faceColors[selectedBlockFace].r = static_cast<uint8_t>(std::max(0, std::min(255, value)));
            return;
        }
        
        // 绿色滑块
        if (mouseY >= slidersStartY + sliderGap && mouseY < slidersStartY + sliderGap + sliderHeight &&
            mouseX >= slidersStartX + 25 && mouseX < slidersStartX + 25 + sliderWidth && mouseDown) {
            int value = static_cast<int>((mouseX - (slidersStartX + 25)) * 255 / sliderWidth);
            faceColors[selectedBlockFace].g = static_cast<uint8_t>(std::max(0, std::min(255, value)));
            return;
        }
        
        // 蓝色滑块
        if (mouseY >= slidersStartY + sliderGap * 2 && mouseY < slidersStartY + sliderGap * 2 + sliderHeight &&
            mouseX >= slidersStartX + 25 && mouseX < slidersStartX + 25 + sliderWidth && mouseDown) {
            int value = static_cast<int>((mouseX - (slidersStartX + 25)) * 255 / sliderWidth);
            faceColors[selectedBlockFace].b = static_cast<uint8_t>(std::max(0, std::min(255, value)));
            return;
        }
        
        // 透明度滑块
        if (mouseY >= slidersStartY + sliderGap * 3 && mouseY < slidersStartY + sliderGap * 3 + sliderHeight &&
            mouseX >= slidersStartX + 25 && mouseX < slidersStartX + 25 + sliderWidth && mouseDown) {
            int value = static_cast<int>((mouseX - (slidersStartX + 25)) * 255 / sliderWidth);
            // 限制透明度最低为0，防止完全透明导致的透视问题
            faceColors[selectedBlockFace].a = static_cast<uint8_t>(std::max(0, std::min(255, value)));
            return;
        }
    }
    
    // 打开方块编辑器
    void toggleBlockEditor() {
        if (gameState != GAME_PLAYING) return; // 只在游戏进行中时可以打开
        
        if (!showBlockEditor) {
            // 开启方块编辑器
            showBlockEditor = true;
            
            // 初始化模板方块
            templateChangeBlock = Block(BLOCK_CHANGE_BLOCK);
            templateChangeBlock.hasCustomColors = true;
            
            // 初始化颜色
            for (int i = 0; i < FACE_COUNT; i++) {
                // 使用之前保存的颜色
                faceColors[i] = savedFaceColors[i];
            }
        } else {
            // 关闭方块编辑器，显示未保存提示
            showBlockEditor = false;
            showSavePrompt = true;
            savePromptText = "EXIT WITHOUT SAVED!";
            savePromptColor = Color(255, 0, 0); // 红色
            savePromptTimer = MAX_SAVE_PROMPT_TIME;
        }
    }
    
    // 获取模板方块
    Block getTemplateChangeBlock() const {
        return templateChangeBlock;
    }
    
    // 检查方块编辑器是否打开
    bool isBlockEditorOpen() const {
        return showBlockEditor;
    }
    
    // 新增方法：复制选中方块的颜色到编辑器
    void copyBlockColorsToEditor(const Block& block) {
        if (block.type != BLOCK_CHANGE_BLOCK || !block.hasCustomColors) {
            return; // 只处理自定义方块
        }
        
        // 复制方块颜色到编辑器
        for (int i = 0; i < FACE_COUNT; i++) {
            faceColors[i] = block.getCustomColor(static_cast<Face>(i));
            savedFaceColors[i] = faceColors[i]; // 直接保存配置
        }
        
        // 将自定义颜色直接应用到模板方块
        templateChangeBlock.type = BLOCK_CHANGE_BLOCK;
        templateChangeBlock.hasCustomColors = true;
        for (int i = 0; i < FACE_COUNT; i++) {
            templateChangeBlock.setCustomColor(static_cast<Face>(i), faceColors[i]);
        }
        
        // 显示保存成功提示
        showSavePrompt = true;
        savePromptText = "COPIED!";
        savePromptColor = Color(0, 255, 0); // 绿色
        savePromptTimer = MAX_SAVE_PROMPT_TIME;
    }

    // 聊天系统相关变量
    bool showChatBox = false;
    std::string chatInputText = "";
    std::vector<std::string> chatHistory;
    std::vector<bool> chatHistoryIsSystem; // 标记消息是系统消息还是用户输入
    size_t chatHistoryIndex = 0;
    int chatCursorBlinkTimer = 0;
    bool chatShowCursor = true;
    bool chatCommandMode = false;
    const size_t MAX_CHAT_HISTORY = 50;
    const size_t MAX_CHAT_INPUT_LENGTH = 255;
    std::vector<float> chatMessageTimers; // 消息显示计时器
    const float CHAT_MESSAGE_DISPLAY_TIME = 5.0f; // 消息显示时间(秒)
    const float CHAT_MESSAGE_FADE_TIME = 1.0f; // 消息淡出时间(秒)
    bool ignoreNextChar = false; // 用于忽略打开聊天框时的首个字符
    size_t chatCursorPosition = 0; // 光标在文本中的位置
    
    // 聊天系统相关方法
    void drawChatBox(Renderer& renderer) {
        // 如果聊天框关闭且没有消息，不绘制任何内容
        if (!showChatBox && chatHistory.empty()) return;
        
        // 计算聊天框大小和位置
        int boxHeight = 20; // 单行高度
        int boxWidth = screenWidth / 2; // 屏幕宽度的一半
        int boxX = 10; // 左侧边距10像素
        int boxY = screenHeight - boxHeight - 10; // 底部边距10像素
        
        if (showChatBox) {
            // 绘制半透明的聊天输入框背景
            renderer.drawRect(boxX, boxY, boxWidth, boxHeight, Color(60, 60, 60, 180));
            
            // 绘制聊天框边框
            renderer.drawRectOutline(boxX, boxY, boxWidth, boxHeight, Color(120, 120, 120, 200));
            
            // 计算显示的文本
            std::string displayText = chatInputText;
            std::string prefix = chatCommandMode ? "/" : "";
            
            // 如果文本太长，只显示最后能适应宽度的部分
            int maxChars = (boxWidth - 20) / 8; // 假设字符宽度为8像素
            if (prefix.length() + displayText.length() > maxChars) {
                displayText = displayText.substr(displayText.length() - maxChars + prefix.length());
            }
            
            // 绘制前缀（如果是命令模式）
            if (chatCommandMode) {
                renderer.drawText(boxX + 5, boxY + 5, "/", Color(255, 255, 255));
            }
            
            // 绘制输入文本 - 直接使用renderer.drawText而不是包装函数，确保字符间距正确
            renderer.drawText(boxX + 5 + (chatCommandMode ? 8 : 0), boxY + 5, displayText, Color(255, 255, 255));
            
            // 绘制光标 - 根据光标位置绘制
            if (chatShowCursor) {
                // 计算光标显示位置 - 显示当前可见文本中的光标位置
                size_t visibleCursorPosition = chatCursorPosition;
                if (displayText != chatInputText) {
                    // 如果显示的文本是截断的，调整光标位置
                    if (chatCursorPosition > displayText.length()) {
                        visibleCursorPosition = displayText.length();
                    } else {
                        // 如果光标位置在显示文本范围内，使用相对位置
                        size_t diff = chatInputText.length() - displayText.length();
                        if (chatCursorPosition >= diff) {
                            visibleCursorPosition = chatCursorPosition - diff;
                        } else {
                            visibleCursorPosition = 0;
                        }
                    }
                }
                // 根据光标位置计算x坐标
                int cursorX = boxX + 5 + (chatCommandMode ? 8 : 0) + visibleCursorPosition * 8;
                renderer.drawRect(cursorX, boxY + 3, 2, boxHeight - 6, Color(255, 255, 255, 200));
            }
        }
        
        // 绘制聊天历史记录
        if (!chatHistory.empty()) {
            // 最多显示5条最新消息
            int maxMessagesToShow = 5;
            int startIdx = chatHistory.size() > maxMessagesToShow ? chatHistory.size() - maxMessagesToShow : 0;
            
            for (size_t i = startIdx; i < chatHistory.size(); i++) {
                int msgY = boxY - (chatHistory.size() - i) * boxHeight;
                // 如果消息超出顶部，不显示
                if (msgY < 5) continue;
                
                // 如果聊天框关闭，应用淡出效果
                float alpha = 1.0f;
                if (!showChatBox && i < chatMessageTimers.size()) {
                    float remainingTime = chatMessageTimers[i];
                    
                    // 如果时间已经结束，不显示消息
                    if (remainingTime <= 0) continue;
                    
                    // 如果进入淡出阶段，计算透明度
                    if (remainingTime < CHAT_MESSAGE_FADE_TIME) {
                        alpha = remainingTime / CHAT_MESSAGE_FADE_TIME;
                    }
                }
                
                // 系统消息和用户消息使用不同的颜色
                Color textColor = chatHistoryIsSystem[i] ? 
                    Color(255, 255, 100, static_cast<uint8_t>(255 * alpha)) : // 系统消息为黄色
                    Color(255, 255, 255, static_cast<uint8_t>(255 * alpha));  // 用户消息为白色
                
                // 绘制半透明背景
                renderer.drawRect(boxX, msgY, boxWidth, boxHeight, 
                                 Color(20, 20, 20, static_cast<uint8_t>(180 * alpha)));
                
                // 绘制消息文本 - 直接使用renderer.drawText而不是包装函数
                renderer.drawText(boxX + 5, msgY + 5, chatHistory[i], textColor);
            }
        }
    }
    
    void handleChatInput(int key) {
        // 处理聊天输入
        if (!showChatBox) return;
        
        chatCursorBlinkTimer++;
        if (chatCursorBlinkTimer > 15) {
            chatShowCursor = !chatShowCursor;
            chatCursorBlinkTimer = 0;
        }
        
        switch (key) {
            case VK_ESCAPE:
                // ESC键关闭聊天框
                closeChatBox();
                break;
                
            case VK_RETURN:
                // Enter键发送消息
                if (!chatInputText.empty() || chatCommandMode) {
                    std::string finalText = chatCommandMode ? "/" + chatInputText : chatInputText;
                    
                    // 添加用户输入到历史记录
                    chatHistory.push_back(finalText);
                    chatHistoryIsSystem.push_back(false); // 标记为用户输入
                    chatMessageTimers.push_back(CHAT_MESSAGE_DISPLAY_TIME + CHAT_MESSAGE_FADE_TIME);
                    
                    if (chatHistory.size() > MAX_CHAT_HISTORY) {
                        chatHistory.erase(chatHistory.begin());
                        chatHistoryIsSystem.erase(chatHistoryIsSystem.begin());
                        chatMessageTimers.erase(chatMessageTimers.begin());
                    }
                    
                    // 处理命令或消息
                    if (chatCommandMode || (chatInputText.length() > 0 && chatInputText[0] == '/')) {
                        // 如果是命令模式或文本以/开头，执行命令
                        std::string cmdText = chatCommandMode ? chatInputText : chatInputText.substr(1);
                        executeCommand(cmdText);
                    }
                    
                    // 重置变量
                    chatInputText = "";
                    chatCursorPosition = 0;
                    chatCommandMode = false;
                    chatHistoryIndex = chatHistory.size();
                }
                // 关闭聊天框
                closeChatBox();
                break;
                
            case VK_BACK:
                // 退格键删除字符
                if (!chatInputText.empty() && chatCursorPosition > 0) {
                    // 删除光标前的字符
                    chatInputText.erase(chatCursorPosition - 1, 1);
                    chatCursorPosition--; // 光标向左移动
                    chatCursorBlinkTimer = 0;
                    chatShowCursor = true;
                } else if (chatCommandMode && chatInputText.empty()) {
                    // 如果已经是空字符串并且在命令模式，退出命令模式
                    chatCommandMode = false;
                }
                break;
                
            case VK_UP:
                // 上箭头键浏览历史用户输入消息
                navigateHistory(-1);
                break;
                
            case VK_DOWN:
                // 下箭头键浏览历史用户输入消息
                navigateHistory(1);
                break;
                
            case VK_LEFT:
                // 左方向键向左移动光标
                if (chatCursorPosition > 0) {
                    chatCursorPosition--;
                    // 重置光标闪烁计时器，确保光标可见
                    chatCursorBlinkTimer = 0;
                    chatShowCursor = true;
                }
                break;
                
            case VK_RIGHT:
                // 右方向键向右移动光标
                if (chatCursorPosition < chatInputText.length()) {
                    chatCursorPosition++;
                    // 重置光标闪烁计时器，确保光标可见
                    chatCursorBlinkTimer = 0;
                    chatShowCursor = true;
                }
                break;
                
            default:
                // 其他按键不处理
                break;
        }
    }
    
    // 导航历史记录，跳过系统消息
    void navigateHistory(int direction) {
        if (chatHistory.empty()) return;
        
        // 创建用户输入消息的索引映射
        std::vector<size_t> userInputIndices;
        for (size_t i = 0; i < chatHistory.size(); i++) {
            if (!chatHistoryIsSystem[i]) {
                userInputIndices.push_back(i);
            }
        }
        
        if (userInputIndices.empty()) return;
        
        // 找到当前历史索引在用户输入索引中的位置
        size_t currentUserInputIndex = userInputIndices.size();
        for (size_t i = 0; i < userInputIndices.size(); i++) {
            if (userInputIndices[i] >= chatHistoryIndex) {
                currentUserInputIndex = i;
                break;
            }
        }
        
        if (direction < 0) {
                            // 向上导航
                if (currentUserInputIndex > 0) {
                    currentUserInputIndex--;
                    chatHistoryIndex = userInputIndices[currentUserInputIndex];
                    chatInputText = chatHistory[chatHistoryIndex];
                    
                    // 如果是命令，去掉前缀"/"
                    if (!chatInputText.empty() && chatInputText[0] == '/') {
                        chatCommandMode = true;
                        chatInputText = chatInputText.substr(1);
                    } else {
                        chatCommandMode = false;
                    }
                    
                    // 将光标设置到文本末尾
                    chatCursorPosition = chatInputText.length();
                    chatCursorBlinkTimer = 0;
                    chatShowCursor = true;
                }
        } else {
            // 向下导航
            if (currentUserInputIndex < userInputIndices.size() - 1) {
                currentUserInputIndex++;
                chatHistoryIndex = userInputIndices[currentUserInputIndex];
                chatInputText = chatHistory[chatHistoryIndex];
                
                // 如果是命令，去掉前缀"/"
                if (!chatInputText.empty() && chatInputText[0] == '/') {
                    chatCommandMode = true;
                    chatInputText = chatInputText.substr(1);
                } else {
                    chatCommandMode = false;
                }
                
                // 将光标设置到文本末尾
                chatCursorPosition = chatInputText.length();
                chatCursorBlinkTimer = 0;
                chatShowCursor = true;
            } else {
                // 已经到底了，清空输入
                chatHistoryIndex = chatHistory.size();
                chatInputText = "";
                chatCursorPosition = 0; // 重置光标位置
                chatCommandMode = false;
                chatCursorBlinkTimer = 0;
                chatShowCursor = true;
            }
        }
    }
    
    void addCharToChat(char c) {
        if (showChatBox && chatInputText.length() < MAX_CHAT_INPUT_LENGTH) {
            // 如果需要忽略下一个字符（打开聊天框时的首个字符）
            if (ignoreNextChar) {
                ignoreNextChar = false;
                return;
            }
            
            // 在光标位置插入字符
            chatInputText.insert(chatCursorPosition, 1, c);
            chatCursorPosition++; // 光标前进一位
            
            // 重置光标闪烁计时器，确保光标可见
            chatCursorBlinkTimer = 0;
            chatShowCursor = true;
        }
    }
    
    void openChatBox(bool asCommand = false) {
        showChatBox = true;
        chatCommandMode = asCommand;
        chatInputText = "";
        chatCursorPosition = 0; // 重置光标位置
        chatCursorBlinkTimer = 0;
        chatShowCursor = true;
        chatHistoryIndex = chatHistory.size();
        ignoreNextChar = true; // 忽略打开聊天框时的首个字符
        
        // 解锁鼠标，允许自由移动
        ShowCursor(TRUE); // 显示鼠标指针
    }
    
    void closeChatBox() {
        showChatBox = false;
        chatCommandMode = false;
        chatCursorBlinkTimer = 0;
        
        // 如果回到游戏状态，重新锁定鼠标到准心
        if (gameState == GAME_PLAYING) {
            lockMouseToCrosshair();
        }
    }
    
    void executeCommand(const std::string& command) {
        // 处理命令
        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;
        
        if (cmd.empty()) {
            // 空命令
            addSystemMessage("Invalid command: Empty command");
            return;
        }
        
        // 转换为小写以实现不区分大小写的命令
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
        
        // 命令处理
        if (cmd == "setblock") {
            executeSetBlockCommand(iss);
        } else if (cmd == "tp") {
            executeTpCommand(iss);
        } else if (cmd == "music") {
            executeMusicCommand(iss);
        } else if (cmd == "help") {
            executeHelpCommand();
        } else if (cmd == "fill") {
            executeFillCommand(iss);
        } else {
            // 未知命令
            addSystemMessage("Unknown command: /" + cmd);
        }
    }
    
    // 执行help命令，显示所有可用命令及其用法
    void executeHelpCommand() {
        addSystemMessage("===== Available Commands =====");
        addSystemMessage("/help - Show all available commands and their usage");
        addSystemMessage("/setblock <x> <y> <z> <Block types> - Set block at specified coordinates");
        addSystemMessage("/tp <x> <y> <z> - Teleport to specified coordinates");
        addSystemMessage("/music <Music name|stop> - Play or stop background music");
        addSystemMessage("/fill <x1> <y1> <z1> <x2> <y2> <z2> <Block typs> - Fill blocks in the specified area");
        addSystemMessage("===========================");
    }
    
    // 执行fill命令，填充指定区域的方块
    void executeFillCommand(std::istringstream& args) {
        std::string x1Str, y1Str, z1Str, x2Str, y2Str, z2Str;
        std::string blockName;
        
        // 读取六个坐标参数和方块类型
        if (!(args >> x1Str >> y1Str >> z1Str >> x2Str >> y2Str >> z2Str >> blockName)) {
            addSystemMessage("Usage: /fill x1 y1 z1 x2 y2 z2 blocktype");
            return;
        }
        
        // 解析坐标，支持相对坐标(~)
        int x1, y1, z1, x2, y2, z2;
        
        // 解析第一组坐标(x1, y1, z1)
        if (!parseCoordinate(x1Str, playerPosition.x, x1) || 
            !parseCoordinate(y1Str, playerPosition.y, y1) || 
            !parseCoordinate(z1Str, playerPosition.z, z1)) {
            addSystemMessage("Invalid coordinates for first point");
            return;
        }
        
        // 解析第二组坐标(x2, y2, z2)
        if (!parseCoordinate(x2Str, playerPosition.x, x2) || 
            !parseCoordinate(y2Str, playerPosition.y, y2) || 
            !parseCoordinate(z2Str, playerPosition.z, z2)) {
            addSystemMessage("Invalid coordinates for second point");
            return;
        }
        
        // 确保坐标是按升序排列的（x1 <= x2, y1 <= y2, z1 <= z2）
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        if (z1 > z2) std::swap(z1, z2);
        
        // 查找对应的方块类型
        BlockType blockType = BLOCK_AIR;
        bool found = false;
        
        // 将方块名称转换为小写
        std::transform(blockName.begin(), blockName.end(), blockName.begin(), ::tolower);
        
        // 匹配方块类型（复用setblock命令的逻辑）
        if (blockName == "dirt") blockType = BLOCK_DIRT, found = true;
        else if (blockName == "grass") blockType = BLOCK_GRASS, found = true;
        else if (blockName == "stone") blockType = BLOCK_STONE, found = true;
        else if (blockName == "sand") blockType = BLOCK_SAND, found = true;
        else if (blockName == "wood") blockType = BLOCK_WOOD, found = true;
        else if (blockName == "leaves") blockType = BLOCK_LEAVES, found = true;
        else if (blockName == "coal" || blockName == "coal_ore") blockType = BLOCK_COAL_ORE, found = true;
        else if (blockName == "iron" || blockName == "iron_ore") blockType = BLOCK_IRON_ORE, found = true;
        else if (blockName == "gold" || blockName == "gold_ore") blockType = BLOCK_GOLD_ORE, found = true;
        else if (blockName == "diamond" || blockName == "diamond_ore") blockType = BLOCK_DIAMOND_ORE, found = true;
        else if (blockName == "bedrock") blockType = BLOCK_BEDROCK, found = true;
        else if (blockName == "obsidian") blockType = BLOCK_OBSIDIAN, found = true;
        else if (blockName == "lava") blockType = BLOCK_LAVA, found = true;
        else if (blockName == "redstone" || blockName == "redstone_ore") blockType = BLOCK_REDSTONE_ORE, found = true;
        else if (blockName == "emerald" || blockName == "emerald_ore") blockType = BLOCK_EMERALD_ORE, found = true;
        else if (blockName == "mossy_stone") blockType = BLOCK_MOSSY_STONE, found = true;
        else if (blockName == "gravel") blockType = BLOCK_GRAVEL, found = true;
        else if (blockName == "clay") blockType = BLOCK_CLAY, found = true;
        else if (blockName == "ice") blockType = BLOCK_ICE, found = true;
        else if (blockName == "snow") blockType = BLOCK_SNOW, found = true;
        else if (blockName == "water") blockType = BLOCK_WATER, found = true;
        else if (blockName == "sandstone") blockType = BLOCK_SANDSTONE, found = true;
        else if (blockName == "cactus") blockType = BLOCK_CACTUS, found = true;
        else if (blockName == "pumpkin") blockType = BLOCK_PUMPKIN, found = true;
        else if (blockName == "netherrack") blockType = BLOCK_NETHERRACK, found = true;
        else if (blockName == "soul_sand") blockType = BLOCK_SOUL_SAND, found = true;
        else if (blockName == "quartz") blockType = BLOCK_QUARTZ, found = true;
        else if (blockName == "glowstone") blockType = BLOCK_GLOWSTONE, found = true;
        else if (blockName == "mycelium") blockType = BLOCK_MYCELIUM, found = true;
        else if (blockName == "end_stone") blockType = BLOCK_END_STONE, found = true;
        else if (blockName == "prismarine") blockType = BLOCK_PRISMARINE, found = true;
        else if (blockName == "magma") blockType = BLOCK_MAGMA, found = true;
        else if (blockName == "nether_wart") blockType = BLOCK_NETHER_WART, found = true;
        else if (blockName == "slime") blockType = BLOCK_SLIME, found = true;
        else if (blockName == "brick") blockType = BLOCK_BRICK, found = true;
        else if (blockName == "bookshelf") blockType = BLOCK_BOOKSHELF, found = true;
        else if (blockName == "air") blockType = BLOCK_AIR, found = true;
        
        if (!found) {
            addSystemMessage("Unknown block type: " + blockName);
            return;
        }
        
        // 计算要填充的方块数量
        int blockCount = (x2 - x1 + 1) * (y2 - y1 + 1) * (z2 - z1 + 1);
        
        // 检查数量是否过大
        const int MAX_FILL_SIZE = 32768; // 最大填充数量限制
        if (blockCount > MAX_FILL_SIZE) {
            addSystemMessage("The area is too large, the maximum number of blocks that can be filled is " + std::to_string(MAX_FILL_SIZE));
            return;
        }
        
        // 设置填充区域的信息
        cmdFillX1 = x1;
        cmdFillY1 = y1;
        cmdFillZ1 = z1;
        cmdFillX2 = x2;
        cmdFillY2 = y2;
        cmdFillZ2 = z2;
        cmdFillBlockType = blockType;
        hasPendingFillCommand = true;
        
        addSystemMessage("Filling region from (" + 
                          std::to_string(x1) + "," + std::to_string(y1) + "," + std::to_string(z1) + ") to (" + 
                          std::to_string(x2) + "," + std::to_string(y2) + "," + std::to_string(z2) + ") with " + 
                          blockName + " (" + std::to_string(blockCount) + " blocks)");
    }
    
    // 解析坐标字符串（支持相对坐标~）
    bool parseCoordinate(const std::string& coordStr, float playerCoord, int& result) {
        if (coordStr[0] == '~') {
            // 相对坐标
            int offset = 0;
            if (coordStr.length() > 1) {
                try {
                    offset = std::stoi(coordStr.substr(1));
                } catch (...) {
                    return false;
                }
            }
            // 使用玩家当前坐标加上偏移量
            result = static_cast<int>(playerCoord) + offset;
            return true;
        } else {
            // 绝对坐标
            try {
                result = std::stoi(coordStr);
                return true;
            } catch (...) {
                return false;
            }
        }
    }
    
    void executeSetBlockCommand(std::istringstream& args) {
        std::string xStr, yStr, zStr;
        std::string blockName;
        
        // 读取三个坐标参数和方块类型
        if (!(args >> xStr >> yStr >> zStr >> blockName)) {
            addSystemMessage("Usage: /setblock x y z blocktype");
            return;
        }
        
        // 解析坐标，支持相对坐标(~)
        int x, y, z;
        
        // 解析X坐标
        if (xStr[0] == '~') {
            // 相对坐标
            int xOffset = 0;
            if (xStr.length() > 1) {
                try {
                    xOffset = std::stoi(xStr.substr(1));
                } catch (...) {
                    addSystemMessage("Invalid X coordinate: " + xStr);
                    return;
                }
            }
            // 使用玩家当前X坐标加上偏移量
            x = static_cast<int>(playerPosition.x) + xOffset;
        } else {
            // 绝对坐标
            try {
                x = std::stoi(xStr);
            } catch (...) {
                addSystemMessage("Invalid X coordinate: " + xStr);
                return;
            }
        }
        
        // 解析Y坐标
        if (yStr[0] == '~') {
            // 相对坐标
            int yOffset = 0;
            if (yStr.length() > 1) {
                try {
                    yOffset = std::stoi(yStr.substr(1));
                } catch (...) {
                    addSystemMessage("Invalid Y coordinate: " + yStr);
                    return;
                }
            }
            // 使用玩家当前Y坐标加上偏移量
            y = static_cast<int>(playerPosition.y) + yOffset;
        } else {
            // 绝对坐标
            try {
                y = std::stoi(yStr);
            } catch (...) {
                addSystemMessage("Invalid Y coordinate: " + yStr);
                return;
            }
        }
        
        // 解析Z坐标
        if (zStr[0] == '~') {
            // 相对坐标
            int zOffset = 0;
            if (zStr.length() > 1) {
                try {
                    zOffset = std::stoi(zStr.substr(1));
                } catch (...) {
                    addSystemMessage("Invalid Z coordinate: " + zStr);
                    return;
                }
            }
            // 使用玩家当前Z坐标加上偏移量
            z = static_cast<int>(playerPosition.z) + zOffset;
        } else {
            // 绝对坐标
            try {
                z = std::stoi(zStr);
            } catch (...) {
                addSystemMessage("Invalid Z coordinate: " + zStr);
                return;
            }
        }
        
        // 查找对应的方块类型
        BlockType blockType = BLOCK_AIR;
        bool found = false;
        
        // 将方块名称转换为小写
        std::transform(blockName.begin(), blockName.end(), blockName.begin(), ::tolower);
        
        // 匹配方块类型
        if (blockName == "dirt") blockType = BLOCK_DIRT, found = true;
        else if (blockName == "grass") blockType = BLOCK_GRASS, found = true;
        else if (blockName == "stone") blockType = BLOCK_STONE, found = true;
        else if (blockName == "sand") blockType = BLOCK_SAND, found = true;
        else if (blockName == "wood") blockType = BLOCK_WOOD, found = true;
        else if (blockName == "leaves") blockType = BLOCK_LEAVES, found = true;
        else if (blockName == "coal" || blockName == "coal_ore") blockType = BLOCK_COAL_ORE, found = true;
        else if (blockName == "iron" || blockName == "iron_ore") blockType = BLOCK_IRON_ORE, found = true;
        else if (blockName == "gold" || blockName == "gold_ore") blockType = BLOCK_GOLD_ORE, found = true;
        else if (blockName == "diamond" || blockName == "diamond_ore") blockType = BLOCK_DIAMOND_ORE, found = true;
        else if (blockName == "bedrock") blockType = BLOCK_BEDROCK, found = true;
        else if (blockName == "obsidian") blockType = BLOCK_OBSIDIAN, found = true;
        else if (blockName == "lava") blockType = BLOCK_LAVA, found = true;
        else if (blockName == "redstone" || blockName == "redstone_ore") blockType = BLOCK_REDSTONE_ORE, found = true;
        else if (blockName == "emerald" || blockName == "emerald_ore") blockType = BLOCK_EMERALD_ORE, found = true;
        else if (blockName == "mossy_stone") blockType = BLOCK_MOSSY_STONE, found = true;
        else if (blockName == "gravel") blockType = BLOCK_GRAVEL, found = true;
        else if (blockName == "clay") blockType = BLOCK_CLAY, found = true;
        else if (blockName == "ice") blockType = BLOCK_ICE, found = true;
        else if (blockName == "snow") blockType = BLOCK_SNOW, found = true;
        else if (blockName == "water") blockType = BLOCK_WATER, found = true;
        else if (blockName == "sandstone") blockType = BLOCK_SANDSTONE, found = true;
        else if (blockName == "cactus") blockType = BLOCK_CACTUS, found = true;
        else if (blockName == "pumpkin") blockType = BLOCK_PUMPKIN, found = true;
        else if (blockName == "netherrack") blockType = BLOCK_NETHERRACK, found = true;
        else if (blockName == "soul_sand") blockType = BLOCK_SOUL_SAND, found = true;
        else if (blockName == "quartz") blockType = BLOCK_QUARTZ, found = true;
        else if (blockName == "glowstone") blockType = BLOCK_GLOWSTONE, found = true;
        else if (blockName == "mycelium") blockType = BLOCK_MYCELIUM, found = true;
        else if (blockName == "end_stone") blockType = BLOCK_END_STONE, found = true;
        else if (blockName == "prismarine") blockType = BLOCK_PRISMARINE, found = true;
        else if (blockName == "magma") blockType = BLOCK_MAGMA, found = true;
        else if (blockName == "nether_wart") blockType = BLOCK_NETHER_WART, found = true;
        else if (blockName == "slime") blockType = BLOCK_SLIME, found = true;
        else if (blockName == "brick") blockType = BLOCK_BRICK, found = true;
        else if (blockName == "bookshelf") blockType = BLOCK_BOOKSHELF, found = true;
        else if (blockName == "air") blockType = BLOCK_AIR, found = true;
        
        if (!found) {
            addSystemMessage("Unknown block type: " + blockName);
            return;
        }
        
        // 设置方块，调用世界对象的setBlock函数
        // 由于UI管理器无法直接访问世界对象，我们将设置一个标志，让主循环处理
        // 这里我们用一个公共变量来存储命令信息
        cmdSetBlockX = x;
        cmdSetBlockY = y;
        cmdSetBlockZ = z;
        cmdSetBlockType = blockType;
        hasPendingSetBlockCommand = true;
        
        addSystemMessage("Set block at (" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ") to " + blockName);
    }
    
    void executeTpCommand(std::istringstream& args) {
        std::string xStr, yStr, zStr;
        
        // 读取三个坐标参数
        if (!(args >> xStr >> yStr >> zStr)) {
            addSystemMessage("Usage: /tp x y z");
            return;
        }
        
        // 解析坐标，支持相对坐标(~)
        float x, y, z;
        
        // 解析X坐标
        if (xStr[0] == '~') {
            // 相对坐标
            float xOffset = 0;
            if (xStr.length() > 1) {
                try {
                    xOffset = std::stof(xStr.substr(1));
                } catch (...) {
                    addSystemMessage("Invalid X coordinate: " + xStr);
                    return;
                }
            }
            // 使用玩家当前X坐标加上偏移量
            x = playerPosition.x + xOffset;
        } else {
            // 绝对坐标
            try {
                x = std::stof(xStr);
            } catch (...) {
                addSystemMessage("Invalid X coordinate: " + xStr);
                return;
            }
        }
        
        // 解析Y坐标
        if (yStr[0] == '~') {
            // 相对坐标
            float yOffset = 0;
            if (yStr.length() > 1) {
                try {
                    yOffset = std::stof(yStr.substr(1));
                } catch (...) {
                    addSystemMessage("Invalid Y coordinate: " + yStr);
                    return;
                }
            }
            // 使用玩家当前Y坐标加上偏移量
            y = playerPosition.y + yOffset;
        } else {
            // 绝对坐标
            try {
                y = std::stof(yStr);
            } catch (...) {
                addSystemMessage("Invalid Y coordinate: " + yStr);
                return;
            }
        }
        
        // 解析Z坐标
        if (zStr[0] == '~') {
            // 相对坐标
            float zOffset = 0;
            if (zStr.length() > 1) {
                try {
                    zOffset = std::stof(zStr.substr(1));
                } catch (...) {
                    addSystemMessage("Invalid Z coordinate: " + zStr);
                    return;
                }
            }
            // 使用玩家当前Z坐标加上偏移量
            z = playerPosition.z + zOffset;
        } else {
            // 绝对坐标
            try {
                z = std::stof(zStr);
            } catch (...) {
                addSystemMessage("Invalid Z coordinate: " + zStr);
                return;
            }
        }
        
        // 设置传送命令
        cmdTeleportX = x;
        cmdTeleportY = y;
        cmdTeleportZ = z;
        hasPendingTeleportCommand = true;
        
        addSystemMessage("Teleported to (" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")");
    }
    
    void executeMusicCommand(std::istringstream& args) {
        std::string path;
        
        // 检查是否是停止命令
        std::string command;
        args >> command;
        
        if (command == "stop") {
            // 停止当前音乐
            if (backgroundMusic) {
                // 记录是否正在准备播放
                bool wasPreparing = isPreparingCustomMusic;
                
                // 停止当前音乐
                stopMusic();
                addSystemMessage("Music stopped.");
                
                // 如果不是在准备状态，才重置标志和设置随机延迟
                if (!wasPreparing) {
                    // 重置随机音乐计时器，让游戏自带音乐可以在延迟后播放
                    lastMusicPlayTime = std::chrono::steady_clock::now();
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> distr(120, 300);
                    randomMusicDelay = distr(gen);
                    
                    std::cout << "[Music] Music stopped, setting new delay of " << randomMusicDelay << " seconds" << std::endl;
                } else {
                    std::cout << "[Music] Music stopped during preparation, maintaining preparation state" << std::endl;
                }
            } else {
                addSystemMessage("No music is currently playing.");
            }
            return;
        }
        
        // 如果不是停止命令，则是播放命令，需要获取路径
        if (command.front() == '\"' && command.back() == '\"') {
            // 如果路径被引号包围，去掉引号
            path = command.substr(1, command.length() - 2);
        } else {
            // 否则直接使用命令作为路径
            path = command;
        }
        
        if (path.empty()) {
            addSystemMessage("Usage: /music \"path/to/music.txt\" or /music stop");
            return;
        }
        
        // 设置音乐播放命令
        cmdMusicPath = path;
        hasPendingMusicCommand = true;
        
        addSystemMessage("Playing music from: " + path);
    }
    
    void addSystemMessage(const std::string& message) {
        chatHistory.push_back(message);
        chatHistoryIsSystem.push_back(true); // 标记为系统消息
        chatMessageTimers.push_back(CHAT_MESSAGE_DISPLAY_TIME + CHAT_MESSAGE_FADE_TIME);
        
        if (chatHistory.size() > MAX_CHAT_HISTORY) {
            chatHistory.erase(chatHistory.begin());
            chatHistoryIsSystem.erase(chatHistoryIsSystem.begin());
            chatMessageTimers.erase(chatMessageTimers.begin());
        }
        chatHistoryIndex = chatHistory.size();
    }
    
    void updateChatMessageTimers(float deltaTime) {
        if (!showChatBox && !chatMessageTimers.empty()) {
            for (size_t i = 0; i < chatMessageTimers.size(); i++) {
                if (chatMessageTimers[i] > 0) {
                    chatMessageTimers[i] -= deltaTime;
                }
            }
        }
    }

    // 命令执行标志和参数 - 公开给主循环使用
    bool hasPendingSetBlockCommand = false;
    int cmdSetBlockX = 0, cmdSetBlockY = 0, cmdSetBlockZ = 0;
    BlockType cmdSetBlockType = BLOCK_AIR;
    
    bool hasPendingTeleportCommand = false;
    float cmdTeleportX = 0.0f, cmdTeleportY = 0.0f, cmdTeleportZ = 0.0f;
    
    bool hasPendingMusicCommand = false;
    std::string cmdMusicPath = "";
    
    // fill命令相关变量
    bool hasPendingFillCommand = false;
    int cmdFillX1 = 0, cmdFillY1 = 0, cmdFillZ1 = 0;
    int cmdFillX2 = 0, cmdFillY2 = 0, cmdFillZ2 = 0;
    BlockType cmdFillBlockType = BLOCK_AIR;
    
    // 聊天系统公开接口
    bool isChatBoxOpen() const {
        return showChatBox;
    }
    
    void toggleChatBox() {
        if (showChatBox) {
            closeChatBox();
        } else {
            openChatBox();
        }
    }
    
    void openCommandInput() {
        openChatBox(true);
    }
    
    void handleChatKeyInput(int key) {
        handleChatInput(key);
    }
    
    void addChatCharacter(char c) {
        addCharToChat(c);
    }
    
    void resetPendingCommands() {
        hasPendingSetBlockCommand = false;
        hasPendingTeleportCommand = false;
        hasPendingMusicCommand = false;
        hasPendingFillCommand = false;
    }
    
    // 添加播放自定义音乐的公共方法
    bool playCustomMusic(const std::string& filePath) {
        if (!musicEnabled) {
            addSystemMessage("Music is disabled in options. Cannot play custom music.");
            return false;
        }
        
        // 检查是否有音乐正在播放
        bool musicWasPlaying = false;
        if (backgroundMusic && !backgroundMusic->isFinished()) {
            musicWasPlaying = true;
            // 停止当前音乐
            stopMusic();
            std::cout << "[Music] Stopping current music before playing new custom music." << std::endl;
        }
        
        // 尝试加载自定义音乐文件
        std::string musicContent;
        bool hasEndMarker = false;
        
        // 先读取文件内容并保存，以便延迟播放时使用
        {
            std::ifstream file(filePath);
            if (file.is_open()) {
                // 读取文件内容并检查是否包含"/"结束标记
                std::stringstream buffer;
                buffer << file.rdbuf();
                musicContent = buffer.str();
                if (musicContent.find('/') != std::string::npos) {
                    hasEndMarker = true;
                }
                file.close();
            } else {
                std::cout << "[Music] Failed to open music file: " << filePath << std::endl;
                return false;
            }
        }
        
        // 确保背景音乐对象存在
        if (!backgroundMusic) {
            // 如果背景音乐对象不存在，创建一个新的
            int volume = static_cast<int>(musicVolumeValue * 127);
            backgroundMusic = std::make_unique<BGM>("", volume);
            musicInitialized = true;
        }
        
        // 设置为自定义音乐模式，阻止游戏内置音乐播放
        isPlayingCustomMusic = true;
        isMusicActuallyStopped = false; // 设置为非停止状态
        
        // 暂停游戏内音乐的随机计时器
        shouldPlayMusic = false;
        
        if (musicWasPlaying) {
            // 如果之前有音乐播放，延迟3秒播放新音乐
            addSystemMessage("Music will start in 3 seconds...");
            std::cout << "[Music] Delaying playback for 3 seconds..." << std::endl;
            
            // 设置准备标记为true
            isPreparingCustomMusic = true;
            
            // 创建并执行延迟播放的线程
            std::thread([this, filePath, hasEndMarker, musicContent]() {
                // 延迟3秒
                std::cout << "[Music] Delay thread started, waiting 3 seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
                std::cout << "[Music] 3-second delay completed, preparing to play music..." << std::endl;
                
                // 确保音乐对象仍然存在并且仍处于自定义音乐模式
                if (!backgroundMusic) {
                    std::cerr << "[Music] Error: backgroundMusic is null after delay!" << std::endl;
                    isPreparingCustomMusic = false;
                    return;
                }
                
                if (!isPlayingCustomMusic) {
                    std::cout << "[Music] Custom music mode was disabled during delay, aborting playback" << std::endl;
                    isPreparingCustomMusic = false;
                    return;
                }
                
                try {
                    // 设置音乐内容
                    if (hasEndMarker) {
                        std::cout << "[Music] Setting music content with end marker..." << std::endl;
                        backgroundMusic->setMusicFromString(musicContent, filePath);
                    } else {
                        std::cout << "[Music] Setting music content without end marker, adding one..." << std::endl;
                        backgroundMusic->setMusicFromString(musicContent + "\n/", filePath);
                    }
                    
                    // 播放音乐
                    std::cout << "[Music] Starting delayed playback..." << std::endl;
                    backgroundMusic->play();
                    std::cout << "[Music] Custom music started playing after delay" << std::endl;
                    
                    // 设置非停止状态
                    isMusicActuallyStopped = false;
                    
                    // 重置准备标记
                    isPreparingCustomMusic = false;
                } catch (const std::exception& e) {
                    std::cerr << "[Music] Failed to play custom music after delay: " << e.what() << std::endl;
                    isPreparingCustomMusic = false;
                    isMusicActuallyStopped = true; // 确保设置为停止状态
                }
            }).detach();
            
            return true;
        } else {
            // 如果之前没有音乐播放，直接播放新音乐
            try {
                // 设置音乐内容
                if (hasEndMarker) {
                    backgroundMusic->setMusicFromString(musicContent, filePath);
                } else {
                    backgroundMusic->setMusicFromString(musicContent + "\n/", filePath);
                }
                
                // 播放音乐
                std::cout << "[Music] Starting custom music playback..." << std::endl;
                backgroundMusic->play();
                std::cout << "[Music] Custom music started playing" << std::endl;
                
                // 设置非停止状态
                isMusicActuallyStopped = false;
                
                return true;
            } catch (const std::exception& e) {
                std::cerr << "[Music] Failed to play custom music: " << e.what() << std::endl;
                isPlayingCustomMusic = false;
                isMusicActuallyStopped = true; // 确保设置为停止状态
                return false;
            }
        }
    }

    // 玩家当前位置，用于相对坐标命令
    Vec3 playerPosition = Vec3(0, 0, 0);

    // 更新玩家位置，用于相对坐标命令
    void updatePlayerPosition(const Vec3& position) {
        playerPosition = position;
    }
};

#endif // UI_MANAGER_H
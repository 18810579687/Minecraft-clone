#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <iostream>
#include "math3d.h"
#include "camera.h"
#include "renderer.h"
#include "world.h"
#include "physics.h"
#include "ui_manager.h"

// 全局变量
int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;
HDC hdc;
HBITMAP hBitmap;
HDC hdcMem;
UIManager* uiManager = nullptr;
void* pBits;
BITMAPINFO bmi = {};
Renderer renderer;
World world;
Camera camera;
Physics physics; // 物理引擎
bool keys[256] = {false};
bool running = true;
float deltaTime = 0.0f;
bool mouseLocked = true; // 鼠标锁定状态
HWND mainWindow; // 主窗口句柄

// 前向声明函数
void resizeBitmap(HWND hwnd);
bool initBitmap(HWND hwnd);

// 锁定或解锁鼠标
void toggleMouseLock(HWND hwnd) {
    mouseLocked = !mouseLocked;
    
    if (mouseLocked) {
        // 隐藏鼠标光标
        while (ShowCursor(FALSE) >= 0) {} // 确保鼠标隐藏
        
        // 获取客户区中心点
        RECT rect;
        GetClientRect(hwnd, &rect);
        POINT center = {rect.right / 2, rect.bottom / 2};
        ClientToScreen(hwnd, &center);
        
        // 将鼠标移动到中心
        SetCursorPos(center.x, center.y);
        
        // 限制鼠标在窗口内
        GetClientRect(hwnd, &rect);
        ClientToScreen(hwnd, (POINT*)&rect.left);
        ClientToScreen(hwnd, (POINT*)&rect.right);
        ClipCursor(&rect);
    } else {
        // 显示鼠标光标
        while (ShowCursor(TRUE) < 0) {} // 确保鼠标显示
        
        // 解除鼠标限制
        ClipCursor(NULL);
    }
}

// 更新鼠标指针显示状态
void updateMouseVisibility(HWND hwnd) {
    if (uiManager) {
        GameState gameState = uiManager->getGameState();
        
        // 在游戏进行中且鼠标锁定时隐藏鼠标指针
        // 在菜单和物品栏界面始终显示鼠标指针
        if (gameState == GAME_PLAYING && mouseLocked) {
            while (ShowCursor(FALSE) >= 0) {} // 确保鼠标隐藏
        } else {
            while (ShowCursor(TRUE) < 0) {} // 确保鼠标显示
        }
    }
}

// 处理窗口消息
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            running = false;
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            // 如果聊天框打开，处理特殊按键输入
            if (uiManager && uiManager->isChatBoxOpen()) {
                uiManager->handleChatKeyInput(wParam);
                // 聊天框打开时不记录按键状态，防止触发游戏操作
                return 0;
            }
            
            // 记录按键状态，以便于processInput使用
            keys[wParam] = true;
            
            // 处理T键打开聊天框
            if (wParam == 'T') {
                if (uiManager && !uiManager->isChatBoxOpen() && uiManager->getGameState() == GAME_PLAYING) {
                    uiManager->toggleChatBox();
                    return 0;
                }
            }
            
            // 处理/键直接打开命令输入
            if (wParam == VK_OEM_2 || wParam == 191) { // VK_OEM_2 is '/' on US keyboards, 191 is the scancode for '/'
                if (uiManager && !uiManager->isChatBoxOpen() && uiManager->getGameState() == GAME_PLAYING) {
                    uiManager->openCommandInput();
                    return 0;
                }
            }
            
            // 处理ESC键 - 打开/关闭菜单或关闭物品栏
            if (wParam == VK_ESCAPE) {
                if (uiManager) {
                    // 使用UIManager的统一处理方法
                    uiManager->handleEscKeyPress();
                    
                    // 根据游戏状态切换鼠标锁定
                    if (uiManager->getGameState() == GAME_PLAYING) {
                        if (!mouseLocked) toggleMouseLock(mainWindow);
                    } else {
                        if (mouseLocked) toggleMouseLock(mainWindow);
                    }
                    
                    // 更新鼠标指针显示状态
                    updateMouseVisibility(hwnd);
                }
                keys[VK_ESCAPE] = false; // 防止被processInput处理
            }
            
            // 处理E键 - 打开/关闭物品栏
            if (wParam == 'E') {
                if (uiManager) {
                    // 如果当前在物品栏界面，关闭物品栏
                    if (uiManager->getGameState() == GAME_INVENTORY) {
                        uiManager->toggleInventory();
                        
                        // 重新锁定鼠标
                        if (!mouseLocked) toggleMouseLock(mainWindow);
                        
                        // 更新鼠标指针显示状态
                        updateMouseVisibility(hwnd);
                        
                        keys['E'] = false; // 防止被processInput处理
                        return 0;
                    }
                }
            }
            
            // 处理R键 - 回到出生点
            if (wParam == 'R' && uiManager && uiManager->getGameState() == GAME_PLAYING) {
                // 获取出生点坐标
                Vec3 spawnPoint = world.getSpawnPoint();
                // 传送玩家到出生点
                camera.position = spawnPoint;
                std::cout << "Teleported to spawn point: X=" << (int)spawnPoint.x << ", Y=" << (int)spawnPoint.y << ", Z=" << (int)spawnPoint.z << std::endl;
                keys['R'] = false; // 防止被processInput处理
            }
            
            // 处理X键 - 切换X-ray模式
            if (wParam == 'X' && uiManager && uiManager->getGameState() == GAME_PLAYING) {
                world.toggleXrayMode();
                keys['X'] = false; // 防止被processInput处理
            }
            
            // 处理F3键 - 显示/隐藏调试信息
            if (wParam == VK_F3) {
                if (uiManager) {
                    uiManager->toggleDebugInfo();
                    // 只在调试信息已启用时在控制台输出调试信息
                    Vec3 playerPos = camera.position;
                    std::cout << "Player position: X=" << playerPos.x << " Y=" << playerPos.y << " Z=" << playerPos.z << std::endl;
                    std::cout << "Look: Pitch=" << camera.pitch << " Yaw=" << camera.yaw << std::endl;
                    std::cout << "FOV: " << (60.0f + uiManager->getFOVValue() * 60.0f) << std::endl;
                    std::cout << "Walk speed: " << uiManager->getActualWalkSpeed() << std::endl;
                    std::cout << "Fly speed: " << uiManager->getActualFlySpeed() << std::endl;
                    std::cout << "World size: " << world.getWidth() << "x" << world.getHeight() << "x" << world.getDepth() << std::endl;
                    std::cout << "World seed: " << world.getSeed() << std::endl;
                }
                keys[VK_F3] = false; // 防止被processInput处理
            }
            
            // 处理F1键 - 切换UI显示
            if (wParam == VK_F1) {
                if (uiManager) {
                    uiManager->toggleUI();
                }
                keys[VK_F1] = false; // 防止被processInput处理
            }
            
            // 处理O键 - 打开方块编辑器（仅在游戏进行中有效）
            if (wParam == 'O' && uiManager) {
                if (uiManager->getGameState() == GAME_PLAYING) {
                    bool willOpenEditor = !uiManager->isBlockEditorOpen(); // 检查是否将要打开编辑器
                    
                    uiManager->toggleBlockEditor();
                    
                    // 如果打开编辑器，解锁鼠标以便操作编辑器界面
                    if (willOpenEditor && mouseLocked) {
                        toggleMouseLock(mainWindow); // 解锁鼠标
                    } 
                    // 如果关闭编辑器，锁定鼠标
                    else if (!willOpenEditor && !mouseLocked) {
                        toggleMouseLock(mainWindow); // 锁定鼠标
                    }
                    
                    // 更新鼠标指针显示状态
                    updateMouseVisibility(hwnd);
                }
                keys['O'] = false; // 防止被processInput处理
            }
            
            // 处理数字键0-9选择物品栏或替换物品
            if ((wParam >= '0' && wParam <= '9')) {
                if (uiManager) {
                    // 如果在物品栏界面，处理替换逻辑
                    if (uiManager->getGameState() == GAME_INVENTORY) {
                        uiManager->handleNumberKeyInput(wParam);
                    } else if (wParam >= '1' && wParam <= '9') {
                        // 直接选择对应的物品栏槽位（0-8）
                        int slot = wParam - '1';
                        // 直接设置选中的槽位
                        uiManager->selectSlot(slot);
                    }
                }
            }
            
            // 处理菜单导航键
            if (uiManager && (uiManager->getGameState() == GAME_PAUSED)) {
                if (wParam == VK_UP) {
                    uiManager->selectPrevMenuOption();
                } else if (wParam == VK_DOWN) {
                    uiManager->selectNextMenuOption();
                } else if (wParam == VK_RETURN) {
                    int result = uiManager->executeSelectedMenuOption();
                    if (result == 0) {
                        running = false; // 退出游戏
                    } else if (result == 2) {
                        // 保存当前玩家位置和视角
                        Vec3 playerPosition = camera.position;
                        Vec3 playerLookDirection = camera.front;
                        float playerPitch = camera.pitch;
                        float playerYaw = camera.yaw;
                        
                        // 获取种子设置
                        unsigned int seed = 0;
                        bool useRandomSeed = uiManager->getSystemGeneratedSeed();
                        
                        if (!useRandomSeed) {
                            // 使用用户输入的种子
                            std::string seedStr = uiManager->getWorldSeed();
                            if (!seedStr.empty()) {
                                try {
                                    seed = static_cast<unsigned int>(std::stoi(seedStr));
                                    std::cout << "Using custom seed: " << seed << std::endl;
                                } catch (...) {
                                    // 如果转换失败，使用随机种子
                                    seed = static_cast<unsigned int>(time(nullptr));
                                    useRandomSeed = true;
                                    std::cout << "Seed conversion failed, using random seed: " << seed << std::endl;
                                }
                            } else {
                                // 空字符串使用随机种子
                                seed = static_cast<unsigned int>(time(nullptr));
                                useRandomSeed = true;
                                std::cout << "No seed provided, using random seed: " << seed << std::endl;
                            }
                        } else {
                            // 使用随机种子
                            seed = static_cast<unsigned int>(time(nullptr));
                            std::cout << "Using random seed: " << seed << std::endl;
                        }
                        
                        // 获取超平坦世界设置
                        bool superFlatWorld = uiManager->getSuperFlatWorld();
                        
                        // 获取世界大小设置
                        int worldSize = uiManager->getActualWorldSize();
                        
                        std::cout << "Resetting world with size: " << worldSize << "x" << 64 << "x" << worldSize << std::endl;
                        std::cout << "World type: " << (superFlatWorld ? "Superflat" : "Normal") << std::endl;
                        std::cout << "World seed: " << seed << std::endl;
                        
                        // 确定超平坦世界的方块类型（根据玩家手持方块）
                        BlockType flatBlockType = BLOCK_GRASS; // 默认使用草方块
                        if (superFlatWorld) {
                            // 获取玩家当前手持的方块类型
                            BlockType heldBlock = uiManager->getCurrentBlockType();
                            // 如果不是空气方块，使用手持方块
                            if (heldBlock != BLOCK_AIR) {
                                flatBlockType = heldBlock;
                                // 输出所选方块类型
                                std::cout << "Superflat block type: " << static_cast<int>(flatBlockType) << std::endl;
                            } else {
                                std::cout << "Superflat block type: Grass (default)" << std::endl;
                            }
                        }
                        
                        std::cout << "Starting world generation..." << std::endl;
                        
                        // 记录开始时间
                        auto startTime = std::chrono::high_resolution_clock::now();
                        
                        // 完全删除世界并重新初始化
                        world = World();
                        
                        if (superFlatWorld) {
                            // 使用超平坦设置初始化世界
                            world.init(worldSize, 64, worldSize, seed, true, flatBlockType);
                        } else {
                            // 使用普通设置初始化世界
                            world.init(worldSize, 64, worldSize, seed);
                        }
                        
                        // 计算生成时间
                        auto endTime = std::chrono::high_resolution_clock::now();
                        std::chrono::duration<double> duration = endTime - startTime;
                        
                        std::cout << "World generation completed in " << duration.count() << " seconds!" << std::endl;
                        
                        // 恢复玩家位置和视角 - 始终保持玩家位置不变
                        camera.position = playerPosition;
                        camera.front = playerLookDirection;
                        camera.pitch = playerPitch;
                        camera.yaw = playerYaw;
                        
                        std::cout << "World reset, player position maintained at: X=" 
                                  << playerPosition.x << " Y=" << playerPosition.y 
                                  << " Z=" << playerPosition.z << std::endl;
                        
                        // 返回游戏状态
                        uiManager->setGameState(GAME_PLAYING);
                        // 锁定鼠标
                        if (!mouseLocked) toggleMouseLock(mainWindow);
                    }
                }
            }
            
            // 处理控制提示界面的返回键
            if (uiManager && uiManager->getControlsState() == UIManager::CONTROLS_SHOWING && wParam == VK_RETURN) {
                uiManager->setControlsState(UIManager::CONTROLS_NONE);
                // 如果是从选项菜单进入的控制界面，返回到暂停菜单
                if (uiManager->getGameState() == GAME_PAUSED) {
                    // 保持在暂停菜单状态
                    uiManager->setGameState(GAME_PAUSED);
                }
            }
            
            // 处理选项菜单的ENTER键 - 已在executeSelectedMenuOption中处理
            // 不需要额外处理
            
            // 只在游戏进行中且鼠标锁定时隐藏鼠标指针
            // 在菜单和物品栏界面始终显示鼠标指针
            if (uiManager && uiManager->getGameState() == GAME_PLAYING && mouseLocked) {
                ShowCursor(FALSE);
            } else {
                ShowCursor(TRUE);
            }
            return 0;
        case WM_KEYUP:
            keys[wParam] = false;
            return 0;
        case WM_MOUSEMOVE: {
            // 如果方块编辑器打开，将鼠标移动传递给它
            if (uiManager && uiManager->isBlockEditorOpen()) {
                POINT mousePos;
                GetCursorPos(&mousePos);
                ScreenToClient(hwnd, &mousePos);
                uiManager->handleBlockEditorMouseInput(mousePos.x, mousePos.y, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
                return 0;
            }

            // 只有在鼠标锁定状态下且游戏正在进行时才处理鼠标移动
            if (mouseLocked && uiManager && uiManager->getGameState() == GAME_PLAYING) {
                static int lastX = SCREEN_WIDTH / 2;
                static int lastY = SCREEN_HEIGHT / 2;
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                float dx = static_cast<float>(x - lastX) * 0.1f;
                float dy = static_cast<float>(y - lastY) * 0.1f;
                
                // 使用修正后的camera.rotate逻辑，不需要对dx、dy取反
                camera.rotate(dx, dy);
                
                // 将鼠标重置到窗口中心
                RECT rect;
                GetClientRect(hwnd, &rect);
                POINT center = {rect.right / 2, rect.bottom / 2};
                ClientToScreen(hwnd, &center);
                SetCursorPos(center.x, center.y);
                
                // 更新上次位置为中心点
                lastX = rect.right / 2;
                lastY = rect.bottom / 2;
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            // 鼠标滚轮用于切换物品栏,只在游戏进行中处理
            if (uiManager && uiManager->getGameState() == GAME_PLAYING) {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                if (delta > 0) {
                    uiManager->selectPrevSlot();
                } else {
                    uiManager->selectNextSlot();
                }
            }
            return 0;
        }
        case WM_MBUTTONDOWN: {
            // 鼠标中键 - 获取方块,只在游戏进行中处理
            if (mouseLocked && uiManager && uiManager->getGameState() == GAME_PLAYING) {
                // 获取玩家视线方向上的方块
                Vec3 rayStart = camera.position;
                Vec3 rayDir = camera.front.normalize();
                float maxDistance = 5.0f; // 最大交互距离
                
                // 简单的射线检测
                for (float t = 0.0f; t < maxDistance; t += 0.1f) {
                    Vec3 pos = rayStart + rayDir * t;
                    int blockX = static_cast<int>(pos.x);
                    int blockY = static_cast<int>(pos.y);
                    int blockZ = static_cast<int>(pos.z);
                    
                    // 检查是否在世界范围内
                    if (world.isInBounds(blockX, blockY, blockZ)) {
                        Block& block = world.getBlock(blockX, blockY, blockZ);
                        if (block.type != BLOCK_AIR && block.isVisible) {
                            // 如果是自定义方块，复制其颜色到编辑器
                            if (block.type == BLOCK_CHANGE_BLOCK && block.hasCustomColors) {
                                uiManager->copyBlockColorsToEditor(block);
                            } else {
                            // 获取方块类型并添加到物品栏
                            uiManager->pickBlock(block.type);
                            }
                            break;
                        }
                    }
                }
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            // 如果方块编辑器打开，将鼠标点击传递给它
            if (uiManager && uiManager->isBlockEditorOpen()) {
                POINT mousePos;
                GetCursorPos(&mousePos);
                ScreenToClient(hwnd, &mousePos);
                uiManager->handleBlockEditorMouseInput(mousePos.x, mousePos.y, true);
                return 0;
            }

            // 左键点击 - 破坏方块,只在游戏进行中处理
            if (mouseLocked && uiManager && uiManager->getGameState() == GAME_PLAYING) {
                // 获取玩家视线方向上的方块
                Vec3 rayStart = camera.position;
                Vec3 rayDir = camera.front.normalize();
                float maxDistance = 5.0f; // 最大交互距离
                
                // 简单的射线检测
                for (float t = 0.0f; t < maxDistance; t += 0.1f) {
                    Vec3 pos = rayStart + rayDir * t;
                    int blockX = static_cast<int>(pos.x);
                    int blockY = static_cast<int>(pos.y);
                    int blockZ = static_cast<int>(pos.z);
                    
                    // 检查是否在世界范围内
                    if (world.isInBounds(blockX, blockY, blockZ)) {
                        Block& block = world.getBlock(blockX, blockY, blockZ);
                        if (block.type != BLOCK_AIR && block.isVisible) {
                            // 破坏方块（设置为空气）
                            block.type = BLOCK_AIR;
                            block.isVisible = false;
                            // 更新周围方块的可见性，特别是下方方块
                            world.updateBlockVisibility(blockX, blockY, blockZ);
                            // 不显示破坏方块的提示信息
                            break;
                        }
                    }
                }
            }
            return 0;
        }
        case WM_RBUTTONDOWN: {
            // 右键点击 - 放置方块,只在游戏进行中处理
            if (mouseLocked && uiManager && uiManager->getGameState() == GAME_PLAYING) {
                // 获取玩家视线方向上的方块
                Vec3 rayStart = camera.position;
                Vec3 rayDir = camera.front.normalize();
                float maxDistance = 5.0f; // 最大交互距离
                
                // 上一个检查的位置（用于放置方块）
                Vec3 lastPos = rayStart;
                
                // 简单的射线检测
                for (float t = 0.0f; t < maxDistance; t += 0.1f) {
                    Vec3 pos = rayStart + rayDir * t;
                    int blockX = static_cast<int>(pos.x);
                    int blockY = static_cast<int>(pos.y);
                    int blockZ = static_cast<int>(pos.z);
                    
                    // 检查是否在世界范围内
                    if (world.isInBounds(blockX, blockY, blockZ)) {
                        Block& block = world.getBlock(blockX, blockY, blockZ);
                        if (block.type != BLOCK_AIR && block.isVisible) {
                            // 找到了一个非空气方块，在上一个位置放置新方块
                            int lastX = static_cast<int>(lastPos.x);
                            int lastY = static_cast<int>(lastPos.y);
                            int lastZ = static_cast<int>(lastPos.z);
                            
                            if (world.isInBounds(lastX, lastY, lastZ)) {
                                Block& lastBlock = world.getBlock(lastX, lastY, lastZ);
                                if (lastBlock.type == BLOCK_AIR) {
                                    // 获取当前选中的方块类型
                                    BlockType blockType = uiManager->getCurrentBlockType();
                                    // 只有当选中的方块类型不是BLOCK_AIR时才放置方块
                                    if (blockType != BLOCK_AIR) {
                                        // 检查放置方块是否会导致玩家被卡住
                                        Vec3 blockPos(lastX, lastY, lastZ);
                                        if (uiManager->canPlaceBlockAt(blockPos, camera.position, physics.isFlying())) {
                                            // 直接使用选中的方块类型，不需要额外的switch判断
                                            lastBlock.type = blockType;
                                            lastBlock.isVisible = true;
                                            
                                            // 如果是自定义方块，复制自定义颜色
                                            if (blockType == BLOCK_CHANGE_BLOCK) {
                                                Block templateBlock = uiManager->getTemplateChangeBlock();
                                                lastBlock.hasCustomColors = templateBlock.hasCustomColors;
                                                // 复制自定义颜色
                                                for (int i = 0; i < FACE_COUNT; i++) {
                                                    lastBlock.setCustomColor(static_cast<Face>(i), templateBlock.getCustomColor(static_cast<Face>(i)));
                                                }
                                            }
                                            
                                            // 更新周围方块的可见性
                                            world.updateBlockVisibility(lastX, lastY, lastZ);
                                        }
                                        // 不显示放置方块的提示信息
                                        // 移除方块放置提示
                                    }
                                }
                            }
                            break;
                        }
                        lastPos = pos;
                    }
                }
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            // 如果方块编辑器打开，将鼠标抬起事件传递给它
            if (uiManager && uiManager->isBlockEditorOpen()) {
                POINT mousePos;
                GetCursorPos(&mousePos);
                ScreenToClient(hwnd, &mousePos);
                uiManager->handleBlockEditorMouseInput(mousePos.x, mousePos.y, false);
                return 0;
            }
            return 0;
        }
        case WM_SIZE:
            // 窗口大小改变时重新创建位图
            if (hBitmap != NULL && wParam != SIZE_MINIMIZED) {
                resizeBitmap(hwnd);
                // 更新鼠标指针显示状态
                updateMouseVisibility(hwnd);
            }
            return 0;
        case WM_CHAR:
        {
            // 如果聊天框打开，处理字符输入
            if (uiManager && uiManager->isChatBoxOpen()) {
                // 忽略控制字符
                if (wParam >= 32 && wParam <= 126) {
                    char c = static_cast<char>(wParam);
                    uiManager->addChatCharacter(c);
                }
                return 0;
            }
            break;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 处理输入
void processInput() {
    // 只有在游戏进行中才处理大部分输入
    bool isGamePlaying = (uiManager && uiManager->getGameState() == GAME_PLAYING);
    
    // 从UI管理器获取当前速度设置
    float walkSpeed = 5.0f;
    float flySpeed = 10.0f;
    
    if (uiManager) {
        walkSpeed = uiManager->getActualWalkSpeed();
        flySpeed = uiManager->getActualFlySpeed();
    }
    
    // 增大移动速度系数，使行走和飞行模式移动更快
    const float MOVE_SPEED_MULTIPLIER = 2.5f;
    
    // 根据当前模式选择速度
    const float moveSpeed = (physics.isFlying() ? flySpeed : walkSpeed) * deltaTime * MOVE_SPEED_MULTIPLIER;
    
    // 只有在游戏进行中且鼠标锁定状态下才处理移动输入
    if (mouseLocked && isGamePlaying) {
        // 根据当前模式处理移动
        if (physics.isFlying()) {
            // 飞行模式 - 直接移动相机
            if (keys['W']) camera.move(Camera::FORWARD, moveSpeed);
            if (keys['S']) camera.move(Camera::BACKWARD, moveSpeed);
            if (keys['A']) camera.move(Camera::RIGHT, moveSpeed);  // A键向左移动(反转LEFT/RIGHT)
            if (keys['D']) camera.move(Camera::LEFT, moveSpeed);   // D键向右移动(反转LEFT/RIGHT)
            if (keys[VK_SPACE]) camera.move(Camera::UP, moveSpeed);
            if (keys[VK_SHIFT]) camera.move(Camera::DOWN, moveSpeed);
        } else {
            // 行走模式 - 使用物理引擎控制移动，添加惯性
            Vec3 moveDirection(0.0f, 0.0f, 0.0f);
            
            // 计算移动方向
            if (keys['W']) {
                Vec3 horizontalFront = camera.front;
                horizontalFront.y = 0;
                horizontalFront = horizontalFront.normalize();
                moveDirection = moveDirection + horizontalFront;
            }
            if (keys['S']) {
                Vec3 horizontalFront = camera.front;
                horizontalFront.y = 0;
                horizontalFront = horizontalFront.normalize();
                moveDirection = moveDirection - horizontalFront;
            }
            if (keys['A']) {
                Vec3 horizontalRight = camera.right;
                horizontalRight.y = 0;
                horizontalRight = horizontalRight.normalize();
                moveDirection = moveDirection + horizontalRight;  // A键向左移动(反转运算符号)
            }
            if (keys['D']) {
                Vec3 horizontalRight = camera.right;
                horizontalRight.y = 0;
                horizontalRight = horizontalRight.normalize();
                moveDirection = moveDirection - horizontalRight;  // D键向右移动(反转运算符号)
            }
            
            // 如果有移动输入，标准化方向向量并应用速度
            if (moveDirection.length() > 0.0f) {
                moveDirection = moveDirection.normalize();
                
                // 获取当前速度
                Vec3 velocity = physics.getVelocity();
                
                // 应用惯性 - 平滑过渡到新的移动方向
                velocity.x = velocity.x * physics.INERTIA_FACTOR + moveDirection.x * moveSpeed * (1.0f - physics.INERTIA_FACTOR) * 2.0f; // 增加水平速度
                velocity.z = velocity.z * physics.INERTIA_FACTOR + moveDirection.z * moveSpeed * (1.0f - physics.INERTIA_FACTOR) * 2.0f; // 增加水平速度
                
                // 更新物理引擎中的速度
                physics.setVelocity(velocity);
            }
            
            // 改进跳跃处理 - 只要空格键被按住，就尝试跳跃
            // 物理引擎内部会处理跳跃冷却时间和地面检测
            if (keys[VK_SPACE]) physics.jump();
        }
        
        // 处理F键 - 切换飞行/行走模式
        static bool fKeyPressed = false;
        if (keys['F'] && !fKeyPressed) {
            physics.toggleFlyingMode();
            fKeyPressed = true;
        } else if (!keys['F']) {
            fKeyPressed = false;
        }
        
        // 处理C键 - 切换穿墙模式（仅在飞行模式下可用）
        static bool cKeyPressed = false;
        if (keys['C'] && !cKeyPressed && physics.isFlying()) {
            physics.toggleNoClip();
            cKeyPressed = true;
        } else if (!keys['C']) {
            cKeyPressed = false;
        }
        
        // 处理E键 - 打开/关闭物品栏
        static bool eKeyPressed = false;
        if (keys['E'] && !eKeyPressed) {
            if (uiManager) {
                uiManager->toggleInventory();
                
                // 切换鼠标锁定状态
                if (uiManager->getGameState() == GAME_INVENTORY) {
                    if (mouseLocked) toggleMouseLock(mainWindow);
                } else {
                    if (!mouseLocked) toggleMouseLock(mainWindow);
                }
                
                // 更新鼠标指针显示状态
                updateMouseVisibility(mainWindow);
                
                eKeyPressed = true;
            }
        } else if (!keys['E']) {
            eKeyPressed = false;
        }
    }
    
    // ESC键现在用于切换菜单，不再用于退出
    // 添加新的退出键 (例如 'Q')
    if (keys['Q']) running = false;
}

// 重新创建位图以适应窗口大小变化
void resizeBitmap(HWND hwnd) {
    // 清理旧资源
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    
    // 重新创建
    hdcMem = CreateCompatibleDC(hdc);
    
    // 获取新的客户区大小
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    SCREEN_WIDTH = clientRect.right - clientRect.left;
    SCREEN_HEIGHT = clientRect.bottom - clientRect.top;
    
    // 更新位图信息
    bmi.bmiHeader.biWidth = SCREEN_WIDTH;
    bmi.bmiHeader.biHeight = -SCREEN_HEIGHT;
    
    // 创建新的位图
    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    SelectObject(hdcMem, hBitmap);
    
    // 更新渲染器和相机
    renderer.resize(SCREEN_WIDTH, SCREEN_HEIGHT, static_cast<uint32_t*>(pBits));
    camera.aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
    
    // 更新UI管理器的屏幕尺寸
    if (uiManager) {
        uiManager->updateScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    }
}

// 初始化位图
bool initBitmap(HWND hwnd) {
    hdc = GetDC(hwnd);
    hdcMem = CreateCompatibleDC(hdc);
    
    // 获取客户区大小
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    SCREEN_WIDTH = clientRect.right - clientRect.left;
    SCREEN_HEIGHT = clientRect.bottom - clientRect.top;
    
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = SCREEN_WIDTH;
    bmi.bmiHeader.biHeight = -SCREEN_HEIGHT; // 负值表示从上到下的DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!hBitmap) return false;
    
    SelectObject(hdcMem, hBitmap);
    return true;
}

// 清理资源
void cleanup() {
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(mainWindow, hdc);
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance [[maybe_unused]], LPSTR lpCmdLine [[maybe_unused]], int nCmdShow) {
    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MinecraftClone";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    
    // Calculate window size to make client area SCREEN_WIDTH x SCREEN_HEIGHT
    RECT rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    
    // Create window
    mainWindow = CreateWindow(
        "MinecraftClone", "Minecraft C++ Clone",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight, NULL, NULL, hInstance, NULL
    );
    
    if (!mainWindow) return 0;
    
    // Initialize bitmap
    if (!initBitmap(mainWindow)) return 0;
    
    // 设置渲染器的窗口句柄
    renderer.setHWND(mainWindow);
    
    // Initialize renderer and world
    renderer.init(SCREEN_WIDTH, SCREEN_HEIGHT, static_cast<uint32_t*>(pBits));
    
    // 输出GPU适配器信息
    const AdapterInfo& gpuInfo = renderer.getGPUAdapterInfo();
    std::cout << "\n===== GPU Information =====" << std::endl;
    std::cout << "Selected GPU: " << gpuInfo.identifier.Description << std::endl;
    std::cout << "Vendor ID: 0x" << std::hex << gpuInfo.identifier.VendorId << std::dec << std::endl;
    std::cout << "Device ID: 0x" << std::hex << gpuInfo.identifier.DeviceId << std::dec << std::endl;
    std::cout << "VRAM: " << gpuInfo.vramMB << " MB" << std::endl;
    std::cout << "GPU Type: " << (gpuInfo.isDiscrete ? "Discrete" : "Integrated") << std::endl;
    std::cout << "Vendor: " << (gpuInfo.isNVIDIA ? "NVIDIA" : gpuInfo.isAMD ? "AMD" : gpuInfo.isIntel ? "Intel" : "Unknown") << std::endl;
    std::cout << "Driver Version: " << gpuInfo.identifier.DriverVersion.QuadPart << std::endl;
    std::cout << "==========================" << std::endl;
    
    // 初始化游戏世界
    unsigned int worldSeed = static_cast<unsigned int>(time(nullptr));
    world.init(64, 64, 64, worldSeed);
    
    // 使用世界的出生点初始化相机位置
    Vec3 spawnPoint = world.getSpawnPoint();
    camera.init(spawnPoint, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
    
    std::cout << "Player spawn position: X=" << spawnPoint.x << " Y=" << spawnPoint.y << " Z=" << spawnPoint.z << std::endl;
    
    // Initialize UI manager
    uiManager = new UIManager(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // 设置UI管理器到渲染器
    renderer.setUIManager(uiManager);
    
    // 将世界信息传递给UI管理器
    uiManager->setWorldInfo(world.getWidth(), world.getHeight(), world.getDepth(), world.getSeed());
    
    // Show window
    ShowWindow(mainWindow, nCmdShow);
    UpdateWindow(mainWindow);
    
    // Initialize mouse lock
    toggleMouseLock(mainWindow);
    
    // Game loop
    MSG msg = {};
    auto lastTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    float frameTime = 0.0f;
    float fps = 0.0f;
    
    while (running) {
        // Process messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Calculate frame time
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Calculate FPS
        fps = 1.0f / deltaTime;
        
        // Update FPS counter
        frameCount++;
        frameTime += deltaTime;
        if (frameTime >= 1.0f) {
            char title[100];
            sprintf_s(title, "Minecraft C++ Clone - FPS: %d - %s", 
                      frameCount, 
                      physics.isFlying() ? "Flying Mode" : "Walking Mode");
            SetWindowTextA(mainWindow, title);
            frameCount = 0;
            frameTime = 0.0f;
        }
        
        // 更新渲染器时间（用于区块缓存系统）
        renderer.updateTime(deltaTime);
        
        // Process input
        processInput();
        
        // 应用各种设置
        float fov = 60.0f + uiManager->getFOVValue() * 60.0f; // 将0-1映射到60-120度
        camera.setFov(fov);
        
        // 应用视距设置
        int renderDistance = uiManager->getActualRenderDistance();
        renderer.setRenderDistance(renderDistance);
        
        // 更新逻辑
        // 应用FOV渲染异常问题 - 调整近平面和远平面距离
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        
        // 根据FOV调整近平面距离，防止视角穿透方块
        if (fov > 90.0f) {
            // 当FOV较大时，增加近平面距离
            nearPlane = 0.1f + (fov - 90.0f) * 0.01f;
        }
        
        // 设置投影矩阵参数
        camera.setPerspective(fov, static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT, nearPlane, farPlane);
        
        // Update physics - 只在游戏进行中更新物理引擎
        if (uiManager && uiManager->getGameState() == GAME_PLAYING) {
            physics.update(camera, world, deltaTime);
        }
        
        // 更新UI管理器的方块名称显示时间
        if (uiManager) {
            uiManager->updateBlockNameDisplay(deltaTime);
        }
        
        // 处理积累的聊天命令
        if (uiManager) {
            // 处理 setblock 命令
            if (uiManager->hasPendingSetBlockCommand) {
                world.setBlock(uiManager->cmdSetBlockX, uiManager->cmdSetBlockY, uiManager->cmdSetBlockZ, uiManager->cmdSetBlockType);
                uiManager->hasPendingSetBlockCommand = false;
            }
            
            // 处理 tp 命令
            if (uiManager->hasPendingTeleportCommand) {
                camera.position.x = uiManager->cmdTeleportX;
                camera.position.y = uiManager->cmdTeleportY;
                camera.position.z = uiManager->cmdTeleportZ;
                uiManager->hasPendingTeleportCommand = false;
            }
            
            // 处理 music 命令
            if (uiManager->hasPendingMusicCommand) {
                if (uiManager->cmdMusicPath == "stop") {
                    // 停止当前音乐
                    uiManager->stopMusic();
                } else {
                    // 播放自定义音乐
                    uiManager->playCustomMusic(uiManager->cmdMusicPath);
                }
                uiManager->hasPendingMusicCommand = false;
            }
            
            // 处理 fill 命令
            if (uiManager->hasPendingFillCommand) {
                int x1 = uiManager->cmdFillX1;
                int y1 = uiManager->cmdFillY1;
                int z1 = uiManager->cmdFillZ1;
                int x2 = uiManager->cmdFillX2;
                int y2 = uiManager->cmdFillY2;
                int z2 = uiManager->cmdFillZ2;
                BlockType blockType = uiManager->cmdFillBlockType;
                
                // 遍历区域内所有的方块并设置
                for (int x = x1; x <= x2; x++) {
                    for (int y = y1; y <= y2; y++) {
                        for (int z = z1; z <= z2; z++) {
                            world.setBlock(x, y, z, blockType);
                        }
                    }
                }
                
                uiManager->hasPendingFillCommand = false;
            }
        }
        
        // 更新UI管理器中的玩家位置，用于相对坐标命令
        if (uiManager) {
            uiManager->updatePlayerPosition(camera.position);
        }
        
        // Clear screen
        renderer.clear(Color(135, 206, 235)); // Sky blue background
        
        // Render world
        renderer.renderWorld(world, camera);
        
        // Render UI
        if (uiManager) {
            uiManager->render(renderer, camera, world, fps);
        }
        
        // 结束帧渲染
        renderer.endFrame();
    }
    
    // Clean up resources
    delete uiManager;
    cleanup();
    
    // Ensure mouse cursor is restored and constraints removed
    ShowCursor(TRUE);
    ClipCursor(NULL);
    
    return 0;
}
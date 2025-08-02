#ifndef PHYSICS_H
#define PHYSICS_H

#include "world.h"
#include "camera.h"

// 物理引擎类
class Physics {
private:
    // Gravity acceleration
    const float GRAVITY = 9.8f;
    // Player height (distance from eyes to feet)
    const float PLAYER_HEIGHT = 1.8f;
    // Player width (collision box radius)
    const float PLAYER_RADIUS = 0.3f;
    // Jump initial velocity
    const float JUMP_VELOCITY = 7.5f; // 增加跳跃初速度
    // Movement speed
    float moveSpeed = 100.0f; // 增加行走模式下的移动速度
    
    // Player state
    bool flyingMode = false;  // 默认为行走模式
    bool noClip = false;     // No-clip mode (disabled by default)
    
    // Player velocity
    Vec3 velocity = Vec3(0.0f, 0.0f, 0.0f);
    
    // Whether player is on ground
    bool onGround = false;
    // Jump cooldown (prevents continuous jumping)
    float jumpCooldown = 0.0f;
    // Maximum jump cooldown time
    const float MAX_JUMP_COOLDOWN = 0.15f; // 更进一步减少跳跃冷却时间
    
public:
    Physics() {}
    
    // 惯性系数 - 控制移动的平滑度
    const float INERTIA_FACTOR = 0.1f; // 大幅减少惯性，使移动不再"滑冰"
    // 地面摩擦系数
    const float GROUND_FRICTION = 0.15f; // 增加摩擦力
    // 空气阻力系数
    const float AIR_RESISTANCE = 0.01f; // 保持空气阻力不变
    // 水平速度限制
    const float MAX_HORIZONTAL_SPEED = 200.0f; // 保持最大水平速度不变
    
    // Update physics state
    void update(Camera& camera, const World& world, float deltaTime) {
        // Update jump cooldown
        if (jumpCooldown > 0.0f) {
            jumpCooldown -= deltaTime;
        }
        
        if (!flyingMode) {
            // Apply gravity in walking mode
            if (!onGround) {
                velocity.y -= GRAVITY * deltaTime;
            } else {
                // 在地面上应用摩擦力
                velocity.x *= (1.0f - GROUND_FRICTION);
                velocity.z *= (1.0f - GROUND_FRICTION);
                
                // Reset vertical velocity when on ground
                if (velocity.y < 0) {
                    velocity.y = 0.0f;
                }
            }
            
            // 应用空气阻力
            if (!onGround) {
                velocity.x *= (1.0f - AIR_RESISTANCE);
                velocity.z *= (1.0f - AIR_RESISTANCE);
            }
            
            // 限制水平速度
            float horizontalSpeed = sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
            if (horizontalSpeed > MAX_HORIZONTAL_SPEED) {
                float scale = MAX_HORIZONTAL_SPEED / horizontalSpeed;
                velocity.x *= scale;
                velocity.z *= scale;
            }
            
            // Apply velocity to camera position
            Vec3 newPosition = camera.position + velocity * deltaTime;
            
            // Check collisions and update position
            if (!noClip) {
                handleCollision(camera, newPosition, world);
            } else {
                camera.position = newPosition;
            }
            
            // Check if player is on ground
            checkGroundContact(camera, world);
        }
    }
    
    // Handle collisions
    void handleCollision(Camera& camera, const Vec3& newPosition, const World& world) {
        // Check movement along each axis separately
        Vec3 finalPosition = camera.position;
        
        // Check X-axis movement
        Vec3 xMove = finalPosition;
        xMove.x = newPosition.x;
        if (!checkCollision(xMove, world)) {
            finalPosition.x = newPosition.x;
        }
        
        // Check Y-axis movement
        Vec3 yMove = finalPosition;
        yMove.y = newPosition.y;
        if (!checkCollision(yMove, world)) {
            finalPosition.y = newPosition.y;
        } else if (velocity.y < 0) {
            // If collision occurs while moving downward, player is on ground
            onGround = true;
            velocity.y = 0;
        } else if (velocity.y > 0) {
            // If collision occurs while moving upward, player hit ceiling
            velocity.y = 0;
        }
        
        // Check Z-axis movement
        Vec3 zMove = finalPosition;
        zMove.z = newPosition.z;
        if (!checkCollision(zMove, world)) {
            finalPosition.z = newPosition.z;
        }
        
        // Update camera position
        camera.position = finalPosition;
    }
    
    // Check if specified position collides with blocks
    bool checkCollision(const Vec3& position, const World& world) {
        // Get player's feet position
        Vec3 feetPosition = position - Vec3(0, PLAYER_HEIGHT, 0);
        
        // 扩大检查范围，确保不会错过任何碰撞
        const float checkRadius = PLAYER_RADIUS * 1.5f; // 增大检查半径
        
        // 计算检查范围（以方块为单位）
        int checkRangeX = static_cast<int>(ceil(checkRadius));
        int checkRangeZ = static_cast<int>(ceil(checkRadius));
        int checkRangeY = static_cast<int>(ceil(PLAYER_HEIGHT));
        
        // Check blocks around player
        for (int y = -checkRangeY; y <= checkRangeY; y++) {
            for (int x = -checkRangeX; x <= checkRangeX; x++) {
                for (int z = -checkRangeZ; z <= checkRangeZ; z++) {
                    // Get block coordinates
                    int blockX = static_cast<int>(floor(position.x + x));
                    int blockY = static_cast<int>(floor(feetPosition.y + y));
                    int blockZ = static_cast<int>(floor(position.z + z));
                    
                    // 确保坐标在世界范围内
                    if (!world.isInBounds(blockX, blockY, blockZ)) continue;
                    
                    // Check if block is solid
                    Block block = world.getBlock(blockX, blockY, blockZ);
                    if (block.type != BLOCK_AIR && block.type != BLOCK_WATER && block.type != BLOCK_LEAVES) {
                        // Calculate block bounding box
                        float minX = static_cast<float>(blockX);
                        float minY = static_cast<float>(blockY);
                        float minZ = static_cast<float>(blockZ);
                        float maxX = minX + 1.0f;
                        float maxY = minY + 1.0f;
                        float maxZ = minZ + 1.0f;
                        
                        // 使用更精确的AABB碰撞检测
                        // 计算玩家碰撞箱
                        float playerMinX = position.x - PLAYER_RADIUS;
                        float playerMaxX = position.x + PLAYER_RADIUS;
                        float playerMinY = feetPosition.y;
                        float playerMaxY = position.y;
                        float playerMinZ = position.z - PLAYER_RADIUS;
                        float playerMaxZ = position.z + PLAYER_RADIUS;
                        
                        // 检查AABB碰撞
                        bool collisionX = playerMaxX > minX && playerMinX < maxX;
                        bool collisionY = playerMaxY > minY && playerMinY < maxY;
                        bool collisionZ = playerMaxZ > minZ && playerMinZ < maxZ;
                        
                        if (collisionX && collisionY && collisionZ) {
                            // 确定碰撞面 - 计算穿透深度
                            float depthX1 = playerMaxX - minX; // 右侧碰撞
                            float depthX2 = maxX - playerMinX; // 左侧碰撞
                            float depthY1 = playerMaxY - minY; // 顶部碰撞
                            float depthY2 = maxY - playerMinY; // 底部碰撞
                            float depthZ1 = playerMaxZ - minZ; // 前方碰撞
                            float depthZ2 = maxZ - playerMinZ; // 后方碰撞
                            
                            // 找出最小穿透深度
                            float minDepthX = std::min(depthX1, depthX2);
                            float minDepthY = std::min(depthY1, depthY2);
                            float minDepthZ = std::min(depthZ1, depthZ2);
                            
                            // 确定碰撞面
                            if (minDepthX < minDepthY && minDepthX < minDepthZ) {
                                // X轴碰撞（左右面）
                                return true;
                            } else if (minDepthY < minDepthX && minDepthY < minDepthZ) {
                                // Y轴碰撞（上下面）
                                return true;
                            } else {
                                // Z轴碰撞（前后面）
                                return true;
                            }
                        }
                    }
                }
            }
        }
        
        return false; // No collision
    }
    
    // Check if player is standing on ground
    void checkGroundContact(const Camera& camera, const World& world) {
        // Get position below player's feet
        Vec3 feetPosition = camera.position - Vec3(0, PLAYER_HEIGHT + 0.05f, 0);
        
        // 检查玩家脚下的多个点，提高地面检测的准确性
        bool foundGround = false;
        
        // 检查玩家脚下的多个点，使用更密集的采样
        for (float offsetX = -PLAYER_RADIUS; offsetX <= PLAYER_RADIUS; offsetX += PLAYER_RADIUS * 0.5f) {
            for (float offsetZ = -PLAYER_RADIUS; offsetZ <= PLAYER_RADIUS; offsetZ += PLAYER_RADIUS * 0.5f) {
                int blockX = static_cast<int>(floor(feetPosition.x + offsetX));
                int blockY = static_cast<int>(floor(feetPosition.y));
                int blockZ = static_cast<int>(floor(feetPosition.z + offsetZ));
                
                // 确保坐标在世界范围内
                if (!world.isInBounds(blockX, blockY, blockZ)) continue;
                
                // Check block below feet
                Block block = world.getBlock(blockX, blockY, blockZ);
                if (block.type != BLOCK_AIR && block.type != BLOCK_WATER && block.type != BLOCK_LEAVES) {
                    foundGround = true;
                    break;
                }
            }
            if (foundGround) break;
        }
        
        onGround = foundGround;
    }
    
    // Jump function
    void jump() {
        if (flyingMode) return; // Can't jump in flying mode
        
        // 对于跳跃，只要处于地面状态就可以一直跳
        if (onGround && jumpCooldown <= 0.0f) {
            velocity.y = JUMP_VELOCITY; // Use the defined jump velocity
            onGround = false;
            jumpCooldown = MAX_JUMP_COOLDOWN; // Set cooldown to prevent spam jumping
        }
    }
    
    // Toggle flying/walking mode
    void toggleFlyingMode() {
        flyingMode = !flyingMode;
        
        // Reset velocity when switching to flying mode
        if (flyingMode) {
            velocity = Vec3(0.0f, 0.0f, 0.0f);
        }
    }
    
    // Toggle no-clip mode
    void toggleNoClip() {
        noClip = !noClip;
    }
    
    // Check if in flying mode
    bool isFlying() const {
        return flyingMode;
    }
    
    // Check if in no-clip mode
    bool isNoClip() const {
        return noClip;
    }
    
    // Set movement speed
    void setMoveSpeed(float speed) {
        moveSpeed = speed;
    }
    
    // Get current movement speed
    float getMoveSpeed() const {
        return moveSpeed;
    }
    
    // Get player velocity
    const Vec3& getVelocity() const {
        return velocity;
    }
    
    // Set player velocity
    void setVelocity(const Vec3& newVelocity) {
        velocity = newVelocity;
    }
};

#endif // PHYSICS_H
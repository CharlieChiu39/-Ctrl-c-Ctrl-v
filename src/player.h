#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>    // For bool type
#include <SDL2/SDL.h> // For SDL_Rect and potentially other SDL types

// == Constants ==
// (Moved from player.c for better visibility and potential external use)
#define PLAYER_INITIAL_HEALTH 100
#define PLAYER_WIDTH 50           // Default width for bounding box
#define PLAYER_HEIGHT 100         // Default height for bounding box
#define MOVE_SPEED 250.0f         // Movement speed (pixels/second)
#define JUMP_STRENGTH 750.0f      // Initial jump velocity (upwards is negative)
#define GRAVITY 2000.0f           // Gravity acceleration (pixels/second^2)
#define ATTACK_DURATION 0.4f      // Attack state duration (seconds)
#define HURT_DURATION 0.5f        // Hurt stun duration (seconds)
#define HURT_INVINCIBILITY_DURATION 0.7f // Invincibility time after getting hit
#define SKILL_DURATION 1.0f        // 假設技能持續時間
#define ANIMATION_FRAME_DURATION 0.1f // 每幀動畫顯示時間 (秒)


// Note: Animation timing and specific hitbox values remain internal to player.c

// == Player State Enum ==
// Defines all possible states the player character can be in
typedef enum {
    PLAYER_STATE_IDLE,      // Standing still
    PLAYER_STATE_WALKING,   // Moving left or right
    PLAYER_STATE_JUMPING,   // Moving upwards from a jump
    PLAYER_STATE_FALLING,   // Falling downwards (after jump peak or off edge)
    PLAYER_STATE_ATTACKING, // Performing an attack
    PLAYER_STATE_HURT,      // Stunned after being hit
    PLAYER_STATE_DEFEATED,  // Health is zero or less
    PLAYER_STATE_CROUCHING, // Crouching state (if implemented)
    PLAYER_STATE_BLOCKING,  // Blocking state (if implemented)
    PLAYER_STATE_SKILL      // Using a skill (if implemented)
} PlayerState;

// == Player Command Enum ==
// Represents commands derived from player input
typedef enum {
    CMD_NONE,         // No specific command / Neutral state
    CMD_LEFT,         // Move left command
    CMD_RIGHT,        // Move right command
    CMD_JUMP,         // Jump command
    CMD_ATTACK,       // Basic attack command
    CMD_CROUCH_START, // Command to start crouching
    CMD_CROUCH_END,   // Command to end crouching
    CMD_BLOCK_START,  // Command to start blocking
    CMD_BLOCK_END,    // Command to end blocking
    CMD_SKILL         // Command to use a skill
} PlayerCommand;


// == Player Structure ==
// Contains all data associated with a player character
typedef struct {
    // -- Core Physics & Position --
    float x, y;             // Position (usually top-left corner)
    float vx, vy;           // Velocity (pixels per second)
    int direction;          // Facing direction (1 for right, -1 for left)
    bool isOnGround;        // Flag indicating if the player is on the ground

    // -- Game State & Attributes --
    int health;             // Current health points
    PlayerState currentState; // The player's current action state
    bool isHittable;        // Can the player currently take damage?

    // -- Collision Boxes (Relative Offsets/Dimensions) --
    SDL_Rect bounding_box;  // Basic physics/boundary box
    SDL_Rect current_hitbox; // Hitbox used for attacks (active only during attacks)

    // -- Timers (for state durations) --
    float attackTimer;      // How much longer the attack state lasts
    float hurtTimer;        // How much longer the hurt state lasts
    float skillTimer;       // How much longer the skill state lasts (if used)
    float invincibilityTimer; // How much longer the player is invincible after being hit

    // -- Animation Control --
    int currentFrame;       // Index of the current animation frame to display
    float frameTimer;       // Timer to control animation frame updates

    // -- State Flags --
    bool isAttacking;       // Is the player currently in attack logic?
    bool isBlocking;        // Is the player currently blocking? (if implemented)
    bool isSkillActive;     // Is the player currently using a skill? (if implemented)

} Player;

// == Function Prototypes ==
// (Functions defined in player.c that need to be called from other files)

/**
 * @brief Creates and initializes a new Player object.
 * @param startX Initial X position.
 * @param startY Initial Y position.
 * @param initialDirection Initial facing direction (1=right, -1=left).
 * @return A newly initialized Player object.
 */
Player createPlayer(float startX, float startY, int initialDirection);

/**
 * @brief Processes a command intended for the player.
 * Modifies player's velocity, state, or triggers actions based on the command.
 * @param player Pointer to the Player object to modify.
 * @param command The command received from input processing.
 */
void handlePlayerCommand(Player *player, PlayerCommand command);

/**
 * @brief Updates the player's state over time (physics, timers, state transitions).
 * Should be called once per game loop iteration.
 * @param player Pointer to the Player object to update.
 * @param deltaTime Time elapsed since the last frame (in seconds).
 * @param groundLevel The Y-coordinate representing the ground.
 */
void updatePlayer(Player *player, float deltaTime, int groundLevel);

/**
 * @brief Calculates and returns the player's main bounding box in world coordinates.
 * Useful for basic collision detection and positioning checks.
 * @param player Pointer to the Player object.
 * @return SDL_Rect representing the bounding box in world space.
 */
SDL_Rect getPlayerBoundingBox(const Player *player);

/**
 * @brief Calculates and returns the player's currently active attack hitbox in world coordinates.
 * Returns an empty/invalid rect if the player is not attacking or the hitbox isn't active.
 * @param player Pointer to the Player object.
 * @return SDL_Rect representing the active hitbox in world space, or {0,0,0,0} if inactive.
 */
SDL_Rect getPlayerHitboxWorld(const Player *player);

/**
 * @brief Applies damage to the player, updates health, and potentially changes state to HURT or DEFEATED.
 * @param player Pointer to the Player object taking damage.
 * @param damage The amount of damage to apply.
 */
void playerTakesDamage(Player *player, int damage);

#endif // PLAYER_H
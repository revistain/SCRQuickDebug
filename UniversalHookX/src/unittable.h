#ifndef UNITTABLE_H
#define UNITTABLE_H
#include <cstdint>

typedef struct {
    // Pointer to the previous sprite
    struct CSprite* prev; // 0x00
    // Pointer to the next sprite
    struct CSprite* next; // 0x04
    // Sprite data
    uint32_t sprite; // 0x08
    // Player ID (creator)
    uint32_t playerID; // 0x0A
    // Index in the selection area (0 <= selectionIndex <= 11)
    uint8_t selectionIndex; // 0x0B
    // Visibility flags (for fog-of-war)
    uint8_t visibilityFlags; // 0x0C
    // Elevation level
    uint8_t elevationLevel; // 0x0D
    // Sprite flags
    uint8_t flags; // 0x0E
    // Selection timer
    uint8_t selectionTimer; // 0x0F
    // Index
    uint16_t index; // 0x10
    // Group dimensions
    uint8_t grpWidth;  // 0x12
    uint8_t grpHeight; // 0x13
    // Position data
    uint16_t posX;       // 0x14 (same as pos.x)
    uint16_t posY;       // 0x16 (same as pos.y)
    // Main graphic image
    uint32_t mainGraphic; // 0x18 (officially 'pImagePrimary', CImage)
    // Head and tail of the image linked list
    uint32_t imageHead; // 0x1C
    uint32_t imageTail; // 0x20
} CSprite;


typedef struct {
    // Pointer to the previous unit
    struct CUnit* prev; // 0x000
    // Pointer to the next unit
    struct CUnit* next; // 0x004
    // Health points (displayed value is ceil(hp/256))
    uint32_t hp; // 0x008
    // Sprite associated with the unit
    CSprite sprite; // 0x00C
    // Target position for movement
    // X and Y target positions for movement
    uint16_t moveTargetX; // 0x010
    uint16_t moveTargetY; // 0x012
    // Pointer to the target unit for movement
    struct CUnit* moveTarget; // 0x014
    // Next waypoint position
    uint32_t nextMovementWaypoint; // 0x018
    // Desired target waypoint
    uint32_t nextTargetWaypoint; // 0x01C
    // Movement flags
    uint8_t movementFlags; // 0x020
    // Current direction unit is facing
    uint8_t currentDirection1; // 0x021
    // Turn speed and radius
    uint8_t turnSpeed;  // 0x022
    uint8_t turnRadius; // 0x022
    // Direction of velocity
    uint8_t velocityDirection1; // 0x023
    // Flingy ID
    uint16_t flingyID; // 0x024
    // Unknown byte
    uint8_t unknown0x26; // 0x026
    // Flingy movement type
    uint8_t flingyMovementType; // 0x027
    // Current position of the unit
    // Position coordinates
    uint16_t posX; // 0x028
    uint16_t posY; // 0x02A
    // Halt positions
    uint32_t haltX; // 0x02C
    uint32_t haltY; // 0x030
    // Top speed and current speed
    uint32_t topSpeed;      // 0x034
    uint32_t currentSpeed1; // 0x038
    uint32_t currentSpeed2; // 0x03C
    // Current velocity
    uint32_t currentVelocityX; // 0x040
    uint32_t currentVelocityY; // 0x044
    // Acceleration
    uint16_t acceleration; // 0x048
    // Current direction 2
    uint8_t currentDirection2; // 0x04A
    // Velocity direction 2
    uint8_t velocityDirection2; // 0x04B
    // Owner/player ID
    uint32_t playerID; // 0x04C
    // Order information
    uint8_t orderID; // 0x04D
    uint8_t orderState;       // 0x04E
    uint8_t orderSignal;      // 0x04F
    uint32_t orderUnitType;   // 0x050
    uint16_t unknown0x52;     // 0x052
    // Cooldown timers
    uint32_t cooldown;            // 0x054
    uint8_t orderTimer;           // 0x054
    uint8_t groundWeaponCooldown; // 0x055
    uint8_t airWeaponCooldown;    // 0x056
    uint8_t spellCooldown;        // 0x057
    // Order target position
    uint16_t orderTargetX;          // 0x058
    uint16_t orderTargetY;          // 0x05A
    // Order target unit
    struct CUnit* orderTarget; // 0x05C
    // Shield information
    uint32_t shield; // 0x060
    // Unit type
    uint16_t unitID;   // 0x064
    uint16_t unknown0x66; // 0x066
    // Previous and next player units
    struct CUnit* prevPlayerUnit; // 0x068
    struct CUnit* nextPlayerUnit; // 0x06C
    // Sub unit
    struct CUnit* subUnit; // 0x070
    // Order queue head and tail
    uint32_t orderQueueHead; // 0x074
    uint32_t orderQueueTail; // 0x078
    // Auto-target unit
    struct CUnit* autoTargetUnit; // 0x07C
    // Connected unit
    struct CUnit* connectedUnit; // 0x080
    // Order queue details
    uint8_t orderQueueCount;   // 0x084
    uint8_t orderQueueTimer;   // 0x085
    uint8_t unknown0x86;       // 0x086
    uint8_t attackNotifyTimer; // 0x087
    // Various unknown and unsupported members
    uint32_t prevUnitType;             // 0x088
    uint8_t lastEventTimer;            // 0x08A
    uint8_t lastEventColor;            // 0x08B
    uint16_t unknown0x8C;              // 0x08C
    uint8_t rankIncrease;              // 0x08E
    uint8_t killCount;                 // 0x08F
    uint8_t lastAttackingPlayer; // 0x090
    uint8_t secondaryOrderTimer;       // 0x091
    uint8_t AIActionFlag;              // 0x092
    uint8_t userActionFlags;           // 0x093
    uint16_t currentButtonSet;         // 0x094
    uint8_t isCloaked;                 // 0x096
    uint8_t movementState;             // 0x097
    uint16_t buildQueue1;              // 0x098
    uint16_t buildQueue2;              // 0x09A
    uint16_t buildQueue3;              // 0x09C
    uint16_t buildQueue4;              // 0x09E
    uint16_t buildQueue5;              // 0x0A0
    uint32_t buildQueue12;             // 0x098
    uint32_t buildQueue34;             // 0x09C
    uint16_t energy;                   // 0x0A2
    uint8_t buildQueueSlot;            // 0x0A4
    uint8_t uniquenessIdentifier;      // 0x0A5
    uint8_t secondaryOrderID;          // 0x0A6
    uint8_t buildingOverlayState;      // 0x0A7
    uint16_t hpGain;                   // 0x0A8
    uint16_t shieldGain;               // 0x0AA
    uint16_t remainingBuildTime;       // 0x0AC
    uint16_t prevHp;                   // 0x0AE
    uint16_t loadedUnit1;              // 0x0B0
    uint16_t loadedUnit2;              // 0x0B2
    uint16_t loadedUnit3;              // 0x0B4
    uint16_t loadedUnit4;              // 0x0B6
    uint16_t loadedUnit5;              // 0x0B8
    uint16_t loadedUnit6;              // 0x0BA
    uint16_t loadedUnit7;              // 0x0BC
    uint16_t loadedUnit8;              // 0x0BE
    // Union section (0xC0 ~ 0xCF)
    uint8_t spiderMineCount;      // 0x0C0
    struct CUnit* inHangarChild;  // 0x0C0
    struct CUnit* outHangarChild; // 0x0C4
    uint8_t inHangarCount;        // 0x0C8
    uint8_t outHangarCount;       // 0x0C9
    struct CUnit* parent;         // 0x0C0
    struct CUnit* prevFighter;    // 0x0C4
    struct CUnit* nextFighter;    // 0x0C8
    uint8_t isOutsideHangar;      // 0x0CC
    uint32_t beaconUnknown0xC0;   // 0x0C0
    uint32_t beaconUnknown0xC4;   // 0x0C4
    uint32_t flagSpawnFrame;      // 0x0C8
    struct CUnit* addon;          // 0x0C0
    uint32_t addonBuildType;      // 0x0C4
    uint16_t upgradeResearchTime; // 0x0C8
    uint32_t abilityID;           // 0x0CC
    uint32_t soundIndex;          // 0x0D0
    uint32_t buildAngle;          // 0x0D4
    uint32_t unknown0xD8;         // 0x0D8
} CUnit;


class UnitTable {

};

#endif
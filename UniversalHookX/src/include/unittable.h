#ifndef UNITTABLE_H
#define UNITTABLE_H
#include <cstdint>
#include <map>
#include <string>

typedef struct CSprite {
    struct CSprite* prev; // 0x00
    struct CSprite* next; // 0x04
    uint32_t sprite; // 0x08
    uint32_t playerID; // 0x0A
    uint8_t selectionIndex; // 0x0B
    uint8_t visibilityFlags; // 0x0C
    uint8_t elevationLevel; // 0x0D
    uint8_t flags; // 0x0E
    uint8_t selectionTimer; // 0x0F
    uint16_t index; // 0x10
    uint8_t grpWidth;  // 0x12
    uint8_t grpHeight; // 0x13
    uint16_t posX;       // 0x14 (same as pos.x)
    uint16_t posY;       // 0x16 (same as pos.y)
    uint32_t mainGraphic; // 0x18 (officially 'pImagePrimary', CImage)
    uint32_t imageHead; // 0x1C
    uint32_t imageTail; // 0x20
} CSprite;


typedef struct CUnit {
    struct CUnit* prev;             // 0x000
    struct CUnit* next;             // 0x004
    uint32_t hp;                    // 0x008
    CSprite* sprite;                // 0x00C
    uint16_t moveTargetX;           // 0x010
    uint16_t moveTargetY;           // 0x012
    struct CUnit* moveTarget;       // 0x014
    uint32_t nextMovementWaypoint;  // 0x018
    uint32_t nextTargetWaypoint;    // 0x01C
    uint8_t movementFlags;          // 0x020
    uint8_t currentDirection1;      // 0x021
    uint8_t flingyTurnRadius;       // 0x022
    uint8_t velocityDirection1;     // 0x023
    uint16_t flingyID;              // 0x024
    uint8_t unknown0x26;            // 0x026
    uint8_t flingyMovementType;     // 0x027
    uint16_t posX;                  // 0x028
    uint16_t posY;                  // 0x02A
    uint32_t haltX;                 // 0x02C
    uint32_t haltY;                 // 0x030
    uint32_t topSpeed;              // 0x034
    uint32_t currentSpeed1;         // 0x038
    uint32_t currentSpeed2;         // 0x03C
    uint32_t currentVelocityX;      // 0x040
    uint32_t currentVelocityY;      // 0x044
    uint16_t acceleration;          // 0x048
    uint8_t currentDirection2;      // 0x04A
    uint8_t velocityDirection2;     // 0x04B
    uint8_t playerID;               // 0x04C
    uint8_t orderID;                // 0x04D
    uint8_t orderState;             // 0x04E
    uint8_t orderSignal;            // 0x04F
    uint16_t orderUnitType;         // 0x050
    uint16_t unknown16_52;          // 0x052
    uint8_t orderTimer;             // 0x054
    uint8_t groundWeaponCooldown;   // 0x055
    uint8_t airWeaponCooldown;      // 0x056
    uint8_t spellCooldown;          // 0x057
    uint16_t orderTargetX;          // 0x058
    uint16_t orderTargetY;          // 0x05A
    struct CUnit* orderTarget;      // 0x05C
    uint32_t shield;                // 0x060
    uint16_t unitID;                // 0x064
    uint16_t unknown0x66;           // 0x066
    struct CUnit* prevPlayerUnit;   // 0x068
    struct CUnit* nextPlayerUnit;   // 0x06C
    struct CUnit* subUnit;          // 0x070
    uint32_t orderQueueHead;        // 0x074
    uint32_t orderQueueTail;        // 0x078
    struct CUnit* autoTargetUnit;   // 0x07C
    struct CUnit* connectedUnit;    // 0x080
    uint8_t orderQueueCount;        // 0x084
    uint8_t orderQueueTimer;        // 0x085
    uint8_t unknown0x86;            // 0x086
    uint8_t attackNotifyTimer;      // 0x087
    // Various unknown and unsupported members
    uint16_t prevUnitType;             // 0x088
    uint8_t lastEventTimer;            // 0x08A
    uint8_t lastEventColor;            // 0x08B
    uint16_t unknown16_8C;             // 0x08C
    uint8_t rankIncrease;              // 0x08E
    uint8_t killCount;                 // 0x08F
    uint8_t lastAttackingPlayer;       // 0x090
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
    struct CUnit* addon;            // 0x0C0
    uint32_t addonBuildType;        // 0x0C4
    uint16_t upgradeResearchTime;   // 0x0C8
    uint32_t abilityID;             // 0x0CC
    uint32_t soundIndex;            // 0x0D0
    uint32_t buildAngle;            // 0x0D4
    uint32_t unknown0xD8;           // 0x0D8
    uint32_t statusFlag;            // 0xDC
    uint8_t owningOreType;          // 0xE0
    uint8_t wireframeRandomizer;    // 0xE1
    uint8_t secondaryOrderState;    // 0xE2
    uint8_t recentOrderTimer;       // 0xE3
    uint32_t visibilityStatus;      // 0xE4
    uint32_t secondaryOrderPosition;// 0xE8
    uint32_t currentBuildUnit;      // 0xEC
    uint32_t previousBurrowedUnit;  // 0xF0
    uint32_t nextBurrowedUnit;      // 0xF4
    uint16_t RallyX;                // 0xF8
    uint16_t RallyY;                // 0xFA
    uint32_t RallyUnit;             // 0xFC
    uint32_t dwPath;                // 0x100
    uint8_t pathingCollisionInterval; // 0x104
    uint8_t pathingFlags;             // 0x105
    uint8_t unknown8_106;             // 0x106
    uint8_t isBeingHealed;            // 0x107
    uint32_t unknown32_108;           // 0x108
    uint32_t unknown32_10C;           // 0x10C
    uint16_t removeTimer;             // 0x110
    uint16_t defenseMatrixDamage;     // 0x112
    uint8_t defenseMatrixTimer;       // 0x114
    uint8_t stimTimer;                // 0x115
    uint8_t ensnareTimer;             // 0x116
    uint8_t lockdownTimer;            // 0x117
    uint8_t irradiateTimer;           // 0x118
    uint8_t stasisTimer;              // 0x119
    uint8_t plagueTimer;              // 0x11A
    uint8_t stormTimer;               // 0x11B
    uint32_t irradiatedBy;            // 0x11C
    uint8_t irradiatePlayerID;        // 0x120
    uint8_t parasiteFlags;            // 0x121
    uint8_t cycleCounter;             // 0x122
    uint8_t isBlind;                  // 0x123
    uint8_t maelstromTimer;           // 0x124
    uint8_t empty8_125;               // 0x125
    uint8_t acidSporeCount;           // 0x126
    uint8_t acidSporeTime0;           // 0x127
    uint8_t acidSporeTime1;           // 0x128
    uint8_t acidSporeTime2;           // 0x129
    uint8_t acidSporeTime3;           // 0x12A
    uint8_t acidSporeTime4;           // 0x12B
    uint8_t acidSporeTime5;           // 0x12C
    uint8_t acidSporeTime6;           // 0x12D
    uint8_t acidSporeTime7;           // 0x12E
    uint8_t acidSporeTime8;           // 0x12F
    uint16_t bullet3Attack;           // 0x130
    uint16_t padding_132;             // 0x132
    uint32_t pAI;                     // 0x134
    uint16_t airStrength;             // 0x138
    uint16_t groundStrength;          // 0x13A
    uint32_t posSortXL;               // 0x13C
    uint32_t posSortXR;               // 0x140
    uint32_t posSortYT;               // 0x144
    uint32_t posSortYB;               // 0x148
    uint8_t _repulseUnknown;          // 0x14C
    uint8_t repulseAngle;             // 0x14D
    uint8_t bRepMtxX;                 // 0x14E
    uint8_t bRepMtxY;                 // 0x14F
} CUnit;
#include <format>
static_assert(sizeof(CUnit) == 0x150, "Size of CUnit is incorrect!");

extern std::map<std::string, int> CSpriteFields;
extern std::map<std::string, int> CUnitFields;
#endif
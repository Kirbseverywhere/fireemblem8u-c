#ifndef GUARD_TYPES_H
#define GUARD_TYPES_H

struct BgCoords
{
    u16 x;
    u16 y;
};

struct Struct03003080
{
    /*0x00*/ struct DispCnt dispcnt;
    /*0x04*/ struct DispStat dispstat;
    /*0x08*/ u8 filler8[4];
    /*0x0C*/ struct BgCnt bg0cnt;
    /*0x10*/ struct BgCnt bg1cnt;
    /*0x14*/ struct BgCnt bg2cnt;
    /*0x18*/ struct BgCnt bg3cnt;
    /*0x1C*/ struct BgCoords bgoffset[4];
    /*0x2C*/ u16 win0h;
    /*0x2C*/ u16 win1h;
    /*0x30*/ u16 win0v;
    /*0x30*/ u16 win1v;
    /*0x34*/ struct WinCnt wincnt;
    /*0x38*/ u16 mosaic;
             u8 filler3A[2];
    /*0x3C*/ struct BlendCnt bldcnt;
    /*0x40*/ u8 filler40[4];
    /*0x44*/ u8 blendCoeffA;
    /*0x45*/ u8 blendCoeffB;
    /*0x46*/ u8 blendY;
    /*0x48*/ u16 bg2pa;
    /*0x4A*/ u16 bg2pb;
    /*0x4C*/ u16 bg2pc;
    /*0x4E*/ u16 bg2pd;
    /*0x50*/ u32 bg2x;
    /*0x54*/ u32 bg2y;
    /*0x58*/ u16 bg3pa;
    /*0x5A*/ u16 bg3pb;
    /*0x5C*/ u16 bg3pc;
    /*0x5E*/ u16 bg3pd;
    /*0x60*/ u32 bg3x;
    /*0x64*/ u32 bg3y;
    /*0x68*/ s8 colorAddition;
};

struct Struct0858791C
{
    u8 filler0[4];
    u16 unk2;
};

struct TileDataTransfer
{
    void *src;
    void *dest;
    u16 size;
    u16 mode;
};

struct OamDataTransfer
{
    void *src;
    void *dest;
    u16 unk8;
    u16 count;
};

struct Struct02024CD4
{
    int unk0;
    int unk4;
};

struct KeyStatusBuffer
{
    u8 repeatDelay;     // initial delay before generating auto-repeat presses
    u8 repeatInterval;  // time between auto-repeat presses
    u8 repeatTimer;     // (decreased by one each frame, reset to repeatDelay when Presses change and repeatInterval when reaches 0)
    u16 heldKeys;       // keys that are currently held down
    u16 repeatedKeys;   // auto-repeated keys
    u16 newKeys;        // keys that went down this frame
    u16 prevKeys;       // keys that were held down last frame
    u16 LastPressState;
    bool16 ABLRPressed; // 1 for Release (A B L R Only), 0 Otherwise
    u16 newKeys2;
    u16 TimeSinceStartSelect; // Time since last Non-Start Non-Select Button was pressed
};

typedef void (*InterruptHandler)(void);

struct Struct0202BCF0 // Chapter Data Struct
{
    /* 00 */ u32 unk0; // a time value
    /* 04 */ u32 unk4; // a time value

    /* 08 */ u32 partyGoldAmount;
    /* 0C */ u8  gameSaveSlot;
    
    /* 0D */ u8  chapterVisionRange; // 0 means no fog
    /* 0E */ s8  chapterIndex;
    
    /* 0F */ u8  chapterPhaseIndex; // 0x00 = Player phase, 0x40 = NPC phase, 0x80 = Enemy phase (0xC0 = link arena 4th team?)

    /* 10 */ u16 chapterTurnNumber;

    /* 12 */ u8  xCursor, yCursor; // map cursor position

    /* 14 */ u8  chapterStateBits; // +0x04 = postgame, +0x10 = in prep screen, +0x40 = difficult mode, +0x80 = link arena?
    /* 15 */ u8  chapterWeatherId;
    /* 16 */ u16 chapterTotalSupportGain;

    /* 18 */ u8  _pad18[0x1B - 0x18]; // unknown yet

    /* 1B */ u8  chapterModeIndex; // 1 for tutorial (ch0-8), 2 for Eirika route, 3 for Ephraim route

    /* 1C */ u8  unk1C[4];

    /* 20 */ char playerName[0x40 - 0x20]; // unused outside of link arena (was tactician name in FE7); Size unknown

    // option byte 1 (of 3)
    u32 unk40_1:5;
    u32 unk40_6:2; // text speed
    u32 unk40_8:1;

    u32 unk41_1:1;
    u32 unk41_2:1;
    u32 unk41_3:6;

    u32 unk42_1:1;
    u32 unk42_2:1;
    u32 unk42_3:1;
    u32 unk42_4:1;
    u32 unk42_5:1;
    u32 unk42_6:1;
    u32 unk42_7:1;
    u32 unk42_8:1;

    u32 unk43_1:8;

    u8  unk44[0x48 - 0x44];

    u16 unk48;

    u16 unk4A_1 : 1;
    u16 unk4A_2 : 3;
};

struct TextBuffer0202A6AC
{
    u8 buffer0202A6AC[0x555];
    u8 buffer0202AC01[0x555];
    u8 buffer0202B156[0x356];
    u8 buffer0202B4AC[0x100];
    u8 buffer0202B5AC[1];  // unknown length
};

struct UnknownItemStruct
{
    u8 filler0[6];
    u16 unk6;
};


struct UnknownStructCTC
{
    struct UnknownStructCTC *unk0;
    s16 unk4;
    s16 unk6;
    u16 unk8;
    u8 fillerA[2];
    const void *unkC;
};


typedef u16 Item;

struct ItemStatBonuses {
    u8 HPBonus;
    u8 PowBonus;
    u8 SklBonus;
    u8 SpdBonus;
    u8 DefBonus;
    u8 ResBonus;
    u8 LckBonus;
    u8 ConBonus;
    u8 MovBonus;
};

struct ItemData {
    u16 nameTextId; //0
    u16 descTextId; //2
    u16 useDescTextId; //4

    u8  number; //6
    u8  weaponType; //7

    u32 attributes; //8

    const struct ItemStatBonuses* pStatBonuses; //c
    const u8* pEffectiveness; //10

    u8  maxUses; //14

    u8  might; //15
    u8  hit; //16
    u8  weight; //17
    u8  crit; //18

    u8 maxRange : 4; //19
    u8 minRange : 4; //19

    u16 costPerUse; //1a
    u8  weaponRank; //1c
    u8  iconId; //1d
    u8  useEffectId; //1e
    u8  weaponEffectId; //1f
    u8  weaponExp; //20

    u8  _u21[3]; //21
};

struct SMSHandle {
    /* 00 */ struct SMSHandle* pNext;

    /* 04 */ short xDisplay;
    /* 06 */ short yDisplay;

    /* 08 */ u16 oam2Base;

    /* 0A */ u8 _u0A;
    /* 0B */ s8 config;
};

struct CharacterData {
    /* 00 */ u16 nameTextId;
    /* 02 */ u16 descTextId;
    /* 04 */ u8 number;
    /* 05 */ u8 defaultClass;
    /* 06 */ u16 portraitId;
    /* 08 */ u8 miniPortrait;
    /* 09 */ u8 affinity;
    /* 0A */ u8 _u0A;

    /* 0B */ u8 baseLevel;
    /* 0C */ s8 baseHP;
    /* 0D */ s8 basePow;
    /* 0E */ s8 baseSkl;
    /* 0F */ s8 baseSpd;
    /* 10 */ s8 baseDef;
    /* 11 */ s8 baseRes;
    /* 12 */ s8 baseLck;
    /* 13 */ s8 baseCon;

    /* 14 */ u8 baseRanks[8];

    /* 1C */ u8 growthHP;
    /* 1D */ u8 growthPow;
    /* 1E */ u8 growthSkl;
    /* 1F */ u8 growthSpd;
    /* 20 */ u8 growthDef;
    /* 21 */ u8 growthRes;
    /* 22 */ u8 growthLck;

    /* 23 */ u8 _u23;
    /* 24 */ u8 _u24;
    /* 25 */ u8 _u25;
    /* 26 */ u8 _u26;
    /* 27 */ u8 _u27;

    /* 28 */ u32 attributes;

    /* 2C */ void* pSupportData;
    /* 30 */ void* _pU30;
};

struct ClassData {
    /* 00 */ u16 nameTextId;
    /* 02 */ u16 descTextId;
    /* 04 */ u8 number;
    /* 05 */ u8 promotion;
    /* 06 */ u8 SMSId;
    /* 07 */ u8 slowWalking;
    /* 08 */ u16 defaultPortraitId;
    /* 0A */ u8 _u0A;

    /* 0B */ u8 baseHP;
    /* 0C */ u8 basePow;
    /* 0D */ u8 baseSkl;
    /* 0E */ u8 baseSpd;
    /* 0F */ u8 baseDef;
    /* 10 */ u8 baseRes;
    /* 11 */ u8 baseCon;
    /* 12 */ u8 baseMov;

    /* 13 */ u8 maxHP;
    /* 14 */ u8 maxPow;
    /* 15 */ u8 maxSkl;
    /* 16 */ u8 maxSpd;
    /* 17 */ u8 maxDef;
    /* 18 */ u8 maxRes;
    /* 19 */ u8 maxCon;

    /* 1A */ u8 classRelativePower;

    /* 1B */ u8 growthHP;
    /* 1C */ u8 growthPow;
    /* 1D */ u8 growthSkl;
    /* 1E */ u8 growthSpd;
    /* 1F */ u8 growthDef;
    /* 20 */ u8 growthRes;
    /* 21 */ u8 growthLck;

    /* 22 */ u8 promotionHP;
    /* 23 */ u8 promotionPow;
    /* 24 */ u8 promotionSkl;
    /* 25 */ u8 promotionSpd;
    /* 26 */ u8 promotionDef;
    /* 27 */ u8 promotionRes;

    /* 28 */ u32 attributes;

    /* 2C */ u8 baseRanks[8];

    /* 34 */ const void* pBattleAnimDef;
    /* 38 */ const u8* pMovCostTable[3]; // standard, rain, snow
    /* 44 */ const u8* pTerrainBonusTables[3]; // def, avo, res

    /* 50 */ const void* _pU50;
};

enum {
	US_NONE = 0x00000000,

	US_HIDDEN = 0x00000001,
	US_UNSELECTABLE = 0x00000002,
	US_DEAD = 0x00000004,
	US_NOT_DEPLOYED = 0x00000008,
	US_RESCUING = 0x00000010,
	US_RESCUED = 0x00000020,
	US_HAS_MOVED = 0x00000040, // Bad name?
	US_CANTOING = 0x00000040, // Alias
	US_UNDER_A_ROOF = 0x00000080,
	// = 0x00000100,
	// = 0x00000200,
	US_HAS_MOVED_AI = 0x00000400,
	US_IN_BALLISTA = 0x00000800,
	US_DROP_ITEM = 0x00001000,
	US_GROWTH_BOOST = 0x00002000,
	US_SOLOANIM_1 = 0x00004000,
	US_SOLOANIM_2 = 0x00008000,
	// = 0x00010000,
	// = 0x00020000,
	// = 0x00040000,
	// = 0x00080000,
	// = 0x00100000,
	// = 0x00200000,
	// = 0x00400000,
	// = 0x00800000,
	// = 0x01000000,
	// = 0x02000000,
	// = 0x04000000,
	// = 0x08000000,
	// = 0x10000000,
	// = 0x20000000,
	// = 0x40000000,
	// = 0x80000000,

	US_DUMMY
};

enum {
	CA_NONE = 0x00000000,
	CA_MOUNTEDAID = 0x00000001,
	CA_CANTO = 0x00000002,
	CA_STEAL = 0x00000004,
	CA_LOCKPICK = 0x00000008,
	CA_DANCE = 0x00000010,
	CA_PLAY = 0x00000020,
	CA_CRITBONUS = 0x00000040,
	CA_BALLISTAE = 0x00000080,
	CA_PROMOTED = 0x00000100,
	CA_SUPPLY = 0x00000200,
	CA_MOUNTED = 0x00000400,
	CA_WYVERN = 0x00000800,
	CA_PEGASUS = 0x00001000,
	CA_LORD = 0x00002000,
	CA_FEMALE = 0x00004000,
	CA_BOSS = 0x00008000,
	CA_LOCK_1 = 0x00010000,
	CA_LOCK_2 = 0x00020000,
	CA_LOCK_3 = 0x00040000, // Dragons or Monster depending of game
	CA_MAXLEVEL10 = 0x00080000,
	CA_UNSELECTABLE = 0x00100000,
	CA_TRIANGLEATTACK_PEGASI = 0x00200000,
	CA_TRIANGLEATTACK_ARMORS = 0x00400000,
	// = 0x00800000,
	// = 0x01000000,
	CA_LETHALITY = 0x02000000,
	// = 0x04000000,
	CA_SUMMON = 0x08000000,
	CA_LOCK_4 = 0x10000000,
	CA_LOCK_5 = 0x20000000,
	CA_LOCK_6 = 0x40000000,
	CA_LOCK_7 = 0x80000000,

	CA_DUMMY
};

struct Unit {
    /* 00 */ const struct CharacterData* pCharacterData;
    /* 04 */ const struct ClassData* pClassData;

    /* 08 */ u8 level;
    /* 09 */ u8 exp;
    /* 0A */ u8 _u0A;

    /* 0B */ s8 index;

    /* 0C */ u32 state;

    /* 10 */ s8 xPos;
    /* 11 */ s8 yPos;

    /* 12 */ u8 maxHP;
    /* 13 */ u8 curHP;
    /* 14 */ u8 pow;
    /* 15 */ u8 skl;
    /* 16 */ u8 spd;
    /* 17 */ u8 def;
    /* 18 */ u8 res;
    /* 19 */ u8 lck;

    /* 1A */ u8 conBonus;
    /* 1B */ u8 rescueOtherUnit;
    /* 1C */ u8 ballistaIndex;
    /* 1D */ u8 movBonus;

    /* 1E */ u16 items[5];
    /* 28 */ u8 ranks[8];

    /* 30 */ u8 statusIndex : 4;
    /* 30 */ u8 statusDuration : 4;

    /* 31 */ u8 torchDuration : 4;
    /* 31 */ u8 barrierDuration : 4;

    /* 32 */ u8 supports[6];
    /* 38 */ u8 unitLeader;
    /* 39 */ u8 supportBits;
    /* 3A */ u8 _u3A;
    /* 3B */ u8 _u3B;

    /* 3C */ struct SMSHandle* pMapSpriteHandle;

    /* 40 */ u16 ai3And4;
    /* 42 */ u8 ai1;
    /* 43 */ u8 ai1data;
    /* 44 */ u8 ai2;
    /* 45 */ u8 ai2data;
    /* 46 */ u8 _u46;
    /* 47 */ u8 _u47;
};

enum {
    // Ballista extdata definitions
    TRAP_EXTDATA_BLST_ITEMID   = 0, // ballista item id
    TRAP_EXTDATA_BLST_RIDDEN   = 2, // "is ridden" boolean
    TRAP_EXTDATA_BLST_ITEMUSES = 3, // ballista item uses

    // Obstacle (Snags and Walls) extdata definitions
    TRAP_EXTDATA_OBSTACLE_HP = 0, // hp left

    // Map Change extdata definitions
    TRAP_EXTDATA_MAPCHANGE_ID = 0, // map change id

    // Trap (Fire/Gas/Arrow) extdata definitions
    TRAP_EXTDATA_TRAP_TURNFIRST = 1, // start turn countdown
    TRAP_EXTDATA_TRAP_TURNNEXT  = 2, // repeat turn countdown
    TRAP_EXTDATA_TRAP_COUNTER   = 3, // turn counter
    TRAP_EXTDATA_TRAP_DAMAGE    = 4, // trap damage (needs confirmation)

    // Torchlight extdata definitions
    TRAP_EXTDATA_LIGHT_TURNSLEFT = 0, // turns left before wearing out

    // Light Rune extdata definitions
    TRAP_EXTDATA_RUNE_REPLACINGTERRAIN = 0, // terrain id of the replaced tile
    TRAP_EXTDATA_RUNE_TURNSLEFT        = 3, // turns left beofre wearing out
};

struct Trap {
    /* 00 */ u8 xPos;
    /* 01 */ u8 yPos;

    /* 02 */ u8 type;

    /* 03 */ u8 data[5]; // extdata (see above enum for per trap type entry allocations)
};

struct Struct0202BCB0 { // Game State Struct
    /* 00 */ u8  mainLoopEndedFlag;

    /* 01 */ s8  gameLogicSemaphore;
    /* 02 */ s8  gameGfxSemaphore;

    /* 03 */ u8  _unk04;

    /* 04 */ u8  gameStateBits;

    /* 05 */ u8  _unk05;

    /* 06 */ u16 prevVCount;

    /* 08 */ u32 _unk08;

    /* 0C */ short xCameraReal;
    /* 0E */ short yCameraReal;
};

struct BattleUnit {
	/* 00 */ struct Unit unit;

	/* 48 */ u16 weaponAfter;
	/* 4A */ u16 weaponBefore;
	/* 4C */ u32 weaponAttributes;
	/* 50 */ u8 weaponType;
	/* 51 */ u8 weaponSlotIndex;

	/* 52 */ u8 canCounter;

	/* 53 */ s8 WTHitModifier;
	/* 54 */ s8 WTAtkModifier;

	/* 55 */ u8 terrainIndex;
	/* 56 */ u8 terrainDefense;
	/* 57 */ u8 terrainAvoid;
	/* 58 */ u8 terrainResistance;
	/* 59 */ u8 _u59;

	/* 5A */ u16 battleAttack;
	/* 5C */ u16 battleDefense;
	/* 5E */ u16 battleAttackSpeed;
	/* 60 */ u16 battleHit;
	/* 62 */ u16 battleAvoid;
	/* 64 */ u16 battleEffectiveHit;
	/* 66 */ u16 battleCrit;
	/* 68 */ u16 battleDodge;
	/* 6A */ u16 battleEffectiveCrit;
	/* 6C */ u16 battleSilencerRate;

	/* 6E */ u8 expGain;
	/* 6F */ u8 statusOut;
	/* 70 */ u8 levelPrevious;
	/* 71 */ u8 expPrevious;

	/* 72 */ u8 currentHP;

	/* 73 */ s8 changeHP;
	/* 74 */ s8 changePow;
	/* 75 */ s8 changeSkl;
	/* 76 */ s8 changeSpd;
	/* 77 */ s8 changeDef;
	/* 78 */ s8 changeRes;
	/* 79 */ s8 changeLck;
	/* 7A */ s8 changeCon;

	/* 7B */ s8 wexpMultiplier;
	/* 7C */ u8 nonZeroDamage;
	/* 7D */ u8 weaponBroke;

	/* 7E */ u8 _u7E;
	/* 7F */ u8 _u7F;
};

struct MapAnimActorState {
	/* 00 */ struct Unit* pUnit;
	/* 04 */ struct BattleUnit* pBattleUnit;
	/* 08 */ struct MUProc* pMUProc;
	/* 0C */ u8 u0C;
	/* 0D */ u8 u0D;
	/* 0E */ u16 u0E;
	/* 10 */ u8 u10;
	/* 11 */ u8 u11;
	/* 12 */ u8 u12;
	/* 13 */ u8 u13;
};

struct MapAnimState {
	/* 00 */ struct MapAnimActorState actors[4];

	/* 50 */ u32* pCurrentRound;
	/* 54 */ const struct ProcCmd* pItemMapAnimProcScript;
	/* 58 */ u8 subjectActorId;
	/* 59 */ u8 targetActorId;
	/* 5A */ u16 roundBits;
	/* 5C */ u16 u5C;
	/* 5E */ u8 actorCount_maybe;
	/* 5F */ u8 u5F;
	/* 60 */ u8 u60;
	/* 61 */ u8 u61;
};

struct MMSData {
    const void* pGraphics;
    const void* pAnimation;
};

#endif  // GUARD_TYPES_H

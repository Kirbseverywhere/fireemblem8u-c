#include "global.h"

#include "proc.h"
#include "ap.h"
#include "items.h"
#include "m4a.h"
#include "soundwrapper.h"
#include "hardware.h"

#include "mu.h"

/*
    "MOVEUNIT" proc and related functions.
    Handles managing and displaying moving map sprites.
*/

// TODO: move this elsewhere
// I can't move this to functions.h because signatures contain types that would be not defined
// So we'll have to wait for the corresponding files to be decompiled/get a header
void sub_8013928(const u16*, int, int, struct Proc* proc);
int GetSpellAssocFacing(Item item);

// TODO: move this elsewhere
// For some reason putting this in variables.h makes the ROM not match???????
// I must be missing something
extern const struct MMSData gUnknown_089A2E00[]; // Moving Map Sprite Table

struct MUStepSoundProc {
	PROC_HEADER;

	/* 29 */ u8 _pad29[0x58 - 0x29];

	/* 58 */ unsigned u58;
	/* 5C */ unsigned u5C;
	/* 60 */ unsigned u60;
	/* 64 */ short u64;
	/* 66 */ short u66;
};

struct SomeProc {
	PROC_HEADER;

	/* 2C */ int xDisplay;
	/* 30 */ int yDisplay;

	/* 34 */ u8 _pad34[0x50 - 0x34];

	/* 50 */ struct APHandle* pAPHandle;
	/* 54 */ u8 _pad54[0x64 - 0x54];
	/* 64 */ short u64;
};

struct MUEffectProc {
	PROC_HEADER;

	/* 29 */ u8 _pad29[0x54 - 0x29];
	/* 54 */ struct MUProc* pMUProc;

	/* 58 */ u8 _pad58[0x64 - 0x58];
	/* 64 */ short timeLeft;
	/* 66 */ short frameIndex;
};

struct MUFadeEffectProc {
	PROC_HEADER;

	/* 2C */ struct MUProc* pMUProc;
	/* 30 */ u8 u30;
};

static struct MUProc* NewMOVEUNIT(u16 x, u16 y, u16 classIndex, int objTileId, unsigned palId);

static void Init6C_89A2938(struct MUStepSoundProc* proc);
static void Call89A2938_PlaySound1(struct MUStepSoundProc* proc);
static void Call89A2938_PlaySound2(struct MUStepSoundProc* proc);

static void nullsub_19(struct MUProc* proc);

static void Moveunit_ExecMoveCommand(struct MUProc* proc);

static void DisplayFogThingMaybe(int x, int y);

static void Call6C_89A2968(struct SomeProc* proc);
static void Loop6C_89A2968_1(struct SomeProc* proc);
static void Loop6C_89A2968_2(struct SomeProc* proc);

static void MOVU_Call5_WaitForSomething(struct MUProc* proc);
static void MOVU_Call4_SetState2(struct MUProc* proc);
static void MOVU_Call3_Wait(struct MUProc* proc);
static void sub_8078C58(struct MUProc* proc);
static void nullsub_54(struct MUProc* proc);
static void MOVU_Call2_Moving(struct MUProc* proc);

static void MOVEUNIT6C_PlaySoundStepByClass(struct MUProc* proc);

static void _6CMOVEUNIT_Loop(struct MUProc* proc);
static void _6CMOVEUNIT_Destructor(struct MUProc* proc);

static void Delete6C__(struct MUProc* proc);

static void sub_80790CC(struct MUProc* proc);

static struct MUConfig* GetNextMoveunitEntryStruct(int objTileId, u8* outIndex_maybe);
static struct MUConfig* sub_807920C(int objTileId, u8* outIndex_maybe);

static void UpdateMOVEUNITGfx_Idle(struct MUProc* proc);
static void UpdateMOVEUNITGfx_Movement(struct MUProc* proc);

static u16 MOVEUNIT6C_GetMovementSpeed(struct MUProc* proc);

static void* GetMOVEUNITGraphicsBuffer(int muIndex);

static const void* GetMovingMapSpriteGfxPtrFromMOVEUNIT(struct MUProc* proc);
static const void* MMS_GetROMTCS(u16 classId);

static void BlendTimer6C_MainLoop(struct MUEffectProc* proc);

static void sub_8079654(struct MUEffectProc* proc);

static void sub_80796D4(u32* data, int frame);

static void sub_8079730(struct MUEffectProc* proc);

static void sub_807988C(struct MUEffectProc* proc);

static void TCS_HaltAnim2(int argAp);
static void sub_807990C(int argAp);
static void TCS_HaltAnim(int argAp);

static void sub_80799A0(struct MUFadeEffectProc* proc);
static void sub_80799A8(struct MUFadeEffectProc* proc);
static void sub_80799C8(struct MUFadeEffectProc* proc);
static void sub_80799EC(struct MUFadeEffectProc* proc);
static void sub_8079A10(struct MUFadeEffectProc* proc);
static void sub_8079A50(struct MUFadeEffectProc* proc);

static void sub_8079AD4(struct MUFadeEffectProc* proc);

static void SetMOVEUNITField44To1(struct Proc* proc);

static struct MUProc* GetMoveunitByIndex(int muIndex);

#define MU_GetDisplayXOrg(proc) ((((proc)->xSubPosition + (proc)->xSubOffset) >> MU_SUBPIXEL_PRECISION) + 8)
#define MU_GetDisplayYOrg(proc) ((((proc)->ySubPosition + (proc)->ySubOffset) >> MU_SUBPIXEL_PRECISION) + 8)

#define MU_AdvanceGetCommand(proc) (proc->pMUConfig->commands[proc->pMUConfig->currentCommand++])

// NON-CONST DATA

// Buffer for graphics
// I do not know how to handle this :/
extern u8 gUnknown_02004BE0[MU_GFX_MAX_SIZE * MU_MAX_COUNT];

static struct MUConfig sMUConfigArray[MU_MAX_COUNT];

// CONST DATA

#ifndef CONST_DATA
#   define CONST_DATA const __attribute__((section(".data")))
#endif // CONST_DATA

static CONST_DATA u16* gUnknown_089A2920[] = {
	gUnknown_0859A140,
	gUnknown_0859A120,
	gUnknown_0859A160,
	gUnknown_0859A180,
	gUnknown_0859A1A0,
	gUnknown_0859A1C0,
};

// gProc_MUStepSound
static CONST_DATA struct ProcCmd gUnknown_089A2938[] = {
	PROC_CALL_ROUTINE(Init6C_89A2938),

	PROC_SLEEP(0),
	PROC_CALL_ROUTINE(Call89A2938_PlaySound1),

	PROC_SLEEP(0),
	PROC_CALL_ROUTINE(Call89A2938_PlaySound2),

	PROC_END
};

static CONST_DATA struct ProcCmd gUnknown_089A2968[] = {
	PROC_CALL_ROUTINE(Call6C_89A2968),
	PROC_LOOP_ROUTINE(Loop6C_89A2968_1),
	PROC_LOOP_ROUTINE(Loop6C_89A2968_2),

	PROC_END
};

// gDirectionMoveOffsetLookup
static CONST_DATA short gUnknown_089A2988[] = {
	-1,  0, // left
	+1,  0, // right
	0,  +1, // up
	0,  -1, // down
};

// START MU SFX DEFINITIONS

// feet sounds
CONST_DATA u16 gUnknown_089A2998[] = {
	0x10, // loop duration (in frames)
	0x02, // idk

	0x96, // TODO: USE SOUND ID DEFINITION

	// 7 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,

	0x97, // TODO: USE SOUND ID DEFINITION

	// 7 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,

};

// heavy sounds
CONST_DATA u16 gUnknown_089A29BC[] = {
	0x20, // loop duration (in frames)
	0x02, // idk

	0xA4, // TODO: USE SOUND ID DEFINITION

	// 15 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,

	0xA5, // TODO: USE SOUND ID DEFINITION

	// 15 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// mounted sounds
CONST_DATA u16 gUnknown_089A2A00[] = {
	0x15, // loop duration (in frames)
	0x03, // idk

	0x9A, // TODO: USE SOUND ID DEFINITION

	// 2 frames of no sounds
	0x00, 0x00,

	0x9B, // TODO: USE SOUND ID DEFINITION

	// 6 frames of no sounds
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,

	0x9C, // TODO: USE SOUND ID DEFINITION

	// 10 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

// wyvern sounds
CONST_DATA u16 gUnknown_089A2A2E[] = {
	0x14, // loop duration (in frames)
	0x01, // idk

	0xA0, // TODO: USE SOUND ID DEFINITION

	// 19 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// pegasi sounds
CONST_DATA u16 gUnknown_089A2A5A[] = {
	0x14, // loop duration (in frames)
	0x01, // idk

	0xA6, // TODO: USE SOUND ID DEFINITION

	// 19 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// unused sounds
CONST_DATA u16 gMUSfxDef_Unused_089A2A86[] = {
	0x14, // loop duration (in frames)
	0x01, // idk

	0x2E0, // TODO: USE SOUND ID DEFINITION

	// 19 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// zombie sounds
CONST_DATA u16 gUnknown_089A2AB2[] = {
	0x0F, // loop duration (in frames)
	0x01, // idk

	0x302, // TODO: USE SOUND ID DEFINITION

	// 14 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

// skelly sounds
CONST_DATA u16 gUnknown_089A2AD4[] = {
	0x0F, // loop duration (in frames)
	0x01, // idk

	0x304, // TODO: USE SOUND ID DEFINITION

	// 14 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

// mogall sounds
CONST_DATA u16 gUnknown_089A2AF6[] = {
	0x14, // loop duration (in frames)
	0x01, // idk

	0x306, // TODO: USE SOUND ID DEFINITION

	// 19 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// spider sounds
CONST_DATA u16 gUnknown_089A2B22[] = {
	0x0A, // loop duration (in frames)
	0x01, // idk

	0x308, // TODO: USE SOUND ID DEFINITION

	// 9 frames of no sounds
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// dog sounds
CONST_DATA u16 gUnknown_089A2B3A[] = {
	0x15, // loop duration (in frames)
	0x01, // idk

	0x30A, // TODO: USE SOUND ID DEFINITION

	// 2 frames of no sounds
	0x00, 0x00,

	0x30A, // TODO: USE SOUND ID DEFINITION

	// 6 frames of no sounds
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,

	0x30A, // TODO: USE SOUND ID DEFINITION

	// 10 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

// gorgon sounds
CONST_DATA u16 gUnknown_089A2B68[] = {
	0x0F, // loop duration (in frames)
	0x01, // idk

	0x30C, // TODO: USE SOUND ID DEFINITION

	// 14 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

// Unused/Unreferenced step sounds
CONST_DATA u16 gMUSfxDef_Unused_089A2BCE[] = {
	0x20, // loop duration (in frames)
	0x02, // idk

	0x2E0, // TODO: USE SOUND ID DEFINITION

	// 31 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// boat sounds
CONST_DATA u16 gUnknown_089A2BCE[] = {
	0x18, // loop duration (in frames)
	0x01, // idk

	0x30F, // TODO: USE SOUND ID DEFINITION

	// 11 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,

	0x30E, // TODO: USE SOUND ID DEFINITION

	// 11 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// myrrh sounds
CONST_DATA u16 gUnknown_089A2C02[] = {
	0x10, // loop duration (in frames)
	0x01, // idk

	0x3BB, // TODO: USE SOUND ID DEFINITION

	// 15 frames of no sounds
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};

// END MU SFX DEFINITIONS

// MU state handler function pointer lookup
static CONST_DATA MUStateHandlerFunc gUnknown_089A2C28[] = {
	sub_8078C58,
	nullsub_54,
	MOVU_Call2_Moving,
	MOVU_Call3_Wait,
	MOVU_Call4_SetState2,
	MOVU_Call5_WaitForSomething,
	nullsub_54,
	nullsub_54,
};

CONST_DATA struct ProcCmd gUnknown_089A2C48[] = {
	PROC_SET_NAME("MOVEUNIT"),
	PROC_SET_MARK(4),

	PROC_SET_DESTRUCTOR(_6CMOVEUNIT_Destructor),
	PROC_LOOP_ROUTINE(_6CMOVEUNIT_Loop)
};

// obj tile id offset by MU id
static CONST_DATA u16 gUnknown_089A2C68[MU_MAX_COUNT] = {
	0x0000, 0x0010, 0x0008, 0x0018
};

// obj tile id offset by MU id (variant?)
static CONST_DATA u16 gUnknown_089A2C70[MU_MAX_COUNT] = { 
	0x0000, 0x0008, 0x0004, 0x0010
};

// Base Movement Speed Lookup
static CONST_DATA u8 gUnknown_089A2C78[2] = {
	2, // fast class base movement speed
	1  // slow class base movement speed
};

// Buffer index by MU index lookup
static CONST_DATA u8 gUnknown_089A2C7A[MU_MAX_COUNT+1] = {
	0, // dummy entry (this array is 1-indexed)

	0, 2, 1, 3
};

// gProc_MUDeathFadeEffect
CONST_DATA struct ProcCmd gUnknown_089A2C80[] = {
	PROC_LOOP_ROUTINE(BlendTimer6C_MainLoop),
	PROC_SLEEP(15),

	PROC_END
};

// gProc_MUBlinkingEffect
CONST_DATA struct ProcCmd gUnknown_089A2C98[] = {
	PROC_LOOP_ROUTINE(sub_8079654),
	PROC_END
};

// Doesn't match without being volatile
// :/
static CONST_DATA vu8 gUnknown_089A2CA8[0x40] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

static CONST_DATA struct ProcCmd gUnknown_089A2CE8[] = {
	PROC_LOOP_ROUTINE(sub_8079730),
	PROC_END
};

CONST_DATA struct ProcCmd gUnknown_089A2CF8[] = {
	PROC_SLEEP(8),
	PROC_CALL_ROUTINE(sub_807988C),

	PROC_END
};

static CONST_DATA struct ProcCmd gUnknown_089A2D10[] = {
	PROC_CALL_ROUTINE(sub_80799A0),
	PROC_SLEEP(1),

	PROC_CALL_ROUTINE(sub_80799A8),
	PROC_SLEEP(2),

	PROC_CALL_ROUTINE(sub_80799C8),
	PROC_SLEEP(3),

	PROC_CALL_ROUTINE(sub_80799A8),
	PROC_SLEEP(2),

	PROC_CALL_ROUTINE(sub_80799C8),
	PROC_SLEEP(3),

	PROC_CALL_ROUTINE(sub_80799A8),
	PROC_SLEEP(1),

	PROC_CALL_ROUTINE(sub_80799EC),
	PROC_LOOP_ROUTINE(sub_8079A10),

	PROC_SLEEP(17),
	PROC_CALL_ROUTINE(sub_8079A50),

	PROC_END
};

static CONST_DATA struct ProcCmd gUnknown_089A2D98[] = {
	PROC_SLEEP(17),
	PROC_CALL_ROUTINE(sub_8079AD4),
	PROC_END
};

// END CONST DATA

void ResetMoveunitStructs(void) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i)
		sMUConfigArray[i].muIndex = 0;
}

struct MUProc* Make6CMOVEUNITForUnit(struct Unit* pUnit, unsigned classIndex, unsigned palId) {
	struct MUProc* proc = NewMOVEUNIT(
		pUnit->xPos,
		pUnit->yPos,

		classIndex,

		-1,
		palId
	);

	proc->pUnit = pUnit;
	proc->boolAttractCamera = TRUE;

	return proc;
}

struct MUProc* MakeMOVEUNITForMapUnit(struct Unit* pUnit) {
	struct MUProc* proc;

	unsigned classIndex = pUnit->pClassData->number;

	if (pUnit->state & US_IN_BALLISTA) {
		struct Trap* blst = GetTrap(pUnit->ballistaIndex);

		// FIXME: use class id definitions

		switch (blst->data[TRAP_EXTDATA_BLST_ITEMID]) {

		case Ballista:
			classIndex = 0x67;
			break;

		case IronBallista:
			classIndex = 0x68;
			break;

		case KillerBallista:
			classIndex = 0x69;
			break;

		} // switch (blst->data[TRAP_EXTDATA_BLST_ITEMID])
	}

	proc = NewMOVEUNIT(
		pUnit->xPos,
		pUnit->yPos,

		classIndex,

		-1,
		GetUnitMapSpritePaletteIndex(pUnit)
	);

	proc->pUnit = pUnit;
	proc->boolAttractCamera = TRUE;

	return proc;
}

void sub_80784D8(struct MUProc* proc) {
	_6CMOVEUNIT_Loop(proc);
}

void MOVEUNIT6C_SetCameraFollow(struct MUProc* proc) {
	proc->boolAttractCamera = TRUE;
}

void MOVEUNIT6C_UnsetCameraFollow(struct MUProc* proc) {
	proc->boolAttractCamera = FALSE;
}

struct MUProc* Make6CMOVEUNITForUI(struct Unit* pUnit, int x, int y) {
	struct MUProc* proc = MakeMOVEUNITForMapUnit(pUnit);

	if (!proc)
		return NULL;

	proc->xSubPosition = x << MU_SUBPIXEL_PRECISION;
	proc->ySubPosition = y << MU_SUBPIXEL_PRECISION;

	proc->stateId = MU_STATE_UI_DISPLAY;

	return proc;
}

void sub_8078524(struct MUProc* proc) {
	SMS_80266F0(
		GetClassStandingMapSpriteId(proc->displayedClassId),
		proc->muIndex
	);
}

static struct MUProc* NewMOVEUNIT(u16 x, u16 y, u16 classIndex, int objTileId, unsigned palId) {
	struct MUConfig* config;
	struct MUProc* proc;
	struct APHandle* ap;

	unsigned otherThing = 0;
	u8 thing = 0;

	if (objTileId == -1)
		config = GetNextMoveunitEntryStruct(objTileId = MU_BASE_OBJ_TILE, &thing);
	else
		config = sub_807920C(objTileId, &thing);

	if (!config)
		return NULL;

	if (Proc_Find(gUnknown_089A2C48))
		otherThing = 0xFE;

	proc = (struct MUProc*) Proc_Create(gUnknown_089A2C48, ROOT_PROC_5);

	if (!proc)
		return NULL;

	proc->pUnit = NULL;
	proc->stateId = MU_STATE_IDLE_EXEC;

	proc->xSubPosition = (x * 16) << MU_SUBPIXEL_PRECISION;
	proc->ySubPosition = (y * 16) << MU_SUBPIXEL_PRECISION;

	proc->xSubOffset = 0;
	proc->ySubOffset = 0;

	proc->directionId_maybe = 11;

	proc->moveTimer = 0;
	proc->stepSoundTimer = otherThing;

	proc->displayedClassId = classIndex;
	proc->boolIsHidden = 0;

	proc->vramPtr_maybe = OBJ_VRAM0 + (0x20 * objTileId);

	proc->muIndex = thing;

	proc->objPriorityBits = 0x800;

	proc->moveSpeedConfig = 0;
	proc->boolForceMaxSpeed = FALSE;

	config->paletteIndex = palId;

	ap = AP_Create(MMS_GetROMTCS(classIndex), 10);
	AP_SwitchAnimation(ap, 4);

	CopyDataWithPossibleUncomp(
		GetMovingMapSpriteGfxPtrFromMOVEUNIT(proc),
		GetMOVEUNITGraphicsBuffer(config->muIndex)
	);

	ap->pGraphics = GetMOVEUNITGraphicsBuffer(config->muIndex);
	ap->tileBase = ((config->paletteIndex & 0xF) << 12) + config->objTileIndex + proc->objPriorityBits;

	proc->pAPHandle = ap;
	proc->pMUConfig = config;

	config->pMUProc = proc;

	return proc;
}

void MOVEUNIT6C_SetSpriteDirection(struct MUProc* proc, int directionId) {
	proc->directionId_maybe = directionId;

	if (directionId == 15)
		sub_8027068(proc->muIndex, proc->vramPtr_maybe);
	else
		AP_SwitchAnimation(proc->pAPHandle, proc->directionId_maybe);
}

void MOVEUNIT6C_SetDefaultSpriteDirection(struct MUProc* proc) {
	if (GetROMClassStruct(proc->displayedClassId)->attributes & CA_MOUNTEDAID)
		MOVEUNIT6C_SetSpriteDirection(proc, 1);
	else
		MOVEUNIT6C_SetSpriteDirection(proc, 2);
}

void _MOVEUNIT6C_SetDefaultFacingDirection(void) {
	struct MUProc* proc = (struct MUProc*) Proc_Find(gUnknown_089A2C48);

	if (proc)
		MOVEUNIT6C_SetDefaultSpriteDirection(proc);
}

void _MOVEUNIT6C_ChangeFutureMovement(const u8 commands[MU_COMMAND_MAX_COUNT]) {
	struct MUProc* proc = (struct MUProc*) Proc_Find(gUnknown_089A2C48);

	if (proc)
		MOVEUNIT6C_ChangeFutureMovement(proc, commands);
}

int DoesMoveunitExist(void) {
	return Proc_Find(gUnknown_089A2C48) ? TRUE : FALSE;
}

#if NONMATCHING

int IsThereAMovingMoveunit(void) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i)
		if ((sMUConfigArray[i].muIndex) && (sMUConfigArray[i].pMUProc->stateId != MU_STATE_IDLE_EXEC))
			return TRUE;

	return FALSE;
}

#else // NONMATCHING

__attribute__((naked))
int IsThereAMovingMoveunit(void) {
	asm(
		".syntax unified\n"

		"push {lr}\n"
		"movs r3, #0\n"
		"ldr r0, _08078764  @ sMUConfigArray\n"
		"adds r2, r0, #0\n"
		"adds r2, #0x48\n"
		"adds r1, r0, #0\n"
	"_08078744:\n"
		"ldrb r0, [r1]\n"
		"cmp r0, #0\n"
		"beq _08078754\n"
		"ldr r0, [r2]\n"
		"adds r0, #0x3f\n"
		"ldrb r0, [r0]\n"
		"cmp r0, #1\n"
		"bne _08078768\n"
	"_08078754:\n"
		"adds r2, #0x4c\n"
		"adds r1, #0x4c\n"
		"adds r3, #1\n"
		"cmp r3, #3\n"
		"ble _08078744\n"
		"movs r0, #0\n"
		"b _0807876A\n"
		".align 2, 0\n"
	"_08078764: .4byte sMUConfigArray\n"
	"_08078768:\n"
		"movs r0, #1\n"
	"_0807876A:\n"
		"pop {r1}\n"
		"bx r1\n"

		".syntax divided\n"
	);
}

#endif // NONMATCHING

int sub_8078770(struct MUProc* proc) {
	if (proc->pMUConfig->muIndex && proc->stateId != MU_STATE_IDLE_EXEC)
		return TRUE;

	return FALSE;
}

void MOVEUNIT6C_ChangeFutureMovement(struct MUProc* proc, const u8 commands[MU_COMMAND_MAX_COUNT]) {
	int i;

	for (i = 0; i < 0x40; ++i)
		proc->pMUConfig->commands[i] = commands[i];

	proc->pMUConfig->currentCommand = 0;
	proc->stateId = MU_STATE_MOVING_EXEC;

	__MOVEUNIT6C_PlaySoundStepByClass(proc);
}

struct MUProc* sub_80787C4(u16 x, u16 y, u16 classIndex, unsigned palId, const u8 commands[MU_COMMAND_MAX_COUNT]) {
	struct MUProc* proc = NewMOVEUNIT(x, y, classIndex, -1, palId);

	if (!proc)
		return NULL;

	MOVEUNIT6C_ChangeFutureMovement(proc, commands);
	return proc;
}

static void Init6C_89A2938(struct MUStepSoundProc* proc) {
	proc->u58 = 0;
	proc->u64 = 0;

	proc->u5C = 0;
	proc->u66 = 0;
}

static void Call89A2938_PlaySound1(struct MUStepSoundProc* proc) {
	PlaySpacialSoundMaybe(proc->u58, proc->u64);
}

static void Call89A2938_PlaySound2(struct MUStepSoundProc* proc) {
	if (proc->u5C)
		PlaySpacialSoundMaybe(proc->u5C, proc->u66);
}

void NewSoundStepPlay6C(int soundId, int b, int hPosition) {
	struct MUStepSoundProc* proc;

	proc = (struct MUStepSoundProc*) Proc_Find(gUnknown_089A2938);

	if (!proc)
		proc = (struct MUStepSoundProc*) Proc_Create(gUnknown_089A2938, ROOT_PROC_3);

	if (!proc->u58) {
		proc->u58 = soundId;
		proc->u64 = hPosition;
	} else if (!proc->u60) { // TODO: FIXME: Is this a bug? u60 is never initialized
		proc->u5C = soundId + b;
		proc->u66 = hPosition;
	}
}

void __MOVEUNIT6C_PlaySoundStepByClass(struct MUProc* proc) {
	MOVEUNIT6C_PlaySoundStepByClass(proc);
}

static void nullsub_19(struct MUProc* proc) {}

static void Moveunit_ExecMoveCommand(struct MUProc* proc) {
	while (TRUE) {
		short command = MU_AdvanceGetCommand(proc);

		switch (command) {

		case MU_COMMAND_9:
			proc->moveTimer = MU_AdvanceGetCommand(proc);
			proc->stateId = MU_STATE_WAITING;

			return;

		case MU_COMMAND_10:
			nullsub_19(proc);

			proc->stateId = MU_STATE_WAITING_FOR_SOMETHING_TO_FINISH;

			DisplayFogThingMaybe(
				(proc->xSubPosition >> MU_SUBPIXEL_PRECISION) - gUnknown_0202BCB0.xCameraReal,
				(proc->ySubPosition >> MU_SUBPIXEL_PRECISION) - gUnknown_0202BCB0.yCameraReal
			);

			return;

		case MU_COMMAND_4:
			sub_80790CC(proc);

			return;

		case MU_COMMAND_FF:
			nullsub_19(proc);
			EndMoveunitMaybe(proc);

			return;

		case MU_COMMAND_0:
		case MU_COMMAND_1:
		case MU_COMMAND_2:
		case MU_COMMAND_3:
			command = command - MU_COMMAND_MOVE_BASE;

			if (command != proc->directionId_maybe) {
				MMS_GetROMTCS(proc->displayedClassId); // TODO: FIXME: is this a bug?
				MOVEUNIT6C_SetSpriteDirection(proc, command);

				proc->stateId = MU_STATE_MOVING_EXEC;
			}

			return;

		case MU_COMMAND_5:
		case MU_COMMAND_6:
		case MU_COMMAND_7:
		case MU_COMMAND_8:
			command = command - MU_COMMAND_TURN_BASE;

			if (command != proc->directionId_maybe) {
				MMS_GetROMTCS(proc->displayedClassId); // TODO: FIXME: is this a bug?
				MOVEUNIT6C_SetSpriteDirection(proc, command);
			}

			break;

		case MU_COMMAND_12:
			proc->moveSpeedConfig = MU_AdvanceGetCommand(proc);
			break;

		case MU_COMMAND_13:
			MOVEUNIT6C_SetCameraFollow(proc);
			break;

		case MU_COMMAND_14:
			MOVEUNIT6C_UnsetCameraFollow(proc);
			break;

		} // switch (command)
	} // while (TRUE)
}

static void DisplayFogThingMaybe(int x, int y) {
	struct APHandle* ap;
	struct SomeProc* proc;

	CopyDataWithPossibleUncomp(
		gUnknown_089ADD4C,
		OBJ_VRAM0 + 0x20 * 0x180
	);

	ap = AP_Create(gUnknown_089A8EF8, 2);

	ap->tileBase = 0x1180;
	AP_SwitchAnimation(ap, 0);

	proc = (struct SomeProc*) Proc_Create(gUnknown_089A2968, ROOT_PROC_3);

	proc->pAPHandle = ap;

	proc->xDisplay = x + 8;
	proc->yDisplay = y - 4;
}

static void Call6C_89A2968(struct SomeProc* proc) {
	PlaySoundEffect(0x77);

	proc->u64 = 0;

	// TODO: maybe a macro that takes angle/xScale/yScale?

	WriteOAMRotScaleData(
		0,  // oam rotscale index

		Div(+COS(0) * 16, 0x200), // pa
		Div(-SIN(0) * 16, 0x200), // pb
		Div(+SIN(0) * 16, 0x200), // pc
		Div(+COS(0) * 16, 0x200)  // pd
	);
}

static void Loop6C_89A2968_1(struct SomeProc* proc) {
	int scale;

	if (proc->u64++ >= 8)
		Proc_ClearNativeCallback((struct Proc*) proc);

	scale = sub_8012DCC(5, 0x200, 0x100, proc->u64, 8);

	WriteOAMRotScaleData(
		0,  // oam rotscale index

		Div(+COS(0) * 16, scale), // pa
		Div(-SIN(0) * 16, scale), // pb
		Div(+SIN(0) * 16, scale), // pc
		Div(+COS(0) * 16, scale)  // pd
	);

	AP_Update(
		proc->pAPHandle,

		(proc->xDisplay - 8),
		(proc->yDisplay - 8) | 0x300
	);
}

static void Loop6C_89A2968_2(struct SomeProc* proc) {
	if (proc->u64++ >= 40)
		Proc_ClearNativeCallback((struct Proc*) proc);

	AP_Update(
		proc->pAPHandle,

		(proc->xDisplay),
		(proc->yDisplay) | 0x100
	);
}

u8 Does6C_89A2968Exist(void) {
	return Proc_Find(gUnknown_089A2968) ? TRUE : FALSE;
}

static void MOVU_Call5_WaitForSomething(struct MUProc* proc) {
	if (!Does6C_89A2968Exist())
		proc->stateId = MU_STATE_WAITING;
}

static void MOVU_Call4_SetState2(struct MUProc* proc) {
	proc->stateId = MU_STATE_MOVING_EXEC;
}

static void MOVU_Call3_Wait(struct MUProc* proc) {
	if (proc->moveTimer == 0)
		proc->stateId = MU_STATE_MOVING_EXEC;
	else
		proc->moveTimer--;
}

static void sub_8078C58(struct MUProc* proc) {}

static void nullsub_54(struct MUProc* proc) {}

static void MOVU_Call2_Moving(struct MUProc* proc) {
	unsigned moveSpeed = MOVEUNIT6C_GetMovementSpeed(proc);

	proc->moveTimer = moveSpeed + proc->moveTimer;

	proc->xSubPosition += moveSpeed * gUnknown_089A2988[proc->directionId_maybe * 2 + 0];
	proc->ySubPosition += moveSpeed * gUnknown_089A2988[proc->directionId_maybe * 2 + 1];

	if ((proc->moveTimer / 16) >= 16) {
		proc->moveTimer -= 0x100;

		proc->xSubPosition -= proc->moveTimer * gUnknown_089A2988[proc->directionId_maybe * 2 + 0];
		proc->ySubPosition -= proc->moveTimer * gUnknown_089A2988[proc->directionId_maybe * 2 + 1];

		proc->moveTimer = 0;

		proc->xSubPosition &= ~0xF;
		proc->ySubPosition &= ~0xF;
	}

	if (proc->boolAttractCamera && !Proc_Find(gUnknown_0859A548)) {
		gUnknown_0202BCB0.xCameraReal = GetSomeAdjustedCameraX(proc->xSubPosition >> MU_SUBPIXEL_PRECISION);
		gUnknown_0202BCB0.yCameraReal = GetSomeAdjustedCameraY(proc->ySubPosition >> MU_SUBPIXEL_PRECISION);
	}

	if (!(proc->moveSpeedConfig & 0x80))
		MOVEUNIT6C_PlaySoundStepByClass(proc);
}

static void MOVEUNIT6C_PlaySoundStepByClass(struct MUProc* proc) {
	// TODO: USE CLASS DEFINITIONS

	const u16* pStepSoundDefinition;

	unsigned cursor;
	struct PositionS16 position;

	if (GetROMClassStruct(proc->displayedClassId)->attributes & CA_MOUNTEDAID) {
		switch (proc->displayedClassId) {

		case 0x1F: // CLASS_WYVERNRIDER
		case 0x20: // CLASS_WYVERNRIDER_F
		case 0x21: // CLASS_WYVERNLORD
		case 0x22: // CLASS_WYVERNLORD_F
		case 0x23: // CLASS_WYVERNKNIGHT
		case 0x24: // CLASS_WYVERNKNIGHT_F
			pStepSoundDefinition = gUnknown_089A2A2E;
			break;

		case 0x5F: // CLASS_MOGALL
		case 0x60: // CLASS_ARCHMOGALL
			pStepSoundDefinition = gUnknown_089A2AF6;
			break;

		case 0x48: // CLASS_PEGASUSKNIGHT
		case 0x49: // CLASS_FALCOKNIGHT
			pStepSoundDefinition = gUnknown_089A2A5A;
			break;

		default: // Any other mounted class
			pStepSoundDefinition = gUnknown_089A2A00;
			break;

		} // proc->displayedClassId
	} else {
		switch (proc->displayedClassId) {

		case 0x52: // CLASS_REVENANT
		case 0x53: // CLASS_ENTOUMBED
			pStepSoundDefinition = gUnknown_089A2AB2;
			break;

		case 0x54: // CLASS_BONEWALKER
		case 0x55: // CLASS_BONEWALKER_BOW
		case 0x56: // CLASS_WIGHT
		case 0x57: // CLASS_WIGHT_BOW
			pStepSoundDefinition = gUnknown_089A2AD4;
			break;

		case 0x58: // CLASS_BAEL
		case 0x59: // CLASS_ELDERBAEL
			pStepSoundDefinition = gUnknown_089A2B22;
			break;

		case 0x5B: // CLASS_MAUTHEDOOG
		case 0x5C: // CLASS_GWYLLGI
			pStepSoundDefinition = gUnknown_089A2B3A;
			break;

		case 0x5D: // CLASS_TARVOS
		case 0x5E: // CLASS_MAELDUIN
			pStepSoundDefinition = gUnknown_089A2A00;
			break;

		case 0x5F: // CLASS_MOGALL
		case 0x60: // CLASS_ARCHMOGALL
			pStepSoundDefinition = gUnknown_089A2AF6;
			break;

		case 0x61: // CLASS_GORGON
			pStepSoundDefinition = gUnknown_089A2B68;
			break;

		case 0x63: // CLASS_GARGOYLE
		case 0x64: // CLASS_DEATHGOYLE
			pStepSoundDefinition = gUnknown_089A2A2E;
			break;

		case 0x09: // CLASS_ARMORKNIGHT
		case 0x0A: // CLASS_ARMORKNIGHT_F
		case 0x0B: // CLASS_GENERAL
		case 0x0C: // CLASS_GENERAL_F
		case 0x3B: // CLASS_MANAKETE?
		case 0x5A: // CLASS_CYCLOPS
		case 0x65: // CLASS_DRACOZOMBIE
		case 0x66: // CLASS_DEMONKING
		case 0x67: // CLASS_BALLISTA
		case 0x68: // CLASS_IRONBALLISTA
		case 0x69: // CLASS_KILLERBALLISTA
			pStepSoundDefinition = gUnknown_089A29BC;
			break;

		case 0x50: // CLASS_FLEET
			pStepSoundDefinition = gUnknown_089A2BCE;
			break;

		case 0x3C: // CLASS_MYRRHMANAKETE
			pStepSoundDefinition = gUnknown_089A2C02;
			break;

		case 0x78: // CLASS_FALLENPRINCE
		case 0x7B: // CLASS_FALLENPEER
			return; // no sounds

		default: // Any other non-mounted class
			pStepSoundDefinition = gUnknown_089A2998;
			break;

		} // switch (proc->displayedClassId)
	}

	cursor = DivRem(proc->stepSoundTimer++, pStepSoundDefinition[0]);
	GetMOVEUNITDisplayPosition(proc, &position);

	if (pStepSoundDefinition[2 + cursor]) {
		NewSoundStepPlay6C(
			pStepSoundDefinition[2 + cursor], // sound id
			pStepSoundDefinition[1], // something
			position.x // horizontal position
		);
	}
}

static void _6CMOVEUNIT_Loop(struct MUProc* proc) {
	if (proc->stateId) {
		if (proc->moveTimer == 0 && (u8)(proc->stateId - MU_STATE_MOVING_EXEC) < 2)
			Moveunit_ExecMoveCommand(proc);

		gUnknown_089A2C28[proc->stateId](proc);
	}

	if (proc->directionId_maybe == 0xF)
		UpdateMOVEUNITGfx_Idle(proc);
	else
		UpdateMOVEUNITGfx_Movement(proc);
}

static void _6CMOVEUNIT_Destructor(struct MUProc* proc) {
	proc->pMUConfig->muIndex = 0;
	AP_Delete(proc->pAPHandle);
}

void ClearMOVEUNITs(void) {
	Proc_DeleteAllWithScript(gUnknown_089A2C48);
}

void EndMoveunitMaybe(struct MUProc* proc) {
	Delete6C__(proc);
}

static void Delete6C__(struct MUProc* proc) {
	Proc_Delete((struct Proc*) proc);
}

static void sub_80790CC(struct MUProc* proc) {
	nullsub_19(proc);
	proc->stateId = MU_STATE_IDLE_EXEC;
}

void BlockAll6CMarked4(void) {
	Proc_BlockEachWithMark(4);
}

void UnblockAll6CMarked4(void) {
	Proc_UnblockEachWithMark(4);
}

void sub_80790F8(int* xOut, int* yOut, const u8* commands) {
	while (TRUE) {
		switch (*commands++) {

		case MU_COMMAND_FF:
			return;

		case MU_COMMAND_0:
			(*xOut)--;
			break;

		case MU_COMMAND_1:
			(*xOut)++;
			break;

		case MU_COMMAND_3:
			(*yOut)--;
			break;

		case MU_COMMAND_2:
			(*yOut)++;
			break;

		case MU_COMMAND_4:
			return;

		case MU_COMMAND_9:
			commands++;
			break;

		}
	}
}

int IsSomeMOVEUNITRelatedStructAvailable(void) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i)
		if (sMUConfigArray[i].muIndex == 0)
			return TRUE;

	return FALSE;
}

void ResetAllMoveunitAnims(void) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i) {
		if (sMUConfigArray[i].muIndex) {
			sMUConfigArray[i].pMUProc->pAPHandle->frameTimer    = 0;
			sMUConfigArray[i].pMUProc->pAPHandle->frameInterval = 0x100;
		}
	}
}

static struct MUConfig* GetNextMoveunitEntryStruct(int objTileId, u8* outIndex_maybe) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i) {
		if (sMUConfigArray[i].muIndex)
			continue;

		sMUConfigArray[i].muIndex = i + 1;
		sMUConfigArray[i].objTileIndex = gUnknown_089A2C68[i] + objTileId;

		*outIndex_maybe = i;

		return sMUConfigArray + i;
	}

	return NULL;
}

static struct MUConfig* sub_807920C(int objTileId, u8* outIndex_maybe) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i) {
		if (sMUConfigArray[i].muIndex)
			continue;

		sMUConfigArray[i].muIndex = i + 1;
		sMUConfigArray[i].objTileIndex = gUnknown_089A2C70[i] + objTileId;

		*outIndex_maybe = i;

		return sMUConfigArray + i;
	}

	return NULL;
}

u8 GetMOVEUNITDisplayPosition(struct MUProc* proc, struct PositionS16* out) {
	if (proc->stateId == MU_STATE_UI_DISPLAY) {
		out->x = (proc->xSubPosition + proc->xSubOffset) >> MU_SUBPIXEL_PRECISION;
		out->y = (proc->ySubPosition + proc->ySubOffset) >> MU_SUBPIXEL_PRECISION;
	} else {
		short x = ((proc->xSubPosition + proc->xSubOffset) >> MU_SUBPIXEL_PRECISION) - gUnknown_0202BCB0.xCameraReal + 8;
		short y = ((proc->ySubPosition + proc->ySubOffset) >> MU_SUBPIXEL_PRECISION) - gUnknown_0202BCB0.yCameraReal + 8;

		out->x = x;
		out->y = y + 8;

		if ((u16)(x + 16) > 0x110)
			return FALSE;

		if (y < -0x10)
			return FALSE;

		if (y > 0xB0)
			return FALSE;
	}

	return TRUE;
}

static void UpdateMOVEUNITGfx_Idle(struct MUProc* proc) {
	if (!proc->boolIsHidden) {
		struct PositionS16 position;

		if (!GetMOVEUNITDisplayPosition(proc, &position))
			return;

		position.x &= 0x01FF;
		position.y &= 0x00FF;

		if (proc->stateId == MU_STATE_7)
			position.y |= 0x400;

		sub_8026FF4(
			proc->muIndex,
			proc->vramPtr_maybe
		);

		sub_8027DB4(
			proc->pAPHandle->objLayer,

			position.x - 8,
			position.y - 16,

			((((unsigned)(proc->vramPtr_maybe - OBJ_VRAM0) & 0x1FFFF) >> 5)
				| ((proc->pMUConfig->paletteIndex & 0xF) << 12))
				+ proc->objPriorityBits,

			proc->displayedClassId,
			proc->muIndex
		);
	}
}

static void UpdateMOVEUNITGfx_Movement(struct MUProc* proc) {
	if (!proc->boolIsHidden) {
		struct PositionS16 position;

		if (!GetMOVEUNITDisplayPosition(proc, &position))
			return;

		position.x &= 0x01FF;
		position.y &= 0x00FF;

		if (proc->stateId != MU_STATE_UI_DISPLAY)
			if (proc->pUnit && (proc->pUnit->index & 0xC0) == 0x80) // TODO: UNIT ALLEGIANCE DEFINITIONS
				if (gUnknown_0202BCF0.chapterVisionRange != 0)
					if (!gUnknown_0202E4E8[MU_GetDisplayYOrg(proc) >> 4][MU_GetDisplayXOrg(proc) >> 4])
						return; // whew

		if (proc->stateId == MU_STATE_7)
			position.y |= 0x400;

		AP_Update(
			proc->pAPHandle,

			position.x,
			position.y
		);
	}
}

static u16 MOVEUNIT6C_GetMovementSpeed(struct MUProc* proc) {
	int config = proc->moveSpeedConfig;

	if (config & 0x80)
		config += 0x80; // I don't really get that one

	if (proc->boolForceMaxSpeed)
		return 0x100;

	if (config != 0x40) {
		if (config != 0x00) {
			int speed = config;

			if (speed & 0x40)
				speed ^= 0x40;
			else if (gUnknown_0202BCF0.unk40_8 || (gKeyStatusPtr->heldKeys & A_BUTTON))
				speed *= 4;

			if (speed > 0x80)
				speed = 0x80;

			return speed;
		}

		if (!sub_8030CC0() && (gKeyStatusPtr->heldKeys & A_BUTTON))
			return 0x80;

		if (gUnknown_0202BCF0.unk40_8)
			return 0x40;
	}

	return 16 * gUnknown_089A2C78[GetROMClassStruct(proc->displayedClassId)->slowWalking];
}

void sub_807953C(struct MUProc* proc, u16 config) {
	if (config > 0x100)
		proc->moveSpeedConfig = 0x100;
	else
		proc->moveSpeedConfig = config;
}

static void* GetMOVEUNITGraphicsBuffer(int muIndex) {
	return gUnknown_02004BE0 + (gUnknown_089A2C7A[muIndex] * MU_GFX_MAX_SIZE);
}

static const void* GetMovingMapSpriteGfxPtrFromMOVEUNIT(struct MUProc* proc) {
	return gUnknown_089A2E00[proc->displayedClassId - 1].pGraphics;
}

static const void* MMS_GetROMTCS(u16 classId) {
	return gUnknown_089A2E00[classId - 1].pAnimation;
}

void MOVEUNIT6C_807959C(struct MUProc* muProc) {
	struct MUEffectProc* proc;

	muProc->stateId = MU_STATE_7;

	proc = (struct MUEffectProc*) Proc_Create(gUnknown_089A2C80, (struct Proc*) muProc);

	proc->pMUProc = muProc;
	proc->timeLeft = 0x20;

	SetSpecialColorEffectsParameters(0, 0x10, 0x10, 0);

	muProc->pAPHandle->frameTimer = 0;
	muProc->pAPHandle->frameInterval = 0;

	sub_8079A74(muProc, 0);

	muProc->pAPHandle->objLayer = 0xD;

	PlaySoundEffect(0xD6); // TODO: SOUND DEFINITIONS

	if (muProc->pUnit->state & US_IN_BALLISTA) {
		TryRemoveUnitFromBallista(muProc->pUnit);
		HideUnitSMS(muProc->pUnit);
	}
}

static void BlendTimer6C_MainLoop(struct MUEffectProc* proc) {
	short time = (proc->timeLeft--) >> 1;

	SetSpecialColorEffectsParameters(0, (u8) time, 0x10, 0);

	if (proc->timeLeft == 0) {
		EndMoveunitMaybe(proc->pMUProc);
		Proc_ClearNativeCallback((struct Proc*) proc);
	}
}

static void sub_8079654(struct MUEffectProc* proc) {
	struct MUProc* muProc = (struct MUProc*) proc->parent;
	int boolHidden = FALSE;

	if ((proc->timeLeft & 0x7) < 4)
		boolHidden = TRUE;

	muProc->boolIsHidden = boolHidden;

	if (--proc->timeLeft < 0) {
		Proc_ClearNativeCallback((struct Proc*) proc);
		muProc->boolIsHidden = TRUE;
	}
}

void sub_807968C(struct MUProc* muProc) {
	struct MUEffectProc* proc;

	muProc->stateId = MU_STATE_7;

	proc = (struct MUEffectProc*) Proc_Create(gUnknown_089A2C98, (struct Proc*) muProc);

	proc->pMUProc = muProc;
	proc->timeLeft = 0x40;

	muProc->pAPHandle->frameTimer = 0;
	muProc->pAPHandle->frameInterval = 0;

	PlaySoundEffect(0xD6); // TODO: SOUND DEFINITIONS
}

static void sub_80796D4(u32* data, int frame) {
	static u32 sKeptPixelsWordMask;
	static u32 sClearedPixelWordMask;

	int i, j;

	int pixel = gUnknown_089A2CA8[frame] % 8;
	int wordId = gUnknown_089A2CA8[frame] / 8;

	sKeptPixelsWordMask = ~(sClearedPixelWordMask = (0xF << (pixel * 4)));

	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			data[wordId] &= sKeptPixelsWordMask;

			data += 8;
		}

		data += 0xE0;
	}
}

static void sub_8079730(struct MUEffectProc* proc) {
	sub_80796D4(
		GetMOVEUNITGraphicsBuffer(((struct MUProc*)(proc->parent))->muIndex),
		proc->frameIndex
	);

	proc->frameIndex++;

	// TODO: FIXME: This may be bugged?
	RegisterTileGraphics(
		gUnknown_02004BE0,
		OBJ_VRAM0 + (MU_BASE_OBJ_TILE * 0x20),
		(0x80 * 0x20)
	);

	if (--proc->timeLeft == 0) {
		EndMoveunitMaybe(proc->pMUProc);
		Proc_ClearNativeCallback((struct Proc*) proc);
	}
}

void sub_8079788(struct MUProc* muProc) {
	struct MUEffectProc* proc;

	muProc->stateId = MU_STATE_7;

	proc = (struct MUEffectProc*) Proc_Create(gUnknown_089A2CE8, (struct Proc*) muProc);

	proc->pMUProc = muProc;

	proc->timeLeft = 0x40;
	proc->frameIndex = 0;

	muProc->pAPHandle->frameTimer = 0;
	muProc->pAPHandle->frameInterval = 0;

	PlaySoundEffect(0xD6); // TODO: SOUND DEFINITIONS
}

void SetMOVEUNITField40To1(struct MUProc* proc) {
	proc->boolIsHidden = TRUE;
}

void sub_80797DC(struct MUProc* proc) {
	proc->boolIsHidden = FALSE;
}

void MOVEUNIT6C_SetDisplayPos(struct MUProc* proc, int x, int y) {
	proc->xSubPosition = x << MU_SUBPIXEL_PRECISION;
	proc->ySubPosition = y << MU_SUBPIXEL_PRECISION;
}

void sub_80797F4(struct MUProc* proc, int xOff, int yOff) {
	proc->xSubOffset = xOff << MU_SUBPIXEL_PRECISION;
	proc->ySubOffset = yOff << MU_SUBPIXEL_PRECISION;
}

void sub_8079804(struct MUProc* proc, int flashType) {
	proc->pAPHandle->tileBase =
		proc->pMUConfig->objTileIndex + (MU_FADE_OBJ_PAL << 12) + proc->objPriorityBits;

	CopyToPaletteBuffer(
		gPaletteBuffer + (0x10 * (0x10 + proc->pMUConfig->paletteIndex)),
		(0x10 + MU_FADE_OBJ_PAL) * 0x20, 0x20
	);

	sub_8013928(
		gUnknown_089A2920[flashType],
		0x15, 8, (struct Proc*) proc
	);
}

void sub_8079858(struct MUProc* muProc) {
	struct MUEffectProc* proc;

	sub_8013928(
		gPaletteBuffer + (0x10 * (0x10 + muProc->pMUConfig->paletteIndex)),
		0x15, 8, (struct Proc*) muProc
	);

	proc = (struct MUEffectProc*) Proc_Create(gUnknown_089A2CF8, ROOT_PROC_3);

	proc->pMUProc = muProc;
}

static void sub_807988C(struct MUEffectProc* proc) {
	struct MUProc* muProc = proc->pMUProc;

	muProc->pAPHandle->tileBase =
		((muProc->pMUConfig->paletteIndex & 0xF) << 12) + muProc->pMUConfig->objTileIndex + muProc->objPriorityBits;
}

void SetupSomeMoveunitAnim(struct MUProc* proc) {
	AP_SwitchAnimation(proc->pAPHandle, 4); // TODO: MU_ANIM_SELECTED

	proc->pAPHandle->frameTimer    = 0;
	proc->pAPHandle->frameInterval = 0x100;

	SetupFutureCall(TCS_HaltAnim2, (int) proc->pAPHandle, 30);
}

static void TCS_HaltAnim2(int argAp) {
	struct APHandle* ap = (struct APHandle*) argAp;

	ap->frameTimer    = 0;
	ap->frameInterval = 0;
}

void sub_80798E8(struct MUProc* proc) {
	proc->pAPHandle->frameTimer    = 0;
	proc->pAPHandle->frameInterval = 0x100;

	SetupFutureCall(sub_807990C, (int) proc->pAPHandle, 30);
}

static void sub_807990C(int argAp) {
	struct APHandle* ap = (struct APHandle*) argAp;

	int actor1 = gUnknown_0203E1F0.subjectActorId;
	int actor2 = 1 - actor1;

	SetBattleAnimFacing(
		actor1, actor2,
		GetSpellAssocFacing(gUnknown_0203E1F0.actors[0].pBattleUnit->weaponBefore)
	);

	ap->frameTimer    = 0;
	ap->frameInterval = 0;
}

void MOVEUNIT_Begin20FramesFastAnim(struct MUProc* proc) {
	proc->pAPHandle->frameTimer    = 0;
	proc->pAPHandle->frameInterval = 0x40;

	SetupFutureCall(TCS_HaltAnim, (int) proc->pAPHandle, 20);
}

static void TCS_HaltAnim(int argAp) {
	struct APHandle* ap = (struct APHandle*) argAp;

	ap->frameTimer    = 0;
	ap->frameInterval = 0;
}

void sub_8079970(struct MUProc* muProc, int flashType) {
	struct MUFadeEffectProc* proc;

	CopyToPaletteBuffer(
		gUnknown_089A2920[flashType],
		(0x10 + MU_FADE_OBJ_PAL) * 0x20, 0x20
	);

	proc = (struct MUFadeEffectProc*) Proc_Create(gUnknown_089A2D10, (struct Proc*) muProc);

	proc->pMUProc = muProc;
}

static void sub_80799A0(struct MUFadeEffectProc* proc) {
	proc->u30 = 0;
}

static void sub_80799A8(struct MUFadeEffectProc* proc) {
	struct MUProc* muProc = proc->pMUProc;

	muProc->pAPHandle->tileBase =
		(MU_FADE_OBJ_PAL << 12) + muProc->pMUConfig->objTileIndex + muProc->objPriorityBits;
}

static void sub_80799C8(struct MUFadeEffectProc* proc) {
	struct MUProc* muProc = proc->pMUProc;

	muProc->pAPHandle->tileBase =
		((muProc->pMUConfig->paletteIndex & 0xF) << 12) + muProc->pMUConfig->objTileIndex + muProc->objPriorityBits;
}

static void sub_80799EC(struct MUFadeEffectProc* proc) {
	sub_8013928(
		gPaletteBuffer + 0x10 * (0x10 + proc->pMUProc->pMUConfig->paletteIndex),
		0x15, 0x14, (struct Proc*) proc
	);
}

static void sub_8079A10(struct MUFadeEffectProc* proc) {
	proc->u30++;

	sub_80797F4(proc->pMUProc, ((proc->u30 & 1) ? 2 : -2), 0);

	if (proc->u30 >= 12) {
		sub_80797F4(proc->pMUProc, 0, 0);
		Proc_ClearNativeCallback((struct Proc*) proc);
	}
}

static void sub_8079A50(struct MUFadeEffectProc* proc) {
	struct MUProc* muProc = proc->pMUProc;

	muProc->pAPHandle->tileBase =
		((muProc->pMUConfig->paletteIndex & 0xF) << 12) + muProc->pMUConfig->objTileIndex + muProc->objPriorityBits;
}

void sub_8079A74(struct MUProc* muProc, int flashType) {
	struct MUFadeEffectProc* proc;

	CopyToPaletteBuffer(
		gUnknown_089A2920[flashType],
		(0x10 + MU_FADE_OBJ_PAL) * 0x20, 0x20
	);

	muProc->pAPHandle->tileBase =
		(MU_FADE_OBJ_PAL << 12) + muProc->pMUConfig->objTileIndex + muProc->objPriorityBits;

	sub_8013928(
		gPaletteBuffer + 0x10 * (0x10 + muProc->pMUConfig->paletteIndex),
		0x15, 0x14, (struct Proc*) muProc
	);

	proc = (struct MUFadeEffectProc*) Proc_Create(gUnknown_089A2D98, (struct Proc*) muProc);

	proc->pMUProc = muProc;
}

static void sub_8079AD4(struct MUFadeEffectProc* proc) {
	struct MUProc* muProc = proc->pMUProc;

	muProc->pAPHandle->tileBase =
		((muProc->pMUConfig->paletteIndex & 0xF) << 12) + muProc->pMUConfig->objTileIndex + muProc->objPriorityBits;
}

void SetAllMOVEUNITField44To1(void) {
	Proc_ForEachWithScript(gUnknown_089A2C48, SetMOVEUNITField44To1);
}

static void SetMOVEUNITField44To1(struct Proc* proc) {
	((struct MUProc*)(proc))->boolForceMaxSpeed = TRUE;
}

void ChangeMOVEUNITDataMaybe(struct MUProc* proc, int displayedClassId, const u16* palette) {
	proc->pAPHandle->frameTimer = 0;
	proc->pAPHandle->frameInterval = 0;

	proc->displayedClassId = displayedClassId;

	AP_SetDefinition(
		proc->pAPHandle,
		MMS_GetROMTCS(proc->displayedClassId)
	);

	CopyDataWithPossibleUncomp(
		GetMovingMapSpriteGfxPtrFromMOVEUNIT(proc),
		GetMOVEUNITGraphicsBuffer(proc->pMUConfig->muIndex)
	);

	CopyToPaletteBuffer(palette, (0x20 * (0x10 + proc->pMUConfig->paletteIndex)), 0x20);
}

#if NONMATCHING

void sub_8079B6C(struct MUProc* proc, int paletteId) {
	proc->pMUConfig->paletteIndex = paletteId;

	proc->pAPHandle->tileBase =
		((proc->pMUConfig->paletteIndex & 0xF) << 12) + proc->pMUConfig->objTileIndex + proc->objPriorityBits;
}

#else // NONMATCHING

__attribute__((naked))
void sub_8079B6C(struct MUProc* proc, int paletteId) {
	asm(
		".syntax unified\n"

		"push {r4, lr}\n"
		"ldr r2, [r0, #0x34]\n"
		"strb r1, [r2, #1]\n"
		"ldr r4, [r0, #0x30]\n"
		"ldr r3, [r0, #0x34]\n"
		"movs r2, #0xf\n"
		"ands r2, r1\n"
		"lsls r2, r2, #0xc\n"
		"ldrh r3, [r3, #2]\n"
		"adds r2, r2, r3\n"
		"adds r0, #0x46\n"
		"ldrh r0, [r0]\n"
		"adds r0, r0, r2\n"
		"strh r0, [r4, #0x22]\n"
		"pop {r4}\n"
		"pop {r0}\n"
		"bx r0\n"

		".syntax divided\n"
	);
}

#endif // NONMATCHING

static struct MUProc* GetMoveunitByIndex(int muIndex) {
	if (!sMUConfigArray[muIndex].muIndex)
		return NULL;

	return sMUConfigArray[muIndex].pMUProc;
}

struct MUProc* GetExistingMoveunitForUnit(struct Unit* unit) {
	int i;

	for (i = 0; i < MU_MAX_COUNT; ++i) {
		struct MUProc* proc = GetMoveunitByIndex(i);

		if (proc->pUnit == unit)
			return proc;
	}

	return NULL;
}

void sub_8079BE0(void) {
	struct MUProc* procs[MU_MAX_COUNT];

	s8 i, j;
	s8 count;

	// Clear proc list
	CpuFill32(0, procs, MU_MAX_COUNT * sizeof(struct MUProc*));
	count = 0;

	// Building proc list
	for (i = 0; i < MU_MAX_COUNT; ++i) {
		struct MUProc* proc = GetMoveunitByIndex(i);

		if (proc) {
			procs[count] = proc;
			count++;
		}
	}

	// Sorting proc list
	for (i = 0; i < (count - 1); ++i) {
		for (j = (i + 1); j < count; ++j) {
			if (procs[j]->ySubPosition < procs[i]->ySubPosition) {
				struct MUProc* tmp = procs[i];

				procs[i] = procs[j];
				procs[j] = tmp;
			}
		}
	}

	// Set obj layer based on order
	for (i = 0; i < count; ++i)
		procs[i]->pAPHandle->objLayer = 10 - i;
}

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_07.h"
#include "z64shrink_window.h"
#include "attributes.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_Obj_Tsubo/z_obj_tsubo.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"

#define MAJORA_TENTACLE_COUNT 30
#define MAJORA_WHIP_LENGTH 55
#define MAJORA_EFFECT_COUNT 60

typedef enum MajoraEffectType {
    /* 0 */ MAJORA_EFFECT_NONE,
    /* 1 */ MAJORA_EFFECT_FLAME
} MajoraEffectType;

typedef enum MajorasWrathAttackSubAction {
    /* 0 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_QUICK_WHIP,
    /* 1 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_FLURRY,
    /* 2 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_DOUBLE_WHIP,
    /* 3 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_LONG_WHIP,
    /* 4 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_SPIN_ATTACK,
    /* 5 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_TAUNT,
    /* 6 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_THREE_HIT,
    /* 7 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_WHIP_ATTACK_MAX,
    /* 7 */ MAJORAS_WRATH_ATTACK_SUB_ACTION_KICK = 7,
} MajorasWrathAttackSubAction;

void Boss07_Init(Actor* thisx, PlayState* play2);
void Boss07_Destroy(Actor* thisx, PlayState* play2);

void Boss07_Wrath_Update(Actor* thisx, PlayState* play2);
void Boss07_IncarnationAfterimage_Update(Actor* thisx, PlayState* play2);
void Boss07_Incarnation_Update(Actor* thisx, PlayState* play2);
void Boss07_Mask_Update(Actor* thisx, PlayState* play2);
void Boss07_Projectile_Update(Actor* thisx, PlayState* play2);
void Boss07_Remains_Update(Actor* thisx, PlayState* play2);
void Boss07_Top_Update(Actor* thisx, PlayState* play2);
void Boss07_BattleHandler_Update(Actor* thisx, PlayState* play2);

void Boss07_Wrath_Draw(Actor* thisx, PlayState* play2);
void Boss07_IncarnationAfterimage_Draw(Actor* thisx, PlayState* play2);
void Boss07_Incarnation_Draw(Actor* thisx, PlayState* play2);
void Boss07_Mask_Draw(Actor* thisx, PlayState* play2);
void Boss07_Projectile_Draw(Actor* thisx, PlayState* play2);
void Boss07_Remains_Draw(Actor* thisx, PlayState* play2);
void Boss07_Top_Draw(Actor* thisx, PlayState* play2);
void Boss07_BattleHandler_Draw(Actor* thisx, PlayState* play2);

void Boss07_Wrath_SetupIntroCutscene(Boss07* this, PlayState* play);
void Boss07_Wrath_IntroCutscene(Boss07* this, PlayState* play);
void Boss07_Wrath_DeathCutscene(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupIdle(Boss07* this, PlayState* play, s16 idleTimer);
void Boss07_Wrath_Idle(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupJump(Boss07* this, PlayState* play);
void Boss07_Wrath_StartJump(Boss07* this, PlayState* play);
void Boss07_Wrath_Jump(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupFlip(Boss07* this, PlayState* play);
void Boss07_Wrath_Flip(Boss07* this, PlayState* play);
void Boss07_Wrath_Sidestep(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupAttack(Boss07* this, PlayState* play);
void Boss07_Wrath_Attack(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupTryGrab(Boss07* this, PlayState* play);
void Boss07_Wrath_TryGrab(Boss07* this, PlayState* play);
void Boss07_Wrath_ThrowPlayer(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupShock(Boss07* this, PlayState* play);
void Boss07_Wrath_ShockWhip(Boss07* this, PlayState* play);
void Boss07_Wrath_ShockStunned(Boss07* this, PlayState* play);
void Boss07_Wrath_SetupThrowTop(Boss07* this, PlayState* play);
void Boss07_Wrath_ThrowTop(Boss07* this, PlayState* play);
void Boss07_Wrath_Stunned(Boss07* this, PlayState* play);
void Boss07_Wrath_Damaged(Boss07* this, PlayState* play);
void Boss07_Wrath_GenShadowTex(u8* tex, Boss07* this, PlayState* play);
void Boss07_Wrath_DrawShadowTex(u8* tex, Boss07* this, PlayState* play);

void Boss07_Incarnation_SetupIntroCutscene(Boss07* this, PlayState* play);
void Boss07_Incarnation_IntroCutscene(Boss07* this, PlayState* play);
void Boss07_Incarnation_SetupTaunt(Boss07* this, PlayState* play);
void Boss07_Incarnation_Taunt(Boss07* this, PlayState* play);
void Boss07_Incarnation_Stunned(Boss07* this, PlayState* play);
void Boss07_Incarnation_Damaged(Boss07* this, PlayState* play);
void Boss07_Incarnation_SetupRun(Boss07* this, PlayState* play);
void Boss07_Incarnation_Run(Boss07* this, PlayState* play);
void Boss07_Incarnation_SetupAttack(Boss07* this, PlayState* play);
void Boss07_Incarnation_Attack(Boss07* this, PlayState* play);
void Boss07_Incarnation_SetupSquattingDance(Boss07* this, PlayState* play);
void Boss07_Incarnation_SquattingDance(Boss07* this, PlayState* play);
void Boss07_Incarnation_SetupMoonwalk(Boss07* this, PlayState* play);
void Boss07_Incarnation_Moonwalk(Boss07* this, PlayState* play);
void Boss07_Incarnation_SetupPirouette(Boss07* this, PlayState* play);
void Boss07_Incarnation_Pirouette(Boss07* this, PlayState* play);
void Boss07_Incarnation_DeathCutscene(Boss07* this, PlayState* play);

void Boss07_Mask_SetupIdle(Boss07* this, PlayState* play);
void Boss07_Mask_Idle(Boss07* this, PlayState* play);
void Boss07_Mask_SetupSpinAttack(Boss07* this, PlayState* play);
void Boss07_Mask_SpinAttack(Boss07* this, PlayState* play);
void Boss07_Mask_Stunned(Boss07* this, PlayState* play);
void Boss07_Mask_Damaged(Boss07* this, PlayState* play);
void Boss07_Mask_SetupFireBeam(Boss07* this, PlayState* play);
void Boss07_Mask_FireBeam(Boss07* this, PlayState* play);
void Boss07_Mask_SetupIntroCutscene(Boss07* this, PlayState* play);
void Boss07_Mask_IntroCutscene(Boss07* this, PlayState* play);
void Boss07_Mask_SetupDeathCutscene(Boss07* this, PlayState* play);
void Boss07_Mask_DeathCutscene(Boss07* this, PlayState* play);

void Boss07_Remains_SetupIntroCutscene(Boss07* this, PlayState* play);
void Boss07_Remains_IntroCutscene(Boss07* this, PlayState* play);
void Boss07_Remains_SetupMove(Boss07* this, PlayState* play);
void Boss07_Remains_Move(Boss07* this, PlayState* play);
void Boss07_Remains_SetupStunned(Boss07* this, PlayState* play);
void Boss07_Remains_Stunned(Boss07* this, PlayState* play);

void Boss07_Top_SetupThrown(Boss07* this, PlayState* play);
void Boss07_Top_Thrown(Boss07* this, PlayState* play);
void Boss07_Top_SetupMove(Boss07* this, PlayState* play);
void Boss07_Top_Move(Boss07* this, PlayState* play);

void Boss07_BattleHandler_UpdateEffects(PlayState* play);
void Boss07_BattleHandler_DrawEffects(PlayState* play);

extern Boss07* sMajorasWrath;
extern Boss07* sMajoraBattleHandler;
extern Boss07* sMajorasMask;
extern Boss07* sMajoraRemains[MAJORA_REMAINS_TYPE_MAX];
extern void Boss07_Mask_SetupStunned(Boss07*, PlayState*);

extern u8 sKillAllProjectiles;
extern u8 sMusicStartTimer;

extern void Boss07_SmoothStop(Boss07*, f32);

typedef struct MajoraEffect {
    /* 0x00 */ u8 type;
    /* 0x02 */ s16 texScroll;
    /* 0x04 */ Vec3f pos;
    /* 0x10 */ Vec3f velocity;
    /* 0x1C */ Vec3f accel;
    /* 0x28 */ UNK_TYPE1 unk28[4];
    /* 0x2C */ s16 alpha;
    /* 0x2E */ UNK_TYPE1 unk2E[2];
    /* 0x30 */ s16 isFadingAway;
    /* 0x34 */ f32 scale;
    /* 0x38 */ UNK_TYPE1 unk38[0x10];
} MajoraEffect; // size = 0x48

extern MajoraEffect sMajoraEffects[MAJORA_EFFECT_COUNT];

extern ColliderJntSphInit sIncarnationBodyColliderJntSphInit;
extern ColliderJntSphInit sWrathKickColliderJntSphInit;
extern ColliderJntSphInit sWrathBodyColliderJntSphInit;
extern ColliderQuadInit sMaskBackQuadInit;
extern ColliderQuadInit sMaskFrontQuadInit;
extern DamageTable sRemainsDamageTable;
extern DamageTable sTopDamageTable;

typedef enum MajorasMaskDamageEffect {
    // Named because everything with this effect is ignored thanks to `CollisionCheck_SetATvsAC`.
    /* 0x0 */ MAJORAS_MASK_DMGEFF_IMMUNE,
    /* 0x2 */ MAJORAS_MASK_DMGEFF_FIRE_ARROW = 2,
    /* 0x3 */ MAJORAS_MASK_DMGEFF_ICE_ARROW,
    /* 0x4 */ MAJORAS_MASK_DMGEFF_LIGHT_ARROW,
    /* 0x9 */ MAJORAS_MASK_DMGEFF_SWORD_BEAM = 9,
    /* 0xF */ MAJORAS_MASK_DMGEFF_DAMAGE = 0xF
} MajorasMaskDamageEffect;

static DamageTable sMajorasMaskDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Deku Stick     */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Horse trample  */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Explosives     */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Zora boomerang */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Normal arrow   */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Hookshot       */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Goron punch    */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Sword          */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Goron pound    */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Fire arrow     */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Ice arrow      */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Light arrow    */ DMG_ENTRY(3, MAJORAS_MASK_DMGEFF_LIGHT_ARROW),
    /* Goron spikes   */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Deku spin      */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Deku bubble    */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Deku launch    */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Zora barrier   */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Normal shield  */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Light ray      */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Thrown object  */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Zora punch     */ DMG_ENTRY(1, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Spin attack    */ DMG_ENTRY(2, MAJORAS_MASK_DMGEFF_DAMAGE),
    /* Sword beam     */ DMG_ENTRY(2, MAJORAS_MASK_DMGEFF_SWORD_BEAM),
    /* Normal Roll    */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Unblockable    */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, MAJORAS_MASK_DMGEFF_IMMUNE),
    /* Powder Keg     */ DMG_ENTRY(2, MAJORAS_MASK_DMGEFF_DAMAGE),
};

typedef enum MajorasWrathDamageEffect {
    // Named because everything with this effect is ignored thanks to `CollisionCheck_SetATvsAC`.
    /* 0x0 */ MAJORAS_WRATH_DMGEFF_IMMUNE,

    // Stuns and surrounds Wrath with fire.
    /* 0x2 */ MAJORAS_WRATH_DMGEFF_FIRE = 2,

    // Stuns and surrounds Wrath with ice that shatters after a short time.
    /* 0x3 */ MAJORAS_WRATH_DMGEFF_FREEZE,

    // Stuns and surrounds Wrath with yellow light orbs.
    /* 0x4 */ MAJORAS_WRATH_DMGEFF_LIGHT_ORB,

    // Deals damage and surrounds Wrath with blue light orbs.
    /* 0x9 */ MAJORAS_WRATH_DMGEFF_BLUE_LIGHT_ORB = 9,

    // Stuns and surrounds Wrath with electric sparks.
    /* 0xA */ MAJORAS_WRATH_DMGEFF_ELECTRIC_SPARKS,

    // When an attack with this effect hits Wrath while it is either stunned or currently playing its damaged animation,
    // it sets the `damagedTimer` to 15 frames, which is longer than the usual 5 frames.
    /* 0xC */ MAJORAS_WRATH_DMGEFF_EXPLOSIVE = 0xC,

    // Deals damage and has no special effect.
    /* 0xD */ MAJORAS_WRATH_DMGEFF_DAMAGE_NONE,

    // When an attack with this effect hits Wrath while it is currently playing its damaged animation, it checks to see
    // if the attack landed within the last 4 frames of the animation. If so, it will restart Wrath's damaged animation.
    // Otherwise, it will set the `disableCollisionTimer` to 30 frames.
    /* 0xE */ MAJORAS_WRATH_DMGEFF_ANIM_FRAME_CHECK,

    // Stuns and has no special effect.
    /* 0xF */ MAJORAS_WRATH_DMGEFF_STUN_NONE
} MajorasWrathDamageEffect;

extern DamageTable sMajorasWrathDamageTable;
extern ColliderCylinderInit sWrathCylinderInit;
extern ColliderCylinderInit sTopCylinderInit;
extern DamageTable sMajorasIncarnationDamageTable;
extern ColliderCylinderInit sTopCylinderInit;

static ColliderCylinderInit sProjectileCylinderInit = {
    {
        COL_MATERIAL_NONE,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEM_MATERIAL_UNK0,
        { 0xF7CFFFFF, 0x04, 0x10 },
        { 0x00300000, 0x00, 0x00 },
        ATELEM_ON | ATELEM_SFX_NORMAL,
        ACELEM_ON,
        OCELEM_ON,
    },
    { 15, 30, -15, { 0, 0, 0 } },
};

typedef enum MajoraDrawDmgEffState {
    /*  0 */ MAJORA_DRAW_DMGEFF_STATE_NONE,
    /*  1 */ MAJORA_DRAW_DMGEFF_STATE_FIRE_INIT,
    /*  2 */ MAJORA_DRAW_DMGEFF_STATE_FIRE_ACTIVE,
    /* 10 */ MAJORA_DRAW_DMGEFF_STATE_FROZEN_INIT = 10,
    /* 11 */ MAJORA_DRAW_DMGEFF_STATE_FROZEN_ACTIVE,
    /* 20 */ MAJORA_DRAW_DMGEFF_STATE_LIGHT_ORB_INIT = 20,
    /* 21 */ MAJORA_DRAW_DMGEFF_STATE_LIGHT_ORB_ACTIVE,
    /* 30 */ MAJORA_DRAW_DMGEFF_STATE_BLUE_LIGHT_ORB_INIT = 30,
    /* 40 */ MAJORA_DRAW_DMGEFF_STATE_ELECTRIC_SPARKS_INIT = 40,
    /* 41 */ MAJORA_DRAW_DMGEFF_STATE_ELECTRIC_SPARKS_ACTIVE
} MajoraDrawDmgEffState;

typedef enum MajorasIncarnationDustSpawnPos {
    /* 0 */ MAJORAS_INCARNATION_DUST_SPAWN_POS_FEET,
    /* 1 */ MAJORAS_INCARNATION_DUST_SPAWN_POS_FOCUS
} MajorasIncarnationDustSpawnPos;


extern Vec3s sRemainsEndTarget[MAJORA_REMAINS_TYPE_MAX];
extern s32 sWhipLength;
extern s16 sProjectileEnvColors[4][3];
extern void Boss07_Incarnation_SetupStunned(Boss07*, PlayState*, s16);
extern void Boss07_Wrath_ChooseJump(Boss07*, PlayState*, u8);
extern void Boss07_RandXZ(Vec3f*, f32);
extern void Boss07_Incarnation_AvoidPlayer(Boss07*);
extern void Boss07_Incarnation_SpawnDust(Boss07*, PlayState*, u8, u8);
extern Vec3f sMajoraSfxPos;
extern void Boss07_SpawnFlameEffect(PlayState*, Vec3f*, Vec3f*, Vec3f*, f32);
extern void Boss07_Mask_StopBeam(Boss07*);
extern void Boss07_Mask_SetupDamaged(Boss07*, PlayState*, u8, Actor*);
extern void Boss07_Remains_UpdateDamage(Boss07*, PlayState*);
extern s32 Boss07_ArePlayerAndActorFacing(Boss07*, PlayState*);
extern void Boss07_Wrath_SetupDamaged(Boss07*, PlayState*, u8, u8);
extern void Boss07_Wrath_SpawnDustAtPos(PlayState*, Vec3f*, u8);

typedef enum MajorasMaskTentacleState {
    /* 0 */ MAJORAS_MASK_TENTACLE_STATE_DEFAULT,
    /* 1 */ MAJORAS_MASK_TENTACLE_STATE_FIRING_BEAM,
    /* 2 */ MAJORAS_MASK_TENTACLE_STATE_DEATH
} MajorasMaskTentacleState;

typedef enum RemainsMoveSubAction {
    /*  0 */ REMAINS_MOVE_SUB_ACTION_WAIT,
    /*  1 */ REMAINS_MOVE_SUB_ACTION_FLY,
    /*  2 */ REMAINS_MOVE_SUB_ACTION_DIE,
    /*  3 */ REMAINS_MOVE_SUB_ACTION_DEAD,
    /* 10 */ REMAINS_MOVE_SUB_ACTION_DAMAGED = 10,
    /* 20 */ REMAINS_MOVE_SUB_ACTION_DETACH_FROM_WALL = 20
} RemainsMoveSubAction;

typedef enum MajorasMaskFireBeamSubAction {
    /* 0 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_CHARGE_UP,
    /* 1 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_FIRE_EYE_BEAMS,
    /* 2 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_GROW_FOCUS_LIGHT_ORB,
    /* 3 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_ACTIVE,
    /* 4 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_REFLECTED,
    /* 5 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END,
    /* 6 */ MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_MAX
} MajorasMaskFireBeamSubAction;

RECOMP_PATCH void Boss07_Mask_FireBeam(Boss07* this, PlayState* play) {
    f32 dx;
    f32 dz;
    f32 dy;
    f32 distXYZ;
    f32 yOffset;
    s16 rotScale;
    s16 i;
    Vec3f diff;
    Vec3f transformedDiff;
    Player* player = GET_PLAYER(play);
    CollisionPoly* poly;
    Vec3f beamTireMarkPos;
    u8 beamIsTouchingPoly = false;
    s32 bgId;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    f32 targetX = player->actor.world.pos.x;
    f32 targetZ = player->actor.world.pos.z;

    switch (Difficulty) {
    case 0:
        targetX += player->actor.velocity.x * 6.0f;
        targetZ += player->actor.velocity.z * 6.0f;
        break;

    case 1:
    default:
        targetX += player->actor.velocity.x * 12.0f;
        targetZ += player->actor.velocity.z * 12.0f;
        break;
    }

    this->damagedTimer = 20;

    Boss07_SmoothStop(this, 0.5f);
    Math_ApproachF(&this->actor.world.pos.y, 300.0f, 0.05f, 1.0f);
    Math_ApproachS(&this->actor.shape.rot.z, 0, 0xA, 0x400);

    if ((player->focusActor != NULL) && (player->stateFlags1 & PLAYER_STATE1_400000)) {
        yOffset = (player->transformation == PLAYER_FORM_HUMAN) ? 20 : 30.0f;
    }
    else {
        yOffset = (player->transformation == PLAYER_FORM_HUMAN) ? 8.0f : 15.0f;
    }

    f32 targetY = player->actor.world.pos.y + yOffset;

    rotScale = (player->stateFlags1 & PLAYER_STATE1_400000) ? 1 : 10;

    dx = targetX - this->actor.world.pos.x;
    dy = targetY - this->actor.world.pos.y;
    dz = targetZ - this->actor.world.pos.z;

    s16 targetYaw = Math_Atan2S(dx, dz);

    f32 maxTurnSpeed = 0xFA0;
    if (this->subAction >= MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_ACTIVE) {
        maxTurnSpeed = (Difficulty >= 1) ? 0x800 : 0x400;
    }

    Math_ApproachF(&this->speedToTarget, maxTurnSpeed, 1.0f, 0xC8);

    Math_ApproachS(&this->actor.shape.rot.y, targetYaw, rotScale, this->speedToTarget);
    Math_ApproachS(&this->actor.shape.rot.x, -Math_Atan2S(dy, sqrtf(SQ(dx) + SQ(dz))), rotScale, this->speedToTarget);

    this->tentacleState = MAJORAS_MASK_TENTACLE_STATE_FIRING_BEAM;

    switch (this->subAction) {
    case MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_CHARGE_UP:
        if (this->timers[0] == 25) {
            Audio_PlaySfx_AtPos(&sMajoraSfxPos, NA_SE_EN_LAST1_BLOW_OLD);
        }

        if (this->timers[0] == 0) {
            this->subAction = MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_FIRE_EYE_BEAMS;
            this->timers[0] = 6;
            this->beamBaseScale = 1.0f;
        }
        break;

    case MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_FIRE_EYE_BEAMS:
        Math_ApproachF(&this->eyeBeamsLengthScale, 1.0f, 1.0f, 0.2f);

        if (this->timers[0] == 0) {
            this->subAction = MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_GROW_FOCUS_LIGHT_ORB;
            this->timers[0] = 8;
        }
        break;

    case MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_GROW_FOCUS_LIGHT_ORB:
        Audio_PlaySfx(NA_SE_EN_LAST1_BEAM_OLD - SFX_FLAG);
        Math_ApproachF(&this->eyeBeamsFocusOrbScale, 1.0f, 0.2f, 0.2f);

        if (this->timers[0] == 0) {
            this->subAction = MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_ACTIVE;
            this->timers[0] = 100;
        }
        break;

    case MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_ACTIVE:
    case MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_REFLECTED:
        Audio_PlaySfx(NA_SE_EN_LAST1_BEAM_OLD - SFX_FLAG);
        FALLTHROUGH;
    case MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END:
        Math_ApproachF(&this->eyeBeamsFocusOrbScale, 1.0f, 0.2f, 0.2f);
        dx = player->actor.world.pos.x - this->beamStartPos.x;
        dy = player->actor.world.pos.y - this->beamStartPos.y + 20.0f;
        dz = player->actor.world.pos.z - this->beamStartPos.z;
        distXYZ = sqrtf(SQ(dx) + SQ(dy) + SQ(dz));
        Math_ApproachF(&this->beamLengthScale, distXYZ * 0.2f, 1.0f, 7.0f);

        if (BgCheck_EntityLineTest1(&play->colCtx, &this->beamStartPos, &this->beamEndPos, &beamTireMarkPos, &poly,
            true, true, true, true, &bgId) &&
            (this->subAction != MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END)) {
            Vec3f flamePos;
            Vec3f flameVelocity;
            Vec3f flameAccel;

            flamePos.x = Rand_CenteredFloat(20.0f) + beamTireMarkPos.x;
            flamePos.y = Rand_CenteredFloat(20.0f) + beamTireMarkPos.y;
            flamePos.z = Rand_CenteredFloat(20.0f) + beamTireMarkPos.z;

            flameVelocity.x = 0.0f;
            flameVelocity.y = 6.0f;
            flameVelocity.z = 0.0f;

            flameAccel.x = flameVelocity.x * -0.05f;
            flameAccel.y = flameVelocity.y * -0.05f;
            flameAccel.z = flameVelocity.z * -0.05f;

            Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel, Rand_ZeroFloat(10.0f) + 25.0f);
            beamIsTouchingPoly = true;
        }

        diff.x = player->actor.world.pos.x - this->beamStartPos.x;
        diff.y = player->actor.world.pos.y - this->beamStartPos.y + 10.0f;
        diff.z = player->actor.world.pos.z - this->beamStartPos.z;
        Matrix_RotateXS(-this->actor.shape.rot.x, MTXMODE_NEW);
        Matrix_RotateYS(-this->actor.shape.rot.y, MTXMODE_APPLY);
        Matrix_MultVec3f(&diff, &transformedDiff);

        if ((fabsf(transformedDiff.x) < 20.0f) && (fabsf(transformedDiff.y) < 50.0f) &&
            (transformedDiff.z > 40.0f) && (transformedDiff.z <= (this->beamLengthScale * 20))) {
            if (Player_HasMirrorShieldEquipped(play) && (player->transformation == PLAYER_FORM_HUMAN) &&
                (player->stateFlags1 & PLAYER_STATE1_400000) &&
                (BINANG_ROT180(player->actor.shape.rot.y - this->actor.shape.rot.y) < 0x2000) &&
                (BINANG_ROT180(player->actor.shape.rot.y - this->actor.shape.rot.y) > -0x2000)) {
                Vec3s reflectedBeamRot;

                this->beamLengthScale = distXYZ * 0.05f;
                Math_ApproachF(&this->reflectedBeamLengthScale, distXYZ * 0.2f, 1.0f, 7.0f);
                Matrix_MtxFToYXZRot(&player->shieldMf, &reflectedBeamRot, 0);
                reflectedBeamRot.y += 0x8000;
                reflectedBeamRot.x = -reflectedBeamRot.x;

                if (this->subAction == MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_ACTIVE) {
                    this->subAction = MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_BEAM_REFLECTED;
                    this->reflectedBeamPitch = reflectedBeamRot.x;
                    this->reflectedBeamYaw = reflectedBeamRot.y;
                }
                else {
                    player->pushedYaw = this->actor.yawTowardsPlayer;
                    player->pushedSpeed = this->beamBaseScale * 0.5f;

                    sMajoraBattleHandler->lensFlareOn = true;
                    sMajoraBattleHandler->lensFlareScale = this->beamBaseScale * 30.0f;
                    sMajoraBattleHandler->lensFlarePos = this->beamEndPos;

                    Math_ApproachS(&this->reflectedBeamPitch, reflectedBeamRot.x, 2, 0x2000);
                    Math_ApproachS(&this->reflectedBeamYaw, reflectedBeamRot.y, 2, 0x2000);

                    diff.x = this->actor.world.pos.x - this->beamEndPos.x;
                    diff.y = this->actor.world.pos.y - this->beamEndPos.y;
                    diff.z = this->actor.world.pos.z - this->beamEndPos.z;
                    distXYZ = sqrtf(SQXYZ(diff));

                    Matrix_RotateXS(-this->reflectedBeamPitch, MTXMODE_NEW);
                    Matrix_RotateYS(-this->reflectedBeamYaw, MTXMODE_APPLY);
                    Matrix_Push();
                    Matrix_MultVec3f(&diff, &transformedDiff);

                    if ((fabsf(transformedDiff.x) < 60.0f) && (fabsf(transformedDiff.y) < 60.0f) &&
                        (transformedDiff.z > 40.0f) &&
                        (transformedDiff.z <= (this->reflectedBeamLengthScale * 16.666668f)) &&
                        (this->subAction != MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END)) {
                        s32 j;
                        Vec3f flamePos;
                        Vec3f flameVelocity;
                        Vec3f flameAccel;

                        this->beamDamageTimer += 2;
                        this->reflectedBeamLengthScale = distXYZ * 0.062f;

                        if (this->beamDamageTimer < 10) {
                            flamePos.x = this->actor.world.pos.x + Rand_CenteredFloat(40.0f);
                            flamePos.y = this->actor.world.pos.y + Rand_CenteredFloat(40.0f);
                            flamePos.z = this->actor.world.pos.z + Rand_CenteredFloat(40.0f);

                            flameVelocity.x = 0.0f;
                            flameVelocity.y = 6.0f;
                            flameVelocity.z = 0.0f;

                            flameAccel.x = flameVelocity.x * -0.05f;
                            flameAccel.y = flameVelocity.y * -0.05f;
                            flameAccel.z = flameVelocity.z * -0.05f;

                            Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel,
                                Rand_ZeroFloat(10.0f) + 25.0f);
                            this->damagedFlashTimer |= 10;
                        }
                        else {
                            this->damagedTimer = 50;
                            this->damagedFlashTimer = 15;
                            AudioSfx_StopByPos(&this->actor.projectedPos);
                            Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_DAMAGE2_OLD);
                            Boss07_Mask_SetupDamaged(this, play, 2, NULL);
                            Boss07_Mask_StopBeam(this);

                            for (j = 0; j < 20; j++) {
                                flamePos.x = this->actor.world.pos.x + Rand_CenteredFloat(50.0f);
                                flamePos.y = this->actor.world.pos.y + Rand_CenteredFloat(50.0f);
                                flamePos.z = this->actor.world.pos.z + Rand_CenteredFloat(50.0f);

                                flameVelocity.x = Rand_CenteredFloat(20.0f);
                                flameVelocity.y = Rand_CenteredFloat(20.0f);
                                flameVelocity.z = Rand_CenteredFloat(20.0f);

                                flameAccel.x = flameVelocity.x * -0.05f;
                                flameAccel.y = flameVelocity.y * -0.05f;
                                flameAccel.z = flameVelocity.z * -0.05f;

                                Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel,
                                    Rand_ZeroFloat(10.0f) + 25.0f);
                            }

                            if ((s8)this->actor.colChkInfo.health <= 0) {
                                this->fireTimer = 200;
                            }
                            else {
                                this->fireTimer = 60;
                            }
                        }
                    }

                    Matrix_Pop();

                    for (i = 0; i < MAJORA_REMAINS_TYPE_MAX; i++) {
                        if (sMajoraRemains[i]->subAction >= REMAINS_MOVE_SUB_ACTION_DIE) {
                            continue;
                        }

                        diff.x = sMajoraRemains[i]->actor.world.pos.x - this->beamEndPos.x;
                        diff.y = sMajoraRemains[i]->actor.world.pos.y - this->beamEndPos.y;
                        diff.z = sMajoraRemains[i]->actor.world.pos.z - this->beamEndPos.z;
                        distXYZ = sqrtf(SQXYZ(diff));
                        Matrix_MultVec3f(&diff, &transformedDiff);

                        if ((fabsf(transformedDiff.x) < 60.0f) && (fabsf(transformedDiff.y) < 60.0f) &&
                            (transformedDiff.z > 40.0f) &&
                            (transformedDiff.z <= (this->reflectedBeamLengthScale * 16.666668f)) &&
                            (this->subAction != MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END)) {
                            s32 j;
                            Vec3f flamePos;
                            Vec3f flameVelocity;
                            Vec3f flameAccel;

                            this->beamDamageTimer += 2;
                            this->reflectedBeamLengthScale = distXYZ * 0.062f;

                            if (Difficulty >= 1 && this->beamDamageTimer >= 18) {
                                this->subAction = MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END;
                                this->timers[0] = 0;
                            }
                            else if (this->beamDamageTimer < 10) {
                                flamePos.x = this->actor.world.pos.x + Rand_CenteredFloat(40.0f);
                                flamePos.y = this->actor.world.pos.y + Rand_CenteredFloat(40.0f);
                                flamePos.z = this->actor.world.pos.z + Rand_CenteredFloat(40.0f);

                                flameVelocity.x = 0.0f;
                                flameVelocity.y = 6.0f;
                                flameVelocity.z = 0.0f;

                                flameAccel.x = flameVelocity.x * -0.05f;
                                flameAccel.y = flameVelocity.y * -0.05f;
                                flameAccel.z = flameVelocity.z * -0.05f;

                                Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel, Rand_ZeroFloat(10.0f) + 25.0f);
                                this->damagedFlashTimer |= 10;
                            }
                            else {
                                this->damagedTimer = 50;
                                this->damagedFlashTimer = 15;
                                AudioSfx_StopByPos(&this->actor.projectedPos);
                                Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_DAMAGE2_OLD);
                                Boss07_Mask_SetupDamaged(this, play, 2, NULL);
                                Boss07_Mask_StopBeam(this);
                            }

                            if (this->beamDamageTimer < 5) {
                                flamePos.x = sMajoraRemains[i]->actor.world.pos.x + Rand_CenteredFloat(40.0f);
                                flamePos.y = sMajoraRemains[i]->actor.world.pos.y + Rand_CenteredFloat(40.0f);
                                flamePos.z = sMajoraRemains[i]->actor.world.pos.z + Rand_CenteredFloat(40.0f);

                                flameVelocity.x = 0.0f;
                                flameVelocity.y = 6.0f;
                                flameVelocity.z = 0.0f;

                                flameAccel.x = flameVelocity.x * -0.05f;
                                flameAccel.y = flameVelocity.y * -0.05f;
                                flameAccel.z = flameVelocity.z * -0.05f;

                                Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel,
                                    Rand_ZeroFloat(10.0f) + 25.0f);
                                sMajoraRemains[i]->damagedFlashTimer |= 10;
                            }
                            else {
                                sMajoraRemains[i]->subAction = REMAINS_MOVE_SUB_ACTION_DIE;
                                sMajoraRemains[i]->fireTimer = 60;
                                Actor_PlaySfx(&this->actor, NA_SE_EN_FOLLOWERS_DEAD);

                                for (j = 0; j < 20; j++) {
                                    flamePos.x = sMajoraRemains[i]->actor.world.pos.x + Rand_CenteredFloat(50.0f);
                                    flamePos.y = sMajoraRemains[i]->actor.world.pos.y + Rand_CenteredFloat(50.0f);
                                    flamePos.z = sMajoraRemains[i]->actor.world.pos.z + Rand_CenteredFloat(50.0f);

                                    flameVelocity.x = Rand_CenteredFloat(20.0f);
                                    flameVelocity.y = Rand_CenteredFloat(20.0f);
                                    flameVelocity.z = Rand_CenteredFloat(20.0f);

                                    flameAccel.x = flameVelocity.x * -0.05f;
                                    flameAccel.y = flameVelocity.y * -0.05f;
                                    flameAccel.z = flameVelocity.z * -0.05f;

                                    Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel,
                                        Rand_ZeroFloat(10.0f) + 25.0f);
                                }
                            }
                        }
                    }

                    if (BgCheck_EntityLineTest1(&play->colCtx, &this->beamEndPos, &this->reflectedBeamEndPos,
                        &beamTireMarkPos, &poly, true, true, true, true, &bgId) &&
                        (this->subAction != MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END)) {
                        Vec3f flamePos;
                        Vec3f flameVelocity;
                        Vec3f flameAccel;

                        beamIsTouchingPoly = true;

                        flamePos.x = Rand_CenteredFloat(20.0f) + beamTireMarkPos.x;
                        flamePos.y = Rand_CenteredFloat(20.0f) + beamTireMarkPos.y;
                        flamePos.z = Rand_CenteredFloat(20.0f) + beamTireMarkPos.z;

                        flameVelocity.x = 0.0f;
                        flameVelocity.y = 6.0f;
                        flameVelocity.z = 0.0f;

                        flameAccel.x = flameVelocity.x * -0.05f;
                        flameAccel.y = flameVelocity.y * -0.05f;
                        flameAccel.z = flameVelocity.z * -0.05f;

                        Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel,
                            Rand_ZeroFloat(10.0f) + 25.0f);
                    }
                }
            }
            else if (!player->bodyIsBurning && (this->subAction != MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END)) {
                s32 j;

                func_800B8D50(play, &this->actor, 5.0f, this->actor.shape.rot.y, 0.0f, 0x10);

                for (j = 0; j < ARRAY_COUNT(player->bodyFlameTimers); j++) {
                    player->bodyFlameTimers[j] = Rand_S16Offset(0, 200);
                }

                player->bodyIsBurning = true;
                Player_PlaySfx(player, player->ageProperties->voiceSfxIdOffset + NA_SE_VO_LI_DEMO_DAMAGE);
            }
        }

        if (beamIsTouchingPoly) {
            if (beamTireMarkPos.y == 0.0f) {
                dx = this->prevBeamTireMarkPos.x - beamTireMarkPos.x;
                dz = this->prevBeamTireMarkPos.z - beamTireMarkPos.z;
                func_800AE930(&play->colCtx, Effect_GetByIndex(this->effectIndex), &beamTireMarkPos, 15.0f,
                    Math_Atan2S(dx, dz), poly, bgId);
                this->beamTireMarkEnabled = true;
            }

            this->prevBeamTireMarkPos = beamTireMarkPos;
        }

        if (this->subAction != MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END) {
            if (this->timers[0] == 0) {
                this->subAction = MAJORAS_MASK_FIRE_BEAM_SUB_ACTION_END;
                this->timers[0] = 20;
            }
        }
        else {
            Math_ApproachZeroF(&this->beamBaseScale, 1.0f, 0.05f);

            if (this->timers[0] == 0) {
                Boss07_Mask_SetupIdle(this, play);
                this->timers[2] = 100;
                Boss07_Mask_StopBeam(this);
            }
        }
        break;

    default:
        break;
    }
}

RECOMP_HOOK("Boss07_Mask_Damaged") void DamagedMask(Boss07* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if ((this->timers[0] == 15) && ((s8)this->actor.colChkInfo.health < 20)) {
            this->startRemainsCs = true;
        }
        break;

    case 1:
        if ((this->timers[0] == 15) && ((s8)this->actor.colChkInfo.health < 28)) {
            this->startRemainsCs = true;
        }
        break;

    default:
        break;
    }
}

RECOMP_PATCH void Boss07_Incarnation_Run(Boss07* this, PlayState* play) {
    f32 dx;
    f32 dz;
    PlayerImpactType playerImpactType;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    f32 maxSpeed = 25.0f;
    f32 avoidDistance = 200.0f;
    s16 stunDuration = 50;
    f32 randomTripChance = 0.01f;
    f32 attackThreshold = 0.25f;
    f32 squatThreshold = 0.50f;
    f32 moonwalkThreshold = 0.75f;
    s16 tauntDelay = 50;
    s16 avoidCooldown = 50;

    if (Difficulty == 0) {
        maxSpeed = 30.0f;
        avoidDistance = 225.0f;
        stunDuration = 40;
        randomTripChance = 0.005f;
        attackThreshold = 0.40f;
        squatThreshold = 0.60f;
        moonwalkThreshold = 0.80f;
        tauntDelay = 80;
        avoidCooldown = 40;
    }
    else if (Difficulty >= 1) {
        maxSpeed = 36.0f;
        avoidDistance = 250.0f;
        stunDuration = 25;
        randomTripChance = 0.0f;
        attackThreshold = 0.60f;
        squatThreshold = 0.73f;
        moonwalkThreshold = 0.86f;
        tauntDelay = 120;
        avoidCooldown = 25;
    }

    Actor_PlaySfx(&this->actor, NA_SE_EN_LAST2_WALK_OLD - SFX_FLAG);
    this->miscTimer++;

    if (this->miscTimer >= 2) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_LAST2_WALK2_OLD);
        this->miscTimer = 0;
    }

    SkelAnime_Update(&this->skelAnime);

    if ((Actor_GetPlayerImpact(play, 5.0f, &this->actor.world.pos, &playerImpactType) >= 0.0f) &&
        (playerImpactType == PLAYER_IMPACT_GORON_GROUND_POUND)) {
        Boss07_Incarnation_SetupStunned(this, play, stunDuration);
        Actor_PlaySfx(&this->actor, NA_SE_EN_LAST2_DAMAGE_OLD);
    }
    else {
        dx = this->targetPos.x - this->actor.world.pos.x;
        dz = this->targetPos.z - this->actor.world.pos.z;

        if ((this->timers[1] == 0) || (SQ(dx) + SQ(dz) < 30000.0f)) {
            if (Rand_ZeroOne() < 0.3f) {
                f32 rand = Rand_ZeroOne();

                if (rand < attackThreshold) {
                    Boss07_Incarnation_SetupAttack(this, play);
                }
                else if (rand < squatThreshold) {
                    Boss07_Incarnation_SetupSquattingDance(this, play);
                }
                else if (rand < moonwalkThreshold) {
                    Boss07_Incarnation_SetupMoonwalk(this, play);
                }
                else if (rand < 1.0f) {
                    Boss07_Incarnation_SetupPirouette(this, play);
                }
            }
            else if (Rand_ZeroOne() < randomTripChance) {
                Boss07_Incarnation_SetupStunned(this, play, stunDuration);
            }
            else {
                Boss07_RandXZ(&this->targetPos, 500.0f);
                this->timers[1] = Rand_ZeroFloat(50.0f) + 20.0f;
                this->speedToTarget = 0.0f;
            }
        }

        Math_ApproachS(&this->actor.world.rot.y, Math_Atan2S(dx, dz), 5, this->speedToTarget);
        Math_ApproachF(&this->speedToTarget, 0xFA0, 1.0f, 0x1F4);
        Math_ApproachF(&this->actor.speed, maxSpeed, 1.0f, 20.0f);

        if (this->timers[0] == 0) {
            Boss07_Incarnation_SetupTaunt(this, play);
        }

        if ((this->actor.xzDistToPlayer < avoidDistance) && (this->timers[2] == 0)) {
            Boss07_Incarnation_AvoidPlayer(this);
            this->timers[0] = tauntDelay;
            this->timers[2] = avoidCooldown;
        }

        Boss07_Incarnation_SpawnDust(this, play, 3, MAJORAS_INCARNATION_DUST_SPAWN_POS_FEET);
        this->fireTimer = 5;
    }
}

RECOMP_HOOK_RETURN("Boss07_Mask_SetupStunned") void MMP1Stun(Boss07* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timers[0] = 40;
        break;

    case 1:
        this->timers[0] = 20;
        break;

    default:
        break;
    }
}

RECOMP_PATCH void Boss07_Mask_Idle(Boss07* this, PlayState* play) {
    s16 targetRotX;
    s16 targetRotY;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distToTargetXZ;
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);
    Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_FLOAT_OLD - SFX_FLAG);

    if (this->timers[0] == 0) {
        if (this->timers[2] == 0) {
            if (((s8)this->actor.colChkInfo.health <= 30) &&
                (Rand_ZeroOne() < 0.75f)) {
                Boss07_Mask_SetupFireBeam(this, play);
            }
            else {
                Boss07_Mask_SetupSpinAttack(this, play);
            }
        }
        else if (Rand_ZeroOne() < 0.15f) {
            this->flySpeedTarget = 2.0f;
            this->timers[0] = Rand_ZeroFloat(50.0f) + 30.0f;
        }
        else {
            Boss07_RandXZ(&this->targetPos, 500.0f);
            this->targetPos.y = Rand_ZeroFloat(350.0f) + 100.0f;
            this->timers[0] = Rand_ZeroFloat(50.0f) + 20.0f;
            this->speedToTarget = 0.0f;
            this->flySpeedTarget = Rand_ZeroFloat(12.0f) + 3.0f;
        }
    }

    dx = this->targetPos.x - this->actor.world.pos.x;
    dy = this->targetPos.y - this->actor.world.pos.y;
    dz = this->targetPos.z - this->actor.world.pos.z;
    targetRotY = Math_Atan2S(dx, dz);
    distToTargetXZ = sqrtf(SQ(dx) + SQ(dz));
    targetRotX = Math_Atan2S(dy, distToTargetXZ);
    targetRotX += (s16)(Math_SinS(this->frameCounter * 0x1388) * 0xFA0);

    Math_ApproachS(&this->actor.world.rot.y, targetRotY, 0xA, this->speedToTarget);
    Math_ApproachS(&this->actor.world.rot.x, targetRotX, 5, this->speedToTarget);
    Math_ApproachF(&this->speedToTarget, 0x7D0, 1.0f, 0x64);
    Math_ApproachF(&this->actor.speed, this->flySpeedTarget, 1.0f, 1.0f);

    if (this->timers[1] != 0) {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 3, 0x3000);
    }
    else if (this->flySpeedTarget < 7.0f) {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0x2000);
    }
    else {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 5, 0x2000);
    }

    if ((player->unk_D57 == 4) && (Rand_ZeroOne() < 0.8f)) {
        this->timers[1] = 20;
    }

    Math_ApproachS(&this->actor.shape.rot.x, 0, 0xA, 0x200);
    Math_ApproachS(&this->actor.shape.rot.z, 0, 0xA, 0x400);

    if (this->shouldStartDeath || KREG(88)) {
        KREG(88) = false;
        this->shouldStartDeath = false;
        SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 1);
        Boss07_Mask_SetupDeathCutscene(this, play);
    }
}

RECOMP_HOOK("Boss07_Wrath_Attack") void evadingreal(Boss07* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float evadeChance = (Difficulty == 1) ? 1.0f : 0.60f;
    float Speed = (Difficulty == 1) ? 1.66f : 1.33f;

    if (Rand_ZeroOne() <= evadeChance) {
        this->canEvade = true;

        switch (this->subAction) {
        case MAJORAS_WRATH_ATTACK_SUB_ACTION_QUICK_WHIP:
        case MAJORAS_WRATH_ATTACK_SUB_ACTION_FLURRY:
        case MAJORAS_WRATH_ATTACK_SUB_ACTION_DOUBLE_WHIP:
        case MAJORAS_WRATH_ATTACK_SUB_ACTION_LONG_WHIP:
        case MAJORAS_WRATH_ATTACK_SUB_ACTION_KICK:
        case MAJORAS_WRATH_ATTACK_SUB_ACTION_THREE_HIT:
            this->skelAnime.playSpeed = Speed;
            break;

        case MAJORAS_WRATH_ATTACK_SUB_ACTION_SPIN_ATTACK:
            this->skelAnime.playSpeed = 1.0f;
            break;

        default:
            break;
        }
    }
    else {
         if (Rand_ZeroOne() <= 0.4) this->canEvade = false;
        this->skelAnime.playSpeed = 1.0f;
    }
}

RECOMP_PATCH void Boss07_Wrath_Idle(Boss07* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);
    Boss07_SmoothStop(this, 2.0f);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (Rand_ZeroOne() < 0.3) this->canEvade = true;
        this->skelAnime.playSpeed = 2.0f;
        break;

    case 1:
        this->canEvade = true;
        this->skelAnime.playSpeed = 3.0f;
        break;

    default:
        break;
    }

    this->rightWhip.mobility = this->leftWhip.mobility = 0.7f;
    this->rightWhip.gravity = this->leftWhip.gravity = -15.0f;
    this->rightWhip.deceleration = this->leftWhip.deceleration = 2.0f;
    this->leftWhip.tension = this->rightWhip.tension = 0.0f;

    if ((this->actor.xzDistToPlayer <= 100.0f) && (player->actor.world.pos.y < 10.0f)) {
        Boss07_Wrath_SetupAttack(this, play);
    }
    else if (this->timers[0] == 0) {
        if (KREG(78) == 1) {
            Boss07_Wrath_SetupThrowTop(this, play);
        }
        else {
            f32 rand = Rand_ZeroOne();
            if (rand < 0.33f) {
                Boss07_Wrath_SetupAttack(this, play);
            }
            else if (rand < 0.66f) {
                Boss07_Wrath_SetupThrowTop(this, play);
            }
            else {
                Boss07_Wrath_SetupTryGrab(this, play);
            }
        }
    }
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0x1000);
}

RECOMP_PATCH void Boss07_Wrath_SetupStunned(Boss07* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        if (Rand_ZeroOne() <= 0.5) {
            Boss07_Wrath_ChooseJump(this, play, true);
        }
        else {
            Boss07_Wrath_SetupAttack(this, play);
        }
    }
    else {

        if (this->actionFunc != Boss07_Wrath_Stunned) {
            this->actionFunc = Boss07_Wrath_Stunned;
            Animation_MorphToPlayOnce(&this->skelAnime, &gMajorasWrathStunAnim, -10.0f);
            this->animEndFrame = Animation_GetLastFrame(&gMajorasWrathStunAnim);
        }

        this->disableCollisionTimer = 10;
        Actor_PlaySfx(&this->actor, NA_SE_EN_LAST3_VOICE_DAMAGE_OLD);
    }
}

RECOMP_PATCH void Boss07_Wrath_UpdateDamage(Boss07* this, PlayState* play) {
    s32 i;
    s32 j;
    u8 damage;
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 5;
        break;

    default:
        break;
    }

    if (this->damagedTimer != 0) {
        return;
    }

    if (this->kickCollider.elements[MAJORAS_WARTH_KICK_COLLIDER_RIGHT_FOOT].base.atElemFlags & ATELEM_HIT) {
        this->kickCollider.elements[MAJORAS_WARTH_KICK_COLLIDER_RIGHT_FOOT].base.atElemFlags &= ~ATELEM_HIT;
        player->pushedYaw = this->actor.yawTowardsPlayer;
        player->pushedSpeed = 20.0f;
        Boss07_Wrath_SpawnDustAtPos(play, &player->actor.world.pos, 12);
        Audio_PlaySfx(NA_SE_IT_HOOKSHOT_STICK_OBJ);
    }

    for (i = 0; i < ARRAY_COUNT(this->bodyColliderElements); i++) {
        if (!(this->bodyCollider.elements[i].base.acElemFlags & ACELEM_HIT)) {
            continue;
        }

        for (j = 0; j < ARRAY_COUNT(this->bodyColliderElements); j++) {
            this->bodyCollider.elements[j].base.acElemFlags &= ~ACELEM_HIT;
        }

        if (this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX) {
            this->drawDmgEffTimer = 0;
        }

        switch (this->actor.colChkInfo.damageEffect) {
        case MAJORAS_WRATH_DMGEFF_FREEZE:
            this->drawDmgEffState = MAJORA_DRAW_DMGEFF_STATE_FROZEN_INIT;
            break;

        case MAJORAS_WRATH_DMGEFF_FIRE:
            this->drawDmgEffState = MAJORA_DRAW_DMGEFF_STATE_FIRE_INIT;
            break;

        case MAJORAS_WRATH_DMGEFF_LIGHT_ORB:
            this->drawDmgEffState = MAJORA_DRAW_DMGEFF_STATE_LIGHT_ORB_INIT;
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->actor.focus.pos.x, this->actor.focus.pos.y,
                this->actor.focus.pos.z, 0, 0, 0, CLEAR_TAG_PARAMS(CLEAR_TAG_LARGE_LIGHT_RAYS));
            break;

        case MAJORAS_WRATH_DMGEFF_ELECTRIC_SPARKS:
            this->drawDmgEffState = MAJORA_DRAW_DMGEFF_STATE_ELECTRIC_SPARKS_INIT;
            Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_FREEZE);
            break;

        case MAJORAS_WRATH_DMGEFF_BLUE_LIGHT_ORB:
            this->drawDmgEffState = MAJORA_DRAW_DMGEFF_STATE_BLUE_LIGHT_ORB_INIT;
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->actor.focus.pos.x, this->actor.focus.pos.y,
                this->actor.focus.pos.z, 0, 0, 3, CLEAR_TAG_PARAMS(CLEAR_TAG_LARGE_LIGHT_RAYS));
            break;

        default:
            break;
        }

        damage = this->actor.colChkInfo.damage;

        if (Difficulty == 1) {
            this->damagedTimer = (this->actor.colChkInfo.damageEffect == MAJORAS_WRATH_DMGEFF_EXPLOSIVE) ? 30 : 20;
            this->damagedFlashTimer = this->damagedTimer;

            u8 isGrabbingPlayer = (&this->actor == player->actor.parent);
            if (this->actor.colChkInfo.health > damage && !isGrabbingPlayer) {
                this->actor.colChkInfo.health -= damage;
                Actor_PlaySfx(&this->actor, NA_SE_EN_LAST3_VOICE_DAMAGE2_OLD);
            }
            else {
                Boss07_Wrath_SetupDamaged(this, play, damage, this->actor.colChkInfo.damageEffect);
            }

            this->whipWrapEndOffset = 0;

            if (isGrabbingPlayer) {
                player->av2.actionVar2 = 101;
                player->actor.parent = NULL;
                player->csAction = PLAYER_CSACTION_NONE;
            }
        }
        else if ((this->actionFunc == Boss07_Wrath_Stunned) || (this->actionFunc == Boss07_Wrath_Damaged)) {
            if ((this->actionFunc == Boss07_Wrath_Stunned) &&
                (this->actor.colChkInfo.damageEffect != MAJORAS_WRATH_DMGEFF_ANIM_FRAME_CHECK) &&
                (this->actor.colChkInfo.damageEffect != MAJORAS_WRATH_DMGEFF_DAMAGE_NONE) &&
                (this->actor.colChkInfo.damageEffect != MAJORAS_WRATH_DMGEFF_BLUE_LIGHT_ORB) &&
                (this->actor.colChkInfo.damageEffect != MAJORAS_WRATH_DMGEFF_EXPLOSIVE)) {
                Boss07_Wrath_SetupStunned(this, play);
                this->damagedTimer = 6;
            }
            else {
                this->damagedFlashTimer = 15;
                this->damagedTimer = (this->actor.colChkInfo.damageEffect == MAJORAS_WRATH_DMGEFF_EXPLOSIVE) ? 15 : 5;
                Boss07_Wrath_SetupDamaged(this, play, damage, this->actor.colChkInfo.damageEffect);
            }
        }
        else {
            this->damagedTimer = 15;
            Boss07_Wrath_SetupStunned(this, play);
            this->whipWrapEndOffset = 0;

            if (&this->actor == player->actor.parent) {
                player->av2.actionVar2 = 101;
                player->actor.parent = NULL;
                player->csAction = PLAYER_CSACTION_NONE;
            }
        }
        break;
    }
}

RECOMP_HOOK("Boss07_Wrath_SetupThrowTop") void yesdodgepartsomething(Boss07* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (Rand_ZeroOne() < 0.3) this->canEvade = true;
        this->skelAnime.playSpeed = 2.0f;
        break;

    case 1:

        this->canEvade = true;
        this->skelAnime.playSpeed = 3.0f;
        break;

    default:
        break;
    }
}

RECOMP_HOOK_RETURN("Boss07_Remains_SetupMove") void RemainsHealth(Boss07* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = 8;
        break;

    case 1:
        this->actor.colChkInfo.health = 12;
        break;

    default:
        break;
    }
}

RECOMP_PATCH void Boss07_Remains_Move(Boss07* this, PlayState* play) {
    s16 targetRotX;
    s16 targetRotY;
    f32 dx;
    f32 dy;
    f32 dz;
    s32 pad;

    f32 shotCooldownBase = 100.0f;
    f32 shotCooldownRand = 200.0f;
    f32 flySpeedBase = 5.0f;
    f32 flySpeedRand = 5.0f;
    f32 slowChance = 0.35f;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        shotCooldownBase = 70.0f;
        shotCooldownRand = 150.0f;
        flySpeedBase = 6.0f;
        flySpeedRand = 6.0f;
        slowChance = 0.25f;
        break;

    case 1:
        shotCooldownBase = 50.0f;
        shotCooldownRand = 100.0f;
        flySpeedBase = 8.0f;
        flySpeedRand = 7.0f;
        slowChance = 0.15f;
        break;

    default:
        break;
    }

    switch (this->subAction) {
    case REMAINS_MOVE_SUB_ACTION_DETACH_FROM_WALL:
        Actor_PlaySfx(&this->actor, NA_SE_EV_MUJURA_FOLLOWERS_FLY - SFX_FLAG);
        this->timers[0] = 80;
        this->timers[2] = 100.0f + Rand_ZeroFloat(100.0f);
        this->flySpeedTarget = 5.0f;
        this->actor.speed = 5.0f;
        this->targetPos = gZeroVec3f;
        this->actor.world.rot.y = Math_Atan2S(-this->actor.world.pos.x, -this->actor.world.pos.z);
        this->subAction = REMAINS_MOVE_SUB_ACTION_FLY;
        this->bgCheckTimer = 100;
        this->generalCollider.base.colMaterial = COL_MATERIAL_HIT3;
        this->actor.flags |= (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOOKSHOT_PULLS_ACTOR);
        Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_DEMO_BREAK);
        break;

    case REMAINS_MOVE_SUB_ACTION_FLY:
        Actor_PlaySfx(&this->actor, NA_SE_EV_MUJURA_FOLLOWERS_FLY - SFX_FLAG);
        if (this->timers[2] == 0) {
            this->tryFireProjectile = true;

            if (Difficulty == 1 && this->timers[1] == 0 && Rand_ZeroOne() < 0.40f) {
                this->timers[1] = 20;
            }

            if (this->timers[1] > 0) {
                this->timers[2] = 5;
            }
            else {
                this->timers[2] = Rand_ZeroFloat(shotCooldownRand) + shotCooldownBase;
            }
        }

        if (this->timers[0] == 0) {
            if (Rand_ZeroOne() < slowChance) {
                this->flySpeedTarget = 1.0f;
                this->timers[0] = Rand_ZeroFloat(50.0f) + 30.0f;
            }
            else {
                Boss07_RandXZ(&this->targetPos, 500.0f);
                this->targetPos.y = Rand_ZeroFloat(350.0f) + 100.0f;
                this->timers[0] = Rand_ZeroFloat(50.0f) + 20.0f;
                this->speedToTarget = 0.0f;
                this->flySpeedTarget = Rand_ZeroFloat(flySpeedRand) + flySpeedBase;
            }
        }

        dx = this->targetPos.x - this->actor.world.pos.x;
        dy = this->targetPos.y - this->actor.world.pos.y;
        dz = this->targetPos.z - this->actor.world.pos.z;
        targetRotY = Math_Atan2S(dx, dz);
        targetRotX = Math_Atan2S(dy, sqrtf(SQ(dx) + SQ(dz)));
        targetRotX += (s16)(Math_SinS(this->frameCounter * 0x1388) * 0xFA0);

        Math_ApproachS(&this->actor.world.rot.y, targetRotY, 0xA, this->speedToTarget);
        Math_ApproachS(&this->actor.world.rot.x, targetRotX, 5, this->speedToTarget);
        Math_ApproachF(&this->speedToTarget, 0x7D0, 1.0f, 0x64);
        Math_ApproachF(&this->actor.speed, this->flySpeedTarget, 1.0f, 1.0f);

        if ((this->flySpeedTarget < 8.0f) && !Play_InCsMode(play)) {
            Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0xA, 0x1000);
        }
        else {
            Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 0xA, 0x1000);
        }

        Actor_UpdateVelocityWithoutGravity(&this->actor);
        Actor_UpdatePos(&this->actor);

        if (this->bgCheckTimer == 0) {
            Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 100.0f, 100,
                UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4);
        }
        else {
            this->bgCheckTimer--;
        }

        Boss07_Remains_UpdateDamage(this, play);
        break;

    case REMAINS_MOVE_SUB_ACTION_DIE:
        Math_ApproachS(&this->actor.shape.rot.x, -0x4000, 1, 0x500);
        Actor_MoveWithGravity(&this->actor);
        Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 100.0f, 100.0f,
            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4);

        if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
            if (this->burnOnLanding) {
                this->fireTimer |= 4;
            }

            Math_ApproachF(&this->actor.scale.z, 0.0f, 1.0f, 0.001f);

            if (this->actor.scale.z == 0.0f) {
                this->subAction = REMAINS_MOVE_SUB_ACTION_DEAD;
                this->actor.draw = NULL;
                this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            }

            Boss07_SmoothStop(this, 2.0f);
        }
        else {
            this->actor.shape.rot.z += 0x200;
        }
        break;

    case REMAINS_MOVE_SUB_ACTION_DAMAGED:
        Actor_MoveWithGravity(&this->actor);
        this->actor.world.pos.y -= 50.0f;
        this->actor.prevPos.y -= 50.0f;

        Actor_UpdateBgCheckInfo(play, &this->actor, 35.0f, 60.0f, 60.0f,
            UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4);

        this->actor.world.pos.y += 50.0f;
        this->actor.prevPos.y += 50.0f;

        if (this->timers[0] == 0) {
            this->subAction = REMAINS_MOVE_SUB_ACTION_FLY;
        }
        break;

    case REMAINS_MOVE_SUB_ACTION_WAIT:
    default:
        break;
    }

    if (this->subAction < REMAINS_MOVE_SUB_ACTION_DIE) {
        Collider_UpdateCylinder(&this->actor, &this->generalCollider);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->generalCollider.base);
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->generalCollider.base);
    }

    if (this->tryFireProjectile) {
        this->tryFireProjectile = false;

        if (Boss07_ArePlayerAndActorFacing(this, play) && (sMajorasMask->actionFunc != Boss07_Mask_FireBeam)) {
            Actor_Spawn(&play->actorCtx, play, ACTOR_BOSS_07, this->actor.world.pos.x, this->actor.world.pos.y,
                this->actor.world.pos.z, 0, 0, 0, MAJORA_PARAMS(MAJORA_TYPE_PROJECTILE_REMAINS));
        }
    }

    if (this->fireTimer != 0) {
        Vec3f flamePos;
        Vec3f flameVelocity;
        Vec3f flameAccel;

        flamePos.x = Rand_CenteredFloat(80.0f) + this->actor.world.pos.x;
        flamePos.z = Rand_CenteredFloat(80.0f) + this->actor.world.pos.z;

        if (this->burnOnLanding) {
            flameAccel.x = flameAccel.z = 0.0f;
            flameAccel.y = 0.03f;
            flamePos.y = Rand_ZeroFloat(10.0f) + this->actor.world.pos.y;
            EffectSsKFire_Spawn(play, &flamePos, &gZeroVec3f, &flameAccel, Rand_ZeroFloat(30.0f) + 30.0f, 0);
            Actor_PlaySfx(&this->actor, NA_SE_EN_COMMON_EXTINCT_LEV - SFX_FLAG);
        }
        else {
            flamePos.y = (Rand_ZeroFloat(30.0f) + this->actor.world.pos.y) - 15.0f;

            flameVelocity.x = 0.0f;
            flameVelocity.y = 5.0f;
            flameVelocity.z = 0.0f;

            flameAccel.x = flameVelocity.x * -0.05f;
            flameAccel.y = flameVelocity.y * -0.05f;
            flameAccel.z = flameVelocity.z * -0.05f;

            Boss07_SpawnFlameEffect(play, &flamePos, &flameVelocity, &flameAccel, Rand_ZeroFloat(10.0f) + 25.0f);
            Actor_PlaySfx(&this->actor, NA_SE_EV_BURN_OUT - SFX_FLAG);
        }
    }
}

RECOMP_PATCH void Boss07_Mask_UpdateDamage(Boss07* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    ColliderElement* acHitElem;
    u8 damage;
    Actor* hitActor;
    s16 yawToAttack;
    s16 yawDiff;

    if (this->damagedTimer == 0 && this->maskShakeTimer == 0) {
        if (this->maskFrontCollider.base.acFlags & AC_HIT) {
            this->maskFrontCollider.base.acFlags &= ~AC_HIT;
            this->maskShakeTimer = 15;
            this->maskBackCollider.base.acFlags &= ~AC_HIT;
            hitActor = this->maskFrontCollider.base.ac;
            Actor_PlaySfx(&this->actor, NA_SE_IT_SHIELD_REFLECT_SW);
            if (hitActor != NULL) CollisionCheck_SpawnShieldParticles(play, &hitActor->world.pos);
        }
        else if (this->maskBackCollider.base.acFlags & AC_HIT) {
            this->maskBackCollider.base.acFlags &= ~AC_HIT;
            hitActor = this->maskBackCollider.base.ac;

            yawToAttack = (hitActor != NULL) ? Actor_WorldYawTowardActor(&this->actor, hitActor) : this->actor.yawTowardsPlayer;
            yawDiff = this->actor.shape.rot.y - yawToAttack;

            if ((yawDiff > -0x2800) && (yawDiff < 0x2800)) {
                this->maskShakeTimer = 15;
                Actor_PlaySfx(&this->actor, NA_SE_IT_SHIELD_REFLECT_SW);
                if (hitActor != NULL) {
                    CollisionCheck_SpawnShieldParticles(play, &hitActor->world.pos);
                }
            }
            else {
                this->maskShakeTimer = 25;

                if ((this->actionFunc == Boss07_Mask_Stunned) || (player->stateFlags3 & PLAYER_STATE3_200)) {
                    acHitElem = this->maskBackCollider.elem.acHitElem;
                    damage = (acHitElem->atDmgInfo.dmgFlags & ~0x8300000) ? this->actor.colChkInfo.damage : 0;
                    this->damagedTimer = 50;
                    this->damagedFlashTimer = 15;
                    AudioSfx_StopByPos(&this->actor.projectedPos);
                    Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_DAMAGE2_OLD);
                    Boss07_Mask_SetupDamaged(this, play, damage, hitActor);
                }
                else {
                    this->damagedTimer = 15;
                    AudioSfx_StopByPos(&this->actor.projectedPos);
                    Actor_PlaySfx(&this->actor, NA_SE_EN_LAST1_DAMAGE1_OLD);
                    Boss07_Mask_SetupStunned(this, play);
                }
            }
        }
    }
}

RECOMP_HOOK("Boss07_Mask_Update") void MaskDefense(Actor* thisx, PlayState* play2) {

    Boss07* this = (Boss07*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }
}

RECOMP_HOOK("Boss07_Incarnation_Update") void IncarnationDefense(Actor* thisx, PlayState* play2) {

    Boss07* this = (Boss07*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 5;
        break;

    default:
        break;
    }
}
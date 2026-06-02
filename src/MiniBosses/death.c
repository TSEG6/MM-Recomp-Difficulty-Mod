#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_death.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_Arrow_Light/z_arrow_light.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

#include "z_en_minideath.h"
#include "overlays/actors/ovl_En_Death/z_en_death.h"
#include "assets/objects/object_death/object_death.h"

void EnDeath_SwingAttack(EnDeath* this, PlayState* play);
void EnDeath_EndSwingAttack(EnDeath* this, PlayState* play);
void EnDeath_SpinAttack(EnDeath* this, PlayState* play);
void EnDeath_SetupBlockProjectile(EnDeath* this);
void EnDeath_SetupApproachPlayer(EnDeath* this);
void EnDeath_SetupSwingAttack(EnDeath* this);
void EnDeath_SetupSpinAttack(EnDeath* this);
void EnDeath_SetupStartBatSwarm(EnDeath* this);
void EnDeath_StartBatSwarm(EnDeath* this, PlayState* play);
void EnDeath_BatSwarm(EnDeath* this, PlayState* play);
void EnDeath_SetupPlayCutscene(EnDeath* this);

extern void EnDeath_SetupDamaged(EnDeath*);
extern s32 EnDeath_ProjectileApproaching(EnDeath*, PlayState*);
extern void EnDeath_Float(EnDeath*);
extern void EnDeath_UpdateSpinAttackTris(EnDeath*);
extern void EnDeath_DrawScytheSpinning(EnDeath*, PlayState*);
extern AnimationHeader gGomessScytheSwingAnim;
extern AnimationHeader gGomessScytheSpinAnim;
extern AnimationHeader gGomessBatSwarmAnim;

typedef enum {
    /* 0x0 */ DMGEFF_NONE = 0,
    /* 0x2 */ DMGEFF_FIRE_ARROW = 2,
    /* 0x3 */ DMGEFF_ICE_ARROW,
    /* 0x4 */ DMGEFF_LIGHT_ARROW,
    /* 0xF */ DMGEFF_EXPLOSIVES = 15
} EnDeathDamageEffect;

static DamageTable sDamageTable = {
    /* Deku Nut       */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Deku Stick     */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Horse trample  */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Explosives     */ DMG_ENTRY(1, DMGEFF_EXPLOSIVES),
    /* Zora boomerang */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Normal arrow   */ DMG_ENTRY(1, DMGEFF_NONE),
    /* UNK_DMG_0x06   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Hookshot       */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Goron punch    */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Sword          */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Goron pound    */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Fire arrow     */ DMG_ENTRY(1, DMGEFF_FIRE_ARROW),
    /* Ice arrow      */ DMG_ENTRY(1, DMGEFF_ICE_ARROW),
    /* Light arrow    */ DMG_ENTRY(2, DMGEFF_LIGHT_ARROW),
    /* Goron spikes   */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Deku spin      */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Deku bubble    */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Deku launch    */ DMG_ENTRY(2, DMGEFF_NONE),
    /* UNK_DMG_0x12   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Zora barrier   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Normal shield  */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Light ray      */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Thrown object  */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Zora punch     */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Spin attack    */ DMG_ENTRY(1, DMGEFF_NONE),
    /* Sword beam     */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Normal Roll    */ DMG_ENTRY(0, DMGEFF_NONE),
    /* UNK_DMG_0x1B   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* UNK_DMG_0x1C   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Unblockable    */ DMG_ENTRY(0, DMGEFF_NONE),
    /* UNK_DMG_0x1E   */ DMG_ENTRY(0, DMGEFF_NONE),
    /* Powder Keg     */ DMG_ENTRY(1, DMGEFF_EXPLOSIVES),
};

RECOMP_HOOK_RETURN("EnDeath_Init") void NewDamage(Actor* thisx, PlayState* play2) {}

RECOMP_HOOK_RETURN("EnDeath_SetupApproachPlayer") void OhYourApproachingMe(EnDeath* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int ATimerT = 140;
    int ATimerF = 100;

    switch (Difficulty) {
    case 0:
        this->actor.speed = 2.0f;
        ATimerT = 70;
        ATimerF = 30;
        break;

    case 1:
        this->actor.speed = 2.5f;
        ATimerT = 45;
        ATimerF = 15;
        break;

    default:
        break;
    }

    if (this->actionFunc == EnDeath_EndSwingAttack || this->actionFunc == EnDeath_SpinAttack) {
        this->actionTimer = ATimerT;
    }
    else {
        this->actionTimer = ATimerF;
    }
}

RECOMP_PATCH void EnDeath_ApproachPlayer(EnDeath* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    float blockProb = 0.4f;
    s16 swingTimerThreshold = 100;
    float swingDistThreshold = 200.0f;
    float randomAttackChance = 0.0f;

    switch (Difficulty) {
    case 0:
        blockProb = 0.5f;
        swingTimerThreshold = 60;
        swingDistThreshold = 160.0f;
        randomAttackChance = 0.01f;
        break;

    case 1:
        blockProb = 0.7f;
        swingTimerThreshold = 80;
        swingDistThreshold = 220.0f;
        randomAttackChance = 0.03f;
        break;

    default:
        break;
    }

    if (play->envCtx.lightSettingOverride == 20) play->envCtx.lightSettingOverride = 26;

    EnDeath_Float(this);
    SkelAnime_Update(&this->skelAnime);

    s16 rotStep = (Difficulty >= 0) ? 0x1500 : 0x1000;
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, rotStep, 0x100);

    if (this->actionTimer > 0) this->actionTimer--;

    if (Rand_ZeroOne() < blockProb && EnDeath_ProjectileApproaching(this, play)) {
        EnDeath_SetupBlockProjectile(this);
        return;
    }

    if (this->actionTimer < swingTimerThreshold && this->actor.xzDistToPlayer < swingDistThreshold) {
        if (Difficulty == 1 && Rand_ZeroOne() < 0.05f) {
            EnDeath_SetupSpinAttack(this);
        }
        else {
            EnDeath_SetupSwingAttack(this);
        }
    }
    else if (this->actionTimer == 0 || Rand_ZeroOne() < randomAttackChance) {
        if (this->actor.params >= 5) {
            if (Difficulty <= 1 && Rand_ZeroOne() < 0.2f) {
                EnDeath_SetupStartBatSwarm(this);
            }
            else {
                EnDeath_SetupSpinAttack(this);
            }
        }
        else {
            if (Rand_ZeroOne() < 0.3f) EnDeath_SetupSpinAttack(this);
            else  EnDeath_SetupSwingAttack(this);
        }
    }
}
RECOMP_HOOK("EnDeath_IntroCutscenePart5") void TomfooleryDeath(EnDeath* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        if (SkelAnime_Update(&this->skelAnime)) {
            CutsceneManager_Stop(this->actor.csId);
            Player_SetCsAction(play, &this->actor, PLAYER_CSACTION_END);
            this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
            this->coreCollider.base.acFlags |= AC_ON;
            this->holdsScythe = true;
            Actor_SetScale(&this->actor, 0.01f);
            EnDeath_SetupSpinAttack(this);

        }
    }
}

RECOMP_PATCH void EnDeath_SetupSwingAttack(EnDeath* this) {
    Animation_Change(&this->skelAnime, &gGomessScytheSwingAnim, 1.0f, 0.0f, 10.0f, ANIMMODE_ONCE, -3.0f);
    this->actionTimer = 0;
    this->floatTimer = 0;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 10.0f;
        break;

    case 1:
        this->actor.speed = 14.0f;
        break;

    default:
        break;
    }

    Actor_PlaySfx(&this->actor, NA_SE_EN_DEATH_ATTACK);
    this->actionFunc = EnDeath_SwingAttack;
}

RECOMP_PATCH void EnDeath_BlockProjectile(EnDeath* this, PlayState* play) {
    EnDeath_Float(this);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->actionTimer--;
    if (this->numScytheAfterImages < 7) {
        this->numScytheAfterImages++;
    }
    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_ROLL);
    }
    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
    EnDeath_UpdateSpinAttackTris(this);
    if (this->actionTimer == 0) {

        if (Difficulty == 1) {
            if (Rand_ZeroOne() < 0.5) EnDeath_SetupSpinAttack(this);
            else {
                EnDeath_SetupApproachPlayer(this);
                this->unk_18C = false;
                this->weaponCollider.base.atFlags &= ~AT_ON;
            }
        }
        else {
            if (this->actor.xzDistToPlayer > 200.0f) {
                EnDeath_SetupSpinAttack(this);
            }
            else {
                EnDeath_SetupApproachPlayer(this);
                this->unk_18C = false;
                this->weaponCollider.base.atFlags &= ~AT_ON;
            }
        }
    }
}

// Commented out due to being able to attack him still issues
/*RECOMP_PATCH void EnDeath_BatSwarm(EnDeath* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    EnDeath_Float(this);

    if (this->numScytheAfterImages < 7) {
        this->numScytheAfterImages++;
    }

    if (Difficulty == 1) {
        this->unk_18C = true;

        if (this->skelAnime.animation != &gGomessScytheSpinAnim) {
            Animation_PlayLoop(&this->skelAnime, &gGomessScytheSpinAnim);
        }

        if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
            Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_ROLL);
        }

        Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
        EnDeath_UpdateSpinAttackTris(this);
        this->unk_18C = true;
    }
    else {
        this->unk_18C = false;

        if (this->skelAnime.animation != &gGomessBatSwarmAnim) {
            Animation_PlayLoop(&this->skelAnime, &gGomessBatSwarmAnim);
        }
    }

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
    SkelAnime_Update(&this->skelAnime);

    if (this->actor.params >= 5) {
        play->envCtx.lightSettingOverride = 26;
        EnDeath_SetupApproachPlayer(this);
        this->unk_18C = false;
        return;
    }
}*/

RECOMP_PATCH void EnDeath_SpinAttack(EnDeath* this, PlayState* play) {
    EnDeath_Float(this);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        Audio_PlaySfx_AtPos(&this->scytheScreenPos, NA_SE_EN_DEATH_ROLL);
    }

    if (this->numScytheAfterImages < 7) {
        this->numScytheAfterImages++;
    }

    this->actionTimer++;

    if (this->actionTimer >= 10) {
        if (Difficulty >= 1) {
            this->actor.speed = 15.0f;
        }
        else if (Difficulty == 0) {
            this->actor.speed = 12.5f;
        }
        else {
            this->actor.speed = 10.0f;
        }
    }

    f32 distToPlayer = this->actor.xzDistToPlayer;

    if (Difficulty >= 1) {
        if (distToPlayer > 300.0f) {
            f32 currentSpeed = (this->actor.speed > 0.0f) ? this->actor.speed : 10.0f;
            f32 framesToIntercept = distToPlayer / currentSpeed;
            framesToIntercept = CLAMP_MAX(framesToIntercept, 20.0f);

            Vec3f predictedPos;
            predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * framesToIntercept);
            predictedPos.y = player->actor.world.pos.y;
            predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * framesToIntercept);

            s16 predictedYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

            s16 turnRate = (this->actionTimer > 15) ? 0x400 : 0x1400;
            Math_SmoothStepToS(&this->actor.world.rot.y, predictedYaw, 1, turnRate, 0x100);
            this->actor.shape.rot.y = this->actor.world.rot.y;
        }
    }
    else if (Difficulty == 0) {
        if (distToPlayer > 250.0f) {
            s16 turnRate = (this->actionTimer > 15) ? 0x400 : 0x1400;
            Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 1, turnRate, 0x100);
            this->actor.shape.rot.y = this->actor.world.rot.y;
        }
    }
    else {
        if (this->actionTimer < 10) {
            Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 8, 0x1000, 0x100);
            this->actor.world.rot.y = this->actor.shape.rot.y;
        }
    }

    EnDeath_UpdateSpinAttackTris(this);

    Math_ScaledStepToS(&this->cloakUpperRotationModifier, (s32)(Math_SinS(this->actionTimer * 0x2000) * 0x800) + 0x3000,
        0x1000);
    Math_ScaledStepToS(&this->cloakLowerRotationModifier, (s32)(Math_SinS(this->actionTimer * 0x2000 - 0x8000) * 0x800),
        0x1000);

    s16 yawDiff = this->actor.yawTowardsPlayer - this->actor.shape.rot.y;
    s16 absYawDiff = ABS_ALT(yawDiff);

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) ||
        (this->weaponCollider.base.atFlags & AT_HIT) ||
        (this->weaponSpinningCollider.base.atFlags & AT_HIT) ||
        (distToPlayer < 60.0f) || ((distToPlayer < 150.0f) && (absYawDiff > 0x4300)) || (this->actionTimer > 100)) {

        this->unk_18C = false;
        this->weaponCollider.base.atFlags &= ~(AT_ON | AT_HIT);
        this->weaponSpinningCollider.base.atFlags &= ~AT_HIT;
        this->actor.speed = 0.0f;
        EnDeath_SetupApproachPlayer(this);
    }
}

RECOMP_PATCH void EnDeath_UpdateDamage(EnDeath* this, PlayState* play) {
    s32 i;

    if (this->explosiveDamageTimer > 0) {
        this->explosiveDamageTimer--;
    }
    if (this->coreCollider.base.acFlags & AC_HIT) {
        this->coreCollider.base.acFlags &= ~AC_HIT;

        if (this->actor.params >= 5 && this->actor.colChkInfo.damageEffect == DMGEFF_EXPLOSIVES) {
            return;
        }

        if (this->actor.params >= 5) {
            this->coreGuarded = true;
            this->coreVelocity = -1.0f;
            this->coreRotation = this->actor.shape.rot.y;
            Math_Vec3s_ToVec3f(&this->corePos, &this->coreCollider.dim.worldSphere.center);
            SoundSource_PlaySfxAtFixedWorldPos(play, &this->corePos, 30, NA_SE_EN_FFLY_DEAD);

            if (this->actor.colChkInfo.damageEffect == DMGEFF_LIGHT_ARROW &&
                this->actionFunc != EnDeath_StartBatSwarm) {
                for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
                    this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_SCATTER;
                }
                play->envCtx.lightSettingOverride = 28;

                this->lightArrowDamageTimer = 20;
                if (this->actionFunc == EnDeath_ApproachPlayer) {
                    this->actionTimer = 100;
                }
            }
            else if (this->actor.colChkInfo.damageEffect == DMGEFF_EXPLOSIVES) {
                this->explosiveDamageTimer = 10;
            }
        }
        else if (this->actor.colChkInfo.damageEffect != DMGEFF_EXPLOSIVES || this->explosiveDamageTimer == 0) {
            this->unk_18C = false;
            this->weaponCollider.base.atFlags &= ~AT_ON;
            this->coreCollider.base.acFlags &= ~AC_ON;

            if (this->actor.colChkInfo.damageEffect == DMGEFF_LIGHT_ARROW) {
                this->dmgEffectAlpha = 3.0f;
                this->dmgEffectScale = 0.8f;
                this->dmgEffect = ACTOR_DRAW_DMGEFF_LIGHT_ORBS;
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_CLEAR_TAG, this->coreCollider.elem.acDmgInfo.hitPos.x,
                    this->coreCollider.elem.acDmgInfo.hitPos.y, this->coreCollider.elem.acDmgInfo.hitPos.z, 0,
                    0, 0, 4);
            }
            if (play->envCtx.lightSettingOverride == 27) {
                play->envCtx.lightSettingOverride = 26;
            }
            this->actor.shape.rot.x = 0;

            if (Actor_ApplyDamage(&this->actor) == 0) {
                Enemy_StartFinishingBlow(play, &this->actor);
                Audio_RestorePrevBgm(); // Stop miniboss BGM
                EnDeath_SetupPlayCutscene(this);
            }
            else {
                for (i = 0; i < ARRAY_COUNT(this->miniDeaths); i++) {
                    this->miniDeaths[i]->actor.params = MINIDEATH_ACTION_RETURN;
                }
                EnDeath_SetupDamaged(this);
            }
        }
    }
}

RECOMP_HOOK("EnDeath_Update") void DeathUpdate(Actor* thisx, PlayState* play) {
    EnDeath* this = (EnDeath*)thisx;
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

// This was a helper of the EnDeath_BatSwarm patch
/*RECOMP_HOOK("EnDeath_Draw") void DrawForce(Actor* thisx, PlayState* play) {

    EnDeath* this = (EnDeath*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        break;

    case 1:
        if (this->actionFunc == EnDeath_BatSwarm) {
            EnDeath_DrawScytheSpinning(this, play);
        }
        break;

    default:
        break;
    }
}*/
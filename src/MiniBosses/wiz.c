#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_wiz_fire.h"
#include "attributes.h"
#include "overlays/actors/ovl_En_Wiz/z_en_wiz.h"
#include "assets/objects/object_wiz/object_wiz.h"
#include "z_en_wiz.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Wiz_Brock/z_en_wiz_brock.h"

void EnWiz_Pool(EnWizFire* this, PlayState* play);
void EnWiz_KillMagicProjectile(EnWizFire* this, PlayState* play);
void EnWiz_SetupDance(EnWiz* this);
void EnWiz_SetupSecondPhaseCutscene(EnWiz* this, PlayState* play);
void EnWiz_SetupWindUp(EnWiz* this);
void EnWiz_SetupAttack(EnWiz* this);
void EnWiz_SetupDisappear(EnWiz* this);

typedef enum {
    /* 1 */ EN_WIZ_ACTION_APPEAR = 1,
    /* 2 */ EN_WIZ_ACTION_RUN_BETWEEN_PLATFORMS,
    /* 3 */ EN_WIZ_ACTION_DISAPPEAR,
    /* 4 */ EN_WIZ_ACTION_DAMAGED,
    /* 5 */ EN_WIZ_ACTION_DEAD,
    /* 6 */ EN_WIZ_ACTION_BURST_INTO_FLAMES,
    /* 7 */ EN_WIZ_ACTION_RUN_IN_CIRCLES,
    /* 8 */ EN_WIZ_ACTION_ATTACK,
    /* 9 */ EN_WIZ_ACTION_DANCE
} EnWizAction;

typedef enum {
    /* 0 */ EN_WIZ_FIRE_ACTION_MOVE_MAGIC_PROJECTILE,
    /* 1 */ EN_WIZ_FIRE_ACTION_SMALL_FLAME,
    /* 2 */ EN_WIZ_FIRE_ACTION_POOL,
    /* 3 */ EN_WIZ_FIRE_ACTION_KILL_MAGIC_PROJECTILE
} EnWizFireAction;

static s32 sPoolHitByIceArrow = false;

typedef enum {
    /* 0 */ EN_WIZ_INTRO_CS_NOT_STARTED,
    /* 1 */ EN_WIZ_INTRO_CS_CAMERA_MOVE_TO_PLATFORM,
    /* 2 */ EN_WIZ_INTRO_CS_APPEAR,
    /* 3 */ EN_WIZ_INTRO_CS_CAMERA_SPIN_TO_FACE_WIZROBE,
    /* 4 */ EN_WIZ_INTRO_CS_WAIT_BEFORE_RUN,
    /* 5 */ EN_WIZ_INTRO_CS_RUN_IN_CIRCLES,
    /* 6 */ EN_WIZ_INTRO_CS_DISAPPEAR,
    /* 7 */ EN_WIZ_INTRO_CS_END
} EnWizIntroCutsceneState;

typedef enum {
    /* 0 */ EN_WIZ_FIGHT_STATE_FIRST_PHASE,
    /* 1 */ EN_WIZ_FIGHT_STATE_SECOND_PHASE_CUTSCENE,
    /* 2 */ EN_WIZ_FIGHT_STATE_SECOND_PHASE_GHOSTS_COPY_WIZROBE,
    /* 3 */ EN_WIZ_FIGHT_STATE_SECOND_PHASE_GHOSTS_RUN_AROUND
} EnWizFightState;

typedef enum EnWizAnimation {
    /* 0 */ EN_WIZ_ANIM_IDLE,
    /* 1 */ EN_WIZ_ANIM_RUN,
    /* 2 */ EN_WIZ_ANIM_DANCE,
    /* 3 */ EN_WIZ_ANIM_WIND_UP,
    /* 4 */ EN_WIZ_ANIM_ATTACK,
    /* 5 */ EN_WIZ_ANIM_DAMAGE,
    /* 6 */ EN_WIZ_ANIM_MAX
} EnWizAnimation;

static AnimationHeader* sAnimations[EN_WIZ_ANIM_MAX] = {
    &gWizrobeIdleAnim,   // EN_WIZ_ANIM_IDLE
    &gWizrobeRunAnim,    // EN_WIZ_ANIM_RUN
    &gWizrobeDanceAnim,  // EN_WIZ_ANIM_DANCE
    &gWizrobeWindUpAnim, // EN_WIZ_ANIM_WIND_UP
    &gWizrobeAttackAnim, // EN_WIZ_ANIM_ATTACK
    &gWizrobeDamageAnim, // EN_WIZ_ANIM_DAMAGE
};

static u8 sAnimationModes[EN_WIZ_ANIM_MAX] = {
    ANIMMODE_LOOP, // EN_WIZ_ANIM_IDLE
    ANIMMODE_LOOP, // EN_WIZ_ANIM_RUN
    ANIMMODE_LOOP, // EN_WIZ_ANIM_DANCE
    ANIMMODE_LOOP, // EN_WIZ_ANIM_WIND_UP
    ANIMMODE_LOOP, // EN_WIZ_ANIM_ATTACK
    ANIMMODE_ONCE, // EN_WIZ_ANIM_DAMAGE
};

void EnWiz_ChangeAnim(EnWiz* this, s32 animIndex, s32 updateGhostAnim) {
    this->animEndFrame = Animation_GetLastFrame(sAnimations[animIndex]);
    Animation_Change(&this->skelAnime, sAnimations[animIndex], 1.0f, 0.0f, this->animEndFrame,
        sAnimationModes[animIndex], -2.0f);
    if (updateGhostAnim) {
        Animation_Change(&this->ghostSkelAnime, sAnimations[animIndex], 1.0f, 0.0f, this->animEndFrame,
            sAnimationModes[animIndex], -2.0f);
    }
}

void EnWiz_HandleIntroCutscene(EnWiz* this, PlayState* play) {
    Camera* subCam;
    Vec3f eyeNext;
    Vec3f atNext;

    if (this->introCutsceneState < EN_WIZ_INTRO_CS_DISAPPEAR) {
        subCam = Play_GetCamera(play, this->subCamId);
        switch (this->introCutsceneState) {
        case EN_WIZ_INTRO_CS_NOT_STARTED:
            this->introCutsceneTimer = 100;
            this->introCutsceneCameraAngle = this->actor.world.rot.y;
            this->introCutsceneState++;
            break;

        case EN_WIZ_INTRO_CS_CAMERA_MOVE_TO_PLATFORM:
            Math_Vec3f_Copy(&eyeNext, &this->actor.world.pos);
            Math_Vec3f_Copy(&atNext, &this->actor.world.pos);
            eyeNext.x += Math_SinS(this->introCutsceneCameraAngle) * 200.0f;
            eyeNext.y += 100.0f;
            eyeNext.z += Math_CosS(this->introCutsceneCameraAngle) * 200.0f;
            atNext.y += 80.0f;
            Math_ApproachF(&subCam->eye.x, eyeNext.x, 0.3f, 30.0f);
            Math_ApproachF(&subCam->eye.z, eyeNext.z, 0.3f, 30.0f);
            Math_ApproachF(&subCam->at.x, atNext.x, 0.3f, 30.0f);
            Math_ApproachF(&subCam->at.z, atNext.z, 0.3f, 30.0f);
            subCam->eye.y = eyeNext.y;
            subCam->at.y = atNext.y;
            if ((fabsf(subCam->eye.x - eyeNext.x) < 2.0f) && (fabsf(subCam->eye.y - eyeNext.y) < 2.0f) &&
                (fabsf(subCam->eye.z - eyeNext.z) < 2.0f) && (fabsf(subCam->at.x - atNext.x) < 2.0f) &&
                (fabsf(subCam->at.y - atNext.y) < 2.0f) && (fabsf(subCam->at.z - atNext.z) < 2.0f)) {
                Player* player = GET_PLAYER(play);
                s32 i;

                this->actor.world.rot.y = this->actor.shape.rot.y =
                    Math_Vec3f_Yaw(&this->actor.world.pos, &player->actor.world.pos);

                for (i = 0; i < this->platformCount; i++) {
                    this->ghostRot[i].y = Math_Vec3f_Yaw(&this->ghostPos[i], &player->actor.world.pos);
                }

                EnWiz_ChangeAnim(this, EN_WIZ_ANIM_IDLE, true);
                this->shouldStartTimer = false;
                this->targetPlatformLightAlpha = 255;
                Math_Vec3f_Copy(&this->platformLightPos, &this->actor.world.pos);
                if (this->fightState == EN_WIZ_FIGHT_STATE_FIRST_PHASE) {
                    Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_UNARI);
                }
                else {
                    Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_VOICE - SFX_FLAG);
                }

                this->introCutsceneTimer = 40;
                this->introCutsceneState++;
            }
            break;

        case EN_WIZ_INTRO_CS_APPEAR:
            if (this->introCutsceneTimer == 0) {
                this->introCutsceneTimer = 20;
                this->introCutsceneState++;
            }
            break;

        case EN_WIZ_INTRO_CS_CAMERA_SPIN_TO_FACE_WIZROBE:
            Math_Vec3f_Copy(&eyeNext, &this->actor.world.pos);
            Math_Vec3f_Copy(&atNext, &this->actor.world.pos);
            eyeNext.x += Math_SinS(this->actor.world.rot.y) * 160.0f;
            eyeNext.y += 70.0f;
            eyeNext.z += Math_CosS(this->actor.world.rot.y) * 140.0f;
            atNext.x += -10.0f;
            atNext.y += 100.0f;
            Math_ApproachF(&subCam->eye.x, eyeNext.x, 0.3f, 30.0f);
            Math_ApproachF(&subCam->eye.z, eyeNext.z, 0.3f, 30.0f);
            Math_ApproachF(&subCam->at.x, atNext.x, 0.3f, 30.0f);
            Math_ApproachF(&subCam->at.z, atNext.z, 0.3f, 30.0f);
            subCam->eye.y = eyeNext.y;
            subCam->at.y = atNext.y;
            if (this->introCutsceneTimer == 0) {
                this->introCutsceneTimer = 10;
                this->introCutsceneState++;
                this->introCutsceneCameraAngle = this->actor.world.rot.y;
            }
            break;

        case EN_WIZ_INTRO_CS_WAIT_BEFORE_RUN:
            if (this->introCutsceneTimer == 0) {
                EnWiz_ChangeAnim(this, EN_WIZ_ANIM_RUN, false);
                this->angularVelocity = 0;
                this->introCutsceneTimer = 34;
                this->introCutsceneState++;
            }
            break;

        case EN_WIZ_INTRO_CS_RUN_IN_CIRCLES:
            Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_RUN - SFX_FLAG);
            if (this->introCutsceneTimer == 0) {
                this->animLoopCounter = this->introCutsceneCameraAngle = 0;
                this->introCutsceneState = EN_WIZ_INTRO_CS_DISAPPEAR;
            }
            else {
                Math_SmoothStepToS(&this->angularVelocity, 0x1388, 0x64, 0x3E8, 0x3E8);
                this->actor.world.rot.y += this->angularVelocity;
            }

            Math_Vec3f_Copy(&eyeNext, &this->actor.world.pos);
            Math_Vec3f_Copy(&atNext, &this->actor.world.pos);
            eyeNext.x += Math_SinS(this->introCutsceneCameraAngle) * 200.0f;
            eyeNext.y += 100.0f;
            eyeNext.z += Math_CosS(this->introCutsceneCameraAngle) * 200.0f;
            atNext.y += 80.0f;
            Math_ApproachF(&subCam->eye.x, eyeNext.x, 0.3f, 30.0f);
            Math_ApproachF(&subCam->eye.z, eyeNext.z, 0.3f, 30.0f);
            Math_ApproachF(&subCam->at.x, atNext.x, 0.3f, 30.0f);
            Math_ApproachF(&subCam->at.z, atNext.z, 0.3f, 30.0f);
            subCam->eye.y = eyeNext.y;
            subCam->at.y = atNext.y;
            break;

        default:
            break;
        }

        if (this->musicStartTimer < 11) {
            this->musicStartTimer++;
            if ((this->type != EN_WIZ_TYPE_FIRE_NO_BGM) && (this->musicStartTimer == 11)) {
                Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
            }
        }
    }
}

RECOMP_PATCH void EnWiz_MoveMagicProjectile(EnWizFire* this, PlayState* play) {
    Vec3f velocity = { 0.0f, 0.0f, 0.0f };

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.5f;
        break;
    case 1:
        speedMultiplier = 3.0f;
        break;
    default:
        break;
    }

    if (speedMultiplier != 1.0f && this->timer > 0 && !this->hasSpawnedSmallFlame) {

        if (this->actor.speed != 0.0f) {
            this->actor.speed *= speedMultiplier;
        }
        else {
            if (this->timer == 100 || this->timer == 99) {
                this->actor.velocity.x *= speedMultiplier;
                this->actor.velocity.y *= speedMultiplier;
                this->actor.velocity.z *= speedMultiplier;
            }
        }
    }

    this->actor.world.rot.z += 5000;

    if (this->type != EN_WIZ_FIRE_TYPE_MAGIC_PROJECTILE) {
        this->targetScale = 0.01f;
    }
    else {
        this->targetScale = 0.02f;
    }

    if ((this->timer == 0) && (this->scale < 0.001f)) {
        Math_Vec3f_Copy(&this->actor.velocity, &gZeroVec3f);
        this->action = EN_WIZ_FIRE_ACTION_KILL_MAGIC_PROJECTILE;
        this->increaseLowestUsedIndexTimer = 0;
        this->actionFunc = EnWiz_KillMagicProjectile;
        return;
    }

    if (this->timer == 0) {
        this->targetScale = 0.0f;
    }

    Math_ApproachF(&this->scale, this->targetScale, 0.2f, 0.01f);

    if (this->wallCheckTimer == 0) {
        if ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) && (this->type == EN_WIZ_FIRE_TYPE_MAGIC_PROJECTILE) &&
            (this->timer != 0) && (this->actor.bgCheckFlags & BGCHECKFLAG_WALL)) {
            sPoolHitByIceArrow = false;
            this->timer = 0;
            this->targetScale = 0.0f;
        }
    }

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && (this->poolTimer == 0)) {
        s32 i;
        s16 arcingProjectileRotY;
        s32 pad;

        if (this->type == EN_WIZ_FIRE_TYPE_ARCING_MAGIC_PROJECTILE) {
            this->increaseLowestUsedIndexTimer = 10;

            Matrix_Push();
            Matrix_RotateYS(TRUNCF_BINANG(Rand_CenteredFloat(0x100)) + this->actor.world.rot.y, MTXMODE_NEW);

            velocity.z = (Rand_CenteredFloat(2.0f) + 8.0f) * speedMultiplier;
            Matrix_MultVec3f(&velocity, &this->actor.velocity);
            Matrix_Pop();

            this->actor.velocity.y = 6.0f * speedMultiplier;
            this->actor.gravity = -0.7f * speedMultiplier;

            if (!this->hasSpawnedSmallFlame) {
                Actor_Spawn(&play->actorCtx, play, ACTOR_EN_WIZ_FIRE, this->actor.world.pos.x,
                    this->actor.world.pos.y - 10.0f, this->actor.world.pos.z, 0, 0, 0,
                    EN_WIZ_FIRE_TYPE_SMALL_FLAME);
                this->hasSpawnedSmallFlame = true;
            }

            this->timer = 0;
            this->scale = 0.0f;
            Math_Vec3f_Copy(&this->actor.velocity, &gZeroVec3f);
            this->action = EN_WIZ_FIRE_ACTION_KILL_MAGIC_PROJECTILE;
            this->increaseLowestUsedIndexTimer = 0;
            this->actionFunc = EnWiz_KillMagicProjectile;
            return;
        }

        if ((this->type == EN_WIZ_FIRE_TYPE_MAGIC_PROJECTILE) && (this->timer != 0)) {
            if (this->actor.floorBgId == BGCHECK_SCENE) {
                this->poolTimer = 100;
                if (!this->isIceType) {
                    arcingProjectileRotY = 0;

                    for (i = 0; i < 5; i++) {
                        Actor_Spawn(&play->actorCtx, play, ACTOR_EN_WIZ_FIRE, this->actor.world.pos.x,
                            this->actor.world.pos.y, this->actor.world.pos.z, 0, arcingProjectileRotY, 0,
                            EN_WIZ_FIRE_TYPE_ARCING_MAGIC_PROJECTILE);
                        arcingProjectileRotY += BINANG_ADD((s32)Rand_CenteredFloat(0x1000), 0x10000 / 5);
                    }

                    Actor_PlaySfx(&this->actor, NA_SE_IT_BOMB_EXPLOSION);
                    this->poolTimer = Rand_S16Offset(70, 30);
                    if (this->poolTimer != 0) {
                        Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_EXP - SFX_FLAG);
                    }
                }
                else if (this->poolTimer != 0) {
                    Actor_PlaySfx(&this->actor, NA_SE_EV_ICE_FREEZE - SFX_FLAG);
                }

                Math_Vec3f_Copy(&this->actor.velocity, &gZeroVec3f);
                this->timer = 0;
                this->action = EN_WIZ_FIRE_ACTION_POOL;
                this->scale = 0.0f;
                this->actionFunc = EnWiz_Pool;
            }

            return;
        }
    }

    if ((this->type != EN_WIZ_FIRE_TYPE_REFLECTED_MAGIC_PROJECTILE) && (this->timer != 0)) {
        if (this->collider.base.acFlags & AC_HIT) {
            this->collider.base.acFlags &= ~AC_HIT;
            if (this->collider.elem.acHitElem->atDmgInfo.dmgFlags == 0x1000) {
                this->timer = 0;
                this->hitByIceArrow = true;
                SoundSource_PlaySfxAtFixedWorldPos(play, &this->actor.world.pos, 50, NA_SE_EV_ICE_MELT);
            }
        }

        if (Player_HasMirrorShieldEquipped(play) && (this->collider.base.atFlags & AT_BOUNCED)) {
            Actor_PlaySfx(&this->actor, NA_SE_IT_SHIELD_REFLECT_MG);
            this->collider.base.atFlags &= ~(AT_TYPE_ENEMY | AT_BOUNCED | AT_HIT);
            this->collider.base.atFlags |= AT_TYPE_PLAYER;
            this->collider.elem.atDmgInfo.dmgFlags = 0x20;
            this->collider.elem.atDmgInfo.damage = 2;
            this->timer = 100;
            this->type = EN_WIZ_FIRE_TYPE_REFLECTED_MAGIC_PROJECTILE;

            this->actor.velocity.x *= -1.0f;
            this->actor.velocity.y *= -0.5f;
            this->actor.velocity.z *= -1.0f;

            if ((this->actor.parent != NULL) && (this->actor.parent->id == ACTOR_EN_WIZ) &&
                (this->actor.parent->update != NULL)) {
                ((EnWiz*)this->actor.parent)->hasActiveProjectile = false;
            }
        }
    }
}

RECOMP_PATCH void EnWiz_Appear(EnWiz* this, PlayState* play) {

    static Vec3f staffTargetFlameScale = { 0.006f, 0.006f, 0.006f };
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    EnWiz_HandleIntroCutscene(this, play);

    if (this->introCutsceneState >= EN_WIZ_INTRO_CS_APPEAR) {
        SkelAnime_Update(&this->skelAnime);

        if ((this->fightState == EN_WIZ_FIGHT_STATE_FIRST_PHASE) &&
            (this->introCutsceneState >= EN_WIZ_INTRO_CS_DISAPPEAR) &&
            ((this->actor.xzDistToPlayer < 200.0f) ||
                ((player->unk_D57 != 0) &&
                    (ABS_ALT(BINANG_SUB(this->actor.yawTowardsPlayer, this->actor.shape.rot.y)) < 0x7D0) &&
                    (ABS_ALT(BINANG_SUB(this->actor.yawTowardsPlayer, BINANG_ADD(player->actor.shape.rot.y, 0x8000))) < 0x7D0)))) {

            EnWiz_SetupDisappear(this);
        }

        Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 0xA, 0xBB8, 0);

        if (this->fightState == EN_WIZ_FIGHT_STATE_FIRST_PHASE) {
            Math_SmoothStepToS(&this->platformLightAlpha, this->targetPlatformLightAlpha, 10, 10, 10);
            if (!this->shouldStartTimer) {

                switch (Difficulty) {
                case 0:
                    this->timer = 10;
                    break;

                case 1:
                    this->timer = 1;
                    break;

                default:
                    break;
                }
                this->shouldStartTimer = true;
            }
        }
        else {
            Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_VOICE - SFX_FLAG);
        }

        switch (Difficulty) {
        case 0:
            if (this->timer == 0) {
                Math_ApproachF(&this->scale, 0.015f, 0.075f, 0.015f);
                Math_SmoothStepToS(&this->alpha, 255, 1, 8, 0);
            }
            break;

        case 1:
            if (this->timer == 0) {
                Math_ApproachF(&this->scale, 0.015f, 0.15f, 0.03f);
                Math_SmoothStepToS(&this->alpha, 255, 1, 15, 0);
            }
            break;

        default:
            break;
        }

        if (this->scale < 0.0138f) {
            return;
        }

        this->action = EN_WIZ_ACTION_RUN_IN_CIRCLES;
        this->actor.flags &= ~ACTOR_FLAG_LOCK_ON_DISABLED;
        this->ghostColliders.elements[0].base.acDmgInfo.dmgFlags = 0x1013A22;
        Math_Vec3f_Copy(&this->staffTargetFlameScale, &staffTargetFlameScale);
        this->targetPlatformLightAlpha = 0;

        if (this->introCutsceneState == EN_WIZ_INTRO_CS_DISAPPEAR) {
            this->timer = 0;
            this->introCutsceneTimer = 20;
            EnWiz_SetupDisappear(this);
        }
        else if (this->introCutsceneState >= EN_WIZ_INTRO_CS_END) {
            if (this->fightState == EN_WIZ_FIGHT_STATE_SECOND_PHASE_CUTSCENE) {
                this->actionFunc = EnWiz_SetupSecondPhaseCutscene;
            }
            else {
                EnWiz_SetupDance(this);
            }
        }
    }
}

RECOMP_HOOK("EnWiz_Dance") void DanceBuff(EnWiz* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (this->animLoopCounter >= 2) {
            this->targetPlatformLightAlpha = 0;
            EnWiz_SetupWindUp(this);
        }
        break;

    case 1:
        if (this->animLoopCounter >= 1) {
            this->targetPlatformLightAlpha = 0;
            EnWiz_SetupWindUp(this);
        }
        break;

    default:
        break;
    }
    this->animLoopCounter = 0;
}

RECOMP_HOOK("EnWiz_WindUp") void WindEdit(EnWiz* this, PlayState* play) {

    f32 curFrame = this->skelAnime.curFrame;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 1.75;
        if (curFrame >= this->animEndFrame) {
            this->animLoopCounter++;
            if (this->animLoopCounter >= 2) {
                EnWiz_SetupAttack(this);
            }
        }
        break;

    case 1:
        this->skelAnime.playSpeed = 3.5;
        if (curFrame >= this->animEndFrame) {
            this->animLoopCounter++;
            if (this->animLoopCounter >= 1) {
                EnWiz_SetupAttack(this);
            }
        }
        break;

    default:
        break;
    }
}

void EnWiz_MoveGhosts(EnWiz* this) {
    s32 i;
    s32 ghostNextPlatformIndex;
    s32 playSfx = false;

    for (i = 0; i < this->platformCount; i++) {
        if (this->ghostPos[i].x != 0.0f && this->ghostPos[i].z != 0.0f) {
            f32 diffX;
            f32 diffZ;

            ghostNextPlatformIndex = this->ghostNextPlatformIndex[i];
            diffX = this->platforms[ghostNextPlatformIndex]->world.pos.x - this->ghostPos[i].x;
            diffZ = this->platforms[ghostNextPlatformIndex]->world.pos.z - this->ghostPos[i].z;
            playSfx++;

            if (sqrtf(SQ(diffX) + SQ(diffZ)) < 30.0f) {
                this->ghostNextPlatformIndex[i]--;
                if (this->ghostNextPlatformIndex[i] < 0) {
                    this->ghostNextPlatformIndex[i] = this->platformCount - 1;
                }
            }

            ghostNextPlatformIndex = this->ghostNextPlatformIndex[i];
            Math_ApproachF(&this->ghostPos[i].x, this->platforms[ghostNextPlatformIndex]->world.pos.x, 0.3f, 30.0f);
            Math_ApproachF(&this->ghostPos[i].y, this->platforms[ghostNextPlatformIndex]->world.pos.y, 0.3f, 30.0f);
            Math_ApproachF(&this->ghostPos[i].z, this->platforms[ghostNextPlatformIndex]->world.pos.z, 0.3f, 30.0f);
            this->ghostRot[i].y =
                Math_Vec3f_Yaw(&this->ghostPos[i], &this->platforms[ghostNextPlatformIndex]->world.pos);
        }
    }

    if (playSfx) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_RUN - SFX_FLAG);
    }
}

RECOMP_PATCH void EnWiz_Attack(EnWiz* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    if (this->fightState >= EN_WIZ_FIGHT_STATE_SECOND_PHASE_GHOSTS_RUN_AROUND) {
        EnWiz_MoveGhosts(this);
    }

    if (this->timer == 0) {
        if (Animation_OnFrame(&this->skelAnime, 6.0f)) {
            Player* player = GET_PLAYER(play);
            Vec3f pos;
            s32 type = this->type;

            Math_Vec3f_Copy(&pos, &this->actor.world.pos);
            pos.x += Math_SinS(this->actor.world.rot.y) * 40.0f;
            pos.y += 60.0f;
            pos.z += Math_CosS(this->actor.world.rot.y) * 40.0f;
            if (type == EN_WIZ_TYPE_FIRE_NO_BGM) {
                type = EN_WIZ_TYPE_FIRE;
            }

            Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_WIZ_FIRE, pos.x, pos.y, pos.z,
                Math_Vec3f_Pitch(&pos, &player->actor.world.pos),
                Math_Vec3f_Yaw(&pos, &player->actor.world.pos), 0, type * 4);
            this->hasActiveProjectile = true;
            Actor_PlaySfx(&this->actor, NA_SE_EN_WIZ_ATTACK);
            Actor_PlaySfx(&this->actor, NA_SE_PL_MAGIC_FIRE);
        }

        if ((curFrame >= 8.0f) && !this->shouldStartTimer) {
            this->timer = 3;
            this->shouldStartTimer = true;
        }

        if (curFrame >= this->animEndFrame) {
            EnWiz_SetupDisappear(this);
        }
    }
}

RECOMP_HOOK("EnWiz_Update") void WizUpdate(Actor* thisx, PlayState* play) {

    EnWiz* this = (EnWiz*)thisx;

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
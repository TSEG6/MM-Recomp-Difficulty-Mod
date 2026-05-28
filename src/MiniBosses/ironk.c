
// welcome to patch city

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_ik.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

s32 EnIk_IsChangingAction(EnIk* this, PlayState* play);
void EnIk_CheckActions(EnIk* this, PlayState* play);
void EnIk_SetupIdle(EnIk* this);
void EnIk_SetupWalk(EnIk* this);
void EnIk_SetupVerticalAttack(EnIk* this);
void EnIk_SetupTakeOutAxe(EnIk* this);
void EnIk_SetupHorizontalDoubleAttack(EnIk* this);
void EnIk_SetupSingleHorizontalAttack(EnIk* this);
void EnIk_SetupEndHorizontalAttack(EnIk* this);
void EnIk_SetupBlock(EnIk* this);
void EnIk_Block(EnIk* this, PlayState* play);
void EnIk_ReactToAttack(EnIk* this, PlayState* play);
void EnIk_PlayCutscene(EnIk* this, PlayState* play);
void EnIk_UpdateDamage(EnIk* this, PlayState* play);
void EnIk_UpdateArmor(EnIk* this, PlayState* play);

s32 isArmorBroken = false;
bool Wake = false;
bool HB = false;

RECOMP_HOOK("EnIk_TakeOutAxe") void fasterTakeout(EnIk* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 2.5f;
        break;

    case 1:
        this->skelAnime.playSpeed = 5.0f;
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnIk_WalkTowardsPlayer") void WalkingButFaster(EnIk* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.25f;
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x420);
        break;

    case 1:
        speedMultiplier = 1.5f;
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x620);
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0) {
        this->skelAnime.playSpeed = speedMultiplier * 1.5;
    }

    if (this->actor.speed <= 2.33f) {
        this->actor.speed *= speedMultiplier;
    }
    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;
}

RECOMP_HOOK("EnIk_RunTowardsPlayer") void RunningButFaster(EnIk* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.25f;
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x420);
        break;

    case 1:
        speedMultiplier = 1.5f;
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x620);
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0) {
        this->skelAnime.playSpeed = speedMultiplier;
    }

    if (this->actor.speed <= 5.0f) {
        this->actor.speed *= speedMultiplier;
    }
    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;
}

RECOMP_PATCH void EnIk_HorizontalDoubleAttack(EnIk* this, PlayState* play) {
    f32 phi_f2;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float lungeMultiplier = 4.5f;

    if (Difficulty == 1) {
        this->skelAnime.playSpeed = 1.25f;
    }
    else if (Difficulty == 0) {
        this->skelAnime.playSpeed = 1.1f;
    }

    this->timer++;

    if (Animation_OnFrame(&this->skelAnime, 1.0f) || Animation_OnFrame(&this->skelAnime, 13.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_IRONNACK_SWING_AXE);
    }

    if (((this->skelAnime.curFrame > 1.0f) && (this->skelAnime.curFrame < 9.0f)) ||
        ((this->skelAnime.curFrame > 12.0f) && (this->skelAnime.curFrame < 20.0f))) {

        if (this->drawArmorFlags != 0 || Difficulty == 0 || Difficulty == 1) {
            s16 targetYaw = this->actor.yawTowardsPlayer;

            if (Difficulty == 1 && this->skelAnime.curFrame > 12.0f) {
                Player* player = GET_PLAYER(play);
                Vec3f predictedPos;
                float impactFrame = 20.0f;
                float framesToImpact = impactFrame - this->skelAnime.curFrame;
                float predictionScale = 1.0f;
                if (this->actor.xzDistToPlayer < 100.0f) {
                    predictionScale = this->actor.xzDistToPlayer / 100.0f;
                }

                float offsetX = player->actor.velocity.x * framesToImpact * predictionScale;
                float offsetZ = player->actor.velocity.z * framesToImpact * predictionScale;
                float maxOffset = 50.0f;
                float offsetSq = (offsetX * offsetX) + (offsetZ * offsetZ);

                if (offsetSq > (maxOffset * maxOffset)) {
                    float scale = maxOffset / sqrtf(offsetSq);
                    offsetX *= scale;
                    offsetZ *= scale;
                }

                predictedPos.x = player->actor.world.pos.x + offsetX;
                predictedPos.z = player->actor.world.pos.z + offsetZ;
                predictedPos.y = player->actor.world.pos.y;

                targetYaw = Actor_WorldYawTowardPoint(&this->actor, &predictedPos);
            }

            Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 0x5DC);
            this->actor.world.rot.y = this->actor.shape.rot.y;

            if (this->skelAnime.curFrame > 12.0f) {
                phi_f2 = this->skelAnime.curFrame - 12.0f;
            }
            else {
                phi_f2 = this->skelAnime.curFrame - 1.0f;
            }

            if (Difficulty == 0) {
                lungeMultiplier = 7.5f;
            }
            else if (Difficulty == 1) {
                lungeMultiplier = 11.0f;

                float distanceScale = this->actor.xzDistToPlayer / 120.0f;

                if (distanceScale < 0.4f) distanceScale = 0.4f;
                if (distanceScale > 1.6f) distanceScale = 1.6f;

                lungeMultiplier *= distanceScale;
            }
            this->actor.speed = Math_SinF((M_PIf / 8.0f) * phi_f2) * lungeMultiplier * this->skelAnime.playSpeed;
        }

        this->colliderQuad.base.atFlags |= AT_ON;
    }
    else {
        this->colliderQuad.base.atFlags &= ~AT_ON;
        this->actor.speed = 0.0f;
    }

    if (SkelAnime_Update(&this->skelAnime)) {
        EnIk_SetupEndHorizontalAttack(this);
    }
}

RECOMP_PATCH void EnIk_VerticalAttack(EnIk* this, PlayState* play) {
    Vec3f particlePos;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->timer++;
    if (!(this->skelAnime.curFrame < 7.0f) || !EnIk_IsChangingAction(this, play)) {
        if (Animation_OnFrame(&this->skelAnime, 15.0f)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_IRONNACK_SWING_AXE);
        }
        else if (Animation_OnFrame(&this->skelAnime, 21.0f)) {
            particlePos.x = this->actor.world.pos.x + (Math_SinS(this->actor.shape.rot.y + 0x6A4) * 70.0f);
            particlePos.z = this->actor.world.pos.z + (Math_CosS(this->actor.shape.rot.y + 0x6A4) * 70.0f);
            particlePos.y = this->actor.world.pos.y;
            Actor_PlaySfx(&this->actor, NA_SE_EN_IRONNACK_HIT_GND);
            Camera_AddQuake(GET_ACTIVE_CAM(play), 2, 25, 5);
            Rumble_Request(this->actor.xyzDistToPlayerSq, 180, 20, 100);
            CollisionCheck_SpawnShieldParticles(play, &particlePos);
        }

        if ((this->skelAnime.curFrame > 13.0f) && (this->skelAnime.curFrame < 23.0f)) {
            this->colliderQuad.base.atFlags |= AT_ON;

            float speedMultiplier = 10.0f;
            bool shouldLunge = (this->drawArmorFlags != 0);

            if (Difficulty == 0) {
                speedMultiplier = 10.0f;
                shouldLunge = true;
                this->skelAnime.playSpeed = 1.1f;
            }
            else if (Difficulty == 1) {
                speedMultiplier = 15.0f;
                shouldLunge = true;
                this->skelAnime.playSpeed = 1.25f;

                float distanceScale = this->actor.xzDistToPlayer / 120.0f;

                if (distanceScale < 0.4f) distanceScale = 0.4f;
                if (distanceScale > 1.6f) distanceScale = 1.6f;

                speedMultiplier *= distanceScale;
            }

            if (shouldLunge) {
                this->actor.speed = Math_SinF((this->skelAnime.curFrame - 13.0f) * (M_PIf / 20)) * speedMultiplier;
            }
        }
        else {
            this->colliderQuad.base.atFlags &= ~AT_ON;
            this->actor.speed = 0.0f;
        }

        if ((this->drawArmorFlags || Difficulty == 0 || Difficulty == 1) && (this->skelAnime.curFrame < 13.0f)) {
            s16 targetYaw = this->actor.yawTowardsPlayer;

            if (Difficulty == 1) {
                Player* player = GET_PLAYER(play);
                Vec3f predictedPos;
                float framesToImpact = 21.0f - this->skelAnime.curFrame;
                float predictionScale = 1.0f;
                if (this->actor.xzDistToPlayer < 100.0f) {
                    predictionScale = this->actor.xzDistToPlayer / 100.0f;
                }

                float offsetX = player->actor.velocity.x * framesToImpact * predictionScale;
                float offsetZ = player->actor.velocity.z * framesToImpact * predictionScale;
                float maxOffset = 50.0f;
                float offsetSq = (offsetX * offsetX) + (offsetZ * offsetZ);

                if (offsetSq > (maxOffset * maxOffset)) {
                    float scale = maxOffset / sqrtf(offsetSq);
                    offsetX *= scale;
                    offsetZ *= scale;
                }

                predictedPos.x = player->actor.world.pos.x + offsetX;
                predictedPos.z = player->actor.world.pos.z + offsetZ;
                predictedPos.y = player->actor.world.pos.y;

                targetYaw = Actor_WorldYawTowardPoint(&this->actor, &predictedPos);
            }
            Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 0x5DC);
            this->actor.world.rot.y = this->actor.shape.rot.y;
        }

        if (SkelAnime_Update(&this->skelAnime)) {
            EnIk_SetupTakeOutAxe(this);
        }
    }
}

RECOMP_PATCH void EnIk_SingleHorizontalAttack(EnIk* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 1.1f;
        break;

    case 1:
        this->skelAnime.playSpeed = 1.25f;
        break;

    default:
        break;
    }

    this->timer++;

    if (Animation_OnFrame(&this->skelAnime, 13.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_IRONNACK_SWING_AXE);
    }

    if ((this->skelAnime.curFrame > 12.0f) && (this->skelAnime.curFrame < 20.0f)) {
        this->colliderQuad.base.atFlags |= AT_ON;

        float speedMultiplier = 0.0f;
        if (Difficulty == 0) {
            speedMultiplier = 10.0f;
        }
        else if (Difficulty == 1) {
            speedMultiplier = 15.0f;
        }

        if (speedMultiplier > 0.0f) {
            float calculatedSpeed = Math_SinF((this->skelAnime.curFrame - 12.0f) * (M_PIf / 8.0f)) * speedMultiplier * this->skelAnime.playSpeed;
            s16 lungeYaw = this->actor.yawTowardsPlayer;

            if (Difficulty == 1) {
                Player* player = GET_PLAYER(play);
                Vec3f predictedPos;
                predictedPos.x = player->actor.world.pos.x + player->actor.velocity.x * (8.0f / this->skelAnime.playSpeed);
                predictedPos.z = player->actor.world.pos.z + player->actor.velocity.z * (8.0f / this->skelAnime.playSpeed);
                predictedPos.y = player->actor.world.pos.y;

                lungeYaw = Actor_WorldYawTowardPoint(&this->actor, &predictedPos);
            }
            this->actor.world.pos.x += Math_SinS(lungeYaw) * calculatedSpeed;
            this->actor.world.pos.z += Math_CosS(lungeYaw) * calculatedSpeed;
        }
        this->actor.speed = 0.0f;
    }
    else {
        this->colliderQuad.base.atFlags &= ~AT_ON;
        this->actor.speed = 0.0f;
    }

    if (this->skelAnime.curFrame < 12.0f) {
        s16 targetYaw = this->actor.yawTowardsPlayer;

        if (Difficulty == 1) {
            Player* player = GET_PLAYER(play);
            Vec3f predictedPos;
            predictedPos.x = player->actor.world.pos.x + player->actor.velocity.x * (8.0f / this->skelAnime.playSpeed);
            predictedPos.z = player->actor.world.pos.z + player->actor.velocity.z * (8.0f / this->skelAnime.playSpeed);
            predictedPos.y = player->actor.world.pos.y;

            targetYaw = Actor_WorldYawTowardPoint(&this->actor, &predictedPos);
        }

        if (Difficulty == 0 || Difficulty == 1 || this->drawArmorFlags) {
            Math_ScaledStepToS(&this->actor.shape.rot.y, targetYaw, 0x5DC);
        }
    }

    this->actor.shape.rot.y += (s16)(this->actor.world.rot.z * this->skelAnime.playSpeed);
    this->actor.world.rot.y = this->actor.shape.rot.y;

    if (SkelAnime_Update(&this->skelAnime)) {
        EnIk_SetupEndHorizontalAttack(this);
    }
}

RECOMP_PATCH s32 EnIk_IsChangingAction(EnIk* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if ((this->drawArmorFlags || Difficulty == 0 || Difficulty == 1) &&
        (this->actionFunc != EnIk_Block) &&
        func_800BE184(play, &this->actor, 100.0f, 0x2710, 0x4000, this->actor.shape.rot.y)) {

        EnIk_SetupBlock(this);
        return true;
    }
    return false;
}

RECOMP_PATCH s32 EnIk_ChooseAttack(EnIk* this) {
    s32 absYawDiff;
    s32 detectionThreshold;
    f32 maxDist = 100.0f;
    f32 maxHeight = 150.0f;
    f32 behindAttackChance = 0.1f;
    f32 verticalAttackChance = 0.5f;
    s32 armorOffThreshold = 0xAAA;
    s32 armorOnThreshold = 0x3FFC;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        maxDist = 100.0f;
        behindAttackChance = 0.4f;
        armorOffThreshold = 0x2000;
        verticalAttackChance = 0.3f;
        break;

    case 1:
        maxDist = 100.0f;
        maxHeight = 200.0f;
        behindAttackChance = 0.4f;
        armorOffThreshold = 0x3FFC;
        verticalAttackChance = 0.6f;
        break;

    default:
        break;
    }

    if ((this->actor.xzDistToPlayer < maxDist) && (fabsf(this->actor.playerHeightRel) < maxHeight)) {

        detectionThreshold = (this->drawArmorFlags == 0) ? armorOffThreshold : armorOnThreshold;
        absYawDiff = ABS_ALT(BINANG_SUB(this->actor.yawTowardsPlayer, this->actor.shape.rot.y));

        if (detectionThreshold >= absYawDiff) {
            if (Rand_ZeroOne() < verticalAttackChance) {
                EnIk_SetupVerticalAttack(this);
                return true;
            }
            EnIk_SetupHorizontalDoubleAttack(this);
            return true;
        }
        else if ((this->drawArmorFlags) || ((absYawDiff > 0x4000) && (Rand_ZeroOne() < behindAttackChance))) {
            EnIk_SetupSingleHorizontalAttack(this);
            return true;
        }
    }
    return false;
}

RECOMP_HOOK("EnIk_Update") void IKUpdate(Actor* thisx, PlayState* play2) {

	EnIk* this = (EnIk*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;

        if (!Wake) {

            if (this->actor.xzDistToPlayer < 150) {

                Audio_PlayBgm_StorePrevBgm(NA_BGM_MINI_BOSS);
                this->actor.hintId = TATL_HINT_ID_IRON_KNUCKLE;
                this->colliderCylinder.base.acFlags &= ~AC_HIT;
                this->invincibilityFrames = 12;
                EnIk_SetupVerticalAttack(this);
                Wake = true;
            }
        }
        break;
    default:
        break;
    }
}

RECOMP_HOOK("EnIk_Destroy") void ResetWake(Actor* thisx, PlayState* play) {

    Wake = false;
    HB = false;
}

RECOMP_PATCH void EnIk_SetupReactToAttack(EnIk* this, s32 arg1) {
    s16 temp_v0;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (arg1) {
        func_800BE504(&this->actor, &this->colliderCylinder);
    }

    if (Difficulty >= 1) {
        this->actor.speed = 3.0f;
    }
    else {
        this->actor.speed = 5.0f;
    }

    temp_v0 = (this->actor.world.rot.y - this->actor.shape.rot.y) + 0x8000;
    if (ABS_ALT(temp_v0) <= 0x4000) {
        Animation_MorphToPlayOnce(&this->skelAnime, &gIronKnuckleFrontHitAnim, -4.0f);
    }
    else {
        Animation_MorphToPlayOnce(&this->skelAnime, &gIronKnuckleBackHitAnim, -4.0f);
    }

    if (Difficulty >= 1 && this->subCamId == SUB_CAM_ID_DONE) {
        this->skelAnime.playSpeed = 4.0f;
    }

    this->actionFunc = EnIk_ReactToAttack;
}

RECOMP_PATCH void EnIk_ReactToAttack(EnIk* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty >= 1) {
        Math_StepToF(&this->actor.speed, 0.0f, 2.0f);
    }
    else {
        Math_StepToF(&this->actor.speed, 0.0f, 1.0f);
    }

    if (this->subCamId != SUB_CAM_ID_DONE) {
        Play_SetCameraAtEye(play, this->subCamId, &this->actor.focus.pos, &Play_GetCamera(play, this->subCamId)->eye);
        if (!HB) {
            this->actor.colChkInfo.health = 18;
            HB = true;
        }
    }
    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->subCamId != SUB_CAM_ID_DONE) {
            CutsceneManager_Stop(this->actor.csId);
            this->subCamId = SUB_CAM_ID_DONE;
            EnIk_SetupIdle(this);
        }
        else {
            EnIk_CheckActions(this, play);
        }
    }
    else {
        switch (Difficulty) {
        case 0:
            this->actor.colorFilterTimer = 24;
            this->invincibilityFrames = 24;
            break;

        case 1:
            this->actor.colorFilterTimer = 40;
            this->invincibilityFrames = 40;
            break;

        default:
            break;
        }
    }
}

// trying to make it so you get hit more when crouching lol, it barely works
void EnIk_SetATKC(PlayState* play, CollisionCheckContext* colChkCtx, Collider* collider) {
    ColliderQuad* quad = (ColliderQuad*)collider;

    quad->dim.quad[1].y -= 75.0f;
    quad->dim.quad[3].y -= 75.0f;
    quad->dim.quad[0].y -= 25.0f;
    quad->dim.quad[2].y -= 25.0f;

    CollisionCheck_SetAT(play, colChkCtx, collider);
}

RECOMP_PATCH void EnIk_Update(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnIk* this = (EnIk*)thisx;

    if (this->actionFunc != EnIk_PlayCutscene) {
        EnIk_UpdateDamage(this, play);
    }
    else {
        this->colliderTris.base.acFlags &= ~AC_BOUNCED;
        this->colliderCylinder.base.acFlags &= ~AC_HIT;
    }
    this->actionFunc(this, play);
    Actor_MoveWithGravity(&this->actor);
    Actor_UpdateBgCheckInfo(play, &this->actor, 75.0f, 30.0f, 30.0f,
        UPDBGCHECKINFO_FLAG_1 | UPDBGCHECKINFO_FLAG_4 | UPDBGCHECKINFO_FLAG_8 |
        UPDBGCHECKINFO_FLAG_10);
    this->actor.focus.rot.y = this->actor.shape.rot.y;
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->colliderCylinder.base);
    if (this->invincibilityFrames == 0) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->colliderCylinder.base);
    }
    else {
        this->invincibilityFrames--;
    }
    if (this->colliderQuad.base.atFlags & AT_ON) {
        EnIk_SetATKC(play, &play->colChkCtx, &this->colliderQuad.base);
    }
    if (this->actionFunc == EnIk_Block) {
        f32 cos;
        f32 sin;
        Vec3f sp5C;
        Vec3f sp50;
        Vec3f sp44;

        cos = Math_CosS(this->actor.shape.rot.y);
        sin = Math_SinS(this->actor.shape.rot.y);

        sp44.x = (this->actor.world.pos.x - (30.0f * cos)) + (20.0f * sin);
        sp44.y = this->actor.world.pos.y;
        sp44.z = this->actor.world.pos.z + (30.0f * sin) + (20.0f * cos);

        sp50.x = this->actor.world.pos.x + (30.0f * cos) + (20.0f * sin);
        sp50.y = this->actor.world.pos.y + 80.0f;
        sp50.z = (this->actor.world.pos.z - (30.0f * sin)) + (20.0f * cos);

        sp5C.x = sp44.x;
        sp5C.y = sp50.y;
        sp5C.z = sp44.z;

        Collider_SetTrisVertices(&this->colliderTris, 0, &sp44, &sp50, &sp5C);
        sp5C.x = sp50.x;
        sp5C.y = sp44.y;
        sp5C.z = sp50.z;
        Collider_SetTrisVertices(&this->colliderTris, 1, &sp44, &sp5C, &sp50);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->colliderTris.base);
    }
    if (this->drawDmgEffAlpha > 0.0f) {
        if (this->drawDmgEffType != ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX) {
            Math_StepToF(&this->drawDmgEffAlpha, 0.0f, 0.05f);
            this->drawDmgEffScale = (this->drawDmgEffAlpha + 1.0f) * 0.325f;
            this->drawDmgEffScale = CLAMP_MAX(this->drawDmgEffScale, 0.65f);
        }
        else if (!Math_StepToF(&this->drawDmgEffFrozenSteamScale, 0.65f, 0.01625f)) {
            Actor_PlaySfx_Flagged(&this->actor, NA_SE_EV_ICE_FREEZE - SFX_FLAG);
        }
    }
    EnIk_UpdateArmor(this, play);
}
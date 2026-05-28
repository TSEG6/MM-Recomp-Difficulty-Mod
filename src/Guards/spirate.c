#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_ge2.h"

void EnGe2_LookAround(EnGe2* this, PlayState* play);

#define GERUDO_PURPLE_STATE_KO (1 << 1)
#define GERUDO_PURPLE_STATE_STUNNED (1 << 2) //!< Specifically by Deku Nuts
#define GERUDO_PURPLE_STATE_CAPTURING (1 << 5) //!< Set but not used

typedef enum {
    /* 0 */ GERUDO_PURPLE_DETECTION_UNDETECTED,
    /* 1 */ GERUDO_PURPLE_DETECTION_HEARD,
    /* 2 */ GERUDO_PURPLE_DETECTION_PROXIMITY //!< Higher priority
} GerudoPurpleDetection;

s32 EnGe2_LookForPlayer(PlayState* play, Actor* actor, Vec3f* pos, s16 yaw, s16 yawRange, f32 xzRange, f32 yRange) {
    s16 yawToPlayer;
    Vec3f posResult;
    CollisionPoly* outPoly;
    Player* player = GET_PLAYER(play);

    if (Player_GetMask(play) == PLAYER_MASK_STONE) {
        return false;
    }
    if (actor->xzDistToPlayer > xzRange) {
        return false;
    }
    if (fabsf(actor->playerHeightRel) > yRange) {
        return false;
    }

    yawToPlayer = actor->yawTowardsPlayer - yaw;
    if ((yawRange > 0) && (yawRange < ABS_ALT(yawToPlayer))) {
        return false;
    }

    if (BgCheck_AnyLineTest1(&play->colCtx, pos, &player->bodyPartsPos[PLAYER_BODYPART_HEAD], &posResult, &outPoly,
        false)) {
        return false;
    }
    else {
        return true;
    }
}

void EnGe2_SpawnEffects(EnGe2* this, PlayState* play) {
    static Vec3f sEffectVelocity = { 0.0f, -0.05f, 0.0f };
    static Vec3f sEffectAccel = { 0.0f, -0.025f, 0.0f };
    static Color_RGBA8 sEffectPrimColor = { 255, 255, 255, 0 };
    static Color_RGBA8 sEffectEnvColor = { 255, 150, 0, 0 };
    s16 effectAngle = play->state.frames * 0x2800;
    Vec3f effectPos;

    effectPos.x = (Math_CosS(effectAngle) * 5.0f) + this->picto.actor.focus.pos.x;
    effectPos.y = this->picto.actor.focus.pos.y + 10.0f;
    effectPos.z = (Math_SinS(effectAngle) * 5.0f) + this->picto.actor.focus.pos.z;
    EffectSsKirakira_SpawnDispersed(play, &effectPos, &sEffectVelocity, &sEffectAccel, &sEffectPrimColor,
        &sEffectEnvColor, 1000, 16);
}

void EnGe2_ThrowPlayerOut(EnGe2* this, PlayState* play) {
    if (this->timer > 0) {
        this->timer--;
    }
    else if (play->nextEntrance != play->setupExitList[GERUDO_PURPLE_GET_EXIT(&this->picto.actor)]) {
        play->nextEntrance = play->setupExitList[GERUDO_PURPLE_GET_EXIT(&this->picto.actor)];
        play->transitionTrigger = TRANS_TRIGGER_START;
        play->transitionType = TRANS_TYPE_38;
    }
}

void EnGe2_TurnToPlayerFast(EnGe2* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->picto.actor.shape.rot.y, this->picto.actor.yawTowardsPlayer, 2, 0x1000, 0x200);
}

void EnGe2_CapturePlayer(EnGe2* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->picto.actor.shape.rot.y, this->picto.actor.yawTowardsPlayer, 2, 0x400, 0x100);
    EnGe2_ThrowPlayerOut(this, play);
}

void EnGe2_SetupCapturePlayer(EnGe2* this) {
    this->picto.actor.speed = 0.0f;
    this->actionFunc = EnGe2_CapturePlayer;
    Animation_Change(&this->skelAnime, &gGerudoPurpleLookingAboutAnim, 1.0f, 0.0f,
        Animation_GetLastFrame(&gGerudoPurpleLookingAboutAnim), ANIMMODE_LOOP, -8.0f);
}

void EnGe2_Charge(EnGe2* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->picto.actor.shape.rot.y, this->picto.actor.yawTowardsPlayer, 2, 0x400, 0x100);
    this->picto.actor.world.rot.y = this->picto.actor.shape.rot.y;

    if (this->picto.actor.xzDistToPlayer < 50.0f) {
        EnGe2_SetupCapturePlayer(this);
    }
    else if (!(this->picto.actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
        this->picto.actor.world.pos = this->picto.actor.prevPos;
        EnGe2_SetupCapturePlayer(this);
    }

    if (this->timer > 0) {
        this->timer--;
    }
    else {
        EnGe2_ThrowPlayerOut(this, play);
    }

    if (Animation_OnFrame(&this->skelAnime, 2.0f) || Animation_OnFrame(&this->skelAnime, 6.0f)) {
        Actor_PlaySfx(&this->picto.actor, NA_SE_EV_PIRATE_WALK);
    }
}

void EnGe2_SetupCharge(EnGe2* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_SmoothStepToS(&this->picto.actor.shape.rot.y, this->picto.actor.yawTowardsPlayer, 2, 0x400, 0x100);
    this->picto.actor.world.rot.y = this->picto.actor.shape.rot.y;

    if (this->picto.actor.shape.rot.y == this->picto.actor.yawTowardsPlayer) {
        Animation_Change(&this->skelAnime, &gGerudoPurpleChargingAnim, 1.0f, 0.0f,
            Animation_GetLastFrame(&gGerudoPurpleChargingAnim), ANIMMODE_LOOP, -8.0f);
        this->timer = 50;
        this->picto.actor.speed = 4.0f;
        this->actionFunc = EnGe2_Charge;
    }
}

void EnGe2_SetupLookAround(EnGe2* this) {
    Animation_Change(&this->skelAnime, &gGerudoPurpleLookingAboutAnim, 1.0f, 0.0f,
        Animation_GetLastFrame(&gGerudoPurpleLookingAboutAnim), ANIMMODE_LOOP, -8.0f);
    this->timer = 60;
    this->picto.actor.speed = 0.0f;
    this->actionFunc = EnGe2_LookAround;
}

void EnGe2_Stunned(EnGe2* this, PlayState* play) {
    if (this->picto.actor.colorFilterTimer == 0) {
        EnGe2_SetupLookAround(this);
        this->detectedStatus = GERUDO_PURPLE_DETECTION_UNDETECTED;
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        this->stateFlags &= ~GERUDO_PURPLE_STATE_STUNNED;
    }
}

void EnGe2_KnockedOut(EnGe2* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    EnGe2_SpawnEffects(this, play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->timer > 0) {
        this->timer--;
        if (Difficulty == 0) this->timer--;
        else {
            this->timer = this->timer - 3;
        }
    }
    else {
        EnGe2_SetupLookAround(this);
        this->detectedStatus = GERUDO_PURPLE_DETECTION_UNDETECTED;
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
        this->stateFlags &= ~GERUDO_PURPLE_STATE_KO;
        this->picto.actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    }
}


RECOMP_PATCH void EnGe2_PatrolDuties(EnGe2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float RangeN = 700.0f;
    float RangeD = 1050.0f;

    switch (Difficulty) {
    case 0:
        RangeN = 700.0f;
        RangeD = 1050.0f;
        break;

    case 1:
        RangeN = 1000.0f;
        RangeD = 1350.0f;
        break;

    default:
        break;
    }

    f32 visionRange = gSaveContext.save.isNight ? RangeN : RangeD;

    if (player->csAction == PLAYER_CSACTION_26) {
        this->picto.actor.speed = 0.0f;
        this->actionFunc = EnGe2_SetupCharge;
        Animation_Change(&this->skelAnime, &gGerudoPurpleLookingAboutAnim, 1.0f, 0.0f,
            Animation_GetLastFrame(&gGerudoPurpleLookingAboutAnim), ANIMMODE_LOOP, -8.0f);
        this->stateFlags |= GERUDO_PURPLE_STATE_CAPTURING;
    }
    else if (CHECK_WEEKEVENTREG(WEEKEVENTREG_80_08)) {
        this->picto.actor.speed = 0.0f;
        this->actionFunc = EnGe2_TurnToPlayerFast;
        Animation_Change(&this->skelAnime, &gGerudoPurpleLookingAboutAnim, 1.0f, 0.0f,
            Animation_GetLastFrame(&gGerudoPurpleLookingAboutAnim), ANIMMODE_LOOP, -8.0f);
    }
    else if (EnGe2_LookForPlayer(play, &this->picto.actor, &this->picto.actor.focus.pos,
        this->picto.actor.shape.rot.y, 0x1800, visionRange, this->verticalDetectRange)) {
        if ((GERUDO_PURPLE_GET_EXIT(&this->picto.actor) != GERUDO_PURPLE_EXIT_NONE) && !Play_InCsMode(play)) {
            this->picto.actor.speed = 0.0f;
            Player_SetCsActionWithHaltedActors(play, &this->picto.actor, PLAYER_CSACTION_26);
            Lib_PlaySfx(NA_SE_SY_FOUND);
            Message_StartTextbox(play, 0x1194, &this->picto.actor);
            this->actionFunc = EnGe2_SetupCharge;
            Animation_Change(&this->skelAnime, &gGerudoPurpleLookingAboutAnim, 1.0f, 0.0f,
                Animation_GetLastFrame(&gGerudoPurpleLookingAboutAnim), ANIMMODE_LOOP, -8.0f);
        }
    }
    else if (this->collider.base.acFlags & AC_HIT) {
        if ((this->collider.elem.acHitElem != NULL) &&
            (this->collider.elem.acHitElem->atDmgInfo.dmgFlags & DMG_DEKU_NUT)) {
            Actor_SetColorFilter(&this->picto.actor, COLORFILTER_COLORFLAG_BLUE, 120, COLORFILTER_BUFFLAG_OPA, 400);
            this->picto.actor.speed = 0.0f;
            this->actionFunc = EnGe2_Stunned;
            this->stateFlags |= GERUDO_PURPLE_STATE_STUNNED;
        }
        else {
            Animation_Change(&this->skelAnime, &gGerudoPurpleFallingToGroundAnim, 1.0f, 0.0f,
                Animation_GetLastFrame(&gGerudoPurpleFallingToGroundAnim), ANIMMODE_ONCE, -8.0f);
            this->timer = 200;
            this->picto.actor.speed = 0.0f;
            this->actionFunc = EnGe2_KnockedOut;
            Actor_PlaySfx(&this->picto.actor, NA_SE_EN_PIRATE_DEAD);
            this->picto.actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
            this->stateFlags |= GERUDO_PURPLE_STATE_KO;
        }
    }
    else if (this->picto.actor.home.rot.x == 0) {
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
}
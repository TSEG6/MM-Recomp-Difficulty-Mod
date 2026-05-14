#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_look_nuts.h"
#include "overlays/effects/ovl_Effect_Ss_Solder_Srch_Ball/z_eff_ss_solder_srch_ball.h"

void EnLookNuts_RunToPlayer(EnLookNuts* this, PlayState* play);
void EnLookNuts_SetupSendPlayerToSpawn(EnLookNuts* this);

extern s32 D_80A6862C;

typedef enum {
    /* 0x01 */ PALACE_GUARD_PATROLLING,
    /* 0x01 */ PALACE_GUARD_WAITING,
    /* 0x02 */ PALACE_GUARD_RUNNING_TO_PLAYER,
    /* 0x03 */ PALACE_GUARD_CAUGHT_PLAYER
} PalaceGuardState;

void EnLookNuts_DetectedPlayer(EnLookNuts* this, PlayState* play) {
    Animation_Change(&this->skelAnime, &gDekuPalaceGuardWalkAnim, 2.0f, 0.0f,
        Animation_GetLastFrame(&gDekuPalaceGuardWalkAnim), ANIMMODE_LOOP, -10.0f);
    this->state = PALACE_GUARD_RUNNING_TO_PLAYER;
    this->eventTimer = 300;
    Message_StartTextbox(play, 0x833, &this->actor);
    this->actionFunc = EnLookNuts_RunToPlayer;
}

void EnLookNuts_RunToPlayer(EnLookNuts* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 1.0f) || Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_WALK);
    }

    this->actor.speed = 4.0f;
    Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 1, 0xBB8, 0);
    if ((this->actor.xzDistToPlayer < 70.0f) || (this->eventTimer == 0)) {
        this->actor.speed = 0.0f;
        EnLookNuts_SetupSendPlayerToSpawn(this);
    }
}


RECOMP_PATCH void EnLookNuts_Update(Actor* thisx, PlayState* play) {

    s32 pad;
    EnLookNuts* this = (EnLookNuts*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->blinkTimer == 0) {
        this->eyeState++;
        if (this->eyeState >= 3) {
            this->eyeState = 0;
            this->blinkTimer = TRUNCF_BINANG(Rand_ZeroFloat(60.0f)) + 20;
        }
    }
    this->actionFunc(this, play);

    if (this->blinkTimer != 0) {
        this->blinkTimer--;
    }
    if (this->eventTimer != 0) {
        this->eventTimer--;
    }


    Actor_MoveWithGravity(&this->actor);

    if (!D_80A6862C) {
        if ((this->state < 2) && (this->actor.xzDistToPlayer < 850.0f) && (this->actor.playerHeightRel < 80.0f)) {

            if (!this->isPlayerDetected) {
                s16 effectFlags = SOLDERSRCHBALL_INVISIBLE;
                s32 isMasked = (Player_GetMask(play) == PLAYER_MASK_STONE);

                float visionSpeed;
                s32 loopRange;
                s32 frequency;

                if (Difficulty >= 1) {
                    visionSpeed = isMasked ? 10.0f : 35.0f;
                    loopRange = isMasked ? 0 : 2;
                    frequency = isMasked ? 4 : 1;
                }
                else {
                    visionSpeed = isMasked ? 6.0f : 25.0f;
                    loopRange = isMasked ? 0 : 1;
                    frequency = isMasked ? 2 : 1;
                }

                if (play->state.frames % frequency == 0) {
                    for (s32 i = -loopRange; i <= loopRange; i++) {
                        Vec3f effectVelocityOffset = { 0.0f, 0.0f, 0.0f };
                        Vec3f effectPos;
                        Vec3f effectVelocity;
                        Vec3f checkDest;
                        CollisionPoly* outPoly;
                        s32 bgId;
                        s16 spreadAngle = i * 0x0AAA;

                        Math_Vec3f_Copy(&effectPos, &this->actor.world.pos);
                        effectPos.x += Math_SinS(this->actor.world.rot.y + TRUNCF_BINANG(this->headRot.y)) * 10.0f;
                        effectPos.y += 30.0f;
                        effectPos.z += Math_CosS(this->actor.world.rot.y + TRUNCF_BINANG(this->headRot.y)) * 10.0f;

                        Matrix_Push();
                        Matrix_RotateYS(this->actor.shape.rot.y + TRUNCF_BINANG(this->headRot.y) + spreadAngle, MTXMODE_NEW);
                        effectVelocityOffset.z = visionSpeed;
                        Matrix_MultVec3f(&effectVelocityOffset, &effectVelocity);
                        Matrix_Pop();

                        checkDest.x = effectPos.x + effectVelocity.x;
                        checkDest.y = effectPos.y + effectVelocity.y;
                        checkDest.z = effectPos.z + effectVelocity.z;

                        if (BgCheck_EntityLineTest1(&play->colCtx, &effectPos, &checkDest, &checkDest, &outPoly, true, true, true, true, &bgId)) {
                        }
                        else {

                            EffectSsSolderSrchBall_Spawn(play, &effectPos, &effectVelocity, &gZeroVec3f, 50,
                                &this->isPlayerDetected, effectFlags);
                        }
                    }
                }
            }

            if ((this->isPlayerDetected == true) || (this->actor.xzDistToPlayer < 20.0f)) {
                Player* player = GET_PLAYER(play);

                if (!(player->stateFlags3 & PLAYER_STATE3_100) && !Play_InCsMode(play)) {
                    Math_Vec3f_Copy(&this->headRotTarget, &gZeroVec3f);
                    this->state = PALACE_GUARD_RUNNING_TO_PLAYER;
                    Audio_PlaySfx(NA_SE_SY_FOUND);
                    Player_SetCsActionWithHaltedActors(play, &this->actor, PLAYER_CSACTION_26);
                    D_80A6862C = true;
                    this->actor.flags |= (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_UPDATE_CULLING_DISABLED);
                    this->actor.gravity = 0.0f;
                    EnLookNuts_DetectedPlayer(this, play);
                }
                else {
                    this->isPlayerDetected = false;
                }
            }
        }

        Math_ApproachF(&this->headRot.x, this->headRotTarget.x, 1.0f, 3000.0f);
        Math_ApproachF(&this->headRot.y, this->headRotTarget.y, 1.0f, 6000.0f);
        Math_ApproachF(&this->headRot.z, this->headRotTarget.z, 1.0f, 2000.0f);
        this->actor.shape.rot.y = this->actor.world.rot.y;
        Collider_UpdateCylinder(&this->actor, &this->collider);
        CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    }
}
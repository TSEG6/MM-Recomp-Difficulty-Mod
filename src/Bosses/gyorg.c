#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_03.h"
#include "overlays/actors/ovl_Door_Warp1/z_door_warp1.h"
#include "overlays/actors/ovl_En_Water_Effect/z_en_water_effect.h"
#include "overlays/actors/ovl_Item_B_Heart/z_item_b_heart.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/object_water_effect/object_water_effect.h"

#define WORK_TIMER_UNK0_A 0 // used in func_809E34B8
#define WORK_TIMER_UNK1_A 1 // used in func_809E34B8
#define WORK_TIMER_UNK2_A 2 // used in func_809E34B8
#define WORK_TIMER_STUNNED 2
#define PLATFORM_HEIGHT 440.0f
#define WATER_HEIGHT 430.0f
#define GYORG_MAX_HEALTH 10

void func_809E344C(Boss03* this, PlayState* play);
void Boss03_SetupChasePlayer(Boss03* this, PlayState* play);
void Boss03_SetupJumpOverPlatform(Boss03* this, PlayState* play);
void Boss03_JumpOverPlatform(Boss03* this, PlayState* play);
void Boss03_SetupPrepareCharge(Boss03* this, PlayState* play);
void Boss03_IntroCutscene(Boss03* this, PlayState* play);


void Boss03_PlayUnderwaterSfx(Vec3f* projectedPos, u16 sfxId) {
    Audio_PlaySfx_Underwater(projectedPos, sfxId);
}

RECOMP_PATCH void func_809E34B8(Boss03* this, PlayState* play) {
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    f32 bodyYRotTarget;
    s32 pad;
    s16 i;
    Player* player = GET_PLAYER(play);

    int difficulty = (int)recomp_get_config_double("diff_option");
    f32 speedMult = (difficulty >= 1) ? 1.6f : ((difficulty == 0) ? 1.25f : 1.0f);
    f32 turnMult = (difficulty >= 1) ? 2.0f : ((difficulty == 0) ? 1.3f : 1.0f);
    s32 timerReduc = (difficulty >= 1) ? 30 : ((difficulty == 0) ? 15 : 0);

    Boss03_PlayUnderwaterSfx(&this->actor.projectedPos, NA_SE_EN_KONB_WAIT_OLD - SFX_FLAG);

    this->unk_276 = 0x800;
    this->skelAnime.playSpeed = 1.0f;
    this->unk_27C = 1.0f;
    this->unk_278 = 10.0f * speedMult;

    SkelAnime_Update(&this->skelAnime);

    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_RotateYS(this->actor.world.rot.y, MTXMODE_APPLY);

    xDiff = this->unk_268.x - this->actor.world.pos.x;
    yDiff = this->unk_268.y - this->actor.world.pos.y;
    zDiff = this->unk_268.z - this->actor.world.pos.z;

    Math_ApproachS(&this->actor.world.rot.x, Math_Atan2S_XY(sqrtf(SQ(xDiff) + SQ(zDiff)), -yDiff), 0xA, (s16)(this->unk_274 * turnMult));

    bodyYRotTarget =
        Math_SmoothStepToS(&this->actor.world.rot.y, Math_Atan2S_XY(zDiff, xDiff), 0xA, (s16)(this->unk_274 * turnMult), 0) * -0.5f;
    Math_ApproachS(&this->bodyYRot, bodyYRotTarget, 5, 0x100);
    Math_ApproachS(&this->unk_274, this->unk_276, 1, 0x100);
    Math_ApproachF(&this->actor.speed, this->unk_278, 1.0f, this->unk_27C);
    Math_ApproachF(&this->unk_260, sinf(this->skelAnime.curFrame * (M_PIf / 5)) * 10.0f * 0.01f, 0.5f, 1.0f);

    if ((this->workTimer[WORK_TIMER_UNK2_A] == 0) && (this->actor.bgCheckFlags & BGCHECKFLAG_WALL)) {
        Matrix_MultVecZ(-500.0f, &this->unk_268);
        this->unk_268.y = Rand_ZeroFloat(100.0f) + 150.0f;
        this->workTimer[WORK_TIMER_UNK2_A] = 60 - timerReduc;
        this->workTimer[WORK_TIMER_UNK0_A] = (s16)(Rand_ZeroFloat(60.0f) + (60.0f - timerReduc));
        this->unk_274 = 0x100;
    }

    if (this->workTimer[WORK_TIMER_UNK2_A] == 0) {
        if ((sqrtf(SQ(xDiff) + SQ(zDiff)) < 100.0f) || (this->workTimer[WORK_TIMER_UNK0_A] == 0)) {
            for (i = 0; i < 200; i++) {
                this->unk_268.x = Rand_CenteredFloat(2500.0f);
                this->unk_268.y = Rand_ZeroFloat(100.0f) + 150.0f;
                this->unk_268.z = Rand_CenteredFloat(2500.0f);

                xDiff = this->unk_268.x - this->actor.world.pos.x;
                zDiff = this->unk_268.z - this->actor.world.pos.z;

                if (sqrtf(SQ(xDiff) + SQ(zDiff)) > 300.0f) {
                    break;
                }
            }

            this->unk_274 = 0x100;
            this->workTimer[WORK_TIMER_UNK0_A] = (s16)(Rand_ZeroFloat(60.0f) + (60.0f - timerReduc));
        }
    }

    Actor_MoveWithoutGravityReverse(&this->actor);
    Math_ApproachS(&this->actor.shape.rot.x, this->actor.world.rot.x, 2, (s16)(this->unk_274 * 2 * turnMult));
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 2, (s16)(this->unk_274 * 2 * turnMult));

    if (this->workTimer[WORK_TIMER_UNK1_A] == 0) {
        // Player is above water && Player is standing on ground
        if ((this->waterHeight < player->actor.world.pos.y) && (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
            Boss03_SetupPrepareCharge(this, play);
        }
        else if ((player->transformation != PLAYER_FORM_GORON) && (player->transformation != PLAYER_FORM_DEKU)) {
            if (KREG(70) == 0) {
                Boss03_SetupChasePlayer(this, play);
            }
            else if (this->numSpawnedSmallFish <= 0) {
                Boss03_SetupChasePlayer(this, play);
            }
        }
    }
}

RECOMP_PATCH void Boss03_Charge(Boss03* this, PlayState* play) {
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    Player* player = GET_PLAYER(play);
    s16 rotXTarget;
    int Difficulty = (int)recomp_get_config_double("diff_option");
    f32 animSpeed = (Difficulty >= 1) ? 2.5f : 2.0f;
    f32 targetSpeed = (Difficulty >= 1) ? 30.0f : 25.0f;
    s16 rotationStep = (Difficulty >= 1) ? 0x14 : 0xA;
    f32 kbScale = (Difficulty >= 1) ? 12.0f : 7.0f;
    f32 kbSpeed = (Difficulty >= 1) ? 10.0f : 7.0f;

    this->skelAnime.playSpeed = animSpeed;
    SkelAnime_Update(&this->skelAnime);
    Boss03_PlayUnderwaterSfx(&this->actor.projectedPos, NA_SE_EN_KONB_PREATTACK_OLD - SFX_FLAG);

    xDiff = this->unk_268.x - this->actor.world.pos.x;
    yDiff = (this->unk_268.y - this->actor.world.pos.y) - 50.0f;
    zDiff = this->unk_268.z - this->actor.world.pos.z;

    Math_ApproachS(&this->actor.world.rot.y, Math_Atan2S_XY(zDiff, xDiff), rotationStep, 0x1000);

    rotXTarget = Math_Atan2S_XY(sqrtf(SQ(xDiff) + SQ(zDiff)), -yDiff);
    Math_ApproachS(&this->actor.world.rot.x, rotXTarget, rotationStep, 0x1000);

    this->actor.shape.rot = this->actor.world.rot;

    Math_ApproachF(&this->actor.speed, targetSpeed, 1.0f, 3.0f);
    Math_ApproachF(&this->unk_260, sinf(this->skelAnime.curFrame * (M_PIf / 5)) * 10.0f * 0.01f, 0.5f, 1.0f);
    Actor_MoveWithoutGravityReverse(&this->actor);

    if (this->actor.speed >= 20.0f) {
        // Jump over platform
        if (this->unk_242 == 1) {
            if (sqrtf(SQXZ(this->actor.world.pos)) < 700.0f) {
                Boss03_SetupJumpOverPlatform(this, play);
                return;
            }
        }

        // Attack platform
        if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
            Audio_PlaySfx(NA_SE_IT_BIG_BOMB_EXPLOSION);
            Actor_RequestQuakeAndRumble(&this->actor, play, 20, 15);
            Actor_Spawn(&play->actorCtx, play, ACTOR_EN_WATER_EFFECT, 0.0f, this->waterHeight, 0.0f, 0, 0, 0x96,
                ENWATEREFFECT_TYPE_GYORG_SHOCKWAVE);

            // Player is above water && Player is standing on ground
            if ((this->waterHeight < player->actor.world.pos.y) && (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND)) {
                func_800B8D50(play, NULL, kbScale, Math_Atan2S_XY(player->actor.world.pos.z, player->actor.world.pos.x),
                    kbSpeed, 0);
            }

            func_809E344C(this, play);
            this->workTimer[WORK_TIMER_UNK1_A] = 50;
        }
    }
}

RECOMP_HOOK("Boss03_Update") void FishUpdate(Actor* thisx, PlayState* play2) {

    Boss03* this = (Boss03*)thisx;
    this->waterHeight = WATER_HEIGHT;
    PlayState* play = play2;
    Player* player = GET_PLAYER(play);
    static s32 callCounter = 0;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:

        if (this->actor.colChkInfo.damage > 0) {
            int reducedDamage = this->actor.colChkInfo.damage / 2;
            this->actor.colChkInfo.damage = (reducedDamage > 1) ? reducedDamage : 1;
        }

        if (this->workTimer[WORK_TIMER_STUNNED] != 0 && (this->waterHeight < player->actor.world.pos.y)) {

            this->workTimer[WORK_TIMER_STUNNED] = this->workTimer[WORK_TIMER_STUNNED] - 2;
        }

        if (this->workTimer[WORK_TIMER_UNK1_A] != 0 && (this->waterHeight < player->actor.world.pos.y)) {

            this->workTimer[WORK_TIMER_UNK1_A] = this->workTimer[WORK_TIMER_UNK1_A] - 1;
        }
        break;

    case 1:

        if (this->actor.colChkInfo.damage > 0) {
            int reducedDamage = (this->actor.colChkInfo.damage + 2) / 3;
            this->actor.colChkInfo.damage = (reducedDamage > 1) ? reducedDamage : 1;
        }

        if (this->workTimer[WORK_TIMER_STUNNED] != 0 && (this->waterHeight < player->actor.world.pos.y)) {
            this->workTimer[WORK_TIMER_STUNNED] = this->workTimer[WORK_TIMER_STUNNED] - 4;
        }
        else {

            this->workTimer[WORK_TIMER_STUNNED] = this->workTimer[WORK_TIMER_STUNNED] - 1;
        }

        if (this->workTimer[WORK_TIMER_UNK1_A] != 0 && (this->waterHeight < player->actor.world.pos.y)) {
            this->workTimer[WORK_TIMER_UNK1_A] = this->workTimer[WORK_TIMER_UNK1_A] - 3;
        }
        else {

            this->workTimer[WORK_TIMER_UNK1_A] = this->workTimer[WORK_TIMER_UNK1_A] - 1;
        }
        break;

    default:
        break;
    }
    if (this->workTimer[WORK_TIMER_UNK1_A] <= 0) this->workTimer[WORK_TIMER_UNK1_A] = 0;

    if (this->actor.colChkInfo.health > 0) {
        callCounter++;

        if (callCounter >= 600) {
            this->actor.colChkInfo.health += 1;

            if (this->actor.colChkInfo.health > GYORG_MAX_HEALTH) {
                this->actor.colChkInfo.health = GYORG_MAX_HEALTH;
            }

            callCounter = 0;
        }
    }
}

#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_poh.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void func_80B2CAA4(EnPoh* this, PlayState* play);
void func_80B2CB60(EnPoh* this);
void func_80B2CD14(EnPoh* this);
void func_80B2CBBC(EnPoh* this, PlayState* play);
void func_80B2D140(EnPoh* this, PlayState* play);
void func_80B2CD64(EnPoh* this, PlayState* play);
void func_80B2D694(EnPoh* this);
void func_80B2D6EC(EnPoh* this, PlayState* play);
void func_80B2DB44(EnPoh* this, PlayState* play);
void func_80B2D7D4(EnPoh* this, PlayState* play);
void func_80B2D980(EnPoh* this, PlayState* play);

extern void func_80B2C9B8(EnPoh*, PlayState*);
extern void func_80B2D924(EnPoh*);
extern void func_80B2E55C(EnPoh*);
extern void func_80B2D76C(EnPoh*);
extern void func_80B2CA4C(EnPoh*);

RECOMP_PATCH void func_80B2CBBC(EnPoh* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float ATKChance = 0.2f;
    float ATKDist = 200.0f;

    switch (Difficulty) {
    case 0:
        ATKChance = 0.5f;
        ATKDist = 250.0f;
        break;

    case 1:
        ATKChance = 0.9f;
        ATKDist = 300.0f;
        if (this->unk_18E != 0) this->unk_18E--;
        break;

    default:
        break;
    }

    SkelAnime_Update(&this->skelAnime);
    Math_StepToF(&this->actor.speed, 1.0f, 0.2f);
    if (Animation_OnFrame(&this->skelAnime, 0.0f) && (this->unk_18E != 0)) {
        this->unk_18E--;
    }

    if (Actor_WorldDistXZToPoint(&this->actor, &this->actor.home.pos) > 400.0f) {
        this->unk_192 = Actor_WorldYawTowardPoint(&this->actor, &this->actor.home.pos);
    }

    Math_ScaledStepToS(&this->actor.world.rot.y, this->unk_192, 0x71C);
    func_80B2C9B8(this, play);
    if ((this->actor.xzDistToPlayer < ATKDist) && (this->unk_18E < 19)) {
        func_80B2CD14(this);
    }
    else if (this->unk_18E == 0) {
        if (Rand_ZeroOne() < ATKChance) {
            func_80B2D694(this);
        }
        else {
            func_80B2CA4C(this);
        }
    }

    if (this->unk_197 == 255) {
        Actor_PlaySfx_Flagged(&this->actor, NA_SE_EN_PO_FLY - SFX_FLAG);
        if (Rand_ZeroOne() < ATKChance / 3) {
            func_80B2D76C(this);
        }
    }
}

RECOMP_PATCH void func_80B2CF28(EnPoh* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    Player* player = GET_PLAYER(play);

    f32 distToPlayer = Math_Vec3f_DistXYZ(&this->actor.world.pos, &player->actor.world.pos);

    SkelAnime_Update(&this->skelAnime);
    if (Animation_OnFrame(&this->skelAnime, 0.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_PO_KANTERA);

        if (this->unk_18E != 0) {
            if (Difficulty >= 1 && this->unk_18E > 10) {
                this->unk_18E -= 2;
            }
            else {
                this->unk_18E--;
            }
        }
    }

    func_80B2C9B8(this, play);

    if (this->unk_18E >= 10) {
        s16 targetYaw = this->actor.yawTowardsPlayer;
        s16 rotStep = 0xAAA;

        if (Difficulty >= 0) {
            rotStep = 0x1555;
        }

        if (Difficulty >= 1) {
            Vec3f targetPos;
            float lookAhead = 8.0f;

            targetPos.x = player->actor.world.pos.x + (player->actor.velocity.x * lookAhead);
            targetPos.z = player->actor.world.pos.z + (player->actor.velocity.z * lookAhead);
            targetPos.y = player->actor.world.pos.y;

            s16 predictedYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &targetPos);
            targetYaw = (s16)((targetYaw * 0.7f) + (predictedYaw * 0.3f));
        }

        Math_ScaledStepToS(&this->actor.world.rot.y, targetYaw, rotStep);
    }
    else if (this->unk_18E == 9) {
        this->actor.speed = (Difficulty >= 0) ? 7.0f : 5.0f;
        this->skelAnime.playSpeed = 2.0f;
    }
    else if (this->unk_18E == 0) {
        if (distToPlayer > 300.0f) {
            func_80B2CB60(this);
            this->attackCount = 0;
        }
        else {
            func_80B2CB60(this);
            this->attackCount = 0;
        }
    }
}

RECOMP_HOOK("EnPoh_Update") void PoeDefense(Actor* thisx, PlayState* play2) {

    EnPoh* this = (EnPoh*)thisx;

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
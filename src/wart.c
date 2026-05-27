#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_04.h"
#include "z64shrink_window.h"
#include "attributes.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "z_en_tanron2.h"
#include "overlays/actors/ovl_Boss_04/z_boss_04.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "assets/objects/object_boss04/object_boss04.h"

void func_809ED224(Boss04* this);

int Bubbles = 0;

Boss04* D_80BB8450;

RECOMP_HOOK("EnTanron2_Init") void BubbleInit(Actor* thisx, PlayState* play) {

    EnTanron2* this = (EnTanron2*)thisx;

    Bubbles++;
}

RECOMP_HOOK("EnTanron2_Destroy") void BubbleDestroy(Actor* thisx, PlayState* play) {

    EnTanron2* this = (EnTanron2*)thisx;

    Bubbles--;
}


RECOMP_PATCH void func_809ECF58(Boss04* this, PlayState* play) {
    Vec3f sp3C;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float targetChance = (Difficulty == 1) ? 0.8f : 0.4f;

    if ((this->unk_1FE == 14) || ((this->actor.bgCheckFlags & BGCHECKFLAG_WALL) && (this->unk_1F8 == 0))) {
        this->unk_1F8 = 20;

        if ((Rand_ZeroOne() < targetChance) && (this->unk_1FE == 0)) {
            this->actor.world.rot.y = this->actor.yawTowardsPlayer;
            this->unk_2D0 = 10000.0f;
            this->unk_2C8 = 100;
        }
        else {
            this->actor.world.rot.y = BINANG_ROT180(TRUNCF_BINANG(Rand_ZeroFloat(8000.0f)) + this->actor.world.rot.y);
        }

        this->actor.speed = 0.0f;

        if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
            Audio_PlaySfx(NA_SE_IT_BIG_BOMB_EXPLOSION);
            Actor_RequestQuakeAndRumble(&this->actor, play, 15, 10);
            this->unk_6F4 = 15;
            sp3C.x = this->actor.focus.pos.x;
            sp3C.y = this->actor.focus.pos.y;
            sp3C.z = this->actor.focus.pos.z;
            func_800BBFB0(play, &sp3C, 100.0f, 40, 500, 10, 0);
        }
    }

    Math_ApproachS(&this->actor.shape.rot.x, Math_SinS(this->unk_1F4 * 0xFB8) * 5000.0f, 5, 0x800);
    Math_ApproachS(&this->actor.shape.rot.z, Math_SinS(this->unk_1F4 * 0xCD0) * 3000.0f, 5, 0x800);

    if (this->unk_6F4 == 0) {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 5, 0x1000);
        if (this->unk_1FA == 0) {
            Math_ApproachF(&this->actor.speed, 20.0f, 1.0f, 1.0f);
            sp3C.x = this->actor.world.pos.x;
            sp3C.y = this->actor.floorHeight + 2.0f;
            sp3C.z = this->actor.world.pos.z;
            EffectSsGRipple_Spawn(play, &sp3C, 1400, 500, 0);
            Actor_PlaySfx(&this->actor, NA_SE_EN_ME_ATTACK - SFX_FLAG);
        }
    }

    if (KREG(88) != 0) {
        KREG(88) = 0;
        func_809ED224(this);
        this->unk_1FE = 100;
        this->unk_200 = 100;
    }
}

RECOMP_HOOK("Boss04_Update") void WartUpdate(Actor* thisx, PlayState* play2) {

	Boss04* this = (Boss04*)thisx;
    PlayState* play = play2;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;
    int respawnTimer = 60;
    float respawnChance = 0.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.75f;
        respawnChance = 0.25f;
        respawnTimer = 60;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 2.5f;
        respawnChance = 0.75f;
        respawnTimer = 20;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0) {
        this->skelAnime.playSpeed = speedMultiplier;
    }

    if (this->actor.speed <= 6.0f) {
        this->actor.speed *= speedMultiplier;
    }

    if (Bubbles < 82) {

        if ((play->gameplayFrames % respawnTimer == 0) && (Rand_ZeroOne() < respawnChance)) {

            Actor* child = Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_EN_TANRON2,
                this->actor.world.pos.x, this->actor.world.pos.y,
                this->actor.world.pos.z, 0, 0, 0, Bubbles);
            Actor_PlaySfx(&this->actor, NA_SE_EN_IKURA_JUMP1);

            if (child != NULL) {
            }
        }
    }
}
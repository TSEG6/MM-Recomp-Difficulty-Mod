#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_wallmas.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Encount1/z_en_encount1.h"
#include "overlays/actors/ovl_Obj_Ice_Poly/z_obj_ice_poly.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void EnWallmas_SetupDrop(EnWallmas* this, PlayState* play);
void EnWallmas_SetupLand(EnWallmas* this, PlayState* play);
void EnWallmas_SetupJumpToCeiling(EnWallmas* this);
void EnWallmas_SetupReturnToCeiling(EnWallmas* this);
void EnWallmas_SetupTakePlayer(EnWallmas* this, PlayState* play);

// Faster recovery time when attacked
RECOMP_HOOK("EnWallmas_Damage") void WMDamaged(EnWallmas* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 2.0;
        break;

    case 1:
        this->skelAnime.playSpeed = 4.0;
        break;

    default:
        break;
    }
}

// Takes away items when grabbed (on hard it takes magic and most of your health)
RECOMP_HOOK("EnWallmas_TakePlayer") void Taken(EnWallmas* this, PlayState* play) {

    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    #define STOLEN_ITEMS 0x8000

    if (!(this->actor.params & STOLEN_ITEMS)) {
        if (this->timer == 30) {

            switch (Difficulty) {
            case 0:
                gSaveContext.save.saveInfo.playerData.rupees = (gSaveContext.save.saveInfo.playerData.rupees >= 50) ? (gSaveContext.save.saveInfo.playerData.rupees - 50) : 0;
                AMMO(ITEM_BOW) = (AMMO(ITEM_BOW) >= 15) ? (AMMO(ITEM_BOW) - 15) : 0;
                AMMO(ITEM_BOMB) = (AMMO(ITEM_BOMB) >= 10) ? (AMMO(ITEM_BOMB) - 10) : 0;
                AMMO(ITEM_DEKU_NUT) = (AMMO(ITEM_DEKU_NUT) >= 2) ? (AMMO(ITEM_DEKU_NUT) - 2) : 0;
                AMMO(ITEM_DEKU_STICK) = (AMMO(ITEM_DEKU_STICK) >= 2) ? (AMMO(ITEM_DEKU_STICK) - 2) : 0;
                AMMO(ITEM_BOMBCHU) = (AMMO(ITEM_BOMBCHU) >= 10) ? (AMMO(ITEM_BOMBCHU) - 10) : 0;
                AMMO(ITEM_MAGIC_BEANS) = (AMMO(ITEM_MAGIC_BEANS) >= 2) ? (AMMO(ITEM_MAGIC_BEANS) - 2) : 0;
                break;

            case 1:
                if (gSaveContext.save.saveInfo.playerData.health < 22) {
                    gSaveContext.save.saveInfo.playerData.health = 0;
                }
                else {
                    gSaveContext.save.saveInfo.playerData.health = 22;
                }
                gSaveContext.save.saveInfo.playerData.rupees = (gSaveContext.save.saveInfo.playerData.rupees >= 200) ? (gSaveContext.save.saveInfo.playerData.rupees - 200) : 0;
                gSaveContext.save.saveInfo.playerData.magic = gSaveContext.save.saveInfo.playerData.magic / 2;
                AMMO(ITEM_BOW) = (AMMO(ITEM_BOW) >= 20) ? (AMMO(ITEM_BOW) - 20) : 0;
                AMMO(ITEM_BOMB) = (AMMO(ITEM_BOMB) >= 20) ? (AMMO(ITEM_BOMB) - 20) : 0;
                AMMO(ITEM_DEKU_NUT) = (AMMO(ITEM_DEKU_NUT) >= 5) ? (AMMO(ITEM_DEKU_NUT) - 5) : 0;
                AMMO(ITEM_DEKU_STICK) = (AMMO(ITEM_DEKU_STICK) >= 5) ? (AMMO(ITEM_DEKU_STICK) - 5) : 0;
                AMMO(ITEM_BOMBCHU) = (AMMO(ITEM_BOMBCHU) >= 20) ? (AMMO(ITEM_BOMBCHU) - 20) : 0;
                AMMO(ITEM_MAGIC_BEANS) = (AMMO(ITEM_MAGIC_BEANS) >= 5) ? (AMMO(ITEM_MAGIC_BEANS) - 5) : 0;
                break;

            default:
                break;
            }
            this->actor.params |= STOLEN_ITEMS;
        }
    }
}

// Faster cooldown
RECOMP_HOOK("EnWallmas_Cooldown") void SmallerCD(EnWallmas* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 3.0;
        break;

    case 1:
        this->skelAnime.playSpeed = 6.0;
        break;

    default:
        break;
    }

}

// Walking speed increases & faster rise
RECOMP_HOOK("EnWallmas_Walk") void WalkSpeedWM(EnWallmas* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int returnTimer = 10;

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 2.0;
        break;

    case 1:
        this->skelAnime.playSpeed = 4.0;
        if (returnTimer == 0) {
            EnWallmas_SetupJumpToCeiling(this);
            returnTimer = 10;
        }
        else {
            returnTimer--;
        }
        break;

    default:
        break;
    }
}

// Faster drop
RECOMP_HOOK("EnWallmas_Drop") void EnWallmas_DropHook(EnWallmas * this, PlayState * play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    f32 speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.0f;
        break;
    case 1:
        speedMultiplier = 1.5f;
        break;
    default:
        break;
    }

    if (this->actor.velocity.y < 0.0f) {
        this->actor.velocity.y *= speedMultiplier;
    }
}

// Faster waiting to drop & range increases
RECOMP_PATCH void EnWallmas_WaitToDrop(EnWallmas* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f* playerPos = &player->actor.world.pos;

    int oldTimer = this->timer;

    this->actor.world.pos = *playerPos;
    this->actor.floorHeight = player->actor.floorHeight;
    this->actor.floorPoly = player->actor.floorPoly;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float bonusRange = 100.0f;

    switch (Difficulty) {
    case 0:
        if (this->timer != 0) {
            this->timer = this->timer - 2;
        }
        bonusRange = 250.0f;
        break;

    case 1:
        if (this->timer != 0) {
            this->timer = this->timer - 4;
        }
        bonusRange = 500.0f;
        break;

    default:
        break;
    }
    if (this->timer < 0) this->timer = 0;

    if ((player->stateFlags1 & (PLAYER_STATE1_100000 | PLAYER_STATE1_8000000)) ||
        (player->stateFlags2 & PLAYER_STATE2_80) || (player->textboxBtnCooldownTimer > 0) ||
        (player->actor.freezeTimer > 0) || !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        ((WALLMASTER_GET_TYPE(&this->actor) == WALLMASTER_TYPE_PROXIMITY) &&
            (Math_Vec3f_DistXZ(&this->actor.home.pos, playerPos) > (120.f + this->detectionRadius + bonusRange)))) {
        AudioSfx_StopById(NA_SE_EN_FALL_AIM);
        this->timer = 130;
    }

    if (oldTimer > 80 && this->timer <= 80) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_FALL_AIM);
    }

    if (this->timer == 0) {
        EnWallmas_SetupDrop(this, play);
    }
}

// Defense
RECOMP_HOOK("EnWallmas_Update") void WMUpdate(Actor* thisx, PlayState* play) {

    EnWallmas* this = (EnWallmas*)thisx;

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
#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_thiefbird.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void func_80C1193C(EnThiefbird* this, PlayState* play);
int killtimer = 0;

RECOMP_HOOK_RETURN("EnThiefbird_Init") void InitalChanges(Actor* thisx, PlayState* play) {

    EnThiefbird* this = (EnThiefbird*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = baseHealth * 2;
        break;

    case 1: {
        this->actor.colChkInfo.health = baseHealth * 5;
        break;
    }

    default:
        break;
    }
}

RECOMP_HOOK("EnThiefbird_Init") void InitalHome(Actor* thisx, PlayState* play) {

    EnThiefbird* this = (EnThiefbird*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    if (Difficulty == 1) {

        Vec3f checkPos;
        f32 floorHeight;
        s32 bgId;
        s32 i;
        s32 found = false;

        Player* player = GET_PLAYER(play);

        for (i = 0; i < 32; i++) {

            checkPos.x = player->actor.world.pos.x + Rand_CenteredFloat(12000.0f);
            checkPos.z = player->actor.world.pos.z + Rand_CenteredFloat(12000.0f);
            checkPos.y = player->actor.world.pos.y + 4000.0f;

            floorHeight = BgCheck_EntityRaycastFloor5(
                &play->colCtx,
                &this->actor.floorPoly,
                &bgId,
                &this->actor,
                &checkPos
            );

            if (floorHeight > BGCHECK_Y_MIN) {

                f32 airHeight = floorHeight + 300.0f;

                this->actor.floorBgId = bgId;
                this->actor.home.pos.x = checkPos.x;
                this->actor.home.pos.y = airHeight;
                this->actor.home.pos.z = checkPos.z;
                this->actor.world.pos.x = checkPos.x;
                this->actor.world.pos.y = airHeight;
                this->actor.world.pos.z = checkPos.z;
                this->actor.prevPos = this->actor.world.pos;

                found = true;
                break;
            }
        }

        if (!found) {

            this->actor.home.pos = player->actor.world.pos;
            this->actor.home.pos.y += 300.0f;

            this->actor.world.pos = this->actor.home.pos;
            this->actor.prevPos = this->actor.world.pos;
        }
    }

    // just in case the return doesn't work
    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = baseHealth * 2;
        break;

    case 1: {
        this->actor.colChkInfo.health = baseHealth * 5;
        break;
    }

    default:
        break;
    }

}

RECOMP_HOOK("EnThiefbird_Update") void movenearbyplayer(Actor* thisx, PlayState* play) {

    EnThiefbird* this = (EnThiefbird*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty != 0) {

        Vec3f checkPos;
        f32 floorHeight;
        s32 bgId;

        if (Difficulty == 1) {

            if (Rand_ZeroOne() < 0.02f) {

                float offsetX = (Rand_ZeroOne() - 0.5f) * 800.0f;
                float offsetZ = (Rand_ZeroOne() - 0.5f) * 800.0f;

                checkPos.x = this->actor.world.pos.x + offsetX;
                checkPos.z = this->actor.world.pos.z + offsetZ;
                checkPos.y = this->actor.world.pos.y + 300.0f;

                floorHeight = BgCheck_EntityRaycastFloor5(
                    &play->colCtx,
                    &this->actor.floorPoly,
                    &bgId,
                    &this->actor,
                    &checkPos
                );

                if (floorHeight > BGCHECK_Y_MIN) {
                    this->actor.floorBgId = bgId;

                    this->actor.home.pos.x = checkPos.x;
                    this->actor.home.pos.y = floorHeight + 300.0f;
                    this->actor.home.pos.z = checkPos.z;
                }
            }
        }
    }

    if (Difficulty == 1) {

        if (STOLEN_ITEM_1 != STOLEN_ITEM_NONE) {

            Player* player = GET_PLAYER(play);

            float dist = Actor_WorldDistXYZToActor(&this->actor, &player->actor);

            if (killtimer >= 300) {
                Actor_Kill(&this->actor);
                return;

            }
            else {

                killtimer++;

            }

            if (dist > 2500.0f) {
                Actor_Kill(&this->actor);
                return;
            }
            else {

                this->actor.world.pos.y = this->actor.world.pos.y + 6;

            }
        }
    }
}

RECOMP_PATCH void func_80C118E4(EnThiefbird* this) {
    
    int Difficulty = (int)recomp_get_config_double("diff_option");
    
    Animation_MorphToLoop(&this->skelAnime, &gTakkuriAttackAnim, -10.0f);
    this->unk_18E = 300;
    this->actionFunc = func_80C1193C;

    switch (Difficulty) {
    case 0:
        this->actor.speed = 6.5f;
        break;

    case 1: {
        this->actor.speed = 8.0f;
        break;
    }

    default:
        this->actor.speed = 5.0f;
        break;
    }
}
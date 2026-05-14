#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_okuta.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"


void EnOkuta_SpawnSmoke(Vec3f* pos, Vec3f* velocity, s16 scaleStep, PlayState* play) {
    static Color_RGBA8 sSmokePrimColor = { 255, 255, 255, 255 };
    static Color_RGBA8 sSmokeEnvColor = { 150, 150, 150, 255 };

    func_800B0DE0(play, pos, velocity, &gZeroVec3f, &sSmokePrimColor, &sSmokeEnvColor, 400, scaleStep);
}


RECOMP_HOOK_RETURN("EnOkuta_Init") void OctoInit(Actor* thisx, PlayState* play2) {

	PlayState* play = play2;
	EnOkuta* this = (EnOkuta*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;


    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = baseHealth * 3;
        break;

    case 1: {
        this->actor.colChkInfo.health = baseHealth * 6;
        break;
    }
    default:
        break;
    }

}

RECOMP_HOOK("EnOkuta_Update") void OctoUpdate(Actor* thisx, PlayState* play2) {

    PlayState* play = play2;
    EnOkuta* this = (EnOkuta*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        break;

    case 1: {
        DECR(this->timer);
        break;
    }
    default:
        break;
    }
}

RECOMP_PATCH void EnOkuta_SpawnProjectile(EnOkuta* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    Vec3f pos;
    Vec3f velocity;
    Vec3f predictedPos;
    f32 projectileSpeed;
    f32 predictFactor;
    s16 targetYaw;

    int Difficulty = (int)recomp_get_config_double("diff_option");


    if (Difficulty == 1) {
        projectileSpeed = 16.0f;
        predictFactor = 0.90f;
    }
    else {
        projectileSpeed = 10.0f;
        predictFactor = 0.40f;
    }

    f32 dx = player->actor.world.pos.x - this->actor.world.pos.x;
    f32 dz = player->actor.world.pos.z - this->actor.world.pos.z;
    f32 distance = sqrtf(SQ(dx) + SQ(dz));
    f32 travelTime = CLAMP(distance / projectileSpeed, 0.0f, 20.0f);

    predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * travelTime * predictFactor);
    predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * travelTime * predictFactor);
    predictedPos.y = player->actor.world.pos.y;

    targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

    f32 sin = Math_SinS(targetYaw);
    f32 cos = Math_CosS(targetYaw);
    pos.x = this->actor.world.pos.x + 25.0f * sin;
    pos.y = this->actor.world.pos.y - 6.0f;
    pos.z = this->actor.world.pos.z + 25.0f * cos;

    Actor* projectile = Actor_Spawn(&play->actorCtx, play, ACTOR_EN_OKUTA, pos.x, pos.y, pos.z,
        this->actor.shape.rot.x, targetYaw, this->actor.shape.rot.z,
        EN_OKUTA_GET_TYPE(&this->actor) + EN_OKUTA_TYPE_PROJECTILE_BASE);

    if (projectile != NULL) {

        projectile->speed = projectileSpeed;
        projectile->velocity.x = projectileSpeed * sin;
        projectile->velocity.z = projectileSpeed * cos;
        projectile->world.rot.y = targetYaw;

        pos.x = this->actor.world.pos.x + (40.0f * sin);
        pos.z = this->actor.world.pos.z + (40.0f * cos);
        pos.y = this->actor.world.pos.y;

        velocity.x = projectileSpeed * sin;
        velocity.y = 0.0f;
        velocity.z = projectileSpeed * cos;
        EnOkuta_SpawnSmoke(&pos, &velocity, 20, play);
    }

    Actor_PlaySfx(&this->actor, NA_SE_EN_NUTS_THROW);
}
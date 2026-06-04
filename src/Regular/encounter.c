#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_encount1.h"
#include "overlays/actors/ovl_En_Grasshopper/z_en_grasshopper.h"
#include "overlays/actors/ovl_En_Wallmas/z_en_wallmas.h"
#include "overlays/actors/ovl_En_Pr2/z_en_pr2.h"

void EnEncount1_SpawnActor(EnEncount1* this, PlayState* play);

// Spawning adjustments making things slightly faster
RECOMP_PATCH void EnEncount1_Init(Actor* thisx, PlayState* play) {
    EnEncount1* this = (EnEncount1*)thisx;

    if (this->actor.params <= 0) {
        Actor_Kill(&this->actor);
        return;
    }

    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->type = ENENCOUNT1_GET_TYPE(&this->actor);

    s32 baseActiveMax = ENENCOUNT1_GET_SPAWN_ACTIVE_MAX(&this->actor);
    s32 baseTotalMax = ENENCOUNT1_GET_SPAWN_TOTAL_MAX(&this->actor);
    s32 baseTimeMin = ENENCOUNT1_GET_SPAWN_TIME_MIN(&this->actor);
    f32 baseDistanceMax = (ENENCOUNT1_GET_SPAWN_DISTANCE_MAX(&this->actor) * 40.0f) + 120.0f;

    if (Difficulty >= 2) {
        this->spawnActiveMax = baseActiveMax + (1 + (Rand_Next() % (Difficulty)));
    }
    else {
        this->spawnActiveMax = baseActiveMax;
    }

    if (baseTotalMax >= ENENCOUNT1_SPAWNS_TOTAL_MAX_INFINITE) {
        this->spawnTotalMax = -1;
    }
    else if (Difficulty >= 3) {
        this->spawnTotalMax = (Rand_ZeroOne() < 0.3f) ? -1 : (baseTotalMax * 2);
    }
    else if (Difficulty == 2) {
        float scaler = 1.5f + (Rand_ZeroOne() * 0.5f);
        this->spawnTotalMax = (s32)(baseTotalMax * scaler);
    }
    else {
        this->spawnTotalMax = baseTotalMax;
    }

    if (Difficulty >= 2 && baseTimeMin > 1) {
        s32 reductionFactor = (Difficulty >= 3) ? 3 : 2;
        this->spawnTimeMin = baseTimeMin / reductionFactor;
        this->spawnTimeMin += (s32)(Rand_ZeroOne() * 5.0f) - 2;
        if (this->spawnTimeMin < 1) this->spawnTimeMin = 1;
    }
    else {
        this->spawnTimeMin = baseTimeMin;
    }

    if (ENENCOUNT1_GET_SPAWN_DISTANCE_MAX(&this->actor) < 0) {
        this->spawnDistanceMax = -1.0f;
    }
    else if (Difficulty >= 2) {
        float randomVariance = (Rand_ZeroOne() * 200.0f) - 100.0f;
        this->spawnDistanceMax = baseDistanceMax + randomVariance;
        if (this->spawnDistanceMax < 150.0f) this->spawnDistanceMax = 150.0f;
    }
    else {
        this->spawnDistanceMax = baseDistanceMax;
    }

    if (this->type == EN_ENCOUNT1_SKULLFISH_2) {
        this->pathIndex = ENENCOUNT1_GET_PATH_INDEX(&this->actor);
        this->path = SubS_GetPathByIndex(play, this->pathIndex, ENENCOUNT1_PATH_INDEX_NONE);
        this->spawnTotalMax = -1;
        this->spawnDistanceMax = -1.0f;

        if (Difficulty >= 2) {
            this->spawnActiveMax = baseActiveMax + Difficulty;
        }
    }

    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->actionFunc = (EnEncount1ActionFunc)EnEncount1_SpawnActor;
}
#include "modding.h"
#include "global.h"
#include "z64actor.h"
#include "recompconfig.h"
#include "recomputils.h"
#include "globalobjects_api.h"

#define PLAYER_PRINT_INTERVAL 60
#define TIME_VARIANCE 250 

static uintptr_t sPHEnemy;
static uintptr_t sWFEnemy;
static uintptr_t sBOEnemy;

GLOBAL_OBJECTS_CALLBACK_ON_READY void onGlobalObjectsReady() {
    sPHEnemy = (uintptr_t)GlobalObjects_getGlobalObject(OBJECT_PH);
    sWFEnemy = (uintptr_t)GlobalObjects_getGlobalObject(OBJECT_WF);
    sBOEnemy = (uintptr_t)GlobalObjects_getGlobalObject(OBJECT_MKK);
}

RECOMP_HOOK_RETURN("Actor_LoadOverlay") void on_return_Actor_LoadOverlay() {
    ActorProfile* profile = recomphook_get_return_ptr();
    if (profile != NULL && profile->id == ACTOR_EN_PEEHAT) {
        profile->objectId = GAMEPLAY_KEEP;
    }
    if (profile != NULL && profile->id == ACTOR_EN_WF) {
        profile->objectId = GAMEPLAY_KEEP;
    }
    if (profile != NULL && profile->id == ACTOR_EN_MKK) {
        profile->objectId = GAMEPLAY_KEEP;
    }
}

RECOMP_HOOK("EnPeehat_Init")
void PH_Init(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);
}

RECOMP_HOOK("EnWf_Init")
void WF_Init(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sWFEnemy);
}

RECOMP_HOOK("EnMkk_Init")
void BO_Init(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sBOEnemy);
}

RECOMP_HOOK("EnPeehat_Update")
void PH_Update(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);
}

RECOMP_HOOK("EnWf_Update")
void WF_Update(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sWFEnemy);
}

RECOMP_HOOK("EnMkk_Update")
void BO_Update(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sBOEnemy);
}

RECOMP_HOOK("EnPeehat_Draw")
void PH_Draw(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x06, sPHEnemy);
    gSPSegment(POLY_XLU_DISP++, 0x06, sPHEnemy);
    CLOSE_DISPS(play->state.gfxCtx);
}

RECOMP_HOOK("EnWf_Draw")
void WF_Draw(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sWFEnemy);

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x06, sWFEnemy);
    gSPSegment(POLY_XLU_DISP++, 0x06, sWFEnemy);
    CLOSE_DISPS(play->state.gfxCtx);
}

RECOMP_HOOK("EnMkk_Draw")
void BO_Draw(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sBOEnemy);

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x06, sBOEnemy);
    gSPSegment(POLY_XLU_DISP++, 0x06, sBOEnemy);
    CLOSE_DISPS(play->state.gfxCtx);
}

typedef enum {
    NORMAL,
    HARD,
    BOTH,
    RANDOM
} SpawnDifficulty;

typedef struct {
    const char* spawnId; // It's funny, but not really useful for other mods, see spawn system for the real ID's
    s16 sceneId;
    s8 roomNum;
    Vec3f pos;
    s16 rotX;
    s16 rotY;
    s16 actorId;
    s16 params;
    SpawnDifficulty difficulty;
    s32 day;
    u16 time;
} EnemySpawn;


static EnemySpawn sEnemySpawns[] = {

    // Termina Field

        // Peahats

     {"tf_peahat_1", SCENE_00KEIKOKU, 0, {1087.20f, -89.48f, 1734.11f}, 0, 0, ACTOR_EN_PEEHAT, 0x0000, HARD, 0, 0},
     {"tf_peahat_2", SCENE_00KEIKOKU, 0, {725.89f, -222.00f, 3645.10f}, 0, 0, ACTOR_EN_PEEHAT, 0x0000, HARD, 0, 0},
     {"tf_peahat_3", SCENE_00KEIKOKU, 0, {-1977.13f, -222.00f, 4232.04f}, 0, 0, ACTOR_EN_PEEHAT, 0x0000, HARD, 0, 0},
     {"tf_peahat_4", SCENE_00KEIKOKU, 0, {-2590.39f, -222.00f, 2852.47f}, 0, 0, ACTOR_EN_PEEHAT, 0x0000, HARD, 0, 0},
     {"tf_peahat_5", SCENE_00KEIKOKU, 0, {3168.22f, 206.45f, 719.55f}, 0, 0, ACTOR_EN_PEEHAT, 0x0000, HARD, 0, 0},

        // Chuchus

        // Leevers

     {"tf_leever_1", SCENE_00KEIKOKU, 0, {-3423.05f, -296.53f, 1263.18f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},
     {"tf_leever_2", SCENE_00KEIKOKU, 0, {-3998.87f, -303.58f, 1129.72f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},
     {"tf_leever_3", SCENE_00KEIKOKU, 0, {-5164.51f, -281.0f, -420.34f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, HARD, 0, 0},
     {"tf_leever_4", SCENE_00KEIKOKU, 0, {-3367.45f, -281.0f, -1791.78f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},
     {"tf_leever_5", SCENE_00KEIKOKU, 0, {-3919.20f, -281.0f, -1827.81f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},
     {"tf_leever_6", SCENE_00KEIKOKU, 0, {-5519.02f, -281.0f, -626.59f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},
     {"tf_leever_7", SCENE_00KEIKOKU, 0, {-6016.58f, -281.0f, -436.55f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},
     {"tf_leever_8", SCENE_00KEIKOKU, 0, {-6697.46f, -356.30f, -682.33f}, 0, 0, ACTOR_EN_NEO_REEBA , 0x0000, NORMAL, 0, 0},

        // Bombchus

        // Dodongos

        // Wolfos

     {"tf_wolfos_1", SCENE_00KEIKOKU, 0, {-3342.42f, 48.45f, 153.84f}, 0, 0, ACTOR_EN_WF, 0x0000, HARD, 0, 0},
     {"tf_wolfos_2", SCENE_00KEIKOKU, 0, {-3342.42f, 48.45f, -959.24f}, 0, 0, ACTOR_EN_WF, 0x0000, HARD, 0, 0},
     {"tf_wolfos_3", SCENE_00KEIKOKU, 0, {3166.50f, 40.15f, -2453.21f}, 0, 0, ACTOR_EN_WF, 0x0000, HARD, 0, 0},

        // Boes (Currently crashes when killed)

     //{"tf_boe_1", SCENE_00KEIKOKU, 0, {2653.50f, 328.0f, 1753.45f}, 0, 0, ACTOR_EN_MKK, 0x0000, NORMAL, 0, 0},
     //{"tf_boe_2", SCENE_00KEIKOKU, 0, {3280.52f, 328.0f, 1038.16f}, 0, 0, ACTOR_EN_MKK, 0x0000, NORMAL, 0, 0},
};

#define ENEMY_SPAWN_COUNT (sizeof(sEnemySpawns) / sizeof(EnemySpawn))

// Has it spawned already
static bool IsEnemySpawned(PlayState* play, size_t spawnIndex) {
    s16 customId = (s16)(spawnIndex + 10000);

    for (int i = 0; i < ACTORCAT_MAX; i++) {
        Actor* actor = play->actorCtx.actorLists[i].first;

        while (actor != NULL) {
            if (actor->home.rot.z == customId &&
                actor->id == sEnemySpawns[spawnIndex].actorId) {
                return true;
            }

            actor = actor->next;
        }
    }

    return false;
}

// Spawning Enemies
static void SpawnEnemies(PlayState* play) {
    Actor* player = play->actorCtx.actorLists[ACTORCAT_PLAYER].first;

    if (player == NULL) {
        return;
    }

    int Difficulty = (int)recomp_get_config_double("diff_option");

    for (size_t i = 0; i < ENEMY_SPAWN_COUNT; i++) {
        if (sEnemySpawns[i].sceneId != play->sceneId)
            continue;

        if (sEnemySpawns[i].roomNum != -1 &&
            sEnemySpawns[i].roomNum != play->roomCtx.curRoom.num)
            continue;

        bool shouldSpawn = false;

        switch (sEnemySpawns[i].difficulty) {
        case NORMAL:
            shouldSpawn = true;
            break;

        case HARD:
            if (Difficulty == 1) {
                shouldSpawn = true;
            }
            break;

        case BOTH:
            shouldSpawn = true;
            break;

        case RANDOM:
            if (gSaveContext.save.day == sEnemySpawns[i].day) {
                u16 currentTime = gSaveContext.save.time;
                u16 targetTime = sEnemySpawns[i].time;

                u16 timeDiff =
                    (currentTime > targetTime)
                    ? (currentTime - targetTime)
                    : (targetTime - currentTime);

                if (timeDiff <= TIME_VARIANCE) {
                    shouldSpawn = true;
                }
            }
            break;
        }

        if (!shouldSpawn)
            continue;

        if (IsEnemySpawned(play, i))
            continue;

        Actor* spawnedEnemy = Actor_Spawn(
            &play->actorCtx,
            play,
            sEnemySpawns[i].actorId,
            sEnemySpawns[i].pos.x,
            sEnemySpawns[i].pos.y,
            sEnemySpawns[i].pos.z,
            sEnemySpawns[i].rotX,
            sEnemySpawns[i].rotY,
            0,
            sEnemySpawns[i].params
        );

        if (spawnedEnemy != NULL) {
            // ID Given
            spawnedEnemy->home.rot.z = (s16)(i + 10000);
        }
    }
}

// Debug Player Printing
static void PrintPlayerPosition(PlayState* play) {
    Player* player = GET_PLAYER(play);
    if (!player) return;

    if ((play->gameplayFrames % PLAYER_PRINT_INTERVAL) == 0) {
        recomp_printf(
            "[POS] Scene %d | Room %d | X %.2f Y %.2f Z %.2f | RotY %d\n",
            play->sceneId,
            play->roomCtx.curRoom.num,
            player->actor.world.pos.x,
            player->actor.world.pos.y,
            player->actor.world.pos.z,
            player->actor.world.rot.y
        );

        for (int i = 0; i < ACTORCAT_MAX; i++) {
            Actor* actor = play->actorCtx.actorLists[i].first;
            while (actor != NULL) {

                s16 index = actor->home.rot.z - 10000;

                if (index >= 0 && index < (s16)ENEMY_SPAWN_COUNT) {
                    const char* customId = sEnemySpawns[index].spawnId;

                    recomp_printf(
                        "[Weird Deard Gaster Has Located An Imposter] ID: \"%s\" Pos: (%.1f, %.1f, %.1f)\n",
                        customId,
                        actor->world.pos.x,
                        actor->world.pos.y,
                        actor->world.pos.z
                    );
                }
                actor = actor->next;
            }
        }
    }
}

// Hooking into Play_Update
RECOMP_HOOK("Play_Update")
void EnemySpawner_PlayUpdateHook(PlayState* play) {
    int AllowedSpawn = (int)recomp_get_config_double("enemy_spawner");

    if (AllowedSpawn == 0) {
        SpawnEnemies(play);
    }

    PrintPlayerPosition(play);
}
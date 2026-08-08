#include "modding.h"
#include "global.h"
#include "z64actor.h"
#include "recompconfig.h"
#include "recomputils.h"
#include "globalobjects_api.h"

#define PLAYER_PRINT_INTERVAL 60
#define TIME_VARIANCE 500 

static uintptr_t sPHEnemy;

GLOBAL_OBJECTS_CALLBACK_ON_READY void onGlobalObjectsReady() {
    sPHEnemy = (uintptr_t)GlobalObjects_getGlobalObject(OBJECT_PH);
}


RECOMP_HOOK_RETURN("Actor_LoadOverlay") void on_return_Actor_LoadOverlay() {
    ActorProfile* profile = recomphook_get_return_ptr();
    if (profile != NULL && profile->id == ACTOR_EN_PEEHAT) {
        profile->objectId = GAMEPLAY_KEEP;
    }
}


RECOMP_HOOK("EnPeehat_Init")
void PH_Init(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);
}


RECOMP_HOOK("EnPeehat_Update")
void PH_Update(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);
}


RECOMP_HOOK("EnPeehat_Draw")
void PH_Draw(Actor* thisx, PlayState* play) {
    gSegments[0x06] = OS_K0_TO_PHYSICAL(sPHEnemy);

    OPEN_DISPS(play->state.gfxCtx);
    gSPSegment(POLY_OPA_DISP++, 0x06, sPHEnemy);
    gSPSegment(POLY_XLU_DISP++, 0x06, sPHEnemy);
    CLOSE_DISPS(play->state.gfxCtx);
}

typedef enum {
    NORMAL,
    HARD,
    BOTH,
    RANDOM
} SpawnDifficulty;

typedef struct {
    const char* spawnId; // It's funny, but not really useful for other mods, see line 143 for the real ID's
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

        // Bombchus

        // Dodongos

        // Wolfos

        // Boes
};

#define ENEMY_SPAWN_COUNT (sizeof(sEnemySpawns) / sizeof(EnemySpawn))
static bool EnemiesSpawned = false;

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

                u16 timeDiff = (currentTime > targetTime) ? (currentTime - targetTime) : (targetTime - currentTime);

                if (timeDiff <= TIME_VARIANCE) {
                    shouldSpawn = true;
                }
            }
            break;
        }

        if (shouldSpawn) {
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

            // Custom ID for enemy using Z Rotation (sadly it's not text lol) (but the ID's are in the 10000s, so the first peahat is 10000 and the second is 10001)
            // Maybe in the future I'll move it over to use the actual text but for now it's purely for fun (I don't know enough or I forgot about how I could share that info to other mods lol feel free to help)
            if (spawnedEnemy != NULL) {
                spawnedEnemy->world.rot.z = (s16)(i + 10000);
            }
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

                s16 index = actor->world.rot.z - 10000;

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
    static s16 lastScene = -1;
    static s16 lastRoom = -1;

    int AllowedSpawn = (int)recomp_get_config_double("enemy_spawner");

    if (lastScene != play->sceneId || lastRoom != play->roomCtx.curRoom.num) {
        EnemiesSpawned = false;
        lastScene = play->sceneId;
        lastRoom = play->roomCtx.curRoom.num;
    }

    if (!EnemiesSpawned && AllowedSpawn == 0) {
        SpawnEnemies(play);
        EnemiesSpawned = true;
    }

    PrintPlayerPosition(play);
}
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <whb/proc.h>

#include "SDL_helper.h"
#include "common.h"
#include "dirbrowse.h"
#include "fs.h"
#include "input_helper.h"
#include "menu_music.h"
#include "state.h"
#include "status_bar.h"
#include "textures.h"
#include "touch_helper.h"
#include "utils.h"

#define MUSIC_GENRE_COLOUR     SDL_MakeColour(97, 97, 97, 255)
#define MUSIC_STATUS_BG_COLOUR SDL_MakeColour(43, 53, 61, 255)
#define MUSIC_SEPARATOR_COLOUR SDL_MakeColour(34, 41, 48, 255)

typedef enum {
    MUSIC_STATE_NONE,   // 0
    MUSIC_STATE_REPEAT, // 1
    MUSIC_STATE_SHUFFLE // 2
} Music_State;

static char playlist[1024][512], title[128];
static int count = 0, selection = 0, state = 0;
static Mix_Music *audio;

static void Menu_GetMusicList(void) {
    DIR *dir;
    struct dirent *entries;
    dir = opendir(cwd);

    if (dir != NULL) {
        while ((entries = readdir(dir)) != NULL) {
            if ((strncasecmp(FS_GetFileExt(entries->d_name), "mp3", 3) == 0) || (strncasecmp(FS_GetFileExt(entries->d_name), "ogg", 3) == 0) || (strncasecmp(FS_GetFileExt(entries->d_name), "wav", 3) == 0)) {
                strcpy(playlist[count], cwd);
                strcpy(playlist[count] + strlen(playlist[count]), entries->d_name);
                count++;
            }
        }

        closedir(dir);
    }
}

static int Music_GetCurrentIndex(char *path) {
    for (int i = 0; i < count; ++i) {
        if (!strcmp(playlist[i], path))
            return i;
    }
    return 0;
}

static void Music_Play(char *path) {
    audio = Mix_LoadMUS(path);

    if (audio == NULL)
        return;

    switch (Mix_GetMusicType(audio)) {
        case MUS_CMD:
            break;
        case MUS_WAV:
            break;
        case MUS_MID:
            break;
        case MUS_MOD:
        case MUS_OGG:
        case MUS_MP3:
            //MP3_Init(path);
            //break;
        case MUS_NONE:
            break;
        default:
            break;
    }

    int ret = 0;
    if ((ret = Mix_PlayMusic(audio, 1)) != 0)
        return;

    selection = Music_GetCurrentIndex(path);

    strncpy(title, /*strlen(ID3.title) == 0? */ strupr(Utils_Basename(path)), /*: strupr(ID3.title), strlen(ID3.title) == 0?*/ strlen(Utils_Basename(path)) + 1 /*: strlen(ID3.title) + 1*/);
}

static void Music_HandleNext(bool forward, int state) {
    if (state == MUSIC_STATE_NONE) {
        if (forward)
            selection++;
        else
            selection--;
    } else if (state == MUSIC_STATE_SHUFFLE) {
        int old_selection = selection;
        time_t t;
        srand((unsigned) time(&t));
        selection = rand() % (count - 1);

        if (selection == old_selection)
            selection++;
    }

    Utils_SetMax(&selection, 0, (count - 1));
    Utils_SetMin(&selection, (count - 1), 0);

    switch (Mix_GetMusicType(audio)) {
        case MUS_MP3:
            //MP3_Exit();
            break;
        default:
            break;
    }

    Mix_HaltMusic();
    Mix_FreeMusic(audio);

    Music_Play(playlist[selection]);
}

static void Music_HandlePause(bool *status) {
    if (*status) {
        Mix_PauseMusic();
        *status = false;
    } else {
        Mix_ResumeMusic();
        *status = true;
    }
}

void Menu_PlayMusic(char *path) {
    Menu_GetMusicList();
    Music_Play(path);

    int title_height = 0;
    title_height = FC_GetHeight(Roboto_large, title);

    bool isPlaying = true;

    int btn_width = 0, btn_height = 0;
    SDL_QueryTexture(btn_pause, NULL, NULL, &btn_width, &btn_height);

    TouchInfo touchInfo;
    Touch_Init(&touchInfo);

    bool locked = false;

    while (AppRunning()) {
        SDL_ClearScreen(RENDERER, MUSIC_STATUS_BG_COLOUR);
        SDL_RenderClear(RENDERER);

        SDL_DrawImage(RENDERER, default_artwork_blur, 0, 0);
        SDL_DrawRect(RENDERER, 0, 0, 1280, 40, MUSIC_GENRE_COLOUR);      // Status bar
        SDL_DrawRect(RENDERER, 0, 140, 1280, 1, MUSIC_SEPARATOR_COLOUR); // Separator

        if (locked)
            SDL_DrawImage(RENDERER, icon_lock, 5, 3);

        SDL_DrawImage(RENDERER, icon_back, 40, 66);

        SDL_DrawText(RENDERER, Roboto_large, 128, 40 + ((100 - title_height) / 2), WHITE, title); // Audio filename

        SDL_DrawRect(RENDERER, 0, 141, 560, 560, MUSIC_GENRE_COLOUR); // Draw album art background
        SDL_DrawImage(RENDERER, default_artwork, 0, 141);             // Default album art

        SDL_DrawRect(RENDERER, 570, 141, 710, 559, SDL_MakeColour(45, 48, 50, 255)); // Draw info box (outer)
        SDL_DrawRect(RENDERER, 575, 146, 700, 549, SDL_MakeColour(46, 49, 51, 255)); // Draw info box (inner)

        /*if (strlen(ID3.artist) != 0)
			SDL_DrawText(RENDERER, Roboto_large, 590, 161, WHITE, ID3.artist);
		if (strlen(ID3.album) != 0)
			SDL_DrawText(RENDERER, Roboto_large, 590, 201, WHITE, ID3.album);
		if (strlen(ID3.year) != 0)
			SDL_DrawText(RENDERER, Roboto_large, 590, 241, WHITE, ID3.year);
		if (strlen(ID3.genre) != 0)
			SDL_DrawText(RENDERER, Roboto_large, 590, 281, WHITE, ID3.genre);*/

        SDL_DrawCircle(RENDERER, 615 + ((710 - btn_width) / 2), 186 + ((559 - btn_height) / 2), 80, SDL_MakeColour(98, 100, 101, 255));   // Render outer circle
        SDL_DrawCircle(RENDERER, (615 + ((710 - btn_width) / 2)), (186 + ((559 - btn_height) / 2)), 60, SDL_MakeColour(46, 49, 51, 255)); // Render inner circle

        if (isPlaying)
            SDL_DrawImage(RENDERER, btn_pause, 570 + ((710 - btn_width) / 2), 141 + ((559 - btn_height) / 2)); // Playing
        else
            SDL_DrawImage(RENDERER, btn_play, 570 + ((710 - btn_width) / 2), 141 + ((559 - btn_height) / 2)); // Paused

        SDL_DrawImage(RENDERER, btn_rewind, (560 + ((710 - btn_width) / 2)) - (btn_width * 2), 141 + ((559 - btn_height) / 2));                                                                                                                           // Rewind
        SDL_DrawImage(RENDERER, btn_forward, (580 + ((710 - btn_width) / 2)) + (btn_width * 2), 141 + ((559 - btn_height) / 2));                                                                                                                          // Forward
        SDL_DrawImageScale(RENDERER, state == MUSIC_STATE_SHUFFLE ? btn_shuffle_overlay : btn_shuffle, (590 + ((710 - (btn_width - 10)) / 2)) - ((btn_width - 10) * 2), 141 + ((559 - (btn_height - 10)) / 2) + 90, (btn_width - 10), (btn_height - 10)); // Shuffle
        SDL_DrawImageScale(RENDERER, state == MUSIC_STATE_REPEAT ? btn_repeat_overlay : btn_repeat, (550 + ((710 - (btn_width - 10)) / 2)) + ((btn_width - 10) * 2), 141 + ((559 - (btn_height - 10)) / 2) + 90, (btn_width - 10), (btn_height - 10));    // Repeat
        StatusBar_DisplayTime();

        SDL_RenderPresent(RENDERER);

        Input_Update();
        Touch_Process(&touchInfo);
        uint32_t kDown = Input_KeysDown();

        if (!Mix_PlayingMusic()) {
            wait(1);

            if (state == MUSIC_STATE_NONE) {
                if (count != 0)
                    Music_HandleNext(true, MUSIC_STATE_NONE);
                break;
            } else if (state == MUSIC_STATE_REPEAT)
                Music_HandleNext(false, MUSIC_STATE_REPEAT);
            else if (state == MUSIC_STATE_SHUFFLE) {
                if (count != 0)
                    Music_HandleNext(false, MUSIC_STATE_SHUFFLE);
            }
        }

        if (kDown & KEY_B) {
            wait(1);
            Mix_HaltMusic();
            break;
        }

        if (kDown & KEY_A)
            Music_HandlePause(&isPlaying);

        if (kDown & KEY_Y) {
            if (state == MUSIC_STATE_REPEAT)
                state = MUSIC_STATE_NONE;
            else
                state = MUSIC_STATE_REPEAT;
        } else if (kDown & KEY_X) {
            if (state == MUSIC_STATE_SHUFFLE)
                state = MUSIC_STATE_NONE;
            else
                state = MUSIC_STATE_SHUFFLE;
        }

        if (!locked) {
            if ((kDown & KEY_LEFT) || (kDown & KEY_L)) {
                wait(1);

                if (count != 0)
                    Music_HandleNext(false, MUSIC_STATE_NONE);
            } else if ((kDown & KEY_RIGHT) || (kDown & KEY_R)) {
                wait(1);

                if (count != 0)
                    Music_HandleNext(true, MUSIC_STATE_NONE);
            }
        }

        if (touchInfo.state == TouchEnded && touchInfo.tapType != TapNone) {
            if (tapped_inside(touchInfo, 40, 66, 108, 114)) {
                wait(1);
                Mix_HaltMusic();
                break;
            }

            if (tapped_inside(touchInfo, 570 + ((710 - btn_width) / 2), 141 + ((559 - btn_height) / 2), (570 + ((710 - btn_width) / 2)) + btn_width, (141 + ((559 - btn_height) / 2) + btn_height)))
                Music_HandlePause(&isPlaying);
            else if (tapped_inside(touchInfo, (590 + ((710 - (btn_width - 10)) / 2)) - ((btn_width - 10) * 2), 141 + ((559 - (btn_height - 10)) / 2) + 90, ((590 + ((710 - (btn_width - 10)) / 2)) - ((btn_width - 10) * 2)) + (btn_width - 10), (141 + ((559 - (btn_height - 10)) / 2) + 90) + (btn_height - 10))) {
                if (state == MUSIC_STATE_SHUFFLE)
                    state = MUSIC_STATE_NONE;
                else
                    state = MUSIC_STATE_SHUFFLE;
            } else if (tapped_inside(touchInfo, (550 + ((710 - (btn_width - 10)) / 2)) + ((btn_width - 10) * 2), 141 + ((559 - (btn_height - 10)) / 2) + 90, ((550 + ((710 - (btn_width - 10)) / 2)) + ((btn_width - 10) * 2)) + (btn_width - 10), (141 + ((559 - (btn_height - 10)) / 2) + 90) + (btn_height - 10))) {
                if (state == MUSIC_STATE_REPEAT)
                    state = MUSIC_STATE_NONE;
                else
                    state = MUSIC_STATE_REPEAT;
            }

            if (!locked) {
                if (tapped_inside(touchInfo, (570 + ((710 - btn_width) / 2)) - (btn_width * 2), 141 + ((559 - btn_height) / 2), (570 + ((710 - btn_width) / 2)) - (btn_width * 2) + btn_width, (141 + ((559 - btn_height) / 2) + btn_height))) {
                    wait(1);

                    if (count != 0)
                        Music_HandleNext(false, MUSIC_STATE_NONE);
                } else if (tapped_inside(touchInfo, (570 + ((710 - btn_width) / 2)) + (btn_width * 2), 141 + ((559 - btn_height) / 2), (570 + ((710 - btn_width) / 2)) + (btn_width * 2) + btn_width, (141 + ((559 - btn_height) / 2) + btn_height))) {
                    wait(1);

                    if (count != 0)
                        Music_HandleNext(true, MUSIC_STATE_NONE);
                }
            }
        }
    }

    switch (Mix_GetMusicType(audio)) {
        case MUS_MP3:
            //MP3_Exit();
            break;
        default:
            break;
    }

    Mix_FreeMusic(audio);
    memset(playlist, 0, sizeof(playlist[0][0]) * 512 * 512);
    count = 0;
    MENU_DEFAULT_STATE = MENU_STATE_HOME;
}
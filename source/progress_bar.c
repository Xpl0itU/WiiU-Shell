#include "progress_bar.h"
#include "SDL_helper.h"
#include "common.h"
#include "config.h"
#include "textures.h"

void ProgressBar_DisplayProgress(char *msg, char *src, uint32_t offset, uint32_t size) {
    int text_width = 0;
    text_width = FC_GetWidth(Roboto, src);

    int width = 0, height = 0;
    SDL_QueryTexture(config_dark_theme ? dialog_dark : dialog, NULL, NULL, &width, &height);

    SDL_DrawImage(RENDERER, config_dark_theme ? dialog_dark : dialog, ((1280 - (width)) / 2), ((720 - (height)) / 2));

    SDL_DrawText(RENDERER, Roboto, ((1280 - (width)) / 2) + 80, ((720 - (height)) / 2) + 45, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, msg);

    SDL_DrawText(RENDERER, Roboto, ((1280 - (text_width)) / 2), ((720 - (height)) / 2) + 111, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, src);

    SDL_DrawRect(RENDERER, ((1280 - (width)) / 2) + 80, ((720 - (height)) / 2) + 178, 720, 12, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    SDL_DrawRect(RENDERER, ((1280 - (width)) / 2) + 80, ((720 - (height)) / 2) + 178, (double) offset / (double) size * 720.0, 12, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR);

    SDL_RenderPresent(RENDERER);
}
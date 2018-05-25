#include <switch.h>

#include "common.h"
#include "progress_bar.h"
#include "SDL_helper.h"
#include "textures.h"

void ProgressBar_DisplayProgress(char *msg, char *src, u32 offset, u32 size)
{
	int text_width = 0;
	TTF_SizeText(Roboto, src, &text_width, NULL);

	int width = 0, height = 0;
	SDL_QueryTexture(dialog, NULL, NULL, &width, &height);

	SDL_DrawImage(RENDERER, dialog, ((1280 - (width)) / 2), ((720 - (height)) / 2), 880, 320);

	SDL_DrawText(Roboto, ((1280 - (width)) / 2) + 80, ((720 - (height)) / 2) + 45, TITLE_COLOUR, msg);

	SDL_DrawText(Roboto, ((1280 - (text_width)) / 2), ((720 - (height)) / 2) + 111, TEXT_MIN_COLOUR, src);

	SDL_DrawRect(RENDERER, ((1280 - (width)) / 2) + 80, ((720 - (height)) / 2) + 178, 720, 12, BAR_COLOUR);
	SDL_DrawRect(RENDERER, ((1280 - (width)) / 2) + 80, ((720 - (height)) / 2) + 178, (double)offset / (double)size * 720.0, 12, PROGRESS_COLOUR);
}
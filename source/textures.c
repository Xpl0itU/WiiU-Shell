#include "textures.h"
#include "SDL_helper.h"
#include "common.h"

SDL_Texture *icon_app, *icon_archive, *icon_audio, *icon_dir, *icon_file, *icon_image, *icon_text, *icon_dir_dark,
        *icon_doc, *icon_check, *icon_uncheck, *icon_check_dark, *icon_uncheck_dark, *icon_radio_off, *icon_radio_on,
        *icon_radio_dark_off, *icon_radio_dark_on, *icon_toggle_on, *icon_toggle_dark_on, *icon_toggle_off,
        *dialog, *options_dialog, *properties_dialog, *dialog_dark, *options_dialog_dark, *properties_dialog_dark,
        *bg_header, *icon_settings, *icon_sd, *icon_secure, *icon_settings_dark, *icon_sd_dark, *icon_secure_dark,
        *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward,
        *btn_repeat, *btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, *icon_lock,
        *icon_nav_drawer, *icon_actions, *icon_back,
        *icon_accept, *icon_accept_dark, *icon_remove, *icon_remove_dark,
        *battery_20, *battery_20_charging, *battery_30, *battery_30_charging, *battery_50, *battery_50_charging,
        *battery_60, *battery_60_charging, *battery_80, *battery_80_charging, *battery_90, *battery_90_charging,
        *battery_full, *battery_full_charging, *battery_low, *battery_unknown;

void Textures_Load(void) {
    SDL_LoadImage(RENDERER, &icon_app, "romfs:/res/drawable/ic_fso_type_executable.png");
    SDL_LoadImage(RENDERER, &icon_archive, "romfs:/res/drawable/ic_fso_type_compress.png");
    SDL_LoadImage(RENDERER, &icon_audio, "romfs:/res/drawable/ic_fso_type_audio.png");
    SDL_LoadImage(RENDERER, &icon_dir, "romfs:/res/drawable/ic_fso_folder.png");
    SDL_LoadImage(RENDERER, &icon_dir_dark, "romfs:/res/drawable/ic_fso_folder_dark.png");
    SDL_LoadImage(RENDERER, &icon_doc, "romfs:/res/drawable/ic_fso_type_document.png");
    SDL_LoadImage(RENDERER, &icon_file, "romfs:/res/drawable/ic_fso_default.png");
    SDL_LoadImage(RENDERER, &icon_image, "romfs:/res/drawable/ic_fso_type_image.png");
    SDL_LoadImage(RENDERER, &icon_text, "romfs:/res/drawable/ic_fso_type_text.png");
    SDL_LoadImage(RENDERER, &icon_check, "romfs:/res/drawable/btn_material_light_check_on_normal.png");
    SDL_LoadImage(RENDERER, &icon_check_dark, "romfs:/res/drawable/btn_material_light_check_on_normal_dark.png");
    SDL_LoadImage(RENDERER, &icon_uncheck, "romfs:/res/drawable/btn_material_light_check_off_normal.png");
    SDL_LoadImage(RENDERER, &icon_uncheck_dark, "romfs:/res/drawable/btn_material_light_check_off_normal_dark.png");
    SDL_LoadImage(RENDERER, &dialog, "romfs:/res/drawable/ic_material_dialog.png");
    SDL_LoadImage(RENDERER, &options_dialog, "romfs:/res/drawable/ic_material_options_dialog.png");
    SDL_LoadImage(RENDERER, &properties_dialog, "romfs:/res/drawable/ic_material_properties_dialog.png");
    SDL_LoadImage(RENDERER, &dialog_dark, "romfs:/res/drawable/ic_material_dialog_dark.png");
    SDL_LoadImage(RENDERER, &options_dialog_dark, "romfs:/res/drawable/ic_material_options_dialog_dark.png");
    SDL_LoadImage(RENDERER, &properties_dialog_dark, "romfs:/res/drawable/ic_material_properties_dialog_dark.png");
    SDL_LoadImage(RENDERER, &bg_header, "romfs:/res/drawable/bg_header.png");
    SDL_LoadImage(RENDERER, &icon_settings, "romfs:/res/drawable/ic_material_light_settings.png");
    SDL_LoadImage(RENDERER, &icon_sd, "romfs:/res/drawable/ic_material_light_sdcard.png");
    SDL_LoadImage(RENDERER, &icon_secure, "romfs:/res/drawable/ic_material_light_secure.png");
    SDL_LoadImage(RENDERER, &icon_settings_dark, "romfs:/res/drawable/ic_material_light_settings_dark.png");
    SDL_LoadImage(RENDERER, &icon_sd_dark, "romfs:/res/drawable/ic_material_light_sdcard_dark.png");
    SDL_LoadImage(RENDERER, &icon_secure_dark, "romfs:/res/drawable/ic_material_light_secure_dark.png");
    SDL_LoadImage(RENDERER, &icon_radio_off, "romfs:/res/drawable/btn_material_light_radio_off_normal.png");
    SDL_LoadImage(RENDERER, &icon_radio_on, "romfs:/res/drawable/btn_material_light_radio_on_normal.png");
    SDL_LoadImage(RENDERER, &icon_radio_dark_off, "romfs:/res/drawable/btn_material_light_radio_off_normal_dark.png");
    SDL_LoadImage(RENDERER, &icon_radio_dark_on, "romfs:/res/drawable/btn_material_light_radio_on_normal_dark.png");
    SDL_LoadImage(RENDERER, &icon_toggle_on, "romfs:/res/drawable/btn_material_light_toggle_on_normal.png");
    SDL_LoadImage(RENDERER, &icon_toggle_dark_on, "romfs:/res/drawable/btn_material_light_toggle_on_normal_dark.png");
    SDL_LoadImage(RENDERER, &icon_toggle_off, "romfs:/res/drawable/btn_material_light_toggle_off_normal.png");
    SDL_LoadImage(RENDERER, &default_artwork, "romfs:/res/drawable/default_artwork.png");
    SDL_LoadImage(RENDERER, &default_artwork_blur, "romfs:/res/drawable/default_artwork_blur.png");
    SDL_LoadImage(RENDERER, &btn_play, "romfs:/res/drawable/btn_playback_play.png");
    SDL_LoadImage(RENDERER, &btn_pause, "romfs:/res/drawable/btn_playback_pause.png");
    SDL_LoadImage(RENDERER, &btn_rewind, "romfs:/res/drawable/btn_playback_rewind.png");
    SDL_LoadImage(RENDERER, &btn_forward, "romfs:/res/drawable/btn_playback_forward.png");
    SDL_LoadImage(RENDERER, &btn_repeat, "romfs:/res/drawable/btn_playback_repeat.png");
    SDL_LoadImage(RENDERER, &btn_shuffle, "romfs:/res/drawable/btn_playback_shuffle.png");
    SDL_LoadImage(RENDERER, &btn_repeat_overlay, "romfs:/res/drawable/btn_playback_repeat_overlay.png");
    SDL_LoadImage(RENDERER, &btn_shuffle_overlay, "romfs:/res/drawable/btn_playback_shuffle_overlay.png");
    SDL_LoadImage(RENDERER, &icon_nav_drawer, "romfs:/res/drawable/ic_material_light_navigation_drawer.png");
    SDL_LoadImage(RENDERER, &icon_actions, "romfs:/res/drawable/ic_material_light_contextual_action.png");
    SDL_LoadImage(RENDERER, &icon_back, "romfs:/res/drawable/ic_arrow_back_normal.png");
    SDL_LoadImage(RENDERER, &icon_accept, "romfs:/res/drawable/ic_material_light_accept.png");
    SDL_LoadImage(RENDERER, &icon_accept_dark, "romfs:/res/drawable/ic_material_light_accept_dark.png");
    SDL_LoadImage(RENDERER, &icon_remove, "romfs:/res/drawable/ic_material_light_remove.png");
    SDL_LoadImage(RENDERER, &icon_remove_dark, "romfs:/res/drawable/ic_material_light_remove_dark.png");
    SDL_LoadImage(RENDERER, &battery_20, "romfs:/res/drawable/battery_20.png");
    SDL_LoadImage(RENDERER, &battery_20_charging, "romfs:/res/drawable/battery_20_charging.png");
    SDL_LoadImage(RENDERER, &battery_30, "romfs:/res/drawable/battery_30.png");
    SDL_LoadImage(RENDERER, &battery_30_charging, "romfs:/res/drawable/battery_30_charging.png");
    SDL_LoadImage(RENDERER, &battery_50, "romfs:/res/drawable/battery_50.png");
    SDL_LoadImage(RENDERER, &battery_50_charging, "romfs:/res/drawable/battery_50_charging.png");
    SDL_LoadImage(RENDERER, &battery_60, "romfs:/res/drawable/battery_60.png");
    SDL_LoadImage(RENDERER, &battery_60_charging, "romfs:/res/drawable/battery_60_charging.png");
    SDL_LoadImage(RENDERER, &battery_80, "romfs:/res/drawable/battery_80.png");
    SDL_LoadImage(RENDERER, &battery_80_charging, "romfs:/res/drawable/battery_80_charging.png");
    SDL_LoadImage(RENDERER, &battery_90, "romfs:/res/drawable/battery_90.png");
    SDL_LoadImage(RENDERER, &battery_90_charging, "romfs:/res/drawable/battery_90_charging.png");
    SDL_LoadImage(RENDERER, &battery_full, "romfs:/res/drawable/battery_full.png");
    SDL_LoadImage(RENDERER, &battery_full_charging, "romfs:/res/drawable/battery_full_charging.png");
    SDL_LoadImage(RENDERER, &battery_low, "romfs:/res/drawable/battery_low.png");
    SDL_LoadImage(RENDERER, &battery_unknown, "romfs:/res/drawable/battery_unknown.png");
    SDL_LoadImage(RENDERER, &icon_lock, "romfs:/res/drawable/ic_material_dialog_fs_locked.png");
}

void Textures_Free(void) {
    SDL_DestroyTexture(icon_lock);
    SDL_DestroyTexture(battery_unknown);
    SDL_DestroyTexture(battery_low);
    SDL_DestroyTexture(battery_full_charging);
    SDL_DestroyTexture(battery_full);
    SDL_DestroyTexture(battery_90_charging);
    SDL_DestroyTexture(battery_80_charging);
    SDL_DestroyTexture(battery_80);
    SDL_DestroyTexture(battery_60_charging);
    SDL_DestroyTexture(battery_60);
    SDL_DestroyTexture(battery_50_charging);
    SDL_DestroyTexture(battery_50);
    SDL_DestroyTexture(battery_30_charging);
    SDL_DestroyTexture(battery_30);
    SDL_DestroyTexture(battery_20_charging);
    SDL_DestroyTexture(battery_20);
    SDL_DestroyTexture(icon_remove_dark);
    SDL_DestroyTexture(icon_remove);
    SDL_DestroyTexture(icon_accept_dark);
    SDL_DestroyTexture(icon_accept);
    SDL_DestroyTexture(icon_back);
    SDL_DestroyTexture(icon_actions);
    SDL_DestroyTexture(icon_nav_drawer);
    SDL_DestroyTexture(btn_shuffle_overlay);
    SDL_DestroyTexture(btn_repeat_overlay);
    SDL_DestroyTexture(btn_shuffle);
    SDL_DestroyTexture(btn_repeat);
    SDL_DestroyTexture(btn_forward);
    SDL_DestroyTexture(btn_rewind);
    SDL_DestroyTexture(btn_pause);
    SDL_DestroyTexture(btn_play);
    SDL_DestroyTexture(default_artwork_blur);
    SDL_DestroyTexture(default_artwork);
    SDL_DestroyTexture(icon_toggle_off);
    SDL_DestroyTexture(icon_toggle_dark_on);
    SDL_DestroyTexture(icon_toggle_on);
    SDL_DestroyTexture(icon_radio_dark_on);
    SDL_DestroyTexture(icon_radio_dark_off);
    SDL_DestroyTexture(icon_radio_on);
    SDL_DestroyTexture(icon_radio_off);
    SDL_DestroyTexture(icon_secure_dark);
    SDL_DestroyTexture(icon_sd_dark);
    SDL_DestroyTexture(icon_settings_dark);
    SDL_DestroyTexture(icon_secure);
    SDL_DestroyTexture(icon_sd);
    SDL_DestroyTexture(icon_settings);
    SDL_DestroyTexture(bg_header);
    SDL_DestroyTexture(properties_dialog_dark);
    SDL_DestroyTexture(options_dialog_dark);
    SDL_DestroyTexture(dialog_dark);
    SDL_DestroyTexture(properties_dialog);
    SDL_DestroyTexture(options_dialog);
    SDL_DestroyTexture(dialog);
    SDL_DestroyTexture(icon_uncheck_dark);
    SDL_DestroyTexture(icon_uncheck);
    SDL_DestroyTexture(icon_check_dark);
    SDL_DestroyTexture(icon_check);
    SDL_DestroyTexture(icon_text);
    SDL_DestroyTexture(icon_image);
    SDL_DestroyTexture(icon_file);
    SDL_DestroyTexture(icon_doc);
    SDL_DestroyTexture(icon_dir_dark);
    SDL_DestroyTexture(icon_dir);
    SDL_DestroyTexture(icon_audio);
    SDL_DestroyTexture(icon_archive);
    SDL_DestroyTexture(icon_app);
}
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "SDL_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_options.h"
#include "osk.h"
#include "progress_bar.h"
#include "textures.h"
#include "utils.h"

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
static int copymode = NOTHING_TO_COPY;
/*
*	Copy Move Origin
*/
static char copysource[512];

static int delete_dialog_selection = 0, row = 0, column = 0;
static bool copy_status = false, cut_status = false;

static int delete_width = 0, delete_height = 0;
static int delete_confirm_width = 0, delete_confirm_height = 0;
static int delete_cancel_width = 0, delete_cancel_height = 0;

static int properties_ok_width = 0, properties_ok_height = 0;

static int options_cancel_width = 0, options_cancel_height = 0;

void FileOptions_ResetClipboard(void) {
    multi_select_index = 0;
    memset(multi_select, 0, sizeof(multi_select));
    memset(multi_select_indices, 0, sizeof(multi_select_indices));
    memset(multi_select_dir, 0, sizeof(multi_select_dir));
    memset(multi_select_paths, 0, sizeof(multi_select_paths));
}

static int FileOptions_CreateFolder(void) {
    OSK_Display("Create Folder", "");

    if (strncmp(osk_buffer, "", 1) == 0)
        return -1;

    char path[500];
    strcpy(path, cwd);
    strcat(path, osk_buffer);
    osk_buffer[0] = '\0';

    int ret = 0;
    if ((ret = FS_RecursiveMakeDir(path)) != 0)
        return ret;

    Dirbrowse_PopulateFiles(true);
    MENU_DEFAULT_STATE = MENU_STATE_HOME;
    return 0;
}

static int FileOptions_Rename(void) {
    int ret = 0;
    File *file = Dirbrowse_GetFileIndex(position);

    if (file == NULL)
        return -1;

    if (strncmp(file->name, "..", 2) == 0)
        return -2;

    char oldPath[500], newPath[500];

    strcpy(oldPath, cwd);
    strcpy(newPath, cwd);
    strcat(oldPath, file->name);

    OSK_Display("Rename", file->name);

    if (strncmp(osk_buffer, "", 1) == 0)
        return -1;

    strcat(newPath, osk_buffer);
    osk_buffer[0] = '\0';

    if ((ret = rename(oldPath, newPath)) != 0)
        return ret;

    Dirbrowse_PopulateFiles(true);
    MENU_DEFAULT_STATE = MENU_STATE_HOME;
    return 0;
}

static int FileOptions_RmdirRecursive(char *path) {
    File *filelist = NULL;
    DIR *directory = opendir(path);

    if (directory) {
        struct dirent *entries;

        while ((entries = readdir(directory)) != NULL) {
            if (strlen(entries->d_name) > 0) {
                if (strcmp(entries->d_name, ".") == 0 || strcmp(entries->d_name, "..") == 0)
                    continue;

                // Allocate Memory
                File *item = (File *) malloc(sizeof(File));
                memset(item, 0, sizeof(File));

                // Copy File Name
                strcpy(item->name, entries->d_name);

                // Set Folder Flag
                item->isDir = entries->d_type == DT_DIR;

                // New List
                if (filelist == NULL)
                    filelist = item;

                // Existing List
                else {
                    File *list = filelist;

                    while (list->next != NULL)
                        list = list->next;

                    list->next = item;
                }
            }
        }
    }

    closedir(directory);

    File *node = filelist;

    // Iterate Files
    for (; node != NULL; node = node->next) {
        // Directory
        if (node->isDir) {
            // Required Buffer Size
            int size = strlen(path) + strlen(node->name) + 2;

            // Allocate Buffer
            char *buffer = (char *) malloc(size);

            // Combine Path
            strcpy(buffer, path);
            strcpy(buffer + strlen(buffer), node->name);
            buffer[strlen(buffer) + 1] = 0;
            buffer[strlen(buffer)] = '/';

            // Recursion Delete
            FileOptions_RmdirRecursive(buffer);

            free(buffer);
        }

        // File
        else {
            // Required Buffer Size
            int size = strlen(path) + strlen(node->name) + 1;

            // Allocate Buffer
            char *buffer = (char *) malloc(size);

            // Combine Path
            strcpy(buffer, path);
            strcpy(buffer + strlen(buffer), node->name);

            // Delete File
            remove(buffer);

            free(buffer);
        }
    }

    Dirbrowse_RecursiveFree(filelist);
    return rmdir(path);
}

static int FileOptions_DeleteFile(void) {
    // Find File
    File *file = Dirbrowse_GetFileIndex(position);

    // Not found
    if (file == NULL)
        return -1;

    if (strcmp(file->name, "..") == 0)
        return -2;

    char path[512];

    // Puzzle Path
    strcpy(path, cwd);
    strcpy(path + strlen(path), file->name);

    // Delete Folder
    if (file->isDir) {
        // Add Trailing Slash
        path[strlen(path) + 1] = 0;
        path[strlen(path)] = '/';

        // Delete Folder
        return FileOptions_RmdirRecursive(path);
    }

    // Delete File
    else
        return remove(path);
}

// Copy file from src to dst
static int FileOptions_CopyFile(char *src, char *dst, bool displayAnim) {
    int chunksize = (512 * 1024);              // Chunk size
    char *buffer = (char *) malloc(chunksize); // Reading buffer

    uint32_t totalwrite = 0; // Accumulated writing
    uint32_t totalread = 0;  // Accumulated reading

    int result = 0; // int

    int in = open(src, O_RDONLY, 0777); // Open file for reading
    uint32_t size = FS_GetFileSize(src);

    // Opened file for reading
    if (in >= 0) {
        if (FS_FileExists(dst))
            remove(dst); // Delete output file (if existing)

        int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0777); // Open output file for writing

        if (out >= 0) // Opened file for writing
        {
            uint32_t b_read = 0; // Read byte count

            // Copy loop (512KB at a time)
            while ((b_read = read(in, buffer, chunksize)) > 0) {
                totalread += b_read;                      // Accumulate read data
                totalwrite += write(out, buffer, b_read); // Write data

                if (displayAnim)
                    ProgressBar_DisplayProgress(copymode == 1 ? "Moving" : "Copying", Utils_Basename(src), totalread, size);
            }

            close(out); // Close output file

            if (totalread != totalwrite) // Insufficient copy
                result = -3;
        }

        else // Output open error
            result = -2;

        close(in); // Close input file
    }

    // Input open error
    else
        result = -1;

    free(buffer);  // Free memory
    return result; // Return result
}

// Recursively copy file from src to dst
static int FileOptions_CopyDir(char *src, char *dst) {
    DIR *directory = opendir(src);

    if (directory) {
        // Create Output Directory (is allowed to fail, we can merge folders after all)
        FS_MakeDir(dst);

        struct dirent *entries;

        // Iterate Files
        while ((entries = readdir(directory)) != NULL) {
            if (strlen(entries->d_name) > 0) {
                // Calculate Buffer Size
                int insize = strlen(src) + strlen(entries->d_name) + 2;
                int outsize = strlen(dst) + strlen(entries->d_name) + 2;

                // Allocate Buffer
                char *inbuffer = (char *) malloc(insize);
                char *outbuffer = (char *) malloc(outsize);

                // Puzzle Input Path
                strcpy(inbuffer, src);
                inbuffer[strlen(inbuffer) + 1] = 0;
                inbuffer[strlen(inbuffer)] = '/';
                strcpy(inbuffer + strlen(inbuffer), entries->d_name);

                // Puzzle Output Path
                strcpy(outbuffer, dst);
                outbuffer[strlen(outbuffer) + 1] = 0;
                outbuffer[strlen(outbuffer)] = '/';
                strcpy(outbuffer + strlen(outbuffer), entries->d_name);

                // Another Folder
                if (entries->d_type == DT_DIR)
                    FileOptions_CopyDir(inbuffer, outbuffer); // Copy Folder (via recursion)

                // Simple File
                else
                    FileOptions_CopyFile(inbuffer, outbuffer, true); // Copy File

                // Free Buffer
                free(inbuffer);
                free(outbuffer);
            }
        }

        closedir(directory);
        return 0;
    }

    return -1;
}

static void FileOptions_Copy(int flag) {
    File *file = Dirbrowse_GetFileIndex(position);

    if (file == NULL)
        return;

    // Copy file source
    strcpy(copysource, cwd);
    strcpy(copysource + strlen(copysource), file->name);

    if ((file->isDir) && (strncmp(file->name, "..", 2) != 0)) // If directory, add recursive folder flag
        flag |= COPY_FOLDER_RECURSIVE;

    copymode = flag; // Set copy flags
}

// Paste file or folder
static int FileOptions_Paste(void) {
    if (copymode == NOTHING_TO_COPY) // No copy source
        return -1;

    // Source and target folder are identical
    char *lastslash = NULL;
    int i = 0;

    for (; i < strlen(copysource); i++)
        if (copysource[i] == '/')
            lastslash = copysource + i;

    char backup = lastslash[1];
    lastslash[1] = 0;
    int identical = strcmp(copysource, cwd) == 0;
    lastslash[1] = backup;

    if (identical)
        return -2;

    char *filename = lastslash + 1; // Source filename

    int requiredlength = strlen(cwd) + strlen(filename) + 1; // Required target path buffer size
    char *copytarget = (char *) malloc(requiredlength);      // Allocate target path buffer

    // Puzzle target path
    strcpy(copytarget, cwd);
    strcpy(copytarget + strlen(copytarget), filename);

    int ret = -3; // Return result

    // Recursive folder copy
    if ((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE) {
        // Check files in current folder
        File *node = files;
        for (; node != NULL; node = node->next) {
            if ((strcmp(filename, node->name) == 0) && (!node->isDir)) // Found a file matching the name (folder = ok, file = not)
                return -4;                                             // Error out
        }

        ret = FileOptions_CopyDir(copysource, copytarget); // Copy folder recursively

        if (((ret) == 0) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH) {
            // Needs to add a forward "/"
            if (!(strcmp(&(copysource[(strlen(copysource) - 1)]), "/") == 0))
                strcat(copysource, "/");

            FileOptions_RmdirRecursive(copysource); // Delete dir
        }
    }

    // Simple file copy
    else {
        ret = FileOptions_CopyFile(copysource, copytarget, true); // Copy file

        if (((ret) == 0) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
            remove(copysource); // Delete file
    }

    // Paste success
    if ((ret) == 0) {
        memset(copysource, 0, sizeof(copysource)); // Erase cache data
        copymode = NOTHING_TO_COPY;
    }

    free(copytarget); // Free target path buffer
    return ret;       // Return result
}

static void HandleDelete(void) {
    if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
        for (int i = 0; i < multi_select_index; i++) {
            if (strlen(multi_select_paths[i]) != 0) {
                if (strncmp(multi_select_paths[i], "..", 2) != 0) {
                    if (FS_DirExists(multi_select_paths[i])) {
                        // Add Trailing Slash
                        multi_select_paths[i][strlen(multi_select_paths[i]) + 1] = 0;
                        multi_select_paths[i][strlen(multi_select_paths[i])] = '/';
                        FileOptions_RmdirRecursive(multi_select_paths[i]);
                    } else if (FS_FileExists(multi_select_paths[i]))
                        remove(multi_select_paths[i]);
                }
            }
        }

        FileOptions_ResetClipboard();
    } else if (FileOptions_DeleteFile() != 0)
        return;

    Dirbrowse_PopulateFiles(true);
    MENU_DEFAULT_STATE = MENU_STATE_HOME;
}

void Menu_ControlDeleteDialog(uint32_t input, TouchInfo touchInfo) {
    if ((input & KEY_RIGHT) || (input & KEY_LSTICK_RIGHT) || (input & KEY_RSTICK_RIGHT))
        delete_dialog_selection++;
    else if ((input & KEY_LEFT) || (input & KEY_LSTICK_LEFT) || (input & KEY_RSTICK_LEFT))
        delete_dialog_selection--;

    Utils_SetMax(&delete_dialog_selection, 0, 1);
    Utils_SetMin(&delete_dialog_selection, 1, 0);

    if (input & KEY_B) {
        delete_dialog_selection = 0;
        MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
    }

    if (input & KEY_A) {
        if (delete_dialog_selection == 0)
            HandleDelete();
        else
            MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;

        delete_dialog_selection = 0;
    }

    if (touchInfo.state == TouchStart) {
        // Confirm Button
        if (tapped_inside(touchInfo, 1010 - delete_confirm_width, (720 - delete_height) / 2 + 225, 1050 + delete_confirm_width, (720 - delete_height) / 2 + 265 + delete_confirm_height))
            delete_dialog_selection = 0;
        // Cancel Button
        else if (tapped_inside(touchInfo, 895 - delete_confirm_width, (720 - delete_height) / 2 + 225, 935 + delete_confirm_width, (720 - delete_height) / 2 + 265 + delete_cancel_height))
            delete_dialog_selection = 1;
    } else if (touchInfo.state == TouchEnded && touchInfo.tapType != TapNone) {
        // Touched outside
        if (tapped_outside(touchInfo, (1280 - delete_width) / 2, (720 - delete_height) / 2, (1280 + delete_width) / 2, (720 + delete_height) / 2))
            MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
        // Confirm Button
        else if (tapped_inside(touchInfo, 1010 - delete_confirm_width, (720 - delete_height) / 2 + 225, 1050 + delete_confirm_width, (720 - delete_height) / 2 + 265 + delete_confirm_height))
            HandleDelete();
        // Cancel Button
        else if (tapped_inside(touchInfo, 895 - delete_confirm_width, (720 - delete_height) / 2 + 225, 935 + delete_confirm_width, (720 - delete_height) / 2 + 265 + delete_cancel_height))
            MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
    }
}

void Menu_DisplayDeleteDialog(void) {
    int text_width = 0;
    text_width = FC_GetWidth(Roboto, "Do you want to continue?");

    delete_confirm_width = FC_GetWidth(Roboto, "YES");
    delete_confirm_height = FC_GetHeight(Roboto, "YES");

    delete_cancel_width = FC_GetWidth(Roboto, "NO");
    delete_cancel_height = FC_GetHeight(Roboto, "NO");

    SDL_QueryTexture(dialog, NULL, NULL, &delete_width, &delete_height);

    SDL_DrawImage(RENDERER, config_dark_theme ? dialog_dark : dialog, ((1280 - (delete_width)) / 2), ((720 - (delete_height)) / 2));
    SDL_DrawText(RENDERER, Roboto, ((1280 - (delete_width)) / 2) + 80, ((720 - (delete_height)) / 2) + 45, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "Confirm deletion");
    SDL_DrawText(RENDERER, Roboto, ((1280 - (text_width)) / 2), ((720 - (delete_height)) / 2) + 130, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Do you wish to continue?");

    if (delete_dialog_selection == 0)
        SDL_DrawRect(RENDERER, (1030 - (delete_confirm_width)) - 20, (((720 - (delete_height)) / 2) + 245) - 20, delete_confirm_width + 40, delete_confirm_height + 40, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (delete_dialog_selection == 1)
        SDL_DrawRect(RENDERER, (915 - (delete_confirm_width)) - 20, (((720 - (delete_height)) / 2) + 245) - 20, delete_confirm_width + 40, delete_cancel_height + 40, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    SDL_DrawText(RENDERER, Roboto, 1030 - (delete_confirm_width), ((720 - (delete_height)) / 2) + 245, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
    SDL_DrawText(RENDERER, Roboto, 910 - (delete_cancel_width), ((720 - (delete_height)) / 2) + 245, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");
}

void Menu_ControlProperties(uint32_t input, TouchInfo touchInfo) {
    if ((input & KEY_A) || (input & KEY_B))
        MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;

    if (touchInfo.state == TouchEnded && touchInfo.tapType != TapNone) {
        if (tapped_outside(touchInfo, 350, 85, 930, 635))
            MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
        // Ok Button
        else if (tapped_inside(touchInfo, 870 - properties_ok_width, 575 - properties_ok_height, 910 + properties_ok_width, 615 + properties_ok_height))
            MENU_DEFAULT_STATE = MENU_STATE_OPTIONS;
    }
}

void Menu_DisplayProperties(void) {
    // Find File
    File *file = Dirbrowse_GetFileIndex(position);

    char path[512];
    strcpy(path, cwd);
    strcpy(path + strlen(path), file->name);

    SDL_DrawImage(RENDERER, config_dark_theme ? properties_dialog_dark : properties_dialog, 350, 85);
    SDL_DrawText(RENDERER, Roboto, 370, 133, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "Actions");

    char utils_size[16];
    uint32_t size = FS_GetFileSize(path);
    Utils_GetSizeString(utils_size, size);

    SDL_DrawTextf(RENDERER, Roboto, 390, 183, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Name: %s", file->name);
    SDL_DrawTextf(RENDERER, Roboto, 390, 233, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Parent: %s", cwd);

    if (!file->isDir) {
        SDL_DrawTextf(RENDERER, Roboto, 390, 283, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Size: %s", utils_size);
        //SDL_DrawText(RENDERER, Roboto, 390, 333, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Created: ");
        //SDL_DrawText(RENDERER, Roboto, 390, 383, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Modified: ");
    }
    properties_ok_width = FC_GetWidth(Roboto, "OK");
    properties_ok_height = FC_GetHeight(Roboto, "OK");
    SDL_DrawRect(RENDERER, (890 - properties_ok_width) - 20, (595 - properties_ok_height) - 20, properties_ok_width + 40, properties_ok_height + 40, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    SDL_DrawText(RENDERER, Roboto, 890 - properties_ok_width, 595 - properties_ok_height, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
}

static void HandleCopy() {
    if ((!copy_status) && (!cut_status)) {
        copy_status = true;
        FileOptions_Copy(COPY_KEEP_ON_FINISH);
        MENU_DEFAULT_STATE = MENU_STATE_HOME;
    } else if (copy_status) {
        if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
            char dest[512];

            for (int i = 0; i < multi_select_index; i++) {
                if (strlen(multi_select_paths[i]) != 0) {
                    if (strncmp(multi_select_paths[i], "..", 2) != 0) {
                        snprintf(dest, 512, "%s%s", cwd, Utils_Basename(multi_select_paths[i]));

                        if (FS_DirExists(multi_select_paths[i]))
                            FileOptions_CopyDir(multi_select_paths[i], dest);
                        else if (FS_FileExists(multi_select_paths[i]))
                            FileOptions_CopyFile(multi_select_paths[i], dest, true);
                    }
                }
            }

            FileOptions_ResetClipboard();
            copymode = NOTHING_TO_COPY;

        } else if (FileOptions_Paste() != 0)
            return;

        copy_status = false;
        Dirbrowse_PopulateFiles(true);
        MENU_DEFAULT_STATE = MENU_STATE_HOME;
    }
}

static void HandleCut() {
    if ((!cut_status) && (!copy_status)) {
        cut_status = true;
        FileOptions_Copy(COPY_DELETE_ON_FINISH);
        MENU_DEFAULT_STATE = MENU_STATE_HOME;
    } else if (cut_status) {
        char dest[512];

        if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
            for (int i = 0; i < multi_select_index; i++) {
                if (strlen(multi_select_paths[i]) != 0) {
                    snprintf(dest, 512, "%s%s", cwd, Utils_Basename(multi_select_paths[i]));

                    if (FS_DirExists(multi_select_paths[i]))
                        rename(multi_select_paths[i], dest);
                    else if (FS_FileExists(multi_select_paths[i]))
                        rename(multi_select_paths[i], dest);
                }
            }

            FileOptions_ResetClipboard();
        } else {
            snprintf(dest, 512, "%s%s", cwd, Utils_Basename(copysource));

            if (FS_DirExists(copysource))
                rename(copysource, dest);
            else if (FS_FileExists(copysource))
                rename(copysource, dest);
        }

        cut_status = false;
        copymode = NOTHING_TO_COPY;
        Dirbrowse_PopulateFiles(true);
        MENU_DEFAULT_STATE = MENU_STATE_HOME;
    }
}

void Menu_ControlOptions(uint32_t input, TouchInfo touchInfo) {
    if ((input & KEY_RIGHT) || (input & KEY_LSTICK_RIGHT) || (input & KEY_RSTICK_RIGHT))
        row++;
    else if ((input & KEY_LEFT) || (input & KEY_LSTICK_LEFT) || (input & KEY_RSTICK_LEFT))
        row--;

    if ((input & KEY_DDOWN) || (input & KEY_LSTICK_DOWN) || (input & KEY_RSTICK_DOWN))
        column++;
    else if ((input & KEY_DUP) || (input & KEY_LSTICK_UP) || (input & KEY_RSTICK_UP))
        column--;

    Utils_SetMax(&row, 0, 1);
    Utils_SetMin(&row, 1, 0);

    Utils_SetMax(&column, 0, 2);
    Utils_SetMin(&column, 2, 0);

    if (input & KEY_A) {
        if (row == 0 && column == 0)
            MENU_DEFAULT_STATE = MENU_STATE_PROPERTIES;
        else if (row == 1 && column == 0)
            FileOptions_CreateFolder();
        else if (row == 0 && column == 1)
            FileOptions_Rename();
        else if (row == 1 && column == 1)
            HandleCopy();
        else if (row == 0 && column == 2)
            HandleCut();
        else if (row == 1 && column == 2)
            MENU_DEFAULT_STATE = MENU_STATE_DELETE_DIALOG;
    }

    if (input & KEY_B) {
        copy_status = false;
        cut_status = false;
        row = 0;
        column = 0;
        MENU_DEFAULT_STATE = MENU_STATE_HOME;
    }

    if (input & KEY_X)
        MENU_DEFAULT_STATE = MENU_STATE_HOME;

    if (touchInfo.state == TouchStart) {
        // Column 0
        if (touchInfo.firstTouch.py >= 188 && touchInfo.firstTouch.py <= 289) {
            column = 0;

            // Row 0
            if (touchInfo.firstTouch.px >= 354 && touchInfo.firstTouch.px <= 638)
                row = 0;
            // Row 1
            else if (touchInfo.firstTouch.px >= 639 && touchInfo.firstTouch.px <= 924)
                row = 1;
        }
        // Column 1
        else if (touchInfo.firstTouch.py >= 291 && touchInfo.firstTouch.py <= 392) {
            column = 1;

            // Row 0
            if (touchInfo.firstTouch.px >= 354 && touchInfo.firstTouch.px <= 638)
                row = 0;
            // Row 1
            else if (touchInfo.firstTouch.px >= 639 && touchInfo.firstTouch.px <= 924)
                row = 1;
        }
        // Column 2
        else if (touchInfo.firstTouch.py >= 393 && touchInfo.firstTouch.py <= 494) {
            column = 2;

            // Row 0
            if (touchInfo.firstTouch.px >= 354 && touchInfo.firstTouch.px <= 638)
                row = 0;
            // Row 1
            else if (touchInfo.firstTouch.px >= 639 && touchInfo.firstTouch.px <= 924)
                row = 1;
        }
    } else if (touchInfo.state == TouchEnded && touchInfo.tapType != TapNone) {
        // Touched outside
        if (tapped_outside(touchInfo, 350, 85, 930, 635))
            MENU_DEFAULT_STATE = MENU_STATE_HOME;
        // Column 0
        else if (touchInfo.firstTouch.py >= 188 && touchInfo.firstTouch.py <= 289) {
            // Row 0
            if (touchInfo.firstTouch.px >= 354 && touchInfo.firstTouch.px <= 638)
                MENU_DEFAULT_STATE = MENU_STATE_PROPERTIES;
            // Row 1
            else if (touchInfo.firstTouch.px >= 639 && touchInfo.firstTouch.px <= 924)
                FileOptions_CreateFolder();
        }
        // Column 1
        else if (touchInfo.firstTouch.py >= 291 && touchInfo.firstTouch.py <= 392) {
            // Row 0
            if (touchInfo.firstTouch.px >= 354 && touchInfo.firstTouch.px <= 638)
                FileOptions_Rename();
            // Row 1
            else if (touchInfo.firstTouch.px >= 639 && touchInfo.firstTouch.px <= 924)
                HandleCopy();
        }
        // Column 2
        else if (touchInfo.firstTouch.py >= 393 && touchInfo.firstTouch.py <= 494) {
            // Row 0
            if (touchInfo.firstTouch.px >= 354 && touchInfo.firstTouch.px <= 638)
                HandleCut();
            // Row 1
            else if (touchInfo.firstTouch.px >= 639 && touchInfo.firstTouch.px <= 924)
                MENU_DEFAULT_STATE = MENU_STATE_DELETE_DIALOG;
        }
        // Cancel Button
        else if (tapped_inside(touchInfo, 880 - options_cancel_width, 585 - options_cancel_height, 920 + options_cancel_width, 625 + options_cancel_height))
            MENU_DEFAULT_STATE = MENU_STATE_HOME;
    }
}

void Menu_DisplayOptions(void) {
    SDL_DrawImage(RENDERER, config_dark_theme ? options_dialog_dark : options_dialog, 350, 85);
    SDL_DrawText(RENDERER, Roboto, 370, 133, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "Actions");

    options_cancel_width = FC_GetWidth(Roboto, "CANCEL");
    options_cancel_height = FC_GetHeight(Roboto, "CANCEL");
    SDL_DrawText(RENDERER, Roboto, 900 - options_cancel_width, 605 - options_cancel_height, config_dark_theme ? TITLE_COLOUR_DARK : TITLE_COLOUR, "CANCEL");

    if (row == 0 && column == 0)
        SDL_DrawRect(RENDERER, 354, 188, 287, 101, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row == 1 && column == 0)
        SDL_DrawRect(RENDERER, 638, 188, 287, 101, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row == 0 && column == 1)
        SDL_DrawRect(RENDERER, 354, 291, 287, 101, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row == 1 && column == 1)
        SDL_DrawRect(RENDERER, 638, 291, 287, 101, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row == 0 && column == 2)
        SDL_DrawRect(RENDERER, 354, 393, 287, 101, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (row == 1 && column == 2)
        SDL_DrawRect(RENDERER, 638, 393, 287, 101, config_dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

    SDL_DrawText(RENDERER, Roboto, 385, 225, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Properties");
    SDL_DrawText(RENDERER, Roboto, 385, 327, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Rename");
    SDL_DrawText(RENDERER, Roboto, 385, 429, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, cut_status ? "Paste" : "Move");

    SDL_DrawText(RENDERER, Roboto, 672, 225, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "New folder");
    SDL_DrawText(RENDERER, Roboto, 672, 327, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, copy_status ? "Paste" : "Copy");
    SDL_DrawText(RENDERER, Roboto, 672, 429, config_dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Delete");
}

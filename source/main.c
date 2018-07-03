#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <switch.h>

#define TARGET_DIR "sdmc:/ttf"

#define FS_ERROR MAKERESULT(Module_Libnx, LibnxError_InitFail_FS)

char fontNames[PlSharedFontType_Total][0x20] = {
    "Standard",
    "ChineseSimplified",
    "ExtendedChineseSimplified",
    "ChineseTraditional",
    "Korean",
    "NintendoExtended"
};

Result dumpFonts(void)
{
    // Create directory if not found
    DIR *dir = opendir(TARGET_DIR);
    if (!dir)
    {
        int fs_res = mkdir(TARGET_DIR, 0644);
        if (fs_res == -1)
            return FS_ERROR;
    }
    else
        closedir(dir);


    PlFontData font_infos[PlSharedFontType_Total];
    u64 total_fonts;

    Result res = plInitialize();

    if (R_FAILED(res))
        return res;
    
    res = plGetSharedFont(SetLanguage_JA, font_infos, PlSharedFontType_Total, &total_fonts);
    if (R_SUCCEEDED(res))
    {
        for (int i = 0; i < total_fonts; i++)
        {
            char path[0x100];
            if (font_infos[i].type >= PlSharedFontType_Total)
                snprintf(path, 0x100, "%s/FontUnknownType%u.ttf", TARGET_DIR, font_infos[i].type);
            else
                snprintf(path, 0x100, "%s/Font%s.ttf", TARGET_DIR, fontNames[font_infos[i].type]);
            printf("%s\n", path);
            FILE *file = fopen(path, "w");
            if (file != NULL)
            {
                fwrite(font_infos[i].address, font_infos[i].size, 1, file);
                fclose(file);
            }
            else
            {
                fprintf(stderr, "Cannot open file %s!\nAborting...", path);
                plExit();
                return FS_ERROR;
            }
        }
    }

    plExit();
    return res;
}

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(NULL);

    printf("Press A to dump fonts\n");
    printf("Press + to exit\n");
    printf("Fonts will be dumped to %s\n", TARGET_DIR);

    int font_dump_started = 0;
    while(appletMainLoop())
    {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) break;
        else if (kDown & KEY_A && !font_dump_started)
        {
            font_dump_started = 1;
            Result res = dumpFonts();
            if (R_FAILED(res))
                fprintf(stderr, "An error occured\nError code: 0x%08x\n", res);
            else
                printf("Operation completed\nPress + to exit\n");
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
    gfxExit();
    return (0);
}
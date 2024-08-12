#define NOBUILD_IMPLEMENTATION
#include "nob.h"

#include "nob_cfg.h"

#define COMPILER "gcc"

#define C_STD 99

#define VENDIR VENDIR_NOSLASH "/"
#define BINDIR BINDIR_NOSLASH "/"
#define SRCDIR SRCDIR_NOSLASH "/"

#define LIBDIR VENDIR "lib/"

#define EDITOR_EXEC BINDIR "editor"
#define EX_MAPREADER_EXEC BINDIR "mapreader"

#define STRIP_EVERYTHING "strip", "-S", "-s", "-R .comment", "-R .gnu.version"

int main(int argc, char* argv[]) {
    GO_REBUILD_URSELF(argc, argv);

    char* cstd = malloc(10 * sizeof(char));
    sprintf(cstd, "-std=c%d", C_STD);

#define CFLAGS "-Wall", "-Wextra", "-O3", "-flto", "-Wno-unused-variable", "-Wno-unused-parameter", cstd
#define IFLAGS "-Isrc", "-I" VENDIR "include"
#define LFLAGS "-L" LIBDIR, "-lraylib", "-l:cimgui.so", "-lm", "-Wl,-R" LIBDIR, "-Wl,--gc-sections", "-ffunction-sections", "-fdata-sections"
#define SFLAGS SRCDIR "editor.c", SRCDIR "ui.c", VENDIR "rlcimgui.c"

    /*Running:
    gcc src/editor.c src/ui.c vendor/rlcimgui.c -Wall -Wextra -O3 -std=c99 -flto -l:cimgui.so -lraylib -lm -Ivendor/include -Lvendor/lib -Wl,-Rvendor/lib -Wno-unused-variable -Wno-unused-parameter -Wl,--gc-sections -ffunction-sections -fdata-sections -o bin/editor
    */

    CMD(COMPILER, CFLAGS, SFLAGS, IFLAGS, LFLAGS, "-o", EDITOR_EXEC);

    CMD(STRIP_EVERYTHING, EDITOR_EXEC);
    CMD(STRIP_EVERYTHING, EX_MAPREADER_EXEC);

    CMD("cloc", ".", "--not-match-f=nob_cfg.h", "--not-match-f=nob.h", "--not-match-f=nob.c",
        "--exclude-dir=" VENDIR_NOSLASH "," BINDIR_NOSLASH "," ".vscode",
        "--exclude-lang=Bourne Again Shell,INI");

    CMD("wc", "-c", EDITOR_EXEC);

    free(cstd);
    return 0;
}
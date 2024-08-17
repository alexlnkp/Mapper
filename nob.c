#define NOBUILD_IMPLEMENTATION
#include "nob.h"

/* To build, run: `cc -o ./nob nob.c src/argparse.c` */

#include "nob_cfg.h"

#define COMPILER "gcc"

#define C_STD 99

#define VENDIR VENDIR_NOSLASH "/"
#define BINDIR BINDIR_NOSLASH "/"
#define SRCDIR SRCDIR_NOSLASH "/"

#include "vendor/argparse.h"

#define LIBDIR VENDIR "lib/"

#define EDITOR_EXEC BINDIR "editor"
#define EX_MAPREADER_EXEC BINDIR "mapreader"

#define STRIP_EVERYTHING "strip", "-S", "-s", "-R .comment", "-R .gnu.version"

static const char *const usages[] = {
    "./nob [options] [[--] args]",
    "./nob [options]",
    NULL,
};

int main(int argc, const char** argv) {
    GO_REBUILD_URSELF(argc, argv);

    int debug = 0;
    int build_examples = 0;

    char* cstd = malloc(10 * sizeof(char));
    sprintf(cstd, "-std=c%d", C_STD);

#define CFLAGS_REL "-Wall", "-Wextra", "-O3", "-flto", "-Wno-unused-variable", "-Wno-unused-parameter", cstd
#define IFLAGS_REL "-Isrc", "-I" VENDIR "include"
#define LFLAGS_REL "-L" LIBDIR, "-lraylib", "-l:cimgui.so", "-lm", "-Wl,-R" LIBDIR, "-Wl,--gc-sections", "-ffunction-sections", "-fdata-sections"
#define SFLAGS_REL SRCDIR "editor.c", SRCDIR "ui.c", VENDIR "rlcimgui.c"

#define CFLAGS_DBG "-Wall", "-Wextra", "-Og", "-g", "-Wno-unused-variable", "-Wno-unused-parameter", cstd
#define IFLAGS_DBG "-Isrc", "-I" VENDIR "include"
#define LFLAGS_DBG "-L" LIBDIR, "-lraylib", "-l:cimgui.so", "-lm", "-Wl,-R" LIBDIR
#define SFLAGS_DBG SRCDIR "editor.c", SRCDIR "ui.c", VENDIR "rlcimgui.c"

    /*Running:
    gcc src/editor.c src/ui.c vendor/rlcimgui.c -Wall -Wextra -O3 -std=c99 -flto -l:cimgui.so -lraylib -lm -Ivendor/include -Lvendor/lib -Wl,-Rvendor/lib -Wno-unused-variable -Wno-unused-parameter -Wl,--gc-sections -ffunction-sections -fdata-sections -o bin/editor
    */

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN('d', "debug", &debug, "build in debug", NULL, 0, 0),
        OPT_BOOLEAN('e', "build-examples", &build_examples, "build examples", NULL, 0, 0),
        OPT_END()
    };

    struct argparse argparse;

    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\n  nobuild (nob) \n\tbuild stuff", "\nsome more info lol");
    argc = argparse_parse(&argparse, argc, argv);

    if (debug != 0) {
        /* building debug!!! */
        CMD(COMPILER, CFLAGS_DBG, SFLAGS_DBG, IFLAGS_DBG, LFLAGS_DBG, "-o", EDITOR_EXEC);
    } else {
        /* building NOT debug!!! */
        CMD(COMPILER, CFLAGS_REL, SFLAGS_REL, IFLAGS_REL, LFLAGS_REL, "-o", EDITOR_EXEC);
        CMD(STRIP_EVERYTHING, EDITOR_EXEC);
    }

    if (build_examples != 0) {
        CMD(COMPILER, CFLAGS_REL, "-Isrc", "examples/readmap.c", "-L" LIBDIR, "-lraylib", "-o", EX_MAPREADER_EXEC);
    }

    CMD("cloc", ".", "--quiet", "--not-match-f=nob_cfg.h", "--not-match-f=nob.h", "--not-match-f=nob.c",
        "--exclude-dir=" VENDIR_NOSLASH "," BINDIR_NOSLASH "," ".vscode",
        "--exclude-lang=Bourne Again Shell,INI,Markdown");

    CMD("wc", "-c", EDITOR_EXEC);

    free(cstd);
    return 0;
}
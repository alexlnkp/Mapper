#define NOB_IMPLEMENTATION
#include "nob.h"

/* To build, run: `cc -o ./nob nob.c vendor/argparse.c` */

#define COMPILER "gcc"

#define C_STD 99

#define VENDIR_NOSLASH "vendor"
#define BINDIR_NOSLASH "bin"
#define SRCDIR_NOSLASH "src"

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

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

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

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN('d', "debug", &debug, "build in debug", NULL, 0, 0),
        OPT_BOOLEAN('e', "build-examples", &build_examples, "build examples", NULL, 0, 0),
        OPT_END()
    };

    struct argparse argparse;

    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\n  nobuild (nob) \n\tbuild stuff", "\nsome more info lol");
    argc = argparse_parse(&argparse, argc, (const char**)argv);

    Nob_Cmd cc = {0};
    Nob_Cmd misc = {0};

    if (debug != 0) {
        /* building debug!!! */
        nob_cmd_append(&cc, COMPILER, CFLAGS_DBG, SFLAGS_DBG, IFLAGS_DBG, LFLAGS_DBG, "-o", EDITOR_EXEC);
        nob_cmd_append(&misc, "true"); /* i've never heard of this command before, but it's hysterical */
    } else {
        /* building NOT debug!!! */
        nob_cmd_append(&cc, COMPILER, CFLAGS_REL, SFLAGS_REL, IFLAGS_REL, LFLAGS_REL, "-o", EDITOR_EXEC);
        nob_cmd_append(&misc, STRIP_EVERYTHING, EDITOR_EXEC);
    }

    nob_cmd_run_sync_and_reset(&cc);
    nob_cmd_run_sync_and_reset(&misc);

    if (build_examples != 0) {
        nob_cmd_append(&cc, COMPILER, CFLAGS_REL, "-Isrc");
        nob_cmd_append(&cc, "examples/readmap.c", "-L" LIBDIR, "-lraylib", "-o", EX_MAPREADER_EXEC);
        nob_cmd_run_sync_and_reset(&cc);
    }

    nob_cmd_append(
        &misc,
        "cloc", ".", "--quiet", "--not-match-f=nob_cfg.h", "--not-match-f=nob.h",
        "--not-match-f=nob.c", "--exclude-dir=" VENDIR_NOSLASH "," BINDIR_NOSLASH "," ".vscode",
        "--exclude-lang=Bourne Again Shell,INI,Markdown"
    );

    nob_cmd_run_sync_and_reset(&misc);

    nob_cmd_append(&misc, "wc", "-c", EDITOR_EXEC);
    nob_cmd_run_sync_and_reset(&misc);

    free(cstd);
    return 0;
}

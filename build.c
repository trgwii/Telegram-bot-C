#/*
/usr/bin/env zig run -lc build.c -- $@
exit $?
#*/

// Change this to your compiler of choice
#define CC "zig cc"

#define INCLUDES "-Ideps/curl-8.0.1_5-win64-mingw/include"

// System dependencies
#define LIBS "-lcurl -lsqlite3"

// "-target x86_64-linux-gnu"
// "-target x86_64-windows-gnu"

#ifdef __clang__
#define FLAGS                                                                  \
  "-Weverything "                                                              \
  "-Werror "                                                                   \
  "-Wno-padded "                                                               \
  "-Wno-disabled-macro-expansion "                                             \
  "-Wno-declaration-after-statement "                                          \
  "-Wno-used-but-marked-unused"
#else
#define FLAGS                                                                  \
  "-Wall "                                                                     \
  "-Wextra "                                                                   \
  "-Wpedantic "                                                                \
  "-Werror"
#endif

#include "build.h"

int main(int argc, char **argv) {

  if (argc <= 1) {
    printf(
        "Usage:\n\t\x1b[37m./build.c [clean] [self] [fetch] "
        "[test] "
        "[build] [all]\x1b[0m\n\n\t\x1b[32mclean\x1b[0m - Remove output "
        "folders\n\t\x1b[32mself\x1b[0m - Run the "
        "compiler on build.c with flags enabled "
        "(self-test)\n\t\x1b[32mfetch\x1b[0m - "
        "Fetch remote dependencies using curl from "
        "CLI\n\t\x1b[32mtest\x1b[0m - Run "
        "tests\n\t\x1b[32mbuild\x1b[0m - Build the bot\n\t\x1b[32mall\x1b[0m "
        "- Do all of the above\n");
    return 1;
  }

  if (cli_command(argc, argv, "clean")) {
    double start = measure_start();
    RM("deps");
    RM("src/json.h");
    RM("o");
    RM("test");
    RM("build");
    measure_end("clean", start);
  }

  if (cli_command(argc, argv, "self")) {
    double start = measure_start();
#ifdef _WIN32
    CMD(CC " " FLAGS " build.c -o - >NUL");
#else
    CMD(CC " " FLAGS " build.c -o - > /dev/null");
#endif
    measure_end("self", start);
  }

  if (cli_command(argc, argv, "fetch")) {
    double start = measure_start();
#ifdef _WIN32
    DIR("deps");
    HTTP_GET("https://curl.se/windows/dl-8.0.1_5/curl-8.0.1_5-win64-mingw.zip",
             "deps/curl-8.0.1_5-win64-mingw.zip");
    SetCurrentDirectory("deps");
    CMD("tar -xf curl-8.0.1_5-win64-mingw.zip");
    SetCurrentDirectory("..");
#endif
    HTTP_GET("https://raw.githubusercontent.com/sheredom/json.h/master/json.h",
             "src/json.h");

    measure_end("fetch", start);
  }

  if (cli_command(argc, argv, "test")) {
    double start = measure_start();
    DIR("test");
    TEST("str");
    measure_end("test", start);
  }

  if (cli_command(argc, argv, "build")) {
    double start = measure_start();
    DIR("o");
    OBJ("str");
    OBJ("bot");

    DIR("build");
    EXE("o/*.o src/main.c -o build/main");
    measure_end("build", start);
  }

  return done();
}

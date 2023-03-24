#/*
/usr/bin/env zig run -lc build.c -- $@
exit $?
#*/

#include "build.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  bool clean = false;
  bool self = false;
  bool fetch = false;
  bool test = false;
  bool build = false;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "clean") == 0)
      clean = true;
    else if (strcmp(argv[i], "self") == 0)
      self = true;
    else if (strcmp(argv[i], "fetch") == 0)
      fetch = true;
    else if (strcmp(argv[i], "test") == 0)
      test = true;
    else if (strcmp(argv[i], "build") == 0)
      build = true;
  }

  if (!clean && !self && !fetch && !test && !build) {
    printf("Usage:\n\t\x1b[37mzig run -lc build.c -- [clean] [self] [fetch] "
           "[test] "
           "[build]\x1b[0m\n\n\t\x1b[32mclean\x1b[0m - Remove output "
           "folders\n\t\x1b[32mself\x1b[0m - Run the "
           "compiler on build.c with flags enabled "
           "(self-test)\n\t\x1b[32mfetch\x1b[0m - "
           "Fetch remote dependencies using curl from "
           "CLI\n\t\x1b[32mtest\x1b[0m - Run "
           "tests\n\t\x1b[32mbuild\x1b[0m - Build the bot\n");
    return 1;
  }

  if (clean) {
    double start = measure_start();
    RM("test");
    RM("o");
    RM("build");
    measure_end("clean", start);
  }

  if (self) {
    double start = measure_start();
    CMD(CC " " FLAGS " build.c -o - > /dev/null");
    measure_end("self", start);
  }

  if (fetch) {
    double start = measure_start();
    HTTP_GET("https://raw.githubusercontent.com/sheredom/json.h/master/json.h",
             "src/json.h");

    measure_end("fetch", start);
  }

  if (test) {
    double start = measure_start();
    DIR("test");
    TEST("str");
    measure_end("test", start);
  }

  if (build) {
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

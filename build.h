#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define CC "zig cc"
#define LIBS "-lcurl -lsqlite3"

#ifdef __clang__
#define FLAGS                                                                  \
  "-Weverything "                                                              \
  "-Werror "                                                                   \
  "-Wno-padded "                                                               \
  "-Wno-disabled-macro-expansion "                                             \
  "-Wno-declaration-after-statement"
#else
#define FLAGS                                                                  \
  "-Wall "                                                                     \
  "-Wextra "                                                                   \
  "-Wpedantic "                                                                \
  "-Werror"
#endif

static bool ok = true;
static char *failed = NULL;
static bool measured = false;
static char measured_col = 0;
static char measured_col_max = 60;
static bool measured_failing = false;
static double measured_total = 0;

#define DIR(name) mkdir(name, 0755)

#define CMD(command)                                                           \
  do {                                                                         \
    char *cmd = command;                                                       \
    if (ok && system(cmd) != 0) {                                              \
      ok = false;                                                              \
      failed = cmd;                                                            \
    }                                                                          \
  } while (false)

#define HTTP_GET(url, file) CMD("curl -s " url " -o " file)

#define OBJ(name) CMD(CC " " FLAGS " src/" name ".c -c -o o/" name ".o")

#define EXE(cmd) CMD(CC " " FLAGS " " cmd " " LIBS)

#define TEST(name)                                                             \
  CMD(CC " " FLAGS " src/" name "_test.c -o test/" name " && ./test/" name     \
         " > /dev/null")

#define RM(name) system("rm -r " name " 2> /dev/null")

static inline double measure_start(void) {
  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);
  return (double)start.tv_sec + (double)start.tv_nsec / 1000000000;
}

static inline void measure_end(char *name, double start) {
  struct timespec end;
  clock_gettime(CLOCK_REALTIME, &end);
  if (measured_failing)
    return;
  double measured_time =
      (((double)end.tv_sec + (double)end.tv_nsec / 1000000000) - start) * 1000;
  printf("%s%s:\t\x1b[33m%f\x1b[0mms",
         measured ? measured_col >= measured_col_max ? "\n" : "\t" : "", name,
         measured_time);
  fflush(stdout);
  if (measured_col >= measured_col_max)
    measured_col = 0;
  measured = true;
  measured_failing = !ok;
  measured_total += measured_time;
  measured_col += 20;
}

static inline bool cli_command(int argc, char **argv, char *command) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "all") == 0 || strcmp(argv[i], command) == 0)
      return true;
  }
  return false;
}

static inline int done(void) {
  printf("%stotal: \x1b[33m%f\x1b[0mms",
         measured_col >= measured_col_max ? "\n" : "\t", measured_total);
  if (measured_col >= measured_col_max)
    measured_col = 0;
  measured_col += 20;
  printf("%s%s%s\n", measured_col >= measured_col_max ? "\n" : "\t",
         ok ? "\x1b[32m== OK ==\x1b[0m" : "\x1b[31m!! FAILED !!\x1b[0m",
         failed ? ": " : "");
  if (failed) {
    printf("\x1b[31m%s\x1b[0m\n", failed);
  }

  return ok == true;
}

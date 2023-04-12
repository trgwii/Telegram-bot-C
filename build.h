#ifdef _WIN32
#include <windows.h>
#else
#define _GNU_SOURCE
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static bool ok = true;
static char *failed = NULL;
static bool measured = false;
static char measured_col = 0;
static char measured_col_max = 60;
static bool measured_failing = false;
static double measured_total = 0;

// Ensure a directory exists
#ifdef _WIN32
#define DIR(name) mkdir(name)
#else
#define DIR(name) mkdir(name, 0755)
#endif

// Run a command
#define CMD(command)                                                           \
  do {                                                                         \
    char *cmd = command;                                                       \
    if (ok && system(cmd) != 0) {                                              \
      ok = false;                                                              \
      failed = cmd;                                                            \
    }                                                                          \
  } while (false)

// Fetch a remote HTTP(S) file
#define HTTP_GET(url, file) CMD("curl -s " url " -o " file)

// Build an object file (src/<name>.c -> o/<name>.o)
#define OBJ(name)                                                              \
  CMD(CC " " FLAGS " " INCLUDES " src/" name ".c -c -o o/" name ".o")

// Build an executable
#define EXE(cmd) CMD(CC " " FLAGS " " INCLUDES " " cmd " " LIBS)

// Build and run a test (src/<name>_test.c -> test/<name>)
#ifdef _WIN32
#define TEST(name)                                                             \
  CMD(CC " " FLAGS " src/" name "_test.c -o test/" name ".exe && test\\" name  \
         " >NUL")
#else
#define TEST(name)                                                             \
  CMD(CC " " FLAGS " src/" name "_test.c -o test/" name " && ./test/" name     \
         " > /dev/null")
#endif

// Recursively remove a directory or file
#ifdef _WIN32
#define RM(name) system("rmdir /s /q " name " 2>NUL")
#else
#define RM(name) system("rm -r " name " 2> /dev/null")
#endif

// Get the current system clock for measuring a fenced code block
static double measure_start(void) {
#ifdef _WIN32
  return 0;
#else
  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);
  return (double)start.tv_sec + (double)start.tv_nsec / 1000000000;
#endif
}

// Complete a measurement by passing in a return value from measure_start()
static void measure_end(char *name, double start) {
  if (measured_failing)
    return;
#ifdef _WIN32
  double measured_time = start * 0;
#else
  struct timespec end;
  clock_gettime(CLOCK_REALTIME, &end);
  double measured_time =
      (((double)end.tv_sec + (double)end.tv_nsec / 1000000000) - start) * 1000;
#endif
  printf("%s\x1b[36m%s\x1b[0m:\t\x1b[33m%f\x1b[0mms",
         measured ? measured_col >= measured_col_max ? "\n" : "\t" : "", name,
         measured_time);
  if (measured_col >= measured_col_max)
    measured_col = 0;
  measured = true;
  measured_failing = !ok;
  measured_total += measured_time;
  measured_col += 20;
}

// Check if a CLI command was passed (or always true if "all" was passed)
static bool cli_command(int argc, char **argv, char *command) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "all") == 0 || strcmp(argv[i], command) == 0)
      return true;
  }
  return false;
}

// Use from build.c's main() to print build results and produce an exit code
static int done(void) {
  printf("%s\x1b[36mtotal\x1b[0m: \x1b[33m%f\x1b[0mms",
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

  return !ok;
}

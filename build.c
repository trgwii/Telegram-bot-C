// zig run -lc build.c

#include <stdlib.h>
#include <sys/stat.h>

#define CC "zig cc"
#define LIBS "-lcurl -lsqlite3"

#define DISABLED                                                               \
  "-Wno-disabled-macro-expansion "                                             \
  "-Wno-padded "                                                               \
  "-Wno-declaration-after-statement"
#define FLAGS                                                                  \
  "-Wall "                                                                     \
  "-Wextra "                                                                   \
  "-Wpedantic "                                                                \
  "-Weverything "                                                              \
  "-Werror " DISABLED

#define HTTP_GET(url, file) system("curl -s " url " -o " file)
#define OBJ(name) system(CC " " FLAGS " src/" name ".c -c -o o/" name ".o")
#define EXE(cmd) system(CC " " FLAGS " " LIBS " " cmd)
#define DIR(name) mkdir(name, 0755)
#define TEST(name)                                                             \
  system(CC " " FLAGS " src/" name "_test.c -o test/" name " && ./test/" name  \
            " > /dev/null")

int main(void) {
  // Deps
  HTTP_GET("https://raw.githubusercontent.com/sheredom/json.h/master/json.h",
           "src/json.h");

  // Tests
  DIR("test");
  TEST("str");

  // Object files
  DIR("o");
  OBJ("str");
  OBJ("bot");

  // Build
  DIR("build");
  EXE("o/*.o src/main.c -o build/main");

  return 0;
}

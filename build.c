// zig run -lc build.c

#include <stdlib.h>
#include <sys/stat.h>

#define CC "zig cc"
#define LIBS "-lcurl"

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

int main(void) {
  HTTP_GET("https://raw.githubusercontent.com/sheredom/json.h/master/json.h",
           "src/json.h");
  DIR("o");
  OBJ("str");
  OBJ("bot");
  DIR("build");
  EXE("o/*.o src/main.c -o build/main");
  return 0;
}

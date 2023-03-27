# Telegram bot in C

Requires `libcurl-dev` and `libsqlite3-dev` to build.

[Zig](https://ziglang.org/) is recommended, but not required.

## Quick Start

For gcc, edit [build.h line 10](build.h#L10) to `#define CC "gcc"`

```sh
./build.c all
# or:
# zig run -lc build.c -- all
# or (with gcc):
# gcc build.c -o build/build && ./build/build all

./build/main <token>
```

## Organization

- `src/` contains all the source files.
  - `src/json.h` is [sheredom/json.h](https://github.com/sheredom/json.h) from GitHub.
- `build.h` / `build.c` make up all the build scripts to build the project.
- `libcurl` and `libsqlite3` are required on the system in order to build the project, as well as `curl` on the command line.
- Building the project will generate the following folders:
  - `o` contains intermediate object files.
  - `test` contains executables used for testing functions in the codebase.
  - `build` contains the output binary (final program).
- Additionally, the bot will create the following at runtime:
  - `bot.db` is the sqlite3 database used to store rep points.
  - `commands/` is a folder used to store saved custom commands.

## TODO

- [x] Cleanup JSON handling in [main](src/main.c#L16)
  - [x] Explore more functions from [json.h](src/json.h)
- [ ] Add people to DB immediately if they try to issue a command and can't be found
- [ ] Investigate building libcurl + libsqlite3 statically from source

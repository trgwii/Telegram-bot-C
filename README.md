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

## TODO

- [x] Cleanup JSON handling in [main](src/main.c#L16)
  - [x] Explore more functions from [json.h](src/json.h)
- [ ] Add people to DB immediately if they try to issue a command and can't be found
- [ ] Investigate building libcurl + libsqlite3 statically from source

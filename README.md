# Telegram bot in C

Requires `libcurl-dev` and `libsqlite3-dev` to build.

[Zig](https://ziglang.org/) is recommended, but not required.

## Quick Start

```sh
./build.c clean self fetch test build
# or:
# zig run -lc build.c -- clean self fetch test build
./build/main <token>
```

## TODO

- [x] Cleanup JSON handling in [main](src/main.c#L16)
  - [x] Explore more functions from [json.h](src/json.h)
- [ ] Add people to DB immediately if they try to issue a command and can't be found

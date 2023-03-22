#include "json.h"
#include <curl/curl.h>
#include <stdbool.h>

typedef struct Bot {
  CURL *curl;
  char *token;
  unsigned long token_len;

  unsigned long url_offset;
  char url[1024];

  unsigned long data_offset;
  char data[1048576];

  unsigned long last_update_id;

  void (*handle_message)(struct Bot *, json_object_t *);
} Bot;

Bot Bot_init(char *token, void (*handle_message)(Bot *, json_object_t *));

void Bot_deinit(Bot *bot);

typedef struct BotInfo {
  uint64_t id;
  bool is_bot;
  char first_name[256];
  char username[256];
} BotInfo;

BotInfo Bot_getMe(Bot *bot);

void Bot_sendTextMessage(Bot *bot, long long chat_id, const char *text);

void Bot_getUpdates(Bot *bot);

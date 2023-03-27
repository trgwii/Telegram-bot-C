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

  char json_scratch[1048576];

  unsigned long last_update_id;

  const char *parse_mode;

  void *handle_message_user_data;

  void (*handle_message)(void *, struct Bot *, json_object_t *);
} Bot;

// Initialize a Bot struct using a token and a callback
Bot Bot_init(char *token, void *handle_message_user_data,
             void (*handle_message)(void *, Bot *, json_object_t *));

// Deinitialize a Bot (cleanup curl instance)
void Bot_deinit(Bot *bot);

// Returned from Bot_getMe()
typedef struct BotInfo {
  uint64_t id;
  bool is_bot;
  char first_name[256];
  char username[256];
} BotInfo;

// Get information about the bot using the API
BotInfo Bot_getMe(Bot *bot);

// Send a text message to chat_id
void Bot_sendTextMessage(Bot *bot, long long chat_id, const char *text);
// Send a text message to chat_id
void Bot_sendTextMessageLen(Bot *bot, long long chat_id, const char *text,
                            size_t text_len);

// Fetch new incoming messages, will call handle_message() for each message
void Bot_getUpdates(Bot *bot);

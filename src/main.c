#include "bot.h"
#include "str.h"
#include <assert.h>
#include <curl/curl.h>

static void handle_message(Bot *bot, json_object_t *message) {
  json_object_element_t *msg = message->start;
  long long chat_id = 0;
  while (msg) {
    if (str_eql("chat", msg->name->string)) {
      assert(msg->value->type == json_type_object);
      json_object_t *chat = msg->value->payload;
      json_object_element_t *cht = chat->start;
      while (cht) {
        if (str_eql("id", cht->name->string)) {
          assert(cht->value->type == json_type_number);
          json_number_t *id = cht->value->payload;
          chat_id = strtol(id->number, NULL, 10);
        }
        cht = cht->next;
      }
    } else if (str_eql("text", msg->name->string)) {
      assert(msg->value->type == json_type_string);
      json_string_t *text = msg->value->payload;
      const char *txt = text->string;
      if (chat_id) {
        if (str_starts_with(txt, "/help"))
          Bot_sendTextMessage(bot, chat_id, "Commands:%0A/fart%0A/cc");
        else if (str_starts_with(txt, "/fart"))
          Bot_sendTextMessage(bot, chat_id, "farted!");
        else if (str_starts_with(txt, "/cc"))
          Bot_sendTextMessage(bot, chat_id, __VERSION__);
      }
    }
    msg = msg->next;
  }
}

int main(int argc, char **argv) {
  assert(argc >= 2);
  curl_global_init(CURL_GLOBAL_DEFAULT);

  Bot bot = Bot_init(argv[1], handle_message);
  BotInfo info = Bot_getMe(&bot);

  printf("id: %lu\n", info.id);
  printf("is_bot: %s\n", info.is_bot ? "true" : "false");
  printf("first_name: %s\n", info.first_name);
  printf("username: %s\n", info.username);

  while (true)
    Bot_getUpdates(&bot);

  // Bot_deinit(&bot);
  // curl_global_cleanup();
  // return 0;
}

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
    } else if (chat_id && str_eql("text", msg->name->string)) {
      assert(msg->value->type == json_type_string);
      json_string_t *text = msg->value->payload;
      const char *txt = text->string;
      if (str_starts_with(txt, "/help"))
        Bot_sendTextMessage(bot, chat_id, "Commands:%0A/fart%0A/cc");
      else if (str_starts_with(txt, "/fart"))
        Bot_sendTextMessage(bot, chat_id, "farted!");
      else if (str_starts_with(txt, "/cc"))
        Bot_sendTextMessage(bot, chat_id, __VERSION__);
    } else if (chat_id && str_eql("new_chat_members", msg->name->string)) {
      assert(msg->value->type == json_type_array);
      json_array_t *members = msg->value->payload;
      json_array_element_t *it = members->start;
      while (it) {
        assert(it->value->type == json_type_object);
        json_object_t *member = it->value->payload;
        json_object_element_t *mem = member->start;
        while (mem) {
          if (str_eql("first_name", mem->name->string)) {
            assert(mem->value->type == json_type_string);
            json_string_t *first_name = mem->value->payload;
            char text[1024];
            str_cpy("Welcome ", text, 8);
            str_cpy(first_name->string, text + 8, first_name->string_size);
            text[8 + first_name->string_size] = 0;
            Bot_sendTextMessage(bot, chat_id, text);
          }
          mem = mem->next;
        }
        it = it->next;
      }
    } else if (chat_id && str_eql("left_chat_member", msg->name->string)) {
      assert(msg->value->type == json_type_object);
      json_object_t *member = msg->value->payload;
      json_object_element_t *mem = member->start;
      while (mem) {
        if (str_eql("first_name", mem->name->string)) {
          assert(mem->value->type == json_type_string);
          json_string_t *first_name = mem->value->payload;
          char text[1024];
          str_cpy("Bye ", text, 4);
          str_cpy(first_name->string, text + 4, first_name->string_size);
          text[4 + first_name->string_size] = 0;
          Bot_sendTextMessage(bot, chat_id, text);
        }
        mem = mem->next;
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

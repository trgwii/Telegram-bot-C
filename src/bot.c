#include "bot.h"
#include "str.h"
#include <assert.h>

static size_t Bot_curl_writefunc(char *ptr, size_t size, size_t nmemb,
                                 void *data) {
  Bot *bot = (Bot *)data;
  size_t size_in_bytes = size * nmemb;
  assert(size_in_bytes < 1048576 - (unsigned long)bot->data_offset);
  for (size_t i = 0; i < size_in_bytes; i++) {
    bot->data[bot->data_offset++] = ptr[i];
  }
  return 0;
}

static void *Bot_json_alloc_func(void *user_data, size_t size) {
  Bot *bot = user_data;
  if (size >= 1048576)
    return NULL;
  return &bot->json_scratch[0];
}

Bot Bot_init(char *token, void *handle_message_user_data,
             void (*handle_message)(void *, Bot *, json_object_t *)) {
  Bot bot;
  bot.curl = curl_easy_init();
  bot.token = token;
  bot.token_len = cstr_len(token);
  bot.last_update_id = 0;
  bot.handle_message_user_data = handle_message_user_data;
  bot.handle_message = handle_message;

  // Set up baseURL
  cstr_cpy("https://api.telegram.org/bot", bot.url, 28);
  cstr_cpy(bot.token, bot.url + 28, bot.token_len);
  bot.url_offset = 28 + bot.token_len + 1;
  cstr_cpy("/", bot.url + 28 + bot.token_len, 1);

  // Set up curl handlers
  curl_easy_setopt(bot.curl, CURLOPT_WRITEFUNCTION, Bot_curl_writefunc);
  return bot;
}

void Bot_deinit(Bot *bot) { curl_easy_cleanup(bot->curl); }

BotInfo Bot_getMe(Bot *bot) {
  cstr_cpy("getMe", bot->url + bot->url_offset, 6);
  curl_easy_setopt(bot->curl, CURLOPT_URL, bot->url);
  bot->data_offset = 0;
  curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, bot);
  curl_easy_perform(bot->curl);
  bot->data[bot->data_offset] = 0;
  BotInfo result = {
      .id = 0,
      .is_bot = false,
      .first_name = {0},
      .username = {0},
  };
  json_value_t *bot_info = json_parse(bot->data, bot->data_offset);
  assert(bot_info->type == json_type_object);
  json_object_t *bot_info_obj = bot_info->payload;
  json_object_element_t *el = bot_info_obj->start;
  while (el) {
    const char *el_str = el->name->string;
    if (cstr_eql("ok", el_str)) {
      assert(el->value->type == json_type_true);
    } else if (cstr_eql("result", el_str)) {
      assert(el->value->type == json_type_object);
      json_object_t *bot_info_result = el->value->payload;
      json_object_element_t *rel = bot_info_result->start;
      while (rel) {
        const char *rel_str = rel->name->string;
        if (cstr_eql("id", rel_str)) {
          assert(rel->value->type == json_type_number);
          json_number_t *num = rel->value->payload;
          result.id = strtoul(num->number, NULL, 10);
        } else if (cstr_eql("is_bot", rel_str)) {
          assert(rel->value->type == json_type_true ||
                 rel->value->type == json_type_false);
          result.is_bot = rel->value->type == json_type_true;
        } else if (cstr_eql("first_name", rel_str)) {
          assert(rel->value->type == json_type_string);
          json_string_t *first_name = rel->value->payload;
          cstr_cpy(first_name->string, result.first_name,
                   first_name->string_size);
          result.first_name[first_name->string_size] = 0;
        } else if (cstr_eql("username", rel_str)) {
          assert(rel->value->type == json_type_string);
          json_string_t *username = rel->value->payload;
          cstr_cpy(username->string, result.username, username->string_size);
          result.username[username->string_size] = 0;
        }
        rel = rel->next;
      }
    }
    el = el->next;
  }
  return result;
}

void Bot_sendTextMessage(Bot *bot, long long chat_id, const char *text) {
  cstr_cpy("sendMessage?chat_id=", bot->url + bot->url_offset, 20);
  unsigned long off =
      (unsigned long)snprintf(bot->url + bot->url_offset + 20,
                              1023 - bot->url_offset + 20, "%lld", chat_id);
  cstr_cpy("&text=", bot->url + bot->url_offset + 20 + off, 6);
  cstr_cpy(text, bot->url + bot->url_offset + 20 + off + 6, cstr_len(text));
  bot->url[bot->url_offset + 20 + off + 6 + cstr_len(text)] = 0;
  curl_easy_setopt(bot->curl, CURLOPT_URL, bot->url);
  bot->data_offset = 0;
  curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, bot);
  printf("GET %s\n", &bot->url[bot->url_offset]);
  curl_easy_perform(bot->curl);
  bot->data[bot->data_offset] = 0;
  printf("%s\n", bot->data);
}

void Bot_getUpdates(Bot *bot) {
  if (bot->last_update_id) {
    cstr_cpy("getUpdates?timeout=30&offset=", bot->url + bot->url_offset, 29);
    unsigned long off = (unsigned long)snprintf(bot->url + bot->url_offset + 29,
                                                1023 - (bot->url_offset + 29),
                                                "%lu", bot->last_update_id + 1);
    bot->url[bot->url_offset + 29 + off] = 0;
  } else {
    cstr_cpy("getUpdates?timeout=30", bot->url + bot->url_offset, 22);
    bot->url[bot->url_offset + 22] = 0;
  }
  printf("GET %s\n", &bot->url[bot->url_offset]);
  curl_easy_setopt(bot->curl, CURLOPT_URL, bot->url);
  bot->data_offset = 0;
  curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, bot);
  curl_easy_perform(bot->curl);
  bot->data[bot->data_offset] = 0;
  json_value_t *updates =
      json_parse_ex(bot->data, bot->data_offset, json_parse_flags_default,
                    Bot_json_alloc_func, bot, NULL);
  // json_value_t *updates = json_parse(bot->data, bot->data_offset);
  if (!updates)
    return;
  if (updates->type != json_type_object)
    return;
  json_object_t *updates_obj = updates->payload;
  json_object_element_t *el = updates_obj->start;
  while (el) {
    const char *el_str = el->name->string;
    if (cstr_eql("ok", el_str)) {
      if (el->value->type != json_type_true)
        return;
    } else if (cstr_eql("result", el_str)) {
      if (el->value->type != json_type_array)
        return;
      json_array_t *arr = el->value->payload;
      json_array_element_t *it = arr->start;
      while (it) {
        if (it->value->type != json_type_object)
          return;
        json_object_t *update = it->value->payload;
        json_object_element_t *kv = update->start;
        while (kv) {
          if (cstr_eql("update_id", kv->name->string)) {
            if (kv->value->type != json_type_number)
              return;
            json_number_t *num = kv->value->payload;
            bot->last_update_id = strtoul(num->number, NULL, 10);
          } else if (cstr_eql("message", kv->name->string)) {
            if (kv->value->type != json_type_object)
              return;
            json_object_t *message = kv->value->payload;
            bot->handle_message(bot->handle_message_user_data, bot, message);
          }
          kv = kv->next;
        }
        it = it->next;
      }
    }
    el = el->next;
  }
}

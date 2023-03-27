#include "bot.h"
#include "json_aux.h"
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
  SB url = SB_fromArray(bot.url);
  SB_append(&url, "https://api.telegram.org/bot");
  SB_appendLen(&url, bot.token, bot.token_len);
  SB_append(&url, "/");
  bot.url_offset = url.len;

  // Set up curl handlers
  curl_easy_setopt(bot.curl, CURLOPT_WRITEFUNCTION, Bot_curl_writefunc);
  return bot;
}

void Bot_deinit(Bot *bot) { curl_easy_cleanup(bot->curl); }

BotInfo Bot_getMe(Bot *bot) {
  SB url = SB_fromPtrLenCap(bot->url, bot->url_offset, sizeof(bot->url));
  SB_append(&url, "getMe");
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
  iterate_object(bot_info, el) {
    const char *el_str = el->name->string;
    if (cstr_eql("ok", el_str)) {
      assert(el->value->type == json_type_true);
    } else if (cstr_eql("result", el_str)) {
      iterate_object(el->value, rel) {
        const char *rel_str = rel->name->string;
        if (cstr_eql("id", rel_str)) {
          result.id = get_ulong(rel->value);
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
      }
    }
  }
  return result;
}

void Bot_sendTextMessage(Bot *bot, long long chat_id, const char *text) {
  Bot_sendTextMessageLen(bot, chat_id, text, cstr_len(text));
}

void Bot_sendTextMessageLen(Bot *bot, long long chat_id, const char *text,
                            size_t text_len) {
  SB url = SB_fromPtrLenCap(bot->url, bot->url_offset, sizeof(bot->url));
  SB_append(&url, "sendMessage?chat_id=");
  url.len +=
      (size_t)snprintf(url.ptr + url.len, url.cap - url.len, "%lld", chat_id);
  SB_append(&url, "&text=");
  SB_appendLen(&url, text, text_len);
  curl_easy_setopt(bot->curl, CURLOPT_URL, bot->url);
  bot->data_offset = 0;
  curl_easy_setopt(bot->curl, CURLOPT_WRITEDATA, bot);
  printf("GET %s\n", &bot->url[bot->url_offset]);
  curl_easy_perform(bot->curl);
  bot->data[bot->data_offset] = 0;
  printf("%s\n", bot->data);
}

void Bot_getUpdates(Bot *bot) {
  SB url = SB_fromPtrLenCap(bot->url, bot->url_offset, sizeof(bot->url));
  if (bot->last_update_id) {
    SB_append(&url, "getUpdates?timeout=30&offset=");
    url.len += (size_t)snprintf(url.ptr + url.len, url.cap - url.len, "%lu",
                                bot->last_update_id + 1);
  } else {
    SB_append(&url, "getUpdates?timeout=30");
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
  iterate_object(updates, el) {
    const char *el_str = el->name->string;
    if (cstr_eql("ok", el_str)) {
      if (el->value->type != json_type_true)
        return;
    } else if (cstr_eql("result", el_str)) {
      if (el->value->type != json_type_array)
        return;
      iterate_array(el->value, it) {
        if (it->value->type != json_type_object)
          return;
        iterate_object(it->value, kv) {
          if (cstr_eql("update_id", kv->name->string)) {
            if (kv->value->type != json_type_number)
              return;
            bot->last_update_id = get_ulong(kv->value);
          } else if (cstr_eql("message", kv->name->string)) {
            if (kv->value->type != json_type_object)
              return;
            json_object_t *message = kv->value->payload;
            bot->handle_message(bot->handle_message_user_data, bot, message);
          }
        }
      }
    }
  }
}

#include "bot.h"
#include "json_aux.h"
#include "str.h"
#include <assert.h>
#include <curl/curl.h>
#include <sqlite3.h>

static void handle_message(void *user_data, Bot *bot, json_object_t *message) {
  sqlite3 *db = user_data;
  long long chat_id = 0;
  long long user_id = 0;
  long long reply_to_user_id = 0;
  const char *first_name = NULL;
  const char *reply_to_first_name = NULL;
  const char *chat_title = NULL;
  for (json_object_element_t *msg = message->start; msg; msg = msg->next) {
    if (cstr_eql("chat", msg->name->string)) {
      iterate_object(msg->value, chat) {
        if (cstr_eql("id", chat->name->string)) {
          chat_id = get_long(chat->value);
        } else if (cstr_eql("title", chat->name->string)) {
          chat_title = get_string(chat->value);
        }
      }
    } else if (cstr_eql("from", msg->name->string)) {
      iterate_object(msg->value, from) {
        if (cstr_eql("id", from->name->string)) {
          user_id = get_long(from->value);
        } else if (cstr_eql("first_name", from->name->string)) {
          first_name = get_string(from->value);
        }
      }
    } else if (cstr_eql("reply_to_message", msg->name->string)) {
      iterate_object(msg->value, rtm) {
        if (cstr_eql("from", rtm->name->string)) {
          iterate_object(rtm->value, rtm_from) {
            if (cstr_eql("id", rtm_from->name->string)) {
              reply_to_user_id = get_long(rtm_from->value);
            }
            if (cstr_eql("first_name", rtm_from->name->string)) {
              reply_to_first_name = get_string(rtm_from->value);
            }
          }
        }
      }
    } else if (chat_id && cstr_eql("text", msg->name->string)) {
      const char *txt = get_string(msg->value);
      if (cstr_starts_with(txt, "/help"))
        Bot_sendTextMessage(bot, chat_id, "Commands:%0A/fart%0A/cc%0A/points");
      else if (cstr_starts_with(txt, "/fart"))
        Bot_sendTextMessage(bot, chat_id, "farted! 🗿");
      else if (cstr_starts_with(txt, "/cc"))
        Bot_sendTextMessage(bot, chat_id, __VERSION__);
      else if (cstr_starts_with(txt, "/points")) {
        char pointsMsgData[1024];
        SB pmsg = SB_fromArray(pointsMsgData);
        SB_append(&pmsg, "Rep points for ");
        const char *rep_points_first_name = first_name;
        long long rep_points_user_id = user_id;
        if (reply_to_user_id) {
          rep_points_user_id = reply_to_user_id;
          rep_points_first_name = reply_to_first_name;
        }
        SB_append(&pmsg, (char *)(uintptr_t)rep_points_first_name);
        if (chat_title) {
          SB_append(&pmsg, " in ");
          SB_append(&pmsg, (char *)(uintptr_t)chat_title);
        }
        SB_append(&pmsg, ": ");
        sqlite3_stmt *stmt;
        assert(sqlite3_prepare(db,
                               "SELECT points FROM rep WHERE chat_id = ? "
                               "AND user_id = ? LIMIT 1",
                               65, &stmt, NULL) == SQLITE_OK);
        assert(sqlite3_bind_int64(stmt, 1, chat_id) == SQLITE_OK);
        assert(sqlite3_bind_int64(stmt, 2, rep_points_user_id) == SQLITE_OK);
        do {
          int result = sqlite3_step(stmt);
          assert(result != SQLITE_ERROR);
          assert(result != SQLITE_MISUSE);
          if (result == SQLITE_ROW) {
            long long points = sqlite3_column_int64(stmt, 0);

            pmsg.len +=
                (size_t)snprintf(pmsg.ptr + pmsg.len, 100, "%lld", points);
            Bot_sendTextMessageLen(bot, chat_id, pmsg.ptr, pmsg.len);
            break;
          } else if (result == SQLITE_DONE) {
            break;
          }
        } while (true);
        sqlite3_finalize(stmt);
      } else if (chat_id && user_id && reply_to_user_id &&
                 user_id != reply_to_user_id &&
                 (cstr_eql("++", txt) || cstr_eql("+1", txt) ||
                  cstr_eql("👍", txt))) {
        sqlite3_stmt *stmt;
        assert(sqlite3_prepare(
                   db,
                   "UPDATE OR IGNORE rep SET points = points + 1 WHERE "
                   "chat_id = ? AND user_id = ?",
                   79, &stmt, NULL) == SQLITE_OK);
        assert(sqlite3_bind_int64(stmt, 1, chat_id) == SQLITE_OK);
        assert(sqlite3_bind_int64(stmt, 2, reply_to_user_id) == SQLITE_OK);
        int result;
        do {
          result = sqlite3_step(stmt);
        } while (result != SQLITE_ERROR && result != SQLITE_MISUSE &&
                 result != SQLITE_DONE);
        assert(result == SQLITE_DONE);
        sqlite3_finalize(stmt);
      }
    } else if (chat_id && cstr_eql("new_chat_members", msg->name->string)) {
      iterate_array(msg->value, it) {
        iterate_object(it->value, mem) {
          if (cstr_eql("first_name", mem->name->string)) {
            assert(mem->value->type == json_type_string);
            json_string_t *first_name_str = mem->value->payload;
            char text[1024];
            SB b = SB_fromArray(text);
            SB_append(&b, "Welcome ");
            SB_appendLen(&b, first_name_str->string,
                         first_name_str->string_size);
            Bot_sendTextMessageLen(bot, chat_id, b.ptr, b.len);
          }
        }
      }
    } else if (chat_id && cstr_eql("left_chat_member", msg->name->string)) {
      iterate_object(msg->value, mem) {
        if (cstr_eql("first_name", mem->name->string)) {
          assert(mem->value->type == json_type_string);
          json_string_t *first_name_str = mem->value->payload;
          char text[1024];
          SB b = SB_fromArray(text);
          SB_append(&b, "Bye ");
          SB_appendLen(&b, first_name_str->string, first_name_str->string_size);
          Bot_sendTextMessageLen(bot, chat_id, b.ptr, b.len);
        }
      }
    }
  }
  if (chat_id && user_id) {
    sqlite3_stmt *stmt;
    assert(sqlite3_prepare(db,
                           "INSERT OR IGNORE INTO rep (chat_id, user_id, "
                           "points) VALUES (?, ?, 0)",
                           70, &stmt, NULL) == SQLITE_OK);
    assert(sqlite3_bind_int64(stmt, 1, chat_id) == SQLITE_OK);
    assert(sqlite3_bind_int64(stmt, 2, user_id) == SQLITE_OK);
    do {
      int result = sqlite3_step(stmt);
      assert(result != SQLITE_ERROR);
      assert(result != SQLITE_MISUSE);
      if (result == SQLITE_DONE)
        break;
    } while (true);
    sqlite3_finalize(stmt);
  }
}

int main(int argc, char **argv) {
  assert(argc >= 2);
  curl_global_init(CURL_GLOBAL_DEFAULT);

  sqlite3 *db;
  assert(sqlite3_open("bot.db", &db) == SQLITE_OK);

  char *errmsg;
  if (sqlite3_exec(db,
                   "CREATE TABLE IF NOT EXISTS rep (chat_id INT, user_id INT, "
                   "points INT, UNIQUE (chat_id, user_id))",
                   NULL, NULL, &errmsg) != SQLITE_OK) {
    printf("SQLite error: %s\n", errmsg);
    sqlite3_free(errmsg);
    return 1;
  }
  sqlite3_free(errmsg);

  Bot *bot = malloc(sizeof(Bot));
  *bot = Bot_init(argv[1], db, handle_message);
  BotInfo info = Bot_getMe(bot);

  printf("id: %lu\n", info.id);
  printf("is_bot: %s\n", info.is_bot ? "true" : "false");
  printf("first_name: %s\n", info.first_name);
  printf("username: %s\n", info.username);

  char forever = 1;
  while (forever)
    Bot_getUpdates(bot);

  Bot_deinit(bot);
  free(bot);

  sqlite3_close(db);
  curl_global_cleanup();
  return 0;
}

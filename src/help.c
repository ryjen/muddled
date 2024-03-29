
/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *                 Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#include "help.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#include "engine.h"
#include "log.h"
#include "macro.h"
#include "str.h"

const char *greeting = 0;

Help *first_help = 0;

const Lookup help_categories[] = {
    {"unknown", HELP_UNKNOWN}, {"information", HELP_INFO}, {"communication", HELP_COMMUNICATION}, {0, 0}};

Help *new_help() {
  Help *help = (Help *)alloc_mem(1, sizeof(Help));
  help->keywords = str_empty;
  help->syntax = str_empty;
  help->text = str_empty;
  help->related = 0;
  help->category = HELP_UNKNOWN;
  return help;
}

void destroy_help(Help *help) {
  free_str(help->keywords);
  free_str(help->text);
  free_str(help->syntax);
  free_mem(help);
}

int load_help_column(Help *help, sql_stmt *stmt, const char *colname, int i) {
  if (!str_cmp(colname, "keywords")) {
    help->keywords = str_dup(sql_column_str(stmt, i));
    return 1;
  }

  else if (!str_cmp(colname, "syntax")) {
    help->syntax = str_dup(sql_column_str(stmt, i));
    return 1;
  }

  else if (!str_cmp(colname, "text")) {
    help->text = str_dup(sql_column_str(stmt, i));
    return 1;
  }

  else if (!str_cmp(colname, "helpId")) {
    help->id = sql_column_int64(stmt, i);
    return 1;
  }

  else if (!str_cmp(colname, "category")) {
    help->category = sql_column_int(stmt, i);
    return 1;
  }

  else {
    log_warn("unknown help column '%s'", colname);
  }
  return 0;
}

int load_related_helps() {
  for (Help *help = first_help; help; help = help->next) {
    char buf[400];
    sql_stmt *stmt;
    int len = sprintf(buf, "select * from help_related where helpId=%" PRId64, help->id);

    if (sql_query(buf, len, &stmt) != SQL_OK) {
      log_data("could not prepare statement");
      return 0;
    }

    while (sql_step(stmt) != SQL_DONE) {
      int related = sql_column_int(stmt, 1);

      for (Help *rel = first_help; help; help = help->next) {
        if (rel->id == related) {
          LINK(help->related, rel, next_related);
          break;
        }
      }
    }

    if (sql_finalize(stmt) != SQL_OK) {
      log_data("could not finalize statement");
    }
  }
  return 1;
}

int load_helps() {
  char buf[400];
  sql_stmt *stmt;
  int total = 0;
  int len = sprintf(buf, "select * from help");

  if (sql_query(buf, len, &stmt) != SQL_OK) {
    log_data("could not prepare statement");
    return 0;
  }

  while (sql_step(stmt) != SQL_DONE) {
    int count = sql_column_count(stmt);
    Help *help = new_help();

    for (int i = 0; i < count; i++) {
      const char *colname = sql_column_name(stmt, i);
      load_help_column(help, stmt, colname, i);
    }

    if (is_name("greeting", help->keywords)) {
      greeting = help->text;
    }
    LINK(first_help, help, next);
    total++;
  }

  if (sql_finalize(stmt) != SQL_OK) {
    log_data("could not finalize statement");
  }
  load_related_helps();
  return total;
}

int save_help(Help *help) {
  field_map help_values[] = {{"keywords", &help->keywords, SQL_TEXT},
                             {"text", &help->text, SQL_TEXT},
                             {"related", &help->related, SQL_TEXT},
                             {"category", &help->category, SQL_INT},
                             {0}};

  if (help->id == 0) {
    if (sql_insert_query(help_values, "help") != SQL_OK) {
      log_data("could not insert help");
      return 0;
    }
    help->id = db_last_insert_rowid();
  }

  else {
    if (sql_update_query(help_values, "help", help->id) != SQL_OK) {
      log_data("could not update help");
      return 0;
    }
  }
  return 1;
}

Help *help_find(const char *keyword) {
  int id = atoi(keyword);

  for (Help *help = first_help; help != 0; help = help->next) {
    if ((id != 0 && id == help->id) || is_name(keyword, help->keywords)) {
      return help;
    }
  }
  return 0;
}

Help *help_match(const char *keyword) {
  for (Help *help = first_help; help != 0; help = help->next) {
    char arg[BUF_SIZ];
    const char *key = one_argument(help->keywords, arg);

    do {
      if (match(arg, keyword)) {
        return help;
      }
      key = one_argument(key, arg);
    }

    while (!nullstr(key));
  }
  return 0;
}

const char *help_related_string(Help *help) {
  static char buf[BUF_SIZ] = {0};

  for (Help *rel = help->related; rel; rel = rel->next_related) {
    strcat(buf, ", ");
    strcat(buf, rel->keywords);
  }
  return &buf[2];
}

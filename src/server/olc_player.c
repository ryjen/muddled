
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
#include <stdlib.h>
#include "area.h"
#include "character.h"
#include "client.h"
#include "engine.h"
#include "flag.h"
#include "lookup.h"
#include "macro.h"
#include "olc.h"
#include "player.h"
#include "private.h"
#include "room.h"
#include "str.h"
#include "telnet.h"
#include "util.h"

Editor *build_player_editor(Character *player) {
  Editor *editor = new_editor();
  editor->data = player;
  editor->edit = player_editor;
  editor->show = player_editor_menu;
  return editor;
}

void player_editor_menu(Client *conn) {
  clear_screen(conn);
  set_cursor(conn, 1, 1);
  Character *player = (Character *)conn->editing->data;
  conn->titlef(conn, "Player Editor - player %d", player->id);
  character_editor_menu(conn, player);
  xwritelnf(conn, "~YG) ~CTitle: ~W%s~x", player->pc->title);
  xwritelnf(conn, "~YH) ~CFlags: ~W%s~x",

            format_flags(player->flags, plr_flags));
  xwritelnf(conn, "~YI) ~CPrompt: ~W%s~x", player->pc->prompt);
  xwritelnf(conn, "~YJ) ~CAlignment: ~W%d~x", player->alignment);
  xwritelnf(conn, "~YK) ~CStats: ~WStr[%d] Int[%d] Wis[%d] Dex[%d] Con[%d] Luck[%d]~x", getCurrStat(player, STAT_STR),
            getCurrStat(player, STAT_INT), getCurrStat(player, STAT_WIS), getCurrStat(player, STAT_DEX),
            getCurrStat(player, STAT_CON), getCurrStat(player, STAT_LUCK));
}

void player_edit_list(Client *conn) {
  int count = 0;

  for (Character *vch = first_player; vch != 0; vch = vch->next_player) {
    xwritelnf(conn, "%2d) %-12.12s ", vch->id, vch->name);

    if (++count % 4 == 0) {
      xwriteln(conn, "");
    }
  }

  if (count % 4 != 0) {
    xwriteln(conn, "");
  }
}

void player_editor(Client *conn, const char *argument) {
  char arg[100];
  argument = one_argument(argument, arg);

  if (!str_prefix(arg, "show")) {
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "list")) {
    player_edit_list(conn);
    return;
  }
  Character *player = (Character *)conn->editing->data;

  if (!str_cmp(arg, "Q")) {
    if (player->pc->conn == 0) {
      destroy_char(player);
    }
    finish_editing(conn);
    return;
  }

  if (!str_cmp(arg, "save")) {
    save_player(player);
    xwriteln(conn, "~CPlayer saved.~x");
    return;
  }

  if (character_editor(conn, player, arg, argument)) {
    return;
  }

  if (!str_cmp(arg, "G") || !str_cmp(arg, "title")) {
    if (!argument || !*argument) {
      xwriteln(conn, "~CYou must provide a title.~x");
      return;
    }
    free_str_dup(&player->pc->title, argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "H") || !str_cmp(arg, "flags")) {
    if (edit_flag("flags", conn, player->flags, argument, plr_flags)) {
      conn->editing->show(conn);
    }
    return;
  }

  if (!str_cmp(arg, "I") || !str_cmp(arg, "prompt")) {
    free_str_dup(&player->pc->prompt, argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "J") || !str_cmp(arg, "alignment")) {
    int a = atoi(argument);

    if (a < -MAX_ALIGN || a > MAX_ALIGN) {
      xwritelnf(conn, "~CValue must be between %d and %d.~x", -MAX_ALIGN, MAX_ALIGN);
      return;
    }
    player->alignment = a;
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "K") || !str_cmp(arg, "stats")) {
    char type[BUF_SIZ];
    argument = one_argument(argument, type);
    long v = value_lookup(stat_table, type);

    if (v == -1) {
      xwritelnf(conn, "~CValid stats are: %s~x", lookup_names(stat_table));
      return;
    }
    int a = atoi(argument);

    if (a > getMaxTrain(player)) {
      xwritelnf(conn, "Stat cannot be higher than %d for %s.", getMaxTrain(player), player->name);
      return;
    }
    player->stats[v] = a;
    conn->editing->show(conn);
    return;
  }
}

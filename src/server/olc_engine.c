
/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> (www.arg3.net)            *
 *             Many thanks to creators of muds before me.                     *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  We hope that you share your changes too.  What goes      *
 *                            around, comes around.                           *
 ******************************************************************************/
#include "client.h"
#include "engine.h"
#include "log.h"
#include "lookup.h"
#include "macro.h"
#include "olc.h"
#include "str.h"
#include "telnet.h"

Editor *build_engine_editor(Engine *eng) {
  Editor *editor = new_editor();
  editor->data = eng;
  editor->edit = engine_editor;
  editor->show = engine_editor_menu;
  return editor;
}

void engine_editor_menu(Client *conn) {
  clear_screen(conn);
  set_cursor(conn, 1, 1);
  conn->title(conn, "Engine Editor");
  Engine *engine = (Engine *)conn->editing->data;
  xwritelnf(conn, "   ~CId: ~W%d", engine->id);
  xwritelnf(conn, "~YA) ~CName: ~W%s~x", engine->name);
  xwritelnf(conn, "~YB) ~CFlags: ~W%s~x",

            format_flags(engine->flags, engine_flags));
}

void engine_editor(Client *conn, const char *argument) {
  char arg[100];
  argument = one_argument(argument, arg);

  if (!str_cmp(arg, "Q")) {
    finish_editing(conn);
    return;
  }

  if (!str_prefix(arg, "show")) {
    conn->editing->show(conn);
    return;
  }
  Engine *engine = (Engine *)conn->editing->data;

  if (!str_cmp(arg, "save")) {
    save_engine();
    xwriteln(conn, "~CEngine saved.~x");
    return;
  }

  if (!str_cmp(arg, "A") || !str_cmp(arg, "name")) {
    if (!argument || !*argument) {
      xwriteln(conn, "~CChange name to what?~x");
      return;
    }
    free_str(engine->name);
    engine->name = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "B") || !str_cmp(arg, "flags")) {
    if (edit_flag("flags", conn, engine->flags, argument, engine_flags)) {
      conn->editing->show(conn);
    }
    return;
  }
}

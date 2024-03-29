
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
#include <ctype.h>
#include "account.h"
#include "channel.h"
#include "character.h"
#include "client.h"
#include "command.h"
#include "connection.h"
#include "engine.h"
#include "forum.h"
#include "grid.h"
#include "olc.h"
#include "player.h"
#include "private.h"
#include "room.h"
#include "str.h"
#include "util.h"

DOFUN(say) {
  if (!argument || !*argument) {
    xwriteln(ch, "Say what?");
    return;
  }
  act(TO_CHAR, ch, argument, say_verb(argument, ch, 0, 0), "~gYou $T '~G$t~g'~x");
  act(TO_ROOM, ch, argument, say_verb(argument, ch, 0, 1), "~g$n $T '~G$t~g'~x");
}

DOFUN(chat) { interpret_channel(ch, gcn_chat, argument); }

DOFUN(admin) { interpret_channel(ch, gcn_admin, argument); }

static void connection_note_text_prompt(Client *conn) {
  xwriteln(conn, "");
  xwriteln(conn, "Enter text. Type ~W\\h~x for help or ~W\\q~x to end note.");
  xwriteln(conn, "~Y=====================================================~x");
}

static const char *szFinishPrompt = "\n\r(~WC~x)ontinue, (~WV~x)iew, (~WP~x)ost or (~WF~x)orget it?";

void connection_note_text(Client *, const char *);

void connection_note_finish(Client *conn, const char *argument) {
  switch (toupper((int)argument[0])) {
    case 'C':
      xwriteln(conn, "Continuing note...");
      xwriteln(conn, conn->account->inProgress->text);
      conn->handler = connection_note_text;
      break;
    case 'V':
      xwriteln(conn, "Text of your note so far:");

      if (!nullstr(conn->account->inProgress->text)) {
        xwriteln(conn, conn->account->inProgress->text);
      }

      else {
        xwriteln(conn, "You haven't written a thing!");
      }
      xwrite(conn, szFinishPrompt);
      break;
    case 'P':
      announce(conn->account->playing, INFO_NOTE, "New note on $n forum from $n. Subj: %s",
               conn->account->inProgress->subject);
      finish_note(conn->account->forum, conn->account->inProgress);
      xwriteln(conn, "Note posted.");
      act(TO_ROOM, conn->account->playing, 0, 0, "~G$n finishes $s note.~x");
      conn->handler = client_command_parser;
      break;
    case 'F':
      xwriteln(conn, "Note cancelled!");
      conn->handler = client_command_parser;
      act(TO_ROOM, conn->account->playing, 0, 0, "~G$n cancels $s note.~x");
      break;
    default: /* invalid response */
      xwriteln(conn, "Huh? Valid answers are:");
      xwrite(conn, szFinishPrompt);
  }
}

void connection_note_text(Client *conn, const char *argument) {
  int action = edit_text((Client *)conn, &conn->account->inProgress->text, argument);

  switch (action) {
    case EDIT_END:
      xwriteln(conn, "");
      xwrite(conn, szFinishPrompt);
      conn->handler = connection_note_finish;
      return;
    case EDIT_CHANGED:
      xwriteln(conn, conn->account->inProgress->text);
      return;
    default:
    case EDIT_NOCHANGE:
      return;
  }
}

void connection_note_expire(Client *conn, const char *argument) {
  time_t expire;
  int days;

  if (nullstr(argument)) {
    days = conn->account->forum->purgeDays;
  }

  else if (!is_number(argument)) {
    xwriteln(conn, "Write the number of days!");
    xwrite(conn, "~YExpire~w:  ");
    return;
  }

  else {
    days = atoi(argument);

    if (days <= 0) {
      xwriteln(conn, "This is a positive MUD. Use positive numbers only! :)");
      xwrite(conn, "~YExpire~w:  ");
      return;
    }
  }

  if (days == -1) {
    days = 365 * 5;
  }
  expire = current_time + (days * 24 * 60 * 60);
  conn->account->inProgress->expire = expire;
  connection_note_text_prompt(conn);
  conn->handler = connection_note_text;
}

void connection_note_subject(Client *conn, const char *argument) {
  char buf[BUF_SIZ];
  strcpy(buf, argument);

  if (!buf[0]) {
    xwriteln(conn, "Please find a meaningful subject!");
    xwrite(conn, "~YSubject~x: ");
  }

  else if (strlen(buf) > 60) {
    xwriteln(conn, "No, no. This is just the Subject. You're note writing the note yet. Twit.");
  }

  else {
    conn->account->inProgress->subject = str_dup(buf);

    if (is_immortal(conn->account->playing)) {
      xwriteln(conn, "");
      xwriteln(conn, "How many days do you want this note to expire in?");
      xwritelnf(conn, "Press Enter for default value for this forum, ~W%d~x days.", conn->account->forum->purgeDays);
      xwrite(conn, "~YExpire~x:  ");
      conn->handler = connection_note_expire;
    }

    else {
      conn->account->inProgress->expire = current_time + (conn->account->forum->purgeDays * 24 * 60 * 60);
      xwritelnf(conn, "This note will expire %s",
                str_time(conn->account->inProgress->expire, conn->account->timezone, 0));
      connection_note_text_prompt(conn);
      conn->handler = connection_note_text;
    }
  }
}

void connection_note_to(Client *conn, const char *argument) {
  char buf[BUF_SIZ];
  strcpy(buf, argument);

  switch (conn->account->forum->type) {
    default:
    case DEF_NORMAL:

      if (!buf[0]) {
        conn->account->inProgress->toList = str_dup(conn->account->forum->defaultTo);
        xwritelnf(conn, "Assumed default recipient: ~W%s~x", conn->account->inProgress->toList);
      }

      else {
        conn->account->inProgress->toList = str_dup(buf);
      }
      break;
    case DEF_INCLUDE:

      if (!is_exact_name(conn->account->forum->defaultTo, buf)) {
        strcat(buf, " ");
        strcat(buf, conn->account->forum->defaultTo);
        conn->account->inProgress->toList = str_dup(buf);
        xwriteln(conn, "");
        xwritelnf(conn, "You did not specify %s as recipient, so it was automatically added.",
                  conn->account->forum->defaultTo);
        xwritelnf(conn, "~YNew To~x :  %s", conn->account->inProgress->toList);
      }

      else {
        conn->account->inProgress->toList = str_dup(buf);
      }
      break;
    case DEF_EXCLUDE:

      if (is_exact_name(conn->account->forum->defaultTo, buf)) {
        xwritelnf(conn, "You are not allowed to send notes to %s on this forum. Try again.",
                  conn->account->forum->defaultTo);
        xwrite(conn, "~YTo~x:      ");
        return;
      }

      else {
        conn->account->inProgress->toList = str_dup(buf);
      }
      break;
  }
  xwriteln(conn, "");
  xwrite(conn, "~YSubject~x: ");
  conn->handler = connection_note_subject;
}

void note_write(Character *ch, const char *argument) {
  if (!ch->pc) {
    return;
  }

  if (ch->level < 2) {
    xwriteln(ch, "You can't seem to write a note.");
    return;
  }
  Client *conn = (Client *)ch->pc->conn;

  if (ch->level < conn->account->forum->writeLevel || conn->account->forum->type == DEF_READONLY) {
    xwriteln(ch, "You cannot post notes on this forum.");
    return;
  }

  if (conn->account->inProgress && nullstr(conn->account->inProgress->text)) {
    xwriteln(ch,
             "Note in progress cancelled because you did not manage to"
             " write any text before losing link.");
    destroy_note(conn->account->inProgress);
    conn->account->inProgress = NULL;
  }

  if (!conn->account->inProgress) {
    conn->account->inProgress = new_note();
    conn->account->inProgress->from = str_dup(ch->name);
  }
  free_str_dup(&conn->account->inProgress->from, ch->name);
  conn->account->inProgress->date = current_time;
  act(TO_ROOM, ch, 0, 0, "~G$n starts writing a note.~x");
  /*
   * Begin writing the note !
   */
  xwritelnf(ch, "You are now %s a new note on the ~W%s~x forum.",
            !nullstr(conn->account->inProgress->text) ? "continuing" : "posting", ch->pc->conn->account->forum->name);
  xwriteln(ch, "Remember to turn off any client alias expansion!");
  xwritelnf(ch, "~YFrom~x:    %s", ch->name);
  xwriteln(ch, "");

  if (nullstr(conn->account->inProgress->text)) {
    switch (ch->pc->conn->account->forum->type) {
      default:
      case DEF_NORMAL:
        xwritelnf(ch, "If you press Return, default recipient \"~x%s~x\" will be chosen.",
                  ch->pc->conn->account->forum->defaultTo);
        break;
      case DEF_INCLUDE:
        xwritelnf(ch, "The recipient list MUST include \"~x%s~x\". If not, it will be added automatically.",
                  ch->pc->conn->account->forum->defaultTo);
        break;
      case DEF_EXCLUDE:
        xwritelnf(ch, "The recipient of this note must NOT include: \"~C%s~x\".",
                  ch->pc->conn->account->forum->defaultTo);
        break;
    }
    xwriteln(ch, "");
    xwrite(ch, "~YTo~x:      ");
    conn->handler = connection_note_to;
  }

  else {
    xwriteln(ch, "");
    xwritelnf(ch, "~YTo~x: %s", conn->account->inProgress->toList);
    xwritelnf(ch, "~YExpires~x: %s",
              (ch->pc->conn->account->forum->purgeDays == -1)
                  ? "Never"
                  : str_time(conn->account->inProgress->expire, ch->pc->conn->account->timezone, 0));
    xwritelnf(ch, "~YSubject~x: %s", conn->account->inProgress->subject);
    xwriteln(ch, "~GYou are note so far:~x");
    xwriteln(ch, conn->account->inProgress->text);
    connection_note_text_prompt(conn);
    conn->handler = connection_note_text;
  }
}

void note_read(Character *ch, const char *argument) {
  Note *p;
  int count = 0, number;
  time_t last_note = account_forum_last_note(ch->pc->account);

  if (!str_cmp(argument, "again")) {
    count = 1;
    Note *lastRead = 0;

    for (p = ch->pc->conn->account->forum->notes; p; p = p->next, count++) {
      if (p->date > last_note) {
        break;
      }

      if (!is_note_to(ch, p)) {
        continue;
      }
      lastRead = p;
    }

    if (lastRead == 0) {
      xwriteln(ch, "No note found.");
    }

    else {
      show_note_to_char(ch, lastRead, count);
    }
  }

  else if (is_number(argument)) {
    number = atoi(argument);

    for (p = ch->pc->conn->account->forum->notes; p; p = p->next)

      if (++count == number) {
        break;
      }

    if (p == 0 || !is_note_to(ch, p)) {
      xwriteln(ch, "No such note.");
    }

    else {
      show_note_to_char(ch, p, count);
      last_note = UMAX(last_note, p->date);
    }
  }

  else {
    count = 1;

    for (p = ch->pc->conn->account->forum->notes; p; p = p->next, count++) {
      if ((p->date > last_note) && is_note_to(ch, p)) {
        show_note_to_char(ch, p, count);
        last_note = UMAX(last_note, p->date);
        return;
      }
    }
    xwriteln(ch, "No new notes in this forum.");
    next_forum(ch);
    xwritelnf(ch, "Changed to next subscribed forum, %s.", ch->pc->conn->account->forum->name);
  }
}

void note_remove(Character *ch, const char *argument) {
  if (!str_cmp(argument, "all") && is_immortal(ch)) {
    for (Note *p_next, *p = ch->pc->account->forum->notes; p; p = p_next) {
      p_next = p->next;

      if (str_cmp(ch->name, p->from)) {
        continue;
      }
      delete_note(ch->pc->account->forum, p);
    }
    xwriteln(ch, "ALL Notes removed!");
  }

  else {
    Note *p;

    if (!is_number(argument)) {
      xwriteln(ch, "Remove which note?");
      return;
    }
    p = find_note(ch, ch->pc->account->forum, atoi(argument));

    if (!p) {
      xwriteln(ch, "No such note.");
      return;
    }

    if (str_cmp(ch->name, p->from) && !is_immortal(ch)) {
      xwriteln(ch, "You are not authorized to remove this note.");
      return;
    }
    delete_note(ch->pc->account->forum, p);
    xwriteln(ch, "Note removed!");
  }
  ch->pc->account->forum->flags |= FORUM_CHANGED;
}

void note_list(Character *ch, const char *argument) {
  int count = 0, show = 0, num = 0, has_shown = 0;
  time_t last_note;
  Note *p;
  Grid *grid = new_grid(scrwidth(ch), 2);

  if (is_number(argument)) {
    show = atoi(argument);

    for (p = ch->pc->account->forum->notes; p; p = p->next)

      if (is_note_to(ch, p)) {
        count++;
      }
  }
  xwritelnf(ch, "~WNotes on the %s forum:~x", ch->pc->account->forum->name);
  grid_addf(grid, ALIGN_LEFT, 1, 0, 0, 0, 0, "~rNum> %-12s Subject", "Author");
  grid_addf(grid, ALIGN_LEFT, 1, 0, 0, 0, 0, "~rNum> %-12s Subject", "Author");
  last_note = account_forum_last_note(ch->pc->account);

  for (p = ch->pc->account->forum->notes; p; p = p->next) {
    num++;

    if (is_note_to(ch, p)) {
      has_shown++;

      if (!show || ((count - show) < has_shown)) {
        grid_addf(grid, ALIGN_LEFT, 1, 0, 0, 0, 0, "~W%3d~x>~B%c~Y%-12.12s~Y %s~x ", num,
                  last_note < p->date ? '*' : ' ', p->from, p->subject);
      }
    }
  }

  if (has_shown == 0) {
    grid_add(grid, ALIGN_LEFT, 2, 0, 0, 0, 0, "~rNo notes on forum.~x");
  }
  grid_print(grid, ch);
  destroy_grid(grid);
}

void note_catchup(Character *ch, const char *argument) {
  Note *p;

  if (is_name("all", argument)) {
    int i, c = 0;

    Forum *forum;
    Note *p;

    for (i = 0; i < max_forum; i++) {
      forum = &forum_table[i];

      if (unread_notes(ch, forum) == FORUM_ERROR) {
        continue;
      }

      if (unread_notes(ch, forum) == 0) {
        continue;
      }
      c++;
      /*
       * Find last note
       */

      for (p = forum->notes; p && p->next; p = p->next)
        ;

      if (!p) {
        account_forum_set_last_note(ch->pc->account, p->date);
        xwritelnf(ch, "All notes in ~W%s~x forum skipped.",

                  forum->name);
      }
    }

    if (c > 0)
      xwritelnf(ch, "All notes in ~W%d~x forum%s were skipped.", c, c > 1 ? "s" : "");

    else {
      xwriteln(ch, "There are no new notes to skip.");
    }
  }
  /*
   * Find last note
   */

  for (p = ch->pc->account->forum->notes; p && p->next; p = p->next)
    ;

  if (!p) {
    xwriteln(ch, "Alas, there are no notes in that forum.");
  }

  else {
    account_forum_set_last_note(ch->pc->account, p->date);
    xwriteln(ch, "All messages skipped.");
  }
}

void note_purge(Character *ch, const char *argument) {
  if (!is_immortal(ch)) {
    return;
  }

  for (int i = 0; i < max_forum; i++) {
    check_notes(&forum_table[i]);
  }
  xwriteln(ch, "Old notes cleaned.");
}

void note_reset(Character *ch, const char *argument) {
  if (!ch->pc) {
    return;
  }

  for (int pos = 0; pos < max_forum; pos++) {
    ch->pc->account->forumData[pos].lastNote = 0;
  }
  xwriteln(ch, "All notes marked as unread.");
}

DOFUN(note) {
  char arg[ARG_SIZ];
  argument = one_argument(argument, arg);

  if (nullstr(arg) || !str_prefix(arg, "read")) {
    note_read(ch, argument);
  }

  else if (!str_prefix(arg, "write")) {
    note_write(ch, argument);
  }

  else if (!str_prefix(arg, "list")) {
    note_list(ch, argument);
  }

  else if (!str_prefix(arg, "remove")) {
    note_remove(ch, argument);
  }

  else if (!str_prefix(arg, "catchup")) {
    note_catchup(ch, argument);
  }

  else if (!str_prefix(arg, "reset")) {
    note_reset(ch, argument);
  }

  else if (!str_prefix(arg, "check")) {
    note_check(ch);
  }

  else {
    xwritelnf(ch, "Syntax: %s read [again]                - read all notes 1 forum at a time.",

              do_name);
    xwritelnf(ch, "        %s write                       - write a note on your current forum.",

              do_name);
    xwritelnf(ch, "        %s list                        - list all notes on your current forum.",

              do_name);
    xwritelnf(ch, "        %s remove [number]             - remove a note.",

              do_name);
    xwritelnf(ch, "        %s catchup                     - mark all notes on current forum as read.",

              do_name);
    xwritelnf(ch, "        %s reset                       - mark all notes on all forums as unread.",

              do_name);
    xwritelnf(ch, "        %s check                       - count how many unread notes you have on all forums.",

              do_name);
    xwritelnf(ch, "        %s clear                       - clear current note in progress.",

              do_name);

    if (is_immortal(ch))
      xwritelnf(ch, "Immortal Only: %s purge - purges expired notes from current forum.",

                do_name);
  }
}

DOFUN(subscribe) {
  int i, count, number;

  if (!ch->pc) {
    return;
  }

  if (nullstr(argument)) {
    count = 1;
    xwriteln(ch, "~RNum         Name Subscribed Description~x");
    xwriteln(ch, "~R=== ============ ========== ===========~x");

    for (i = 0; i < max_forum; i++) {
      if (unread_notes(ch, &forum_table[i]) == FORUM_ERROR) {
        continue;
      }
      xwritelnf(ch, "~W%2d~x> ~g%12s~x [  %-8s~x] %s~x", count,

                forum_table[i].name, !is_subscribed(ch->pc->account, i) ? "~rNO" : "~gYES",

                forum_table[i].description);
      count++;
    }
    return;
  }

  if (is_number(argument)) {
    count = 0;
    number = atoi(argument);

    for (i = 0; i < max_forum; i++)

      if (unread_notes(ch, &forum_table[i]) != FORUM_ERROR)

        if (++count == number) {
          break;
        }
  }

  else {
    for (i = 0; i < max_forum; i++)

      if (!str_prefix(argument, forum_table[i].name)) {
        break;
      }
  }

  if (i == max_forum) {
    xwriteln(ch, "No such forum.");
    return;
  }

  if (unread_notes(ch, &forum_table[i]) == FORUM_ERROR) {
    xwriteln(ch, "No such forum.");
    return;
  }

  if (forum_table[i].flags & FORUM_NOUNSUB) {
    xwriteln(ch, "You cannot un-subscribe from that forum.");
    return;
  }

  if (!is_subscribed(ch->pc->account, i)) {
    set_subscribed(ch->pc->account, i);
    xwritelnf(ch, "You are now subscribed to the ~W%s~x forum.",

              forum_table[i].name);
  }

  else {
    remove_subscribed(ch->pc->account, i);
    xwritelnf(ch, "You are no longer subscribed to the ~W%s~x forum.",

              forum_table[i].name);
  }
}

DOFUN(forum) {
  int i, number, count;
  Note *p;

  if (!ch->pc) {
    return;
  }

  if (nullstr(argument)) {
    show_forum(ch, false);
    return;
  }

  else if (!str_cmp(argument, "all")) {
    show_forum(ch, true);
    return;
  }

  else if (is_immortal(ch) && !str_cmp(argument, "save")) {
    save_forums();
    xwriteln(ch, "Forums saved.");
    return;
  }

  if (is_number(argument)) {
    count = 0;
    number = atoi(argument);

    for (i = 0; i < max_forum; i++)

      if (unread_notes(ch, &forum_table[i]) != FORUM_ERROR)

        if (++count == number) {
          break;
        }
  }

  else {
    for (i = 0; i < max_forum; i++)

      if (!str_prefix(argument, forum_table[i].name)) {
        break;
      }
  }

  if (i == max_forum) {
    xwriteln(ch, "No such forum.");
    return;
  }

  if (unread_notes(ch, &forum_table[i]) == FORUM_ERROR) {
    xwriteln(ch, "No such forum.");
    return;
  }
  ch->pc->account->forum = &forum_table[i];
  xwritelnf(ch, "Current forum changed to ~W%s~x. %s.",

            forum_table[i].name,
            (ch->level <

             forum_table[i].writeLevel)
                ? "You can only read here"
                : "You can both read and write here");

  if ((p = last_note(ch, &forum_table[i])) != NULL)
    xwritelnf(ch, "Last message was from ~W%s~x concerning ~W%s~x.", p->from, p->subject);
}

DOFUN(think) {
  if (nullstr(argument)) {
    xwriteln(ch, "Think what?");
    return;
  }
  act(TO_ROOM, ch, 0, argument, "~C$n~C . o O ( $T~C )~x");
  act(TO_CHAR, ch, 0, argument, "~CYou . o O ( $T~C )~x");
  return;
}

DOFUN(whisper) {
  char arg[BUF_SIZ];
  Character *victim;
  argument = one_argument(argument, arg);

  if (nullstr(arg)) {
    xwriteln(ch, "Whisper to who?");
    return;
  }

  if (nullstr(argument)) {
    xwriteln(ch, "Whisper what?");
    return;
  }

  if ((victim = get_char_room(ch, arg)) == NULL) {
    xwriteln(ch, "They aren't here.");
    return;
  }
  act(TO_VICT, ch, argument, victim, "~m$n~m whispers to you '~M$t~m'~x");
  act(TO_CHAR, ch, argument, victim, "~mYou whisper to $N '~M$t~m'~x");
  act(TO_NOTVICT, ch, argument, victim, "~M$n whispers something to $N.~x");
  return;
}

DOFUN(shout) {
  if (nullstr(argument)) {
    xwriteln(ch, "Shout what?");
    return;
  }
  act(TO_CHAR, ch, argument, 0, "~wYou shout '~W$t~w'~x");

  for (Character *victim = first_player; victim; victim = victim->next_player) {
    if (victim != ch && victim->inRoom->area == ch->inRoom->area) {
      act(TO_VICT, ch, argument, victim, "~w$n~w shouts '~W$t~w'~x");
    }
  }
  return;
}

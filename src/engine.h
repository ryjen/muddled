
/** @mainpage
 * @htmlonly
 <pre>
 ******************************************************************************
 *         __  __           _     _         ____  _       _
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/
 *                                  |___/
 *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net
 *             Many thanks to creators of muds before me.
 *
 *        In order to use any part of this Mud, you must comply with the
 *     license in 'license.txt'.  In particular, you may not remove either
 *                        of these copyright notices.
 *
 *       Much time and thought has gone into this software and you are
 *     benefitting.  I hope that you share your changes too.  What goes
 *                            around, comes around.
 ******************************************************************************
 </pre>
 * @endhtmlonly
 * @section todo TODO
 *
 * todo
 */

#ifndef MUDDLED_ENGINE_H
#define MUDDLED_ENGINE_H

#include <time.h>
#include <stdio.h>
#include "db.h"
#include "cdecl.h"
#include "typedef.h"
#include "macro.h"
#include "lookup.h"

#define BUF_SIZ     512

#define ARG_SIZ     1024

#define OUT_SIZ     2048

#define KB(s)       ((s) * 1024)

#define ID_HASH     256

#define MAX_PLAYABLE_LEVEL  100
#define LEVEL_IMMORTAL      101
#define MAX_LEVEL       102

#define LEVEL_GROUPS    5
#define EXP_TABLE_SIZ   ((MAX_PLAYABLE_LEVEL / LEVEL_GROUPS) + 1)

extern const char *weekdays[];
extern const char *months[];
extern const char *seasons[];

extern const float exp_table[EXP_TABLE_SIZ];

enum
{
    TO_CHAR = (1 << 0),
    TO_ROOM = (1 << 1),
    TO_VICT = (1 << 2),
    TO_NOTVICT = (1 << 3),
    TO_WORLD = (1 << 4)
};

enum
{
    ENGINE_OFF
};

struct engine
{
    identifier_t id;
    const char *name;
    const char *root_path;
    Flag *flags;
    int total_logins;
    int logins;
    Flag *logging;
    sql *db;
};

extern Engine engine_info;

extern time_t current_time;

extern const Lookup engine_flags[];
extern const Lookup stat_table[];

BEGIN_DECL
FILE *engine_open_file(const char *, const char *);
FILE *engine_open_file_in_dir(const char *, const char *, const char *);

int engine_query(const char *, size_t, sql_stmt *);

int load_engine(const char *db_file, const char *root_path);
int save_engine();
void *alloc_mem(size_t, size_t);
void free_mem(void *);

void initialize_engine(const char *db_file, const char *root_path);

END_DECL

#endif              /* // #ifndef ENGINE_H */

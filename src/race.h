
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

#ifndef MUDDLED_RACE_H
#define MUDDLED_RACE_H

typedef struct race Race;

#include "engine.h"

struct race
{
    identifier_t id;
    Race *next;
    const char *name;
    const char *description;
    Flag *flags;
    int stats[MAX_STAT];
    int statMods[MAX_STAT];
};

enum
{
    RACE_PC
};

extern Race *first_race;
Race *race_lookup(const char *);
Race *get_race_by_id(identifier_t);
int load_races();
int save_races();

extern const Lookup race_flags[];
#endif              /* // #ifndef RACE_H */

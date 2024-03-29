
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

#ifndef MUDDLED_COLOR_H
#define MUDDLED_COLOR_H

#include "cdecl.h"

typedef struct
{
    short attr;
    short value;
}  color_t;

BEGIN_DECL
const char *make_terminal_color(color_t *);
const char *make_html_color(color_t *);
const char *finish_html_color();
const char *convert_color_code(const char *, color_t *);
END_DECL

#define COLOR_CODE '~'
#define BLACK   30
#define RED 31
#define GREEN   32
#define YELLOW  33
#define BLUE    34
#define MAGENTA 35
#define CYAN    36
#define WHITE   37
#define BG_MOD  10
#define BOLD    1
#define NORMAL  0
#define BLINK   5
#endif              /* // #ifndef COLOR_H */

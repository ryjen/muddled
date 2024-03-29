
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

#ifndef MUDDLED_BUFFER_H
#define MUDDLED_BUFFER_H

#define MAX_TEMP_BUF 5
#define TEMP_BUF_SIZ BUFSIZ

#include "cdecl.h"
#include "typedef.h"

BEGIN_DECL

char *get_temp_buf();

Buffer *new_buf();

void destroy_buf(Buffer *);

bool buf_add(Buffer *, const char *);

bool buf_add_len(Buffer *, const char *, size_t);

bool buf_addln(Buffer *, const char *);

bool buf_addf(Buffer *, const char *, ...) __attribute__((format(printf, 2, 3)));

bool buf_addlnf(Buffer *, const char *, ...) __attribute__((format(printf, 2, 3)));

void clear_buf(Buffer *);

char *buf_string(Buffer *);

END_DECL

#endif /* // #ifndef BUFFER_H */

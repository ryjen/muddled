/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <check.h>
#include <stdlib.h>
#include "check_engine.h"
#include "engine.h"
#include "script.h"
#include "db.h"

int main()
{
    int number_failed;

    SRunner *sr = srunner_create (database_suite());
    srunner_add_suite(sr, account_suite());
    srunner_add_suite(sr, area_suite());
    srunner_add_suite(sr, character_suite());
    srunner_add_suite(sr, color_suite());
    srunner_add_suite(sr, flags_suite());
    srunner_add_suite(sr, explored_suite());
    srunner_add_suite(sr, string_suite());
    srunner_add_suite(sr, object_suite());
    srunner_add_suite(sr, hashmap_suite());
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}


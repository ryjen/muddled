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
#include "engine.h"
#include "db.h"
#include "lookup.h"
#include "object.h"
#include "fight.h"


START_TEST(test_obj_values)
{
	Object *obj = new_object();

	vset_int(&obj->value[0], value_lookup(dam_types, "bash"));

	dam_t value = vget_int(&obj->value[0]);

	fail_if(value != DAM_BASH);

}
END_TEST


Suite *object_suite (void)
{
	Suite *s = suite_create ("Objects");

	TCase *tc_writing = tcase_create("Core");
	tcase_add_test (tc_writing, test_obj_values);
	suite_add_tcase(s, tc_writing);

	return s;
}


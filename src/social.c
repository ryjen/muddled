
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
 *	           Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/

#include <muddyengine/social.h>
#include <muddyengine/macro.h>
#include <muddyengine/db.h>
#include <stdio.h>
#include <inttypes.h>
#include <muddyengine/log.h>
#include <muddyengine/engine.h>
#include <muddyengine/character.h>
#include <muddyengine/flag.h>
#include <muddyengine/lookup.h>
#include <muddyengine/util.h>
#include <ctype.h>

Social *first_social = 0;

Social *new_social(  )
{

	Social *soc = ( Social * ) alloc_mem( 1, sizeof( Social ) );

	soc->name = str_empty;

	soc->charAuto = str_empty;

	soc->minPosition = POS_RESTING;

	soc->charNoArg = str_empty;

	soc->othersNoArg = str_empty;

	soc->charFound = str_empty;

	soc->othersFound = str_empty;

	soc->victFound = str_empty;

	soc->charNotFound = str_empty;

	soc->othersAuto = str_empty;

	soc->charObjFound = str_empty;

	soc->othersObjFound = str_empty;

	return soc;

}

void destroy_social( Social * soc )
{

	free_str( soc->name );

	free_str( soc->charAuto );

	free_str( soc->othersAuto );

	free_str( soc->charNoArg );

	free_str( soc->othersNoArg );

	free_str( soc->charFound );

	free_str( soc->othersFound );

	free_str( soc->victFound );

	free_str( soc->charNotFound );

	free_str( soc->charObjFound );

	free_str( soc->othersObjFound );

	free_mem( soc );

}

int
interpret_social( Character * ch, const char *command, const char *argument )
{

	char arg[BUF_SIZ];

	Character *victim;

	Social *soc;

	for ( soc = first_social; soc != 0; soc = soc->next )
	{

		if ( UPPER( command[0] ) == UPPER( soc->name[0] )
			 && !str_prefix( command, soc->name ) )
		{

			break;

		}

	}

	if ( soc == 0 )

		return 0;

	if ( ch->position > soc->minPosition )
	{

		writelnf( ch, "You can't do that while your %s.",
				  position_table[ch->position].name );

		return 1;

	}

	one_argument( argument, arg );

	victim = NULL;

	if ( arg[0] == 0 )
	{

		act_pos( TO_ROOM, soc->minPosition, ch, NULL, victim,
				 soc->othersNoArg );

		act_pos( TO_CHAR, soc->minPosition, ch, NULL, victim, soc->charNoArg );

	}
	else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{

		writeln( ch, "They aren't here." );

	}
	else if ( victim == ch )
	{

		act_pos( TO_ROOM, soc->minPosition, ch, NULL, victim, soc->othersAuto );

		act_pos( TO_CHAR, soc->minPosition, ch, NULL, victim, soc->charAuto );

	}
	else
	{

		act_pos( TO_NOTVICT, soc->minPosition, ch, NULL, victim,
				 soc->othersFound );

		act_pos( TO_CHAR, soc->minPosition, ch, NULL, victim, soc->charFound );

		act_pos( TO_VICT, soc->minPosition, ch, NULL, victim, soc->victFound );

		if ( !ch->npc && victim->npc && victim->position < POS_SLEEPING )
		{

			switch ( number_bits( 4 ) )
			{

				case 0:

				case 1:

				case 2:

				case 3:

				case 4:

				case 5:

				case 6:

				case 7:

				case 8:

					act_pos( TO_NOTVICT, soc->minPosition, victim,
							 NULL, ch, soc->othersFound );

					act_pos( TO_CHAR, soc->minPosition, victim, NULL,
							 ch, soc->charFound );

					act_pos( TO_VICT, soc->minPosition, victim, NULL,
							 ch, soc->victFound );

					break;

				case 9:

				case 10:

				case 11:

				case 12:

					act( TO_NOTVICT, victim, NULL, ch, "$n slaps $N." );

					act( TO_CHAR, victim, NULL, ch, "You slap $N." );

					act( TO_VICT, victim, NULL, ch, "$n slaps you." );

					break;

			}

		}

	}

	return 1;

}

int load_socials(  )
{

	char buf[400];

	sqlite3_stmt *stmt;

	int total = 0;

	int len = sprintf( buf, "select * from social" );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{

		log_sqlite3( "could not prepare statement" );

		return 0;

	}

	while ( sqlite3_step( stmt ) != SQLITE_DONE )
	{

		int count = sqlite3_column_count( stmt );

		Social *soc = new_social(  );

		for ( int i = 0; i < count; i++ )
		{

			const char *colname = sqlite3_column_name( stmt, i );

			if ( !str_cmp( colname, "name" ) )
			{

				soc->name = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "charNoArg" ) )
			{

				soc->charNoArg = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "othersNoArg" ) )
			{

				soc->othersNoArg = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "charFound" ) )
			{

				soc->charFound = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "othersFound" ) )
			{

				soc->othersFound = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "victFound" ) )
			{

				soc->victFound = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "charNotFound" ) )
			{

				soc->charNotFound = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "charAuto" ) )
			{

				soc->charAuto = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "othersAuto" ) )
			{

				soc->othersAuto = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "charObjFound" ) )
			{

				soc->charObjFound = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "othersObjFound" ) )
			{

				soc->othersObjFound = str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "socialId" ) )
			{

				soc->id = sqlite3_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "minPosition" ) )
			{

				soc->minPosition = sqlite3_column_int( stmt, i );

			}
			else
			{

				log_warn( "unknown social column '%s'", colname );

			}

		}

		LINK( first_social, soc, next );

		total++;

	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{

		log_sqlite3( "could not finalize statement" );

	}

	return total;

}

int save_social( Social * soc )
{

	char buf[OUT_SIZ];

	struct dbvalues svals[] = {
		{"name", &soc->name, SQLITE_TEXT},
		{"charNoArg", &soc->charNoArg, SQLITE_TEXT},
		{"othersNoArg", &soc->othersNoArg, SQLITE_TEXT},
		{"charFound", &soc->charFound, SQLITE_TEXT},
		{"othersFound", &soc->othersFound, SQLITE_TEXT},
		{"victFound", &soc->victFound, SQLITE_TEXT},
		{"charNotFound", &soc->charNotFound, SQLITE_TEXT},
		{"charAuto", &soc->charAuto, SQLITE_TEXT},
		{"othersAuto", &soc->othersAuto, SQLITE_TEXT},
		{"charObjFound", &soc->charObjFound, SQLITE_TEXT},
		{"othersObjFound", &soc->othersObjFound, SQLITE_TEXT},
		{"minPosition", &soc->minPosition, SQLITE_INTEGER},
		{0}
	};

	if ( soc->id == 0 )
	{

		char names[BUF_SIZ] = { 0 };

		char values[OUT_SIZ] = { 0 };

		build_insert_values( svals, names, values );

		sprintf( buf, "insert into social (%s) values(%s)", names, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{

			log_sqlite3( "could not insert social" );

			return 0;

		}

		soc->id = sqlite3_last_insert_rowid( sqlite3_instance );

	}
	else
	{

		char values[OUT_SIZ] = { 0 };

		build_update_values( svals, values );

		sprintf( buf, "update social set %s where socialId=%"PRId64, values,
				 soc->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{

			log_sqlite3( "could not update social" );

			return 0;

		}

	}

	return 1;

}

void save_socials(  )
{

	db_begin_transaction(  );

	for ( Social * soc = first_social; soc; soc = soc->next )

	{

		save_social( soc );

	}

	db_end_transaction(  );

}

Social *social_lookup( const char *arg )
{

	if ( is_number( arg ) )
	{

		int id = atoi( arg );

		for ( Social * soc = first_social; soc; soc = soc->next )

		{

			if ( soc->id == id )

				return soc;

		}

	}

	else

	{

		for ( Social * soc = first_social; soc; soc = soc->next )

		{

			if ( !str_prefix( arg, soc->name ) )

				return soc;

		}

	}

	return 0;

}

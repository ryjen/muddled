
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
#include <stdio.h>
#include <muddyengine/lookup.h>
#include <muddyengine/engine.h>
#include <muddyengine/db.h>
#include <muddyengine/string.h>
#include <muddyengine/log.h>
#include <muddyengine/script.h>
#include <muddyengine/forum.h>
#include <muddyengine/race.h>
#include <muddyengine/class.h>
#include <muddyengine/skill.h>
#include <muddyengine/area.h>
#include <muddyengine/social.h>
#include <muddyengine/social.h>
#include <muddyengine/help.h>
#include <muddyengine/hint.h>

Engine engine_info;

time_t current_time;

const char *weekdays[] =
	{ "Muday", "Aberday", "Dikuday", "Circleday", "Mercday",
	"Smaugday", "Romday"
};

const char *months[] =
	{ "Dragon", "Sword", "Dagger", "Centaur", "Sea", "Shield",
	"Mountain", "Sun", "Moon", "Wolf", "Griffon", "Horse"
};

const char *seasons[] = { "Darkness", "Life", "Light", "Leaves" };

const float exp_table[EXP_TABLE_SIZ] = {
	5.9f,
	5.9f,
	5.9f,
	5.9f,
	6.2f,
	6.2f,
	6.2f,
	6.2f,
	6.5f,
	6.5f,
	6.5f,
	6.5f,
	6.8f,
	6.8f,
	6.8f,
	6.8f,
	7.1f,
	7.1f,
	7.1f,
	7.1f,
	7.2f
};

const Lookup stat_table[] = {
	{STAT_STR, "strength"},
	{STAT_INT, "intelligence"},
	{STAT_WIS, "wisdom"},
	{STAT_DEX, "dexterity"},
	{STAT_CON, "constitution"},
	{STAT_LUCK, "luck"},
	{0, 0}
};

const Lookup engine_flags[] = {
	{0, 0}
};

void initialize_default_engine(  )
{
	engine_info.name = str_dup( "Muddy Plains" );

	set_bit( engine_info.logging, LOG_BUG );
	set_bit( engine_info.logging, LOG_ERR );
	set_bit( engine_info.logging, LOG_WARN );
	set_bit( engine_info.logging, LOG_INFO );
	set_bit( engine_info.logging, LOG_SQLITE3 );
	set_bit( engine_info.logging, LOG_DEBUG );
	set_bit( engine_info.logging, LOG_TRACE );
}

int load_engine(  )
{
	char buf[500];
	sqlite3_stmt *stmt;
	int len = sprintf( buf, "select * from engine" );

	engine_info.flags = new_flag(  );
	engine_info.logging = new_flag(  );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare sql statement" );
		return 0;
	}

	if ( sqlite3_step( stmt ) == SQLITE_DONE )
	{
		if ( sqlite3_finalize( stmt ) != SQLITE_OK )
			log_sqlite3( "could not find engine info records" );

		initialize_default_engine(  );
		return 0;
	}

	int i, cols = sqlite3_column_count( stmt );
	for ( i = 0; i < cols; i++ )
	{
		const char *colname = sqlite3_column_name( stmt, i );

		if ( !str_cmp( colname, "engineId" ) )
		{
			engine_info.id = sqlite3_column_int( stmt, i );
		}
		else if ( !str_cmp( colname, "name" ) )
		{
			free_str_dup( &engine_info.name, sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "logins" ) )
		{
			engine_info.total_logins = sqlite3_column_int( stmt, i );
		}
		else if ( !str_cmp( colname, "flags" ) )
		{
			parse_flags( engine_info.flags,
						 sqlite3_column_str( stmt, i ), engine_flags );
		}
		else if ( !str_cmp( colname, "logging" ) )
		{
			parse_flags( engine_info.logging,
						 sqlite3_column_str( stmt, i ), logging_flags );
		}
		else
		{
			log_warn( "unknown account column '%s'", colname );
		}
	}
	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize sql statement" );
	}

	log_info( "Starting %s", engine_info.name );

	return 1;
}

int save_engine(  )
{
	char buf[BUF_SIZ];

	struct dbvalues enginevalues[] = {
		{"name", &engine_info.name, SQLITE_TEXT},
		{"logins", &engine_info.total_logins, SQLITE_INTEGER},
		{0, 0, 0}
	};

	if ( engine_info.id == 0 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ] = { 0 };

		build_insert_values( enginevalues, names, values );

		sprintf( buf,
				 "insert into engine (%s,flags,logging) values(%s,'%s','%s')",
				 names, values, format_flags( engine_info.flags,
											  engine_flags ),
				 format_flags( engine_info.logging, logging_flags ) );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not insert engine" );
			return 0;
		}

		engine_info.id = sqlite3_last_insert_rowid( sqlite3_instance );
	}
	else
	{
		char values[OUT_SIZ] = { 0 };

		build_update_values( enginevalues, values );

		sprintf( buf, "update engine set %s,flags='%s',logging='%s'",
				 values, format_flags( engine_info.flags, engine_flags ),
				 format_flags( engine_info.logging, logging_flags ) );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not update engine" );
			return 0;
		}
	}

	return 1;
}

void *alloc_mem( size_t num, size_t size )
{
	return calloc( num, size );
}

void free_mem( void *data )
{
	free( data );
}

void initialize_engine(  )
{
	current_time = time( 0 );

	init_sqlite3(  );

	init_lua(  );

	db_begin_transaction(  );

	load_engine(  );

	synchronize_tables(  );

	log_info( "loaded %d races", load_races(  ) );

	log_info( "loaded %d classes", load_classes(  ) );

	log_info( "loaded %d skills", load_skills(  ) );

	log_info( "loaded %d areas", load_areas(  ) );

	log_info( "loaded %d socials", load_socials(  ) );

	log_info( "loaded %d helps", load_helps(  ) );

	log_info( "loaded %d hints", load_hints(  ) );

	log_info( "loaded %d forums", load_forums(  ) );

	db_end_transaction(  );

}

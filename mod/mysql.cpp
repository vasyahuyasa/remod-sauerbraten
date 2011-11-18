/*
* remod:    mysql.cpp
* date:     2011
* author:   ^o_o^
*
* Mysql DB support for Remod
*/

/*** Example how to post in databse and read values
*
* ///// API v2
* sqlite3_open	[str] (db)						// return: -1 error othervise return db handle number
* sqlite3_query	[db]  [str] (req_handle)		// return: -1 error, "" request is done do finalize othervise return $req_handle
* sqlite3_colnames [req_handle] (col1 col2 col3 ...)	// return column names
* sqlite3_getrow   [req_handle] (field1 field2 field3 ...) // return: "" request is done (automaticaly finalize) othervise return list of fields
* sqlite3_finalize [req_handle]					// free query result before all rows processed
* sqlite3_error    [db] (str)					// return last error or ""
* sqlite3_close	   [db]							// close database
*
**********************
**** Insert values ***
**********************
*
* // open database
* db = (sqlite3_open "players3.db")
*
* // check for errors here
* ...
*
* // make queries
* // for debug compare $e with -1 and check sqlite3_error
* e = (sqlite3_query $db "BEGIN")
* e = (sqlite3_query $db "INSERT into PLAYERS values name='megakiller' passkey='sfdsfdsfgh445'")
*
* // or with slashing sql query parameters (HIGHLY RECOMMENDED)
* e = (sqlite3_pquery $db "INSERT into PLAYERS values name=':0' passkey=':1'" "player'e play`e'rpassword666")
*
* e = (sqlite3_query $db "INSERT into PLAYERS values name='Pussy' passkey='fgr45sdd345fgh445'")
* e = (sqlite3_query $db "INSERT into PLAYERS values name='Vasiliy' passkey='sfdsfhlg5aas3h445'")
* e = (sqlite3_query $db "COMMIT")
*
* // close database
* sqlite3_close $db
*
*********************
*** Select values ***
*********************
*
* // open database
* db = (sqlite3_open "players3.db")
*
* // check for errors here
* ...
*
* // select all players from db
* query = "SELECT * from PLAYERS"
* req = (sqlite3_query $db $query) // $req should have query result handle
*
* // check for errors
* if (= $req -1) [ echo (sqlite3_error) ] ...
*
* // print all rows
* row = (sqlite3_getrow $req)
*
* while (!=s $row "") [
*	echo $row
*	row = (sqlite3_getrow $req)
* ]
*
* // close database
* sqlite3_close $db
*
***/

#include "fpsgame.h"
#include <my_global.h>
#include <mysql.h>

#include "db.h"
#include "remod.h"

#define inrange(n, _max) (n>=0 && n<_max)

#define MAXDB 10
#define MAXSTMT 50

EXTENSION(MYSQL);

namespace remod
{
namespace db
{

	vector_storage<MYSQL> mysql_dbs;
	vector_storage<MYSQL_RES> mysql_stmts;


    // open database and return -1 or $db_id
    void cs_mysql_open(const char *conn_str)
    {
    	MYSQL *db;

    	vector<char*> conndata;
    	conndata = parse_connection_string((char *)conn_str);

        db = mysql_init(NULL);

        mysql_options(db, MYSQL_OPT_RECONNECT, "1");

        // open DB
        if (mysql_real_connect(db, conndata[3], conndata[1], conndata[2], conndata[5], atoi(conndata[4]), NULL, 0)) {

        	intret(mysql_dbs.add(db));
        } else {
        	//print error to log
        	conoutf("%s", mysql_error(db));
        	intret(-1);
        }

        loopi(conndata.length()) {
        	DELETEA(conndata[i]);
        }
    }

    // make sql query -1 on error, "" request is done, req_uid need make steps
    void cs_mysql_query(int *dbuid, const char* query) {
    	// check if db in range
		if (!mysql_dbs.exists(*dbuid)) {
			intret(-1); return;
		}

        // statement for work
        MYSQL_RES *stmt;

        // select database for work
        MYSQL *db = mysql_dbs[*dbuid];

        if (mysql_query(db, query)) {
        	conoutf("%s", mysql_error(db));
        	intret(-1);
        	return;
        }
        stmt = mysql_use_result(db);

        intret(mysql_stmts.add(stmt));
    }


    void cs_mysql_pquery(int *dbuid, const char *query, const char *params) {
    	char *new_query = build_query(query, params);
    	cs_mysql_query(dbuid, new_query);
    	DELETEA(new_query);
    }

   /* void cs_mysql_colnames(int *requid) {
		// check if request uid in range
		if (!mysql_stmts.exists(*requid)) {
			result(""); return;
		}

		// get statment from array
		MYSQL_RES *stmt = mysql_stmts[*requid];

		if(stmt) {
			// return column names
			vector<char> buf;

			// add every column name to result
			loopi(sqlite3_data_count(stmt))
			{
				if(buf.length()) buf.add(' ');
				defformatstring(colname)("%s", sqlite3_column_name(stmt, i));
				buf.put(colname, strlen(colname));
			}
			buf.add('\0');
			result(buf.getbuf());
		}
		else
		{
			// return empty line
			result("");
		}
	}*/

	void cs_mysql_getrow(int *requid)
	{
		/// check if request uid in range
		if (!mysql_stmts.exists(*requid)) {
			result(""); return;
		}

		// get statement from array
		MYSQL_RES *stmt = mysql_stmts[*requid];

		if(stmt) {
			// if pointer not NULL
			// return list of fields
			vector<char> buf;

			MYSQL_ROW row = mysql_fetch_row(stmt);

			if (row) {
				int num_fields = mysql_num_fields(stmt);
				for (int i = 0; i < num_fields; i++) {
					defformatstring(field)("%s", row[i]);
					buf.put(field, strlen(field));
					buf.add(' ');
				}
				buf.add('\0');
				char *res = stripslashes(buf.getbuf());
				result(res);
				DELETEA(res);
			} else {
				result("");
			}
		} else {
			// if NULL pointer return empty list
			result("");
		}

	}

	// finalize not
	void cs_mysql_finalize(int *requid)
	{

		// check if request uid in range
		if (!mysql_stmts.exists(*requid)) {
			return;
		}

		// get statement from array
		MYSQL_RES *stmt = mysql_stmts[*requid];

		if(stmt) {
			// if statemnt not NULL finalize and make it NULL
			mysql_free_result(stmt);
			mysql_stmts.remove(*requid);
		}
	}

	// return last error str
	void cs_mysql_error(int *dbuid)
	{
		// check if db uid in range
		if (!mysql_dbs.exists(*dbuid)) {
			result(""); return;
		}

		// select DB from list
		MYSQL *db = mysql_dbs[*dbuid];

		// check if DB not null
		const char *errmsg = mysql_error(db);

		if (errmsg) {
			// return error
			result(errmsg);
			return;
		}

		result("");
	}

	// close DB and free handler
	void cs_mysql_close(int *dbuid)
	{
		// check if db uid in range
		if (!mysql_dbs.exists(*dbuid)) {
			result(""); return;
		}

		// select DB from list
		MYSQL *db = mysql_dbs[*dbuid];

		mysql_close(db);
		mysql_dbs.remove(*dbuid);
	}

    // registering commands
    COMMANDN(mysql_open,      cs_mysql_open,    "s");
    COMMANDN(mysql_query,     cs_mysql_query,   "is");
    COMMANDN(mysql_pquery,    cs_mysql_pquery,  "iss");
    //COMMANDN(mysql_colnames,  cs_mysql_colnames,"i");
    COMMANDN(mysql_getrow,    cs_mysql_getrow,  "i");
    COMMANDN(mysql_finalize,  cs_mysql_finalize,"i");
    COMMANDN(mysql_error,     cs_mysql_error,   "i");
    COMMANDN(mysql_close,     cs_mysql_close,   "i");
}
}

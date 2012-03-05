/*
* remod:    geoip.cpp
* date:     2011
* author:   degrave
*
* Sqlite3 DB support for Remod
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
#include "sqlite3.h"

#include "db.h"

EXTENSION(SQLITE3);

namespace remod
{
namespace db
{
    // list of databases
	vector_storage<sqlite3> sqlite3_dbs;

    // list of statments
	vector_storage<sqlite3_stmt> sqlite3_stmts;

    // open databse and return -1 or $db_id
    void cs_sqlite3_open(const char *name)
    {
        const char *fname = findfile(name, "a"); // full path
        sqlite3 *db;
        int rc;

        // open DB
        rc = sqlite3_open(fname, &db);
        if(rc)
        {
            // error while opening DB
            sqlite3_close(db);
            intret(-1);
        }
        else
        {
        	intret(sqlite3_dbs.add(db));
        }
    }

    // make sql query -1 on error, "" request is done, req_uid need make steps
    void cs_sqlite3_query(int *dbuid, const char* query)
    {
        // check if db in range
    	if (!sqlite3_dbs.exists(*dbuid)) {
    		intret(-1); return;
    	}

        int rc;

        // statement for work
        sqlite3_stmt *stmt;

        // select database for work
        sqlite3 *db = sqlite3_dbs[*dbuid];

        // compile SQL query to bytecode
        if(sqlite3_prepare(db, query, -1, &stmt, NULL))
        {
            // error in query
            sqlite3_finalize(stmt);
            intret(-1);
            return;
        }

        // do request
        rc = sqlite3_step(stmt);
        switch(rc)
        {
            case SQLITE_ROW:
            {
                // return statement uid
            	intret(sqlite3_stmts.add(stmt));
            	return;
            }

            case SQLITE_DONE:
            {
                // request done return ""
                result("");
                break;
            }

            default:
            {
                // unknow result
                intret(-1);
                break;
            }
        }

        // request is done
        sqlite3_finalize(stmt);
    }


    void cs_sqlite3_pquery(int *dbuid, const char *query, const char *params) {
    	char *new_query = build_query(query, params);
    	cs_sqlite3_query(dbuid, new_query);
    	DELETEA(new_query);
    }

    void cs_sqlite3_colnames(int *requid)
    {
        // check if request uid in range
    	if (!sqlite3_stmts.exists(*requid)) {
    		result(""); return;
    	}


        // get statment from array
        sqlite3_stmt *stmt = sqlite3_stmts[*requid];

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

    void cs_sqlite3_getrow(int *requid)
    {
        /// check if request uid in range
    	if (!sqlite3_stmts.exists(*requid)) {
    		result(""); return;
    	}

        // get statement from array
        sqlite3_stmt *stmt = sqlite3_stmts[*requid];


		// if pointer not NULL
		// return list of fields
		vector<char> buf;

		loopi(sqlite3_data_count(stmt))
		{
			if(buf.length()) buf.add(' ');
			defformatstring(field)("%s", sqlite3_column_text(stmt, i));
			buf.put(field, strlen(field));
			buf.add(' ');
		}
		buf.add('\0');

		char *res = stripslashes(buf.getbuf());
		result(res);
		DELETEA(res);

		// do next step
		int rc = sqlite3_step(stmt);

		switch(rc)
		{
			case SQLITE_DONE:
			{
				// if request was finished do finalize
				sqlite3_finalize(stmt);

				// NULL'ize pointer in list
				sqlite3_stmts.remove(*requid);
				break;
			}

			case SQLITE_ROW:
			{
				// we have one more row
				// do nothing
				break;
			}

			default:
			{
				// error happaned
				// finalize
				sqlite3_finalize(stmt);
				sqlite3_stmts.remove(*requid);
				break;
			}
		}


    }

    // finalize not
    void cs_sqlite3_finalize(int *requid)
    {

    	// check if request uid in range
		if (!sqlite3_stmts.exists(*requid)) {
			return;
		}

        // get statement from array
        sqlite3_stmt *stmt = sqlite3_stmts[*requid];


		// if statemnt not NULL finalize and mke it NULL
		sqlite3_finalize(stmt);
		sqlite3_stmts.remove(*requid);

    }

    // return last error str
    void cs_sqlite3_error(int *dbuid)
    {
    	// check if db uid in range
		if (!sqlite3_dbs.exists(*dbuid)) {
			result(""); return;
		}

        // select DB from list
        sqlite3 *db = sqlite3_dbs[*dbuid];

        // check if DB not null
		const char *errmsg = sqlite3_errmsg(db);

		if(errmsg)
		{
			// return error
			result(errmsg);
			return;
		}

    }

    // close DB and free handler
    void cs_sqlite3_close(int *dbuid)
    {
    	// check if db uid in range
		if (!sqlite3_dbs.exists(*dbuid)) {
			result(""); return;
		}

        // select DB from list
        sqlite3 *db = sqlite3_dbs[*dbuid];

        // check if DB not null
        sqlite3_close(db);
        sqlite3_dbs.remove(*dbuid);
    }

    // registering commands
    COMMANDN(sqlite3_open,      cs_sqlite3_open,    "s");
    COMMANDN(sqlite3_query,     cs_sqlite3_query,   "is");
    COMMANDN(sqlite3_pquery,    cs_sqlite3_pquery,  "iss");
    COMMANDN(sqlite3_colnames,  cs_sqlite3_colnames,"i");
    COMMANDN(sqlite3_getrow,    cs_sqlite3_getrow,  "i");
    COMMANDN(sqlite3_finalize,  cs_sqlite3_finalize,"i");
    COMMANDN(sqlite3_error,     cs_sqlite3_error,   "i");
    COMMANDN(sqlite3_close,     cs_sqlite3_close,   "i");
}
}

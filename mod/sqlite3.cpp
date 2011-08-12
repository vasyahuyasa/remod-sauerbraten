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
* sqlite3_getrow   [req_handle] (field1 field2 field3 ...) // return: "" request is done othervise return list of fields and automaticaly do finalize
* sqlite3_finalize [req_handle]					// free query result before all rows processed
* sqlite3_error	 (str)							// return last error or ""
* sqlite3_close	 [db]							// close database
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
*	print $row
*	row = (sqlite3_getrow $req)
* ]
*
* // close database
* sqlite3_close $db
*
***/

#include "fpsgame.h"
#include "sqlite3.h"

namespace remod
{

}

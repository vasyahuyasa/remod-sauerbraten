/*
 * db.h
 *
 *  Created on: Sep 4, 2011
 *      Author: stormchild
 */

#ifndef DB_H_
#define DB_H_

#include "remod.h"


namespace remod
{
namespace db
{

	char *addslashes(char *str);
	char *build_query(const char* query, const char *params);

}
}


#endif /* DB_H_ */

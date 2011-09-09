/*
 * db.cpp
 *
 *  Created on: Sep 4, 2011
 *      Author: stormchild
 */

#include "db.h"
#include "remod.h"
#include "fpsgame.h"



extern char *strreplace(const char *s, const char *oldval, const char *newval);
extern void explodelist(const char *s, vector<char *> &elems);

namespace remod
{
namespace db
{
	char *addslashes(char *str) {
		char *s = (char *)str;

		vector<char> buf;

		for(;;) {
			char *found;
			if((found = strchr(s, '\'')) || (found = strchr(s, '"')) || (found = strchr(s, '`'))) {
				while(s < found) buf.add(*s++);
				buf.add('\\');
				buf.add(*found);
				s = found + 1;
			} else {
				while(*s) buf.add(*s++);
				buf.add('\0');
				return newstring(buf.getbuf(), buf.length());
			}
		}
	}

	char *build_query(const char* query, const char *params) {
		vector<char*> p;
		char *res = newstring(query);

		explodelist(params, p);

		loopv(p) {
			char *newparam = remod::db::addslashes(p[i]);
			char *ii = newstring(intstr(i));
			char *c = concatpstring(newstring(":"), ii);
			DELETEA(ii);
			char *newres = strreplace(res, c, newparam);
			DELETEA(res);
			DELETEA(c);
			DELETEA(newparam);
			DELETEA(p[i]);

			res = newres;
		}
		return res;
	}

}
}




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

	char *addslashes(char *s) {
		vector<char> buf;

		int characters_len = sizeof(CHARACTERS)/sizeof(char);

		for(;;) {
			char *found = 0;
			for (int i = 0; i < characters_len; i++){
				char *newfound = strchr(s, CHARACTERS[i]);
				if (newfound && (!found || ((size_t) newfound < (size_t) found))) found = newfound;
			}

			if (found) {
				while(s < found) buf.add(*s++);
				buf.add('\\');
				buf.add(*found);
				s = found + 1;
			} else {
				while(*s) buf.add(*s++);
				buf.add('\0');
				return newstring(buf.getbuf(), buf.length()-1);
			}
		}
		return 0;
	}

	char *stripslashes(char *s) {
		vector<char> buf;
		int characters_len = sizeof(CHARACTERS)/sizeof(char);
		while(s && *s) {
			if (*s == '\\') {
				char *prev = s++;
				bool found = false;
				for (int i = 0; i < characters_len; i++){
					found = *s == CHARACTERS[i];
					if (found) break;
				}
				if (!found) {
					//current not found in CHARACTERS - it's not character to escape, adding \ as normal character
					buf.add(*prev);
				}
			}
			buf.add(*s++);
		}
		buf.add('\0');
		return newstring(buf.getbuf(), buf.length()-1);
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

	vector<char*> parse_connection_string(char *s) {
		char *c = strstr(s, "://");

		vector<char*> res;
		char *scheme = NULL;
		char *login = NULL;
		char *password = NULL;
		char *server = NULL;
		char *port = NULL;
		char *path = NULL;

		if (c) {
			scheme = newstring(c, (size_t) (c - s));
			c += 3;
		} else {
			c = s;
		}

		char *at = strchr(c, '@');
		if (at) {
			char *semicolon = strchr(c, ':');
			if (semicolon < at) {
				login = newstring(c, (size_t) (semicolon - c));
				c = semicolon+1;
				password = newstring(c, (size_t) (at - c));
			} else {
				login = newstring(c, (size_t) (at - c));
			}
			c = at+1;
		}

		char *semicolon = strchr(c, ':');
		char *slash = strchr(c, '/');


		if (slash) {
			path = newstring(slash+1);
		}

		if (semicolon && (((semicolon < slash) && slash) || !slash) ) {
			server = newstring(c, (size_t) (semicolon -  c));
			port = slash ? newstring(semicolon+1, (size_t) (slash -  semicolon) - 1) : newstring(semicolon+1);
		} else {
			server = slash ? newstring(c, (size_t) (slash - c)) : newstring(c);
		}

		res.add(scheme);
		res.add(login);
		res.add(password);
		res.add(server);
		res.add(port);
		res.add(path);

		return res;
	}


ICOMMAND(addslashes, "C", (char *s), char *c = addslashes(s); result(c); DELETEA(c));
ICOMMAND(stripslashes, "C", (char *s), char *c = stripslashes(s); result(c); DELETEA(c));
}
}




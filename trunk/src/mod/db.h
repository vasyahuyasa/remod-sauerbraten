/*
 * db.h
 *
 *  Created on: Sep 4, 2011
 *      Author: stormchild
 */

#ifndef DB_H_
#define DB_H_

#include "remod.h"

enum databaseType { DB_NONE, DB_SQLITE3, DB_MYSQL, NUMDBS };

namespace remod {
namespace db {

// Database connection interface
struct Database
{
    uint id;
    databaseType type;
    virtual void query(const char *callback, const char *query);
};


/**
 * characters to escape
 */
const char CHARACTERS[4] = {'\'', '"', '`', '\\'};

char *addslashes(char *str);
char *stripslashes(char *str);
char *build_query(tagval *args, int numargs, int startfrom);

/**
 * parses connection string
 *
 * scheme://login:password@server:port/database -> {scheme, login, password, server, port, database}
 */
vector<char*> parse_connection_string(char *s);
/**
 * Structure to store database connections or results
 */
template<class T> struct vector_storage {

	vector<T*> items;

	int add(T *item) {
		int l = items.length();
		for (int i = 0; i < l; i++) {
			if (items[i] == NULL) {
				items[i] = item;
				return i;
			}
		}
		items.add(item);
		return l;
	}

	bool exists(int i) {
		return (i >= 0) && (items.length() > i) && items[i];
	}

	void remove(int i) {
		if (exists(i))
			items[i] = NULL;
	}

	T *get(int i) {
		if (exists(i)) {
			return items[i];
		} else {
			return NULL;
		}
	}

	T *operator[](int i) { return get(i); }
};
}
}

#endif /* DB_H_ */

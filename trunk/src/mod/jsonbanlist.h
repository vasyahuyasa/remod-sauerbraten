/*
 * jsonbanlist.h
 *
 *  Created on: Jan 1, 2013
 *  Author: degrave
 */

// example of usage
// jsonbanlist hopmod http://sauer.nomorecheating.org/hopmod/view_bans.php http://83.169.44.106/hopmod/gbans.php
// jsonbanlist ourbans http://ourbans.butchers.su/bans.php?format=json
// jsonupdate hopmod 60 // every hour
// jsonupdate ourbans 10
// jsontimebans hopmod 0
// jsontimebans ourbans 1

// curl -A "" http://sauer.nomorecheating.org/hopmod/gbans.php
// "id":24, "address":"190.43/16", "admin":"ADMIN", "expire":-1, "reason":"unknown", "time":1321377751, "expired":0
// id			- database ban id
// address	- bannedip range
// admin		- who added ban
// expire		- time when ban expire, -1 perm ban
// reson		- ban reason
// time		- when added
// expired		- field can be ignored

#ifndef __JSONBANLIST_H__
#define __JSONBANLIST_H__

namespace remod
{
    namespace jsonbanlist
    {

    }
}

#endif

/* Copyright (C) 2008 Sun Microsystems, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* Character set configuration (hard-coded) */

//We use only latin1 charset in remod

#define HAVE_CHARSET_latin1 1

#undef HAVE_CHARSET_utf16
#undef HAVE_CHARSET_utf32
#define HAVE_CHARSET_utf8mb3 1
#define HAVE_CHARSET_utf8mb4 1
#undef HAVE_UCA_COLLATIONS

#define MYSQL_DEFAULT_CHARSET_NAME "latin1"
#define MYSQL_DEFAULT_COLLATION_NAME "latin1_general_ci"
#define USE_MB 1

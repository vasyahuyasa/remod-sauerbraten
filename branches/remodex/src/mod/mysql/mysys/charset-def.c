/* Copyright (C) 2000 MySQL AB

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

#include "mysys_priv.h"

/*
  Include all compiled character sets into the client
  If a client don't want to use all of them, he can define his own
  init_compiled_charsets() that only adds those that he wants
*/

#ifdef HAVE_UCA_COLLATIONS

#ifdef HAVE_CHARSET_utf32
extern CHARSET_INFO my_charset_utf32_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf32_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf32_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf32_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf32_polish_uca_ci;
extern CHARSET_INFO my_charset_utf32_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf32_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf32_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf32_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf32_czech_uca_ci;
extern CHARSET_INFO my_charset_utf32_danish_uca_ci;
extern CHARSET_INFO my_charset_utf32_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf32_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf32_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf32_roman_uca_ci;
extern CHARSET_INFO my_charset_utf32_persian_uca_ci;
extern CHARSET_INFO my_charset_utf32_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf32_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf32_sinhala_uca_ci;
#endif /* HAVE_CHARSET_utf32 */
 
 
#ifdef HAVE_CHARSET_utf16
extern CHARSET_INFO my_charset_utf16_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf16_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf16_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf16_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf16_polish_uca_ci;
extern CHARSET_INFO my_charset_utf16_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf16_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf16_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf16_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf16_czech_uca_ci;
extern CHARSET_INFO my_charset_utf16_danish_uca_ci;
extern CHARSET_INFO my_charset_utf16_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf16_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf16_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf16_roman_uca_ci;
extern CHARSET_INFO my_charset_utf16_persian_uca_ci;
extern CHARSET_INFO my_charset_utf16_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf16_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf16_sinhala_uca_ci;
#endif  /* HAVE_CHARSET_utf16 */
 

#ifdef HAVE_CHARSET_utf8mb3
extern CHARSET_INFO my_charset_utf8mb3_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_polish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_czech_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_danish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_roman_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_persian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb3_sinhala_uca_ci;
#ifdef HAVE_UTF8_GENERAL_CS
extern CHARSET_INFO my_charset_utf8mb3_general_cs;
#endif
#endif /* HAVE_CHARSET_utf8mb3 */

#ifdef HAVE_CHARSET_utf8mb4
extern CHARSET_INFO my_charset_utf8mb4_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_polish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_czech_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_danish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_roman_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_persian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_sinhala_uca_ci;
#endif /* HAVE_CHARSET_utf8mb4 */

#endif /* HAVE_UCA_COLLATIONS */


my_bool init_compiled_charsets(myf flags __attribute__((unused)))
{
  CHARSET_INFO *cs;

  add_compiled_collation(&my_charset_bin);
  add_compiled_collation(&my_charset_filename);
  
  add_compiled_collation(&my_charset_latin1);
  add_compiled_collation(&my_charset_latin1_bin);
  add_compiled_collation(&my_charset_latin1_german2_ci);



#ifdef HAVE_CHARSET_utf8mb3
  add_compiled_collation(&my_charset_utf8mb3_general_ci);
  add_compiled_collation(&my_charset_utf8mb3_bin);
#ifdef HAVE_UTF8_GENERAL_CS
  add_compiled_collation(&my_charset_utf8mb3_general_cs);
#endif
#ifdef HAVE_UCA_COLLATIONS
  add_compiled_collation(&my_charset_utf8mb3_unicode_ci);
  add_compiled_collation(&my_charset_utf8mb3_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_polish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_czech_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_danish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_roman_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_persian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb3_sinhala_uca_ci);
#endif /* HAVE_UCA_COLLATIONS  */
#endif /* HAVE_CHARSET_utf8mb3 */


#ifdef HAVE_CHARSET_utf8mb4
  add_compiled_collation(&my_charset_utf8mb4_general_ci);
  add_compiled_collation(&my_charset_utf8mb4_bin);
#ifdef HAVE_UCA_COLLATIONS
  add_compiled_collation(&my_charset_utf8mb4_unicode_ci);
  add_compiled_collation(&my_charset_utf8mb4_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_polish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_czech_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_danish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_roman_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_persian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_sinhala_uca_ci);
#endif /* HAVE_UCA_COLLATIONS  */
#endif /* HAVE_CHARSET_utf8mb4 */


#ifdef HAVE_CHARSET_utf16
  add_compiled_collation(&my_charset_utf16_general_ci);
  add_compiled_collation(&my_charset_utf16_bin);
#ifdef HAVE_UCA_COLLATIONS
  add_compiled_collation(&my_charset_utf16_unicode_ci);
  add_compiled_collation(&my_charset_utf16_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf16_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf16_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf16_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf16_polish_uca_ci);
  add_compiled_collation(&my_charset_utf16_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf16_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf16_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf16_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf16_czech_uca_ci);
  add_compiled_collation(&my_charset_utf16_danish_uca_ci);
  add_compiled_collation(&my_charset_utf16_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf16_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf16_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf16_roman_uca_ci);
  add_compiled_collation(&my_charset_utf16_persian_uca_ci);
  add_compiled_collation(&my_charset_utf16_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf16_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf16_sinhala_uca_ci);
#endif /* HAVE_UCA_COLLATIOINS */
#endif /* HAVE_CHARSET_utf16 */


#ifdef HAVE_CHARSET_utf32
  add_compiled_collation(&my_charset_utf32_general_ci);
  add_compiled_collation(&my_charset_utf32_bin);
#ifdef HAVE_UCA_COLLATIONS
  add_compiled_collation(&my_charset_utf32_unicode_ci);
  add_compiled_collation(&my_charset_utf32_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf32_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf32_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf32_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf32_polish_uca_ci);
  add_compiled_collation(&my_charset_utf32_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf32_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf32_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf32_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf32_czech_uca_ci);
  add_compiled_collation(&my_charset_utf32_danish_uca_ci);
  add_compiled_collation(&my_charset_utf32_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf32_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf32_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf32_roman_uca_ci);
  add_compiled_collation(&my_charset_utf32_persian_uca_ci);
  add_compiled_collation(&my_charset_utf32_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf32_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf32_sinhala_uca_ci);
#endif /* HAVE_UCA_COLLATIONS */
#endif /* HAVE_CHARSET_utf32 */
  

  /* Copy compiled charsets */
  for (cs=compiled_charsets; cs->name; cs++)
    add_compiled_collation(cs);
  
  return FALSE;
}

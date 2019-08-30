
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the source code for bruiser's auto-completion and suggestions.*/
/*Copyright (C) 2017 Farzad Sadeghi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "./lua-5.3.4/src/lua.h"
#include "./lua-5.3.4/src/lauxlib.h"
#include "./lua-5.3.4/src/lualib.h"
/*standard headers*/
#include "inttypes.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
/*other*/
#include "CompletionHints.h"
#include "completions.h"
#include "linenoise/linenoise.h"
/**********************************************************************************************************************/
size_t get_str_len(const char* str) {
  int size_counter = 0;
  while (true) {
    if (str[size_counter] != '\0') {
      size_counter++;
    } else {
      return size_counter;
    }
  }
}

int devi_find_last_word(char const * const str, char* delimiters, int delimiter_size) {
  size_t str_len = get_str_len(str);

  for (int i = 0; i < delimiter_size; ++i) {
    if (delimiters[i] == str[str_len-1]) return -1;
  }

  for (int i = str_len -1; i >=0; --i) {
    for (int j = 0; j < delimiter_size;++j) {
      if (delimiters[j] == str[i]) {
        return i + 1;
      } else {
        /*intentionally left blank*/
      }
    }
  }

  return 0;
}

word_pos_t devi_find_middle_word(char const * const str, char* delimiters, int delimiter_size, size_t pos) {
  size_t str_len = get_str_len(str);
  word_pos_t result;
  result.begin = -1;
  result.end = -1;

  if (NULL == str) return result;

  bool begin_matched = false;
  bool end_matched = false;

  for (int i = 0; i < delimiter_size; ++i) {
    if (str[pos] == delimiters[i]) {
      return result;
    }
  }

  for (int i = pos; i >=0; --i) {
    for (int j = 0; j < delimiter_size;++j) {
      if (delimiters[j] == str[i]) {
        result.begin = i + 1;
        begin_matched = true;
      } else {
        /*intentionally left blank*/
      }
    }
  }

  for (int i = pos; i <= str_len - 1; ++i) {
    for (int j = 0; j < delimiter_size;++j) {
      if (delimiters[j] == str[i]) {
        result.end = i - 1;
        end_matched = true;
      } else {
        /*intentionally left blank*/
      }
    }
  }

  if (!begin_matched) {
    result.begin = 0;
  }

  if (!end_matched) {
    result.end = strlen(str) - 1;
  }

  return result;
}

size_t devi_rfind(char const * const str, char const * const substr) {
  size_t str_len = get_str_len(str);
  size_t substr_len = get_str_len(substr);
  for (size_t i = str_len-1; i >=0 ; --i) {
    if (substr[substr_len-1] != str[i]) {
      continue;
    } else {
      bool matched = true;
      for (int j = substr_len-1; j >= 0; --j) {
        if (substr[j] == str[i - (substr_len - j -1)]) {
          continue;
        } else {
          matched = false;
        }
      }
      if (matched) return (i - substr_len + 1);
    }
  }

  return -1;
}

size_t devi_lfind(char const * const str, char const * const substr) {
  size_t str_len = get_str_len(str);
  size_t substr_len = get_str_len(substr);
  for (size_t i = 0; i < str_len; ++i) {
    if (substr[0] != str[i]) {
      continue;
    } else {
      bool matched = true;
      for (size_t j = 0; j < substr_len; ++j) {
        if (i + j >= str_len) return -1;
        if (substr[j] == str[i+j]) {
          continue;
        } else {
          matched = false;
        }
      }
      if (matched) return (i);
    }
  }

  return -1;
}

void shell_completion(const char* buf, linenoiseCompletions* lc, size_t pos) {
  if (0 != pos) {
    word_pos_t word_pos = devi_find_middle_word(buf, ID_BREAKERS, NELEMS(ID_BREAKERS), pos - 1);

    if (word_pos.begin == -1 || word_pos.end == -1) {
      return;
    }

    char* last_word = malloc(word_pos.end - word_pos.begin + 2);
    last_word[word_pos.end - word_pos.begin + 1] = '\0';
    memcpy(last_word, &buf[word_pos.begin], word_pos.end - word_pos.begin + 1);
    char* ln_matched;

    // custom list completion candidates
    for(int i=0; i < NELEMS(LUA_FUNCS); ++i) {
      if ( 0 == devi_lfind(LUA_FUNCS[i], last_word)) {
        ln_matched = malloc(strlen(buf) + strlen(LUA_FUNCS[i]) - strlen(last_word) + 1);
        ln_matched[strlen(buf) + strlen(LUA_FUNCS[i]) - strlen(last_word)] = '\0';
        memcpy(ln_matched, buf, word_pos.begin);
        memcpy(&ln_matched[word_pos.begin], LUA_FUNCS[i], strlen(LUA_FUNCS[i]));

        if (0 == (strlen(buf) - word_pos.end - 1)) {
          /* No Op */
        } else {
          memcpy(&ln_matched[word_pos.begin + strlen(LUA_FUNCS[i])], &buf[pos], strlen(buf) - pos + 1);
        }

        linenoiseAddCompletion(lc, ln_matched, pos + strlen(LUA_FUNCS[i]) - strlen(last_word));
        free(ln_matched);
      }
    }

#if 0
    // Lua global table completions
    lua_pushglobaltable(ls);
    lua_pushnil(ls);
    while(0 != lua_next(ls, -2)) {
      if ( 0 == devi_lfind(lua_tostring(ls, -2), last_word)) {
        char* ss = lua_tostring(ls, -2);
        ln_matched = malloc(strlen(buf) + strlen(ss) - strlen(last_word) + 1);
        ln_matched[strlen(buf) + strlen(ss) - strlen(last_word)] = '\0';
        memcpy(ln_matched, buf, word_pos.begin);
        memcpy(&ln_matched[word_pos.begin], ss, strlen(ss));

        if (0 == (strlen(buf) - word_pos.end - 1)) {
          /* No Op */
        } else {
          memcpy(&ln_matched[word_pos.begin + strlen(ss)], &buf[pos], strlen(buf) - pos + 1);
        }

        linenoiseAddCompletion(lc, ln_matched, pos + strlen(ss) - strlen(last_word));
        free(ln_matched);
      }
      lua_pop(ls, 1);
    }
    lua_pop(ls, 1);
#endif

    free(last_word);
  }
}

char* shell_hint(const char* buf, int* color, int* bold) {
  if (NULL != buf) {
      *color = 35;
      *bold = 0;
  } else {
    return "";
  }

  int last_word_index = devi_find_last_word(buf, ID_BREAKERS, NELEMS(ID_BREAKERS));
  char* last_word = malloc(strlen(buf) - last_word_index+1);
  last_word[strlen(buf) - last_word_index] = '\0';
  memcpy(last_word, &buf[last_word_index], strlen(buf) - last_word_index);
  char* ln_matched;
  for(int i=0; i < NELEMS(LUA_FUNCS); ++i) {
    size_t match_index = devi_lfind(LUA_FUNCS[i], last_word);
    if (0 == match_index) {
      ln_matched = malloc(strlen(LUA_FUNCS[i])-strlen(last_word)+1);
      ln_matched[strlen(LUA_FUNCS[i]) - strlen(last_word)] = '\0';
      memcpy(ln_matched, &LUA_FUNCS[i][match_index+strlen(last_word)], strlen(LUA_FUNCS[i]) - strlen(last_word));
      free(last_word);
      return ln_matched;
    }
  }

#if 0
  lua_pushglobaltable(ls);
  lua_pushnil(ls);
  while(0 != lua_next(ls, -2)) {
    size_t match_index = devi_lfind(lua_tostring(ls, -2), last_word);
    if (0 == match_index) {
      ln_matched = malloc(strlen(lua_tostring(ls, -2))-strlen(last_word)+1);
      ln_matched[strlen(lua_tostring(ls, -2)) - strlen(last_word)] = '\0';
      memcpy(ln_matched, &lua_tostring(ls, -2)[match_index+strlen(last_word)], strlen(lua_tostring(ls, -2)) - strlen(last_word));
      free(last_word);
      return ln_matched;
    }
    lua_pop(ls, 1);
  }
  lua_pop(ls, 1);
#endif

  free(last_word);
  return NULL;
}
/**********************************************************************************************************************/
/*last line intentionally left blank*/


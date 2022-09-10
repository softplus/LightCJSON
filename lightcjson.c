/*
  Copyright (c) 2022-2022 John Mueller and LightCJSON contributors
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* lightcjson.c  */
/* Lightweight JSON parser in C. */

#include <stdio.h>
#include <string.h>
#include "lightcjson.h"

#define DOUBLEQUOTE ('\"')
#define is_space_(x) ( ((x)==' ') || ((x)=='\t') || ((x)=='\n') || ((x)=='\r') )
#define is_doublequote_(x)   ( (x)==DOUBLEQUOTE )
#define is_escape_(x)        ( (x)=='\\' )
#define is_bracket_open_(x)  ( ((x)=='[') || ((x)=='{') )
#define is_bracket_close_(x) ( ((x)==']') || ((x)=='}') )
#define is_number_(x)        ( ((x)>='0') && ((x)<='9') )

// trim beginning & end unnecessary spacing
char *jsonTrim(const char *src, char *dest) {
  char *ptr = dest;
  char *input = (char *)src;
  // beginning
  while (*input) {
    if (!is_space_(*input)) break;
    input++;
  }
  while (*input) {
    *dest=*input; dest++; input++;
  }
  *dest = '\0';
  dest--; // cleanup from behind
  while (dest>=ptr) {
    if (!is_space_(*dest)) {
      *(dest+1) = '\0';
      break;
    }
    dest--;
  }
  return ptr;
}

// remove all unnecessary spacing from the JSON string... preserving strings
// dest can = json to save space. 
// returns pointer to jsonOutput.
// dest must have at least the same space as json
char *jsonRemoveSpacing(const char *json, char *dest) {
  int quote_count = 0;
  char *ptr = dest; // keep pointer to start of output
  char prev = 0;
  char *input = (char *)json;

  while (*input) {
    char ch = *input;
    // is it spacing?
    if (is_space_(ch)) {
        if (quote_count % 2) { // inside a string: keep
          *dest = ch; 
          dest++;
        }
    } else {
      if (is_doublequote_(ch)) { // is double quote?
          if (!is_escape_(prev)) { // if not escaped, count it
            quote_count++;
          }
      }
      *dest = ch; 
      dest++;
    }
    prev = ch;
    input++;
  }
  *dest = 0;
  return ptr;
}


// retrieve item in indexed list; trims unnecessary space start/end
// eg .
char *jsonIndexList(const char *json, int index, char *dest, int size) {
  int level = 1; // number of braces seen { = +1 } = -1
  int item_idx = 0;
  char *input = (char *)json;
  char *ptr_start = input;
  char ch;

  while (level>0) {
    ch = *input;
    //printf("[%c,%d,%d,%d]", ch,level,item_idx,(ptr_start-json));
    if (is_bracket_close_(ch)) level--;
    if (is_bracket_open_(ch)) level++;

    if (level==1) {
      if (ch==',' || is_bracket_close_(ch) || ch=='\0') {
        if (item_idx==index) { // got it 
          int len = input - ptr_start;
          if (is_bracket_close_(ch)) len++; // include closing bracket
          if (len>=size) len=size-1;
          strncpy(dest, ptr_start, len);
          *(dest+len)='\0'; // terminate string
          jsonTrim(dest, dest);
          return dest;
        } else if (!is_bracket_close_(ch)) {
          item_idx++;
          ptr_start = input+1;
        }
      }
    }
    if (ch=='\0') break;
    input++;
  }
  *dest = '\0';
  return NULL;
}

// todo: rewrite
// return a sub-json struct
char *jsonExtract(const char *json, const char *key_name, char *dest, int size) {
  char *ptr_start, *ptr_end;
  char name[strlen(key_name)+3];

  snprintf(name, sizeof(name), "\"%s\"", key_name);
  //printf("Searching for '%s'\n", name);

  ptr_start = strstr(json, name);
  if (!ptr_start) { // not found at all
    dest='\0'; return NULL;
  }
  ptr_start += strlen(name) + 1; // include : after key
  //printf("\nRest: %s\n", ptr_start);

  // skip spaces
  while (*ptr_start && is_space_(*ptr_start)) ptr_start++;
  if (!*ptr_start) {
    *dest = '\0'; return dest;
  }
  ptr_end = ptr_start;

  // options: number, quote, [list], {struct}
  if (is_number_(*ptr_start) || (*ptr_start=='-')) { // number
    ptr_end = ptr_start+1;
    while (*ptr_end && (is_number_(*ptr_end) || (*ptr_end=='.'))) ptr_end++;

  } else if (is_doublequote_(*ptr_start)) { // string
    ptr_end = ptr_start+1;
    char prev = *ptr_start;
    while (*ptr_end) {
      if (is_doublequote_(*ptr_end) && (!is_escape_(prev))) break;
      prev = *ptr_end; ptr_end++;
    }
    if (*ptr_end) ptr_end++;

  } else if (is_bracket_open_(*ptr_start)) { // struct or list
    ptr_end = ptr_start+1;
    char prev = *ptr_start;
    int level = 0;
    while (*ptr_end) {
      if (is_doublequote_(*ptr_end) && (!is_escape_(prev))) {
        ptr_end++;
        while (*ptr_end) { // spool through string
          if (is_doublequote_(*ptr_end) && (!is_escape_(prev))) break;
          prev = *ptr_end; ptr_end++;
        }
        if (!(*ptr_end)) break;
      }
      if (is_bracket_open_(*ptr_end) && (!is_escape_(prev))) level++;
      if (is_bracket_close_(*ptr_end) && (!is_escape_(prev))) level--;
      if (level<0) break;
      prev = *ptr_end; ptr_end++;
    }
    if (*ptr_end) ptr_end++;

  } else { // undefined type, give up
    dest='\0'; return NULL;
  }
  // copy value
  int len = ptr_end - ptr_start;
  if (len>size) len=size;
  strncpy(dest, ptr_start, len);
  dest[len] = '\0';
  return dest;
}

// escape/unescape a json string
char *jsonEscape(const char *input, char *dest, int size) {
  char *ptr_src = (char *)input;
  char *ptr_dest = dest;

  if (!(*input)) {
    *dest='\0'; return dest;
  }

  while ((*ptr_src) && ((ptr_dest-dest)<size-2)) {
    if (is_doublequote_(*ptr_src)) {
      *ptr_dest = '\\'; ptr_dest++;
    }
    *ptr_dest = *ptr_src; ptr_dest++; ptr_src++; 
  }
  *ptr_dest = '\0';
  return dest;
}

char *jsonUnescape(const char *input, char *dest, int size) {
  char *ptr_src = (char *)input;
  char *ptr_dest = dest;

  if (!(*input)) {
    *dest='\0'; return dest;
  }
  char next;
  while ((*ptr_src) && ((ptr_dest-dest)<size-1)) {
    next=*(ptr_src+1);
    if (is_escape_(*ptr_src) && is_doublequote_(next)) ptr_src++;
    *ptr_dest = *ptr_src; ptr_dest++; ptr_src++; 
  }
  *ptr_dest = '\0';
  return dest;
}

// create/parse a JSON quote-string
char *jsonQuote(const char *input, char *dest, int size) {
  char *ptr_dest = dest;
  *ptr_dest = DOUBLEQUOTE; ptr_dest++;
  jsonEscape(input, ptr_dest, size-2);
  ptr_dest[strlen(ptr_dest)+1] = '\0';
  ptr_dest[strlen(ptr_dest)+0] = DOUBLEQUOTE;
  return dest;
}

char *jsonUnquote(const char *input, char *dest, int size) {
  char *ptr_src = (char *)input;
  if ((ptr_src[0]!=DOUBLEQUOTE) || (ptr_src[strlen(ptr_src)-1]!=DOUBLEQUOTE)) {
    // not in quotes
    strncpy(dest, input, size);
    dest[strlen(input)] = '\0';
    return dest;
  }
  jsonUnescape((input+1), dest, size);
  dest[strlen(dest)-1] = '\0';
  return dest;
}
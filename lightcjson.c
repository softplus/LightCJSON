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

// key-value pair builder
char *jsonAppendItem(const char *key, const char *value, char *dest, int size) {
  // check min room
  if (size < strlen(dest) + strlen(key) + strlen(value) + 5+2) {
    return NULL;
  }
  if (strlen(dest)==0) sprintf(dest, "{}");

  int rem = size-strlen(dest);
  if (strlen(dest)>2) strcpy(dest + strlen(dest)-1, ",}\0");

  rem = size-strlen(dest);
  jsonQuote(key, dest + strlen(dest) - 1, rem);

  strcpy(dest + strlen(dest), ":\0");

  rem = size-strlen(dest);
  jsonQuote(value, dest + strlen(dest), rem);

  strcpy(dest + strlen(dest), "}\0");

  return dest;
}

// extract key-value JSON pair at position
// returns 1=ok, 0=error
int jsonGetKeyValue(const char *input, char *key, char *value, int item_size) {
  // expected: "key":"value" -> key, value
  // or "ke\"y":123 -> ke"y, 123
  // skips rest of input
  char *ptr_in = (char *)input;
  char *ptr_out;
  // expected: key is string
  if (!is_doublequote_(*ptr_in)) { return 0; }
  ptr_in++; 

  char prev = *ptr_in;
  ptr_out = key;
  while (*ptr_in) {
    if (is_doublequote_(*ptr_in) && (!is_escape_(prev))) break;
    if (!is_escape_(*ptr_in) || is_escape_(prev)) {
      *ptr_out=*ptr_in; ptr_out++;
    } 
    prev = *ptr_in; ptr_in++;
    if ((ptr_out-key)>item_size-1) return 0; // ran out of room for key name
  }
  *ptr_out = '\0';
  // have key, just fetch now
  char *ret = jsonExtract(input, key, value, item_size);
  return ((ret!=NULL)?1:0); 
}

// returns offset to next key/value pair, or 0 for none remaining here.
// returns 0 when time to add new_input
// returns -1 on buffer overfill
// set start_offset = last_offset to continue parsing
// buffer should be >2x new_input size, >2x+5 max key+value size
// skips named structs ("key":{"item":"value"}), and lists. ("key":["a","b"])
#define JKVERR 
int jsonStreamKeyValues(const char *new_input, char *buffer, int max_buffer, 
    int start_offset, int *last_offset) {
  //printf(">jskv(in l=%d, buff l=%d, so=%d)\n", 
  //  (int)((new_input!=NULL)?strlen(new_input):0), (int)strlen(buffer), start_offset);
  int new_start_offset = start_offset;
  if (new_input) { // add to buffer 
    //printf("adding to buffer\n");
    if (strlen(new_input) + strlen(buffer)<max_buffer-1) { // just append
      //printf("just append\n");
      int max_len = max_buffer -1 - strlen(buffer);
      strncpy(buffer+strlen(buffer), new_input, max_len);  
    } else { // shift by new buffer item
      if (strlen(new_input)+max_buffer-start_offset>max_buffer-1) { 
        //printf("err: no room\n"); 
        return -1; 
      }
      //printf("offset\n");
      int offset=1+strlen(new_input);
      //printf("delta=%i\n", offset);
      char *ptr = buffer;
      while (*(ptr+offset)) { *ptr = *(ptr+offset); ptr++; }
      strncpy(ptr, new_input, max_buffer-(ptr-buffer));
      new_start_offset = start_offset - offset;      
      if (new_start_offset<0) {
        //printf("not enough buffer to parse\n"); 
        return -1;
      }
    }
  }
  //printf("buff='%s'\n", buffer);
  //printf("buff len=%d, start=%d (%c)\n", (int)strlen(buffer), new_start_offset, *(buffer+new_start_offset));
  // read until quote
  char *ptr = buffer + new_start_offset;
  while ((!is_doublequote_(*ptr)) && *ptr) ptr++;
  if (!*ptr) { 
    //printf("no quotes found\n");
    *last_offset = ptr-buffer; return 0; } // no quotes found
  char *key_start = ptr; // ptr is at start of key

  char prev=*ptr; ptr++;
  while (*ptr && !(is_doublequote_(*ptr) || is_escape_(prev))) {
    prev = *ptr; ptr++;
  }
  if (!*ptr) { 
    //printf("end of string in key\n");
    *last_offset = new_start_offset; return 0; }
  // ptr is at end of quote for key
  //printf("got key\n");
  // expect ":"
  ptr++; 
  if (!*ptr) {
    //printf("end of string before : after key\n");
    *last_offset = new_start_offset; return 0; }
  if (*ptr!=':') { // if no :, try from here
    //printf("no : found after key (got %c)\n", *ptr);
    return jsonStreamKeyValues(NULL, buffer, max_buffer, (ptr-buffer), last_offset);
  }
  ptr++;
  // await: number, quote, comma, {, }, [, ]
  while (*ptr && is_space_(*ptr)) ptr++;
  if (!*ptr) { 
    //printf("end of string before value\n");
    *last_offset = new_start_offset; return 0; }
  if (is_bracket_open_(*ptr) || is_bracket_close_(*ptr) || (*ptr==',')) {
    //printf("got bracket or comma (%c), restart\n", *ptr);
    return jsonStreamKeyValues(NULL, buffer, max_buffer, (ptr-buffer), last_offset);
  }
  if ((*ptr=='-') || is_number_(*ptr)) {
    //printf("got number\n");
    ptr++;
    while ((*ptr) && (is_number_(*ptr) || (*ptr=='.'))) ptr++;
    if (!*ptr) { 
      //printf("end of string before end of value\n");
      *last_offset = new_start_offset; return 0; }
    // looks ok, let's push it
    //printf("got value complete\n");
    *last_offset = (ptr-buffer);
    return (key_start-buffer);
  } else if (*ptr=='"') {
    //printf("got string\n");
    ptr++;prev=*ptr;
    while (*ptr && !(is_doublequote_(*ptr) || is_escape_(prev))) {
      prev = *ptr; ptr++;
    }
    if (!*ptr) { 
      //printf("end of string before end of value\n");
      *last_offset = new_start_offset; return 0; }
    // looks ok
    //printf("got value complete\n");
    ptr++;
    *last_offset = (ptr-buffer);
    return (key_start-buffer);
  }
  //printf("json wonky, restart\n");
  // broken json, just continue from here
  return jsonStreamKeyValues(NULL, buffer, max_buffer, (ptr-buffer), last_offset);
}


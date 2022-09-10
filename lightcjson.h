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

/* lightcjson.h */
/* Lightweight JSON parser in C. */

#ifndef LIGHTCJSON_H
#define LIGHTCJSON_H

// just trim beginning / trailing unnecessary spaces
char *jsonTrim(const char *src, char *dest);

// remove all unnecessary spacing from a JSON string... preserving strings
char *jsonRemoveSpacing(const char *json, char *dest);

// Get an item from a JSON list
char *jsonIndexList(const char *json, int index, char *dest, int size);

// extract a json component from JSON
char *jsonExtract(const char *json, const char *name, char *dest, int size);

// escape/unescape a json value
char *jsonEscape(const char *input, char *dest, int size);
char *jsonUnescape(const char *json, char *dest, int size);

// create/parse a JSON quote-string
char *jsonQuote(const char *input, char *dest, int size);
char *jsonUnquote(const char *input, char *dest, int size);

#endif
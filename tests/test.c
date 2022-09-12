#include <stdio.h>
#include <string.h>
#include "lightcjson.h"

typedef char *((*functiontype3)(const char *, char *, int));

int test_jsonTrim();
int t_jsonTrim(char *input, char *expected);

int test_jsonRemoveSpacing();
int t_jsonRemoveSpacing(char *input, char *expected);

int test_jsonIndexList();
int t_jsonIndexList(char *input, int index, char *expected, int expect_null);

int test_jsonExtract();
int t_jsonExtract(char *input, char *key_name, char *expected, int expect_null);

int test_jsonEscape();
int test_jsonQuote();
int t_func(char *input, char *expected, functiontype3 f, char *name);

int test_jsonGetKeyValue();
int t_jsonGetKeyValue(char *input, char *expect_key, char *expect_value, int expect_null);

int test_jsonAppendItem();
int test_jsonStreamKeyValues();

int expect_num(int is, int expect, char *name);
int expect_str(char *is, char *expect, char *name);

int main() {
    printf("\nSimple tests of lightcjson\n\n");
    int fail=0;
    fail += test_jsonTrim();
    fail += test_jsonRemoveSpacing();
    fail += test_jsonIndexList();
    fail += test_jsonExtract();
    fail += test_jsonEscape();
    fail += test_jsonQuote();
    fail += test_jsonAppendItem();
    fail += test_jsonGetKeyValue();
    fail += test_jsonStreamKeyValues();

    printf("\nTests failed: %d\n", fail);
    return 0;
}



// returns offset to next key/value pair, or 0 for none remaining here.
// returns 0 when time to add new_input
// returns -1 on buffer overfill
// set start_offset = last_offset to continue parsing
// buffer should be >2x new_input size, >2x+5 max key+value size
// skips named structs ("key":{"item":"value"}), and lists. ("key":["a","b"])
//int jsonStreamKeyValues(const char *new_input, char *buffer, int max_buffer, 
//    int start_offset, int *last_offset) {

int test_jsonStreamKeyValues() {
    char buff[30], in[200];
    int pos, last_offset;
    int err=0;

    printf("jsonStreamKeyValues():\n");
    buff[0]='\0';
    strcpy(in, "{\"key\":1");
    printf("buf='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), 0, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 0, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "}");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 1, "return");
    err += expect_num(last_offset, 8, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "key", "1", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 9, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, ",\"1234567890abc");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 9, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "123");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 9, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "123\":2");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 2, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, ",");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 3, "return");
    err += expect_num(last_offset, 26, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "1234567890abc123123", "2", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 27, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "\"k\": \"v\"");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 18, "return");
    err += expect_num(last_offset, 26, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "k", "\"v\"", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 26, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, ",\"c\": {\"w\":\"v\"}");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 17, "return");
    err += expect_num(last_offset, 24, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "w", "\"v\"", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 25, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, ",\"a\":\"b\",\"c\":2,\"d");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 8, "return");
    err += expect_num(last_offset, 15, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "a", "\"b\"", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 16, "return");
    err += expect_num(last_offset, 21, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "c", "2", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 21, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "\",\"a\":[1,2],\"c\":2,\"d");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 15, "return");
    err += expect_num(last_offset, 20, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "c", "2", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 20, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "\",\"1234567890abc");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 7, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "1234567890abc");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, -1, "return");
    err += expect_num(last_offset, 7, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "\":123");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 7, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, "1234");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 2, "last_offset");
    if (err) return err;

    printf("\n");
    strcpy(in, ",");
    printf("buf+='%s'\n", in);
    pos = jsonStreamKeyValues(in, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 3, "return");
    err += expect_num(last_offset, 26, "last_offset");
    err += t_jsonGetKeyValue(buff+pos, "1234567890abc", "1231234", 0);
    if (err) return err;

    printf("\n");
    pos = jsonStreamKeyValues(NULL, buff, sizeof(buff), last_offset, &last_offset);
    err += expect_num(pos, 0, "return");
    err += expect_num(last_offset, 27, "last_offset");
    if (err) return err;

    printf("Tests failed: %d\n\n", err);

    return err;
}

int expect_num(int is, int expect, char *name) {
    if (expect != is) {
        printf("  %s: expected %d, was %d\n", name, expect, is);
        return 1;
    }
    return 0;
}

int expect_str(char *is, char *expect, char *name) {
    if (strcmp(expect,is)!=0) {
        printf("  %s: expected %s, was %s\n", name, expect, is);
        return 1;
    }
    return 0;
}


int test_jsonGetKeyValue() {
    int run=0, fail=0;

    run++; fail+=t_jsonGetKeyValue("", "", "", 1);
    run++; fail+=t_jsonGetKeyValue("abc", "", "", 1);
    run++; fail+=t_jsonGetKeyValue("\"key\":", "key", "", 0);
    run++; fail+=t_jsonGetKeyValue("\"key\": ", "key", "", 0);
    run++; fail+=t_jsonGetKeyValue("\"k\":0", "k", "0", 0);
    run++; fail+=t_jsonGetKeyValue("\"k\":1,", "k", "1", 0);
    run++; fail+=t_jsonGetKeyValue("\"k\":    1,", "k", "1", 0);
    run++; fail+=t_jsonGetKeyValue("\"k\":\"1\",", "k", "\"1\"", 0);
    run++; fail+=t_jsonGetKeyValue("\"k\":\"1\",\"c\":2", "k", "\"1\"", 0);
 
    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;

}

int t_jsonGetKeyValue(char *input, char *expect_key, char *expect_value, int expect_null) {
    //int jsonGetKeyValue(const char *input, char *key, char *value, int item_size) {
    char buff_key[1024];
    char buff_val[1024];
    int err = 0, ret = 0;

    //int jsonGetKeyValue(const char *input, char *key, char *value, int item_size) {
    printf("jsonGetKeyValue(%s):", input);
    ret = jsonGetKeyValue(input, buff_key, buff_val, sizeof(buff_key));
    if (expect_null) {
        printf("  is: %s - expected: %s\n", (ret==0)?"0":"NOT-0", "0");
        if (ret!=0) {
            printf("  FAILED: result code mismatch\n"); err++;
        }
    } else {
        printf("  is: %s=%s - expected: %s=%s\n", 
            buff_key, buff_val, expect_key, expect_value);
        if (ret != 1) {
            printf("  FAILED: result code mismatch\n"); err++;
        }
        if (strcmp(buff_key, expect_key)!=0) {
            printf("  FAILED: result key mismatch\n"); err++;
        }
        if (strcmp(buff_val, expect_value)!=0) {
            printf("  FAILED: result value mismatch\n"); err++;
        }
    }
    return err;
}

//char *jsonAppendItem(const char *key, const char *value, char *dest, int size);
int test_jsonAppendItem() {
    int fail = 0;

    char buff[1024];
    char exp[1024];
    buff[0] = '\0';
    printf("jsonAppendItem()\n");
    jsonAppendItem("key", "value", buff, sizeof(buff));
    sprintf(exp, "%s", "{\"key\":\"value\"}");
    printf("  is: '%s', expected: '%s'\n", buff, exp);
    if (strcmp(buff, exp) != 0) {
        fail++; printf("  FAIL\n");
    }

    jsonAppendItem("key2", "val\"ue2", buff, sizeof(buff));
    sprintf(exp, "%s", "{\"key\":\"value\",\"key2\":\"val\\\"ue2\"}");
    printf("  is: '%s', expected: '%s'\n", buff, exp);
    if (strcmp(buff, exp) != 0) {
        fail++; printf("  FAIL\n");
    }
    return fail;
}


//char *jsonEscape(const char *input, char *dest, int size);
//char *jsonUnescape(const char *json, char *dest, int size);
int test_jsonEscape() {
    int run=0, fail=0;

    run++; fail+=t_func("", "", &jsonEscape, "jsonEscape");
    run++; fail+=t_func("a", "a", &jsonEscape, "jsonEscape");
    run++; fail+=t_func("abc", "abc", &jsonEscape, "jsonEscape");
    run++; fail+=t_func("a\"bc", "a\\\"bc", &jsonEscape, "jsonEscape");

    run++; fail+=t_func("", "", &jsonUnescape, "jsonUnescape");
    run++; fail+=t_func("a\"bc", "a\"bc", &jsonUnescape, "jsonUnescape");
    run++; fail+=t_func("a\\\"bc", "a\"bc", &jsonUnescape, "jsonUnescape");

    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;
}


//char *jsonQuote(const char *input, char *dest, int size);
//char *jsonUnquote(const char *input, char *dest, int size);
int test_jsonQuote() {
    int run=0, fail=0;

    run++; fail+=t_func("", "\"\"", &jsonQuote, "jsonQuote");
    run++; fail+=t_func("a", "\"a\"", &jsonQuote, "jsonQuote");
    run++; fail+=t_func("ab", "\"ab\"", &jsonQuote, "jsonQuote");
    run++; fail+=t_func("a\"b", "\"a\\\"b\"", &jsonQuote, "jsonQuote");

    run++; fail+=t_func("", "", &jsonUnquote, "jsonUnquote");
    run++; fail+=t_func("\"\"", "", &jsonUnquote, "jsonUnquote");
    run++; fail+=t_func("\"a\"", "a", &jsonUnquote, "jsonUnquote");
    run++; fail+=t_func("\"a\\\"b\"", "a\"b", &jsonUnquote, "jsonUnquote");

    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;
}


int t_func(char *input, char *expected, functiontype3 func, char *name) {
    char *ptr;
    char buff_in[1024];
    char buff_out[1024];
    int err = 0;
    strncpy(buff_in, input, sizeof(buff_in)-1);
    buff_in[sizeof(buff_in)-1] = 0;
    memset(buff_out, 32, sizeof(buff_out)-1);
    buff_out[sizeof(buff_out)-1] = 0;

    printf("%s(%s):", name, buff_in);
    ptr = func(buff_in, buff_out, sizeof(buff_out));

    printf("  is: %s - expected: %s\n", buff_out, expected);
    if (ptr != buff_out) {
        printf("  FAILED: result code mismatch\n"); err++;
    }
    if (strcmp(buff_out, expected)!=0) {
        printf("  FAILED: result value mismatch\n"); err++;
    }
    return err;
}


int test_jsonExtract() {
    int run=0, fail=0;

    run++; fail+=t_jsonExtract("", "", "", 1);
    run++; fail+=t_jsonExtract("", "key", "", 1);
    run++; fail+=t_jsonExtract("\"key\":", "key", "", 0);
    run++; fail+=t_jsonExtract("\"key\": ", "key", "", 0);
    run++; fail+=t_jsonExtract("\"k\":0", "k", "0", 0);
    run++; fail+=t_jsonExtract("\"key\":0", "key", "0", 0);
    run++; fail+=t_jsonExtract("\"key\":1", "key", "1", 0);
    run++; fail+=t_jsonExtract("\"key\":9", "key", "9", 0);
    run++; fail+=t_jsonExtract("\"key\":123", "key", "123", 0);
    run++; fail+=t_jsonExtract("\"key\":  123", "key", "123", 0);
    run++; fail+=t_jsonExtract("\"key\":123 ", "key", "123", 0);
    run++; fail+=t_jsonExtract("\"key\":12.3", "key", "12.3", 0);
    run++; fail+=t_jsonExtract("\"key\":-1", "key", "-1", 0);
    run++; fail+=t_jsonExtract("\"key\":1-", "key", "1", 0);
    run++; fail+=t_jsonExtract("\"key\":1.", "key", "1.", 0);
    run++; fail+=t_jsonExtract("\"key\":.1", "key", "", 1);
    run++; fail+=t_jsonExtract("\"key\":\"\"", "key", "\"\"", 0);
    run++; fail+=t_jsonExtract("\"key\":\"a\"", "key", "\"a\"", 0);
    run++; fail+=t_jsonExtract("\"key\":\"ab\"", "key", "\"ab\"", 0);
    run++; fail+=t_jsonExtract("\"key\":\"a\\\"b\"", "key", "\"a\\\"b\"", 0);
    run++; fail+=t_jsonExtract("\"key\":{}", "key", "{}", 0);
    run++; fail+=t_jsonExtract("\"key\":[]", "key", "[]", 0);
    run++; fail+=t_jsonExtract("\"key\":[1]", "key", "[1]", 0);
    run++; fail+=t_jsonExtract("\"key\":[[]]", "key", "[[]]", 0);
    run++; fail+=t_jsonExtract("\"key\":[{}]", "key", "[{}]", 0);
    run++; fail+=t_jsonExtract("\"key\":[\"\"]", "key", "[\"\"]", 0);
    run++; fail+=t_jsonExtract("\"key\":[\"{}\"]", "key", "[\"{}\"]", 0);
    run++; fail+=t_jsonExtract("\"key\":[\"]\"]", "key", "[\"]\"]", 0);
    run++; fail+=t_jsonExtract("\"key\":[\"a\\\"b\"]", "key", "[\"a\\\"b\"]", 0);
    run++; fail+=t_jsonExtract("\"key\":{\"key\":1}", "key", "{\"key\":1}", 0);
    run++; fail+=t_jsonExtract("\"abc\":{\"def\":1}", "def", "1", 0);
 
    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;

}

int t_jsonExtract(char *input, char *key_name, char *expected, int expect_null) {
    //char *jsonExtract(const char *json, const char *key_name, char *dest, int size) {
    char *ptr;
    char buff_in[1024];
    char buff_out[1024];
    int err = 0;
    strncpy(buff_in, input, sizeof(buff_in)-1);
    buff_in[sizeof(buff_in)-1] = 0;
    memset(buff_out, 32, sizeof(buff_out)-1);
    buff_out[sizeof(buff_out)-1] = 0;

    printf("jsonExtract(%s,%s):", buff_in, key_name);
    ptr = jsonExtract(buff_in, key_name, buff_out, sizeof(buff_out));
    if (expect_null) {
        printf("  is: %s - expected: %s\n", 
            (ptr==NULL)?"NULL":"NON-NULL", "NULL");
        if (ptr!=NULL) {
            printf("  FAILED: result code mismatch\n"); err++;
        }
    } else {
        printf("  is: %s - expected: %s\n", buff_out, expected);
        if (ptr != buff_out) {
            printf("  FAILED: result code mismatch\n"); err++;
        }
        if (strcmp(buff_out, expected)!=0) {
            printf("  FAILED: result value mismatch\n"); err++;
        }
    }
    return err;
}


int test_jsonTrim() {
    int run=0, fail=0;

    run++; fail+=t_jsonTrim("", "");
    run++; fail+=t_jsonTrim(" ", "");
    run++; fail+=t_jsonTrim(" \t\r", "");
    run++; fail+=t_jsonTrim("1", "1");
    run++; fail+=t_jsonTrim(" 1", "1");
    run++; fail+=t_jsonTrim("1 ", "1");
    run++; fail+=t_jsonTrim(" 1 ", "1");
    run++; fail+=t_jsonTrim("12", "12");
    run++; fail+=t_jsonTrim("123", "123");
    run++; fail+=t_jsonTrim(" 123", "123");
    run++; fail+=t_jsonTrim("123 ", "123");
    run++; fail+=t_jsonTrim(" 123 ", "123");
    run++; fail+=t_jsonTrim("  123  ", "123");
    run++; fail+=t_jsonTrim("\t123\r", "123");
    run++; fail+=t_jsonTrim("\n123", "123");
    run++; fail+=t_jsonTrim(" 1 2 3 ", "1 2 3");

    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;
}

int t_jsonTrim(char *input, char *expected) {
    //char *jsonTrim(const char *src, char *dest);
    char *ptr;
    char buff_in[1024];
    char buff_out[1024];
    int err = 0;
    strncpy(buff_in, input, sizeof(buff_in)-1);
    buff_in[sizeof(buff_in)-1] = 0;
    memset(buff_out, 32, sizeof(buff_out)-1);
    buff_out[sizeof(buff_out)-1] = 0;

    printf("jsonTrim(%s):", buff_in);
    ptr = jsonTrim(buff_in, buff_out);
    printf("  is: %s - expected: %s\n", buff_out, expected);
    if (ptr != buff_out) {
        printf("  FAILED: result code mismatch\n"); err++;
    }
    if (strcmp(buff_out, expected)!=0) {
        printf("  FAILED: result value mismatch\n"); err++;
    }
    return err;
}


int test_jsonIndexList() {
    int run=0, fail=0;

    run++;fail+=t_jsonIndexList("1", 0, "1", 0);
    run++;fail+=t_jsonIndexList("1,2", 0, "1", 0);
    run++;fail+=t_jsonIndexList("1,2", 1, "2", 0);
    run++;fail+=t_jsonIndexList("1,2,3", 0, "1", 0);
    run++;fail+=t_jsonIndexList("1,2,3", 1, "2", 0);
    run++;fail+=t_jsonIndexList("1,2,3", 2, "3", 0);
    run++;fail+=t_jsonIndexList("1,2,3", 3, "", 1);
    run++;fail+=t_jsonIndexList(" 1, 2, 3", 0, "1", 0);
    run++;fail+=t_jsonIndexList("  1, 2, 3", 0, "1", 0);
    run++;fail+=t_jsonIndexList("1, 2, 3", 0, "1", 0);
    run++;fail+=t_jsonIndexList("1, 2, 3", 1, "2", 0);
    run++;fail+=t_jsonIndexList("1, 2 , 3", 1, "2", 0);
    run++;fail+=t_jsonIndexList("1,2 , 3", 1, "2", 0);
    run++;fail+=t_jsonIndexList("1, 2, 3", 2, "3", 0);
    run++;fail+=t_jsonIndexList("1, 2, 3 ", 2, "3", 0);
    run++;fail+=t_jsonIndexList("12,23,34", 0, "12", 0);
    run++;fail+=t_jsonIndexList("12,23,34", 1, "23", 0);
    run++;fail+=t_jsonIndexList("12,23,34", 2, "34", 0);
    run++;fail+=t_jsonIndexList("[1,2,3]", 0, "[1,2,3]", 0);
    run++;fail+=t_jsonIndexList("[1,2,3],4", 0, "[1,2,3]", 0);
    run++;fail+=t_jsonIndexList("[1,2,3],4", 1, "4", 0);
    run++;fail+=t_jsonIndexList("[1,2,3],12", 1, "12", 0);
    run++;fail+=t_jsonIndexList("[1,2,3],123", 1, "123", 0);
    run++;fail+=t_jsonIndexList("4,[1,2,3]", 0, "4", 0);
    run++;fail+=t_jsonIndexList("4,[1,2,3]", 1, "[1,2,3]", 0);
    run++;fail+=t_jsonIndexList("{1,2,3}", 0, "{1,2,3}", 0);
    run++;fail+=t_jsonIndexList("{1,2,3},4", 0, "{1,2,3}", 0);
    run++;fail+=t_jsonIndexList("{1,2,3},4", 1, "4", 0);
    run++;fail+=t_jsonIndexList("4,{1,2,3}", 0, "4", 0);
    run++;fail+=t_jsonIndexList("4,{1,2,3}", 1, "{1,2,3}", 0);
    run++;fail+=t_jsonIndexList("[a,b],{1,2,3}", 0, "[a,b]", 0);
    run++;fail+=t_jsonIndexList("[a,b],{1,2,3}", 1, "{1,2,3}", 0);

    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;
}

int t_jsonIndexList(char *input, int index, char *expected, int expect_null) {
    //char *jsonIndexList(const char *json, int index, char *dest, int size) {
    char *ptr;
    char buff_in[1024];
    char buff_out[1024];
    int err = 0;

    strncpy(buff_in, input, sizeof(buff_in)-1);
    buff_in[sizeof(buff_in)-1] = 0;
    memset((void *)buff_out, 32, sizeof(buff_out)-1);

    printf("jsonIndexList(%s, %d): ", buff_in, index);
    ptr = jsonIndexList(buff_in, index, buff_out, sizeof(buff_out)-1);
    if (expect_null) {
        printf("  is: %s - expected: %s\n", 
            (ptr==NULL)?"NULL":"NON-NULL", "NULL");
        if (ptr!=NULL) {
            printf("  FAILED: result code mismatch\n"); err++;
        }
    } else {
        printf("  is: %s - expected: %s\n", buff_out, expected); 
        if (ptr != buff_out) {
            printf("  FAILED: result code mismatch\n"); err++;
        }
        if (strcmp(buff_out, expected)!=0) {
            printf("  FAILED: result value mismatch\n"); err++;
        }
    }
    return err;
}


int test_jsonRemoveSpacing() {
    int run=0, fail=0;
    
    run++;fail+=t_jsonRemoveSpacing("{JSON FILE HERE}", "{JSONFILEHERE}");
    run++;fail+=t_jsonRemoveSpacing("{\"JSON FILE HERE\"}", "{\"JSON FILE HERE\"}");
    run++;fail+=t_jsonRemoveSpacing("{\"JSON \\\"FILE HERE\"}", "{\"JSON \\\"FILE HERE\"}");
    run++;fail+=t_jsonRemoveSpacing("{1, 2,3}", "{1,2,3}");
    run++;fail+=t_jsonRemoveSpacing("[1, 2,3]", "[1,2,3]");
    run++;fail+=t_jsonRemoveSpacing("[1, \"key\":12,3]", "[1,\"key\":12,3]");
    run++;fail+=t_jsonRemoveSpacing("[1, \"key\": 12,3]", "[1,\"key\":12,3]");
    run++;fail+=t_jsonRemoveSpacing("[1, \"key\":12 ,3]", "[1,\"key\":12,3]");
    run++;fail+=t_jsonRemoveSpacing("[1, \"key\":12, 3]", "[1,\"key\":12,3]");
    run++;fail+=t_jsonRemoveSpacing(" {1,2,3}", "{1,2,3}");
    run++;fail+=t_jsonRemoveSpacing(" {1,2,3} ", "{1,2,3}");
    run++;fail+=t_jsonRemoveSpacing("{1,2,3}  ", "{1,2,3}");
    run++;fail+=t_jsonRemoveSpacing("{1,2,\t3}", "{1,2,3}");
    run++;fail+=t_jsonRemoveSpacing("{1,2,\r3}", "{1,2,3}");
    run++;fail+=t_jsonRemoveSpacing("{1,2,\n3}", "{1,2,3}");

    printf("Tests run: %d, failed: %d\n\n", run, fail);
    return fail;
}

int t_jsonRemoveSpacing(char *input, char *expected) {
    char *ptr;
    char buff[1024];
    int err = 0;
    strncpy(buff, input, sizeof(buff)-1);
    buff[sizeof(buff)-1] = 0;

    printf("jsonRemoveSpacing(%s):", buff);
    ptr = jsonRemoveSpacing(buff, buff);
    printf("  is: %s - expected: %s\n", buff, expected);
    if (ptr != buff) {
        printf("  FAILED: result code mismatch\n"); err++;
    }
    if (strcmp(buff, expected)!=0) {
        printf("  FAILED: result value mismatch\n"); err++;
    }
    return err;
}
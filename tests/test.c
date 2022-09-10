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

int main() {
    printf("\nSimple tests of lightcjson\n\n");
    int fail=0;
    fail += test_jsonTrim();
    fail += test_jsonRemoveSpacing();
    fail += test_jsonIndexList();
    fail += test_jsonExtract();
    fail += test_jsonEscape();
    fail += test_jsonQuote();

    printf("\nTests failed: %d\n", fail);
    return 0;
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
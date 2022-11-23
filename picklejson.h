#ifndef PICKLEJSON_H__
#define PICKLEJSON_H__

#include <stddef.h> /* size_t */

#define pickle_set_null(v) pickle_free(v)

#define pickle_init(v) do { (v)->type = PICKLE_NULL; } while(0)

typedef enum{PICKLE_NULL, PICKLE_FALSE, PICKLE_TRUE, PICKLE_NUMBER, PICKLE_STRING, PICKLE_ARRAY, PICKLE_OBJECT} pickle_type;

/*
 * 结点
 */
typedef struct {
    union{
        struct{
            char* s;
            size_t  len;
        } s;                            /* string */
        double n;                       /* number */
    } u;
    pickle_type type;
}pickle_value;

/*
 * 分析返回结果枚举
 */
enum {
    PICKLE_PARSE_OK = 0,
    PICKLE_PARSE_EXPECT_VALUE,
    PICKLE_PARSE_INVALID_VALUE,
    PICKLE_PARSE_ROOT_NOT_SINGULAR,
    PICKLE_PARSE_NUMBER_TOO_BIG,
    PICKLE_PARSE_MISS_QUOTATION_MARK,
    PICKLE_PARSE_INVALID_STRING_ESCAPE,
    PICKLE_PARSE_INVALID_STRING_CHAR
};
void pickle_free(pickle_value* v);

int pickle_parse(pickle_value* v, const char* json);

pickle_type  pickle_get_type(const pickle_value* v);

void pickle_set_string(pickle_value* v, const char* s, size_t size);
const char* pickle_get_string(const pickle_value* v);
size_t pickle_get_string_len(const pickle_value* v);

int pickle_get_boolean(const pickle_value* v);
void pickle_set_boolean(pickle_value* v, int b);

void pickle_set_number(pickle_value* v,double n);
double pickle_get_number(const pickle_value* v);
#endif

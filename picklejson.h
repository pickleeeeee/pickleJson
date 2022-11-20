#ifndef PICKLEJSON_H__
#define PICKLEJSON_H__

typedef enum{PICKLE_NULL, PICKLE_FALSE, PICKLE_TRUE, PICKLE_NUMBER, PICKLE_STRING, PICKLE_ARRAY, PICKLE_OBJECT} pickle_type;

typedef struct {
    double n;
    pickle_type type;
}pickle_value;

enum {
    PICKLE_PARSE_OK = 0,
    PICKLE_PARSE_EXPECT_VALUE,
    PICKLE_PARSE_INVALID_VALUE,
    PICKLE_PARSE_ROOT_NOT_SINGULAR,
    PICKLE_PARSE_NUMBER_TOO_BIG
};

int pickle_parse(pickle_value* v, const char* json);

pickle_type  pickle_get_type(const pickle_value* v);

//当且仅当pickle_type是PICKLE_NUMBER的时候,n才表示json的数字的数值
double pickle_get_number(const pickle_value* v);

#endif

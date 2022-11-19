#ifndef PICKLEJSON_H__
#define PICKLEJSON_H__

typedef enum{PICKLE_NULL, PICKLE_FALSE, PICKLE_TRUE, PICKLE_NUMBER, PICKLE_STRING, PICKLE_ARRAY, PICKLE_OBJECT} pickle_type;

typedef struct {
    pickle_type type;
}pickle_value;

enum {
    PICKLE_PARSE_OK = 0,
    PICKLE_PARSE_EXPECT_VALUE,
    PICKLE_PARSE_INVALID_VALUE,
    PICKLE_PARSE_ROOT_NOT_SINGULAR
};

int pickle_parse(pickle_value* v, const char* json);

pickle_type  pickle_get_type(const pickle_value* v);

#endif

//*******************************
// Created by Pickle on 2022/11/19.
//*******************************
#include "picklejson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* errno, NULL */
#include <math.h>   /* ERANGE, HUGE_VAL */
#include <string.h> /* memcpy() */

#define EXPECT(c, ch)    do { assert(*c->json==(ch)); c->json++; } while(0)
#define ISDIGIT(ch)      ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)      ((ch) >= '1' && (ch) <= '9')

typedef struct{
    const char* json;
    char* stack;
    size_t size,top;
}pickle_context;


/**
 * 删除字符串之前的空白
 * @param c
 */
static void pickle_parse_whitespace(pickle_context* c){
    const char* p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int pickle_parse_literal(pickle_context* c, pickle_value* v,const char* literal,pickle_type type){
    size_t i;
    EXPECT(c,literal[0]);
    for(i=0; literal[i + 1]; i++)
        if(c->json[i] != literal[i + 1])
            return PICKLE_PARSE_INVALID_VALUE;
    c->json+=i;
    v->type = type;
    return PICKLE_PARSE_OK;
}

static int pickle_parse_number(pickle_context* c,pickle_value* v){
    const char* p = c->json;
    if(*p == '-') p++;
    if(*p == '0') p++;
    else{
        if(!ISDIGIT1TO9(*p)) return PICKLE_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p); p++);
    }
    if(*p == '.'){
        p++;
        if(!ISDIGIT(*p)) return PICKLE_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p);p++);
    }
    if(*p == 'e' || *p == 'E'){
        p++;
        if(*p == '+' || *p == '-') p++;
        if(!ISDIGIT(*p)) return PICKLE_PARSE_INVALID_VALUE;
        for(p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->u.n = strtod(c->json,NULL);
    if(errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return PICKLE_PARSE_NUMBER_TOO_BIG;
    v->type = PICKLE_NUMBER;
    c->json = p;
    return PICKLE_PARSE_OK;
}

static int pickle_parse_value(pickle_context* c, pickle_value* v){
    switch (*c->json) {
        case 'n':   return pickle_parse_literal(c,v,"null",PICKLE_NULL);
        case 't':   return pickle_parse_literal(c,v,"true",PICKLE_TRUE);
        case 'f':   return pickle_parse_literal(c,v,"false",PICKLE_FALSE);
        default:    return pickle_parse_number(c,v);
        case '\0':  return PICKLE_PARSE_EXPECT_VALUE;
    }
}

int pickle_parse(pickle_value* v, const char* json){
    pickle_context c;
    int ret = 0;
    assert(v != NULL);
    c.json = json;
    //初始化栈区
    c.stack = NULL;
    c.size = c.top = 0;
    //如果解析失败v将会返回NULL值，函数中先将v设置成了NULL值
    pickle_init(v);
    //移除字符前的空白
    pickle_parse_whitespace(&c);
    //只有再成功解析后才分析是不是又多个root
    if((ret = pickle_parse_value(&c,v)) == PICKLE_PARSE_OK){
        pickle_parse_whitespace(&c);
        if(c.json[0]!='\0')
            ret = PICKLE_PARSE_ROOT_NOT_SINGULAR;
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

pickle_type pickle_get_type(const pickle_value* v){
    assert(v != NULL);
    return v->type;
}

/**
 * 释放节点字符数组所占的内存空间
 * @param v
 */
void pickle_free(pickle_value* v){
    assert(v != NULL);
    if(v->type == PICKLE_STRING)
        //如果是string类型，直接释放s指向的字符数组空间
        free(v->u.s.s);
    v->type = PICKLE_NULL;
}

/**
 * @param v 存储字符的结点
 * @param s 被保存的字符数组
 * @param len 字符数组长度
 */
void pickle_set_string(pickle_value* v,const char* s, size_t len){
    assert(v != NULL && (s != NULL || len == 0));
    //释放字符数组内存
    pickle_free(v);
    v->u.s.s = (char*) malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = PICKLE_STRING;
}



#ifndef PICKLE_PARSE_STACK_INIT_SIZE
#define PICKLE_PARSE_STACK_INTI_SIZE 256
#endif
//缓冲区的push操作
static void* pickle_context_push(pickle_context* c, size_t size){
    void* ret;
    assert(size > 0);
    //栈空间扩容
    if(c->top + size >= c->size){
        if(c->size == 0)
            //如果当前的栈空间大小是0则初始化缓冲区大小
            c->size = PICKLE_PARSE_STACK_INTI_SIZE;
        while(c->top + size >= c->size)
            //每次扩容1.5倍
            c->size += c->size >> 1;
        c->stack = realloc(c->stack, c->size);
    }
    //返回当前栈顶指针
    ret = c->stack + c->top;
    //移动栈顶位置
    c->top += size;
    return ret;
}
//缓冲区的pop操作
static void* pickle_context_pop(pickle_context* c,size_t size){
    assert(c->top >= size);
    return  c->stack + (c->top -= size);
}

//字符一个一个的压栈
#define PUTC(c, ch) do{ *(char*)pickle_context_push(c, sizeof(char)) = (ch); } while(0)

static int pickle_parse_string(pickle_context* c, pickle_value* v){
    //备份栈顶，也就是一个待解析字符串最开始的位置
    size_t head = c->top,len;
    const char* p;
    //字符串应该以"开头
    EXPECT(c,'\"');
    //执行完EXPECT之后，指针已经移到"后面的字符上了
    p = c ->json;
    for(;;){
        //ch = *p; ++
        char ch = *p++;
        switch(ch){
            //再次遇到"直接代表字符串结束了
            case '\"':
                len = c->top - head;
                pickle_set_string(v, pickle_context_pop(c,len),len);
                //c->json指向后续没有处理完的字符串
                c->json = p;
                return PICKLE_PARSE_OK;
            case '\0':
                //恢复栈顶
                c->top = head;
                return PICKLE_PARSE_MISS_QUOTATION_MARK;
            default:
                PUTC(c,ch);
        }
    }
}


int pickle_get_boolean(const pickle_value* v){
    assert(v != NULL && (v->type == PICKLE_TRUE || v->type == PICKLE_FALSE));
    return v->type;
}



void pickle_set_number(pickle_value* v, double n){

}
double pickle_get_number(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_NUMBER);
    return v->u.n;
}

const char* pickle_get_string(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_STRING);
    return v->u.s.s;
}
size_t pickle_get_len(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_STRING);
    return v->u.s.len;
}
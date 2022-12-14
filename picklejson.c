//*******************************
// Created by Pickle on 2022/11/19.
//*******************************
#include "picklejson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* errno, NULL */
#include <math.h>   /* ERANGE, HUGE_VAL */
#include <string.h> /* memcpy() */
#include <ctype.h> /* isspace */

//**********************************************************************
/*
 * 宏定义
 */

/*
 * 字符串解析失败后返回操作
 */
#define STRING_ERROR(ret) do { c-> top = head; return ret; } while(0)

/*
 * 缓冲区扩充默认大小
 */
#ifndef PICKLE_PARSE_STACK_INIT_SIZE
#define PICKLE_PARSE_STACK_INTI_SIZE 256
#endif
/*
 * 一个字符的压栈
 */
#define PUTC(c, ch) do{ *(char*)pickle_context_push(c, sizeof(char)) = (ch); } while(0)
/*
 * 检查第一个字符并且<指针后移>
 */
#define EXPECT(c, ch)    do { assert(*c->json==(ch)); c->json++; } while(0)
/*
 *  0-9字符
 */
#define ISDIGIT(ch)      ((ch) >= '0' && (ch) <= '9')
/*
 * 1-9字符
 */
#define ISDIGIT1TO9(ch)      ((ch) >= '1' && (ch) <= '9')

/*
 * 空格
 */
#define ISSPACE(ch)         ((ch) == ' ')

/*
 * 小数点
 */
#define  ISDOT(ch)          ((ch) == '.')
//**********************************************************************
/*
 * 数据结构
 */

/*
 * 定义存放json文本，以及json解析字符放入的栈区以及栈顶，栈大小
 */
typedef struct{
    const char* json;
    char* stack;
    size_t size,top;
}pickle_context;

//**********************************************************************
/*
 * 解析字符串用到的缓冲区操作函数
 */

/**
 * 缓冲区的push操作
 * @param c
 * @param size
 * @return
 */
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
/**
 * 缓冲区的pop操作
 * @param c
 * @param size
 * @return
 */
static void* pickle_context_pop(pickle_context* c,size_t size){
    assert(c->top >= size);
    return  c->stack + (c->top -= size);
}

//**********************************************************************
/*
 * 功能函数
 */

/**
 * 释放<节点>字符数组所占的内存空间,并且把<节点>的类型设置为PICKLE_NULL
 * 递归调用释放数组中的元素，最后再释放本身的v->u.a.e
 * @param v
 */
void pickle_free(pickle_value* v){
    size_t i;
    assert(v != NULL);
    switch(v->type){
        case PICKLE_STRING:
            free(v->u.s.s);
            break;
        case PICKLE_ARRAY:
            //如果当前是一个数组，遍历当前数组内所有的元素
            for(i = 0; i < v->u.a.size;i++){
                pickle_free(&v->u.a.e[i]);
            }
            free(v->u.a.e);
            break;
        default:
            break;
    }
    v->type = PICKLE_NULL;
}
/**
 * 删除字符串之前的空白
 * @param c
 */
static void pickle_parse_whitespace(pickle_context* c){
    const char* p = c->json;
//    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    while(isspace(*p))
        p++;
    c->json = p;
}
/**
 * 解析固定字符性--false、true、null
 * @param c
 * @param v
 * @param literal
 * @param type
 * @return
 */
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
/**
 * 解析实数
 * @param c
 * @param v
 * @return
 */
static int pickle_parse_number(pickle_context* c,pickle_value* v){
    const char* p = c->json;
    if(*p == '-') p++;
    if(*p == '0'){
        //0开始的字符后面只能是空格或者小数点或者结束标志
        p++;
        pickle_parse_whitespace(c);
        if(!ISSPACE(*p) && !ISDOT(*p) && *p != '\0')
            return PICKLE_PARSE_ROOT_NOT_SINGULAR;
    }
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

/**
 * 用一个unsigned int类型的u的前16个位存放4个十六进制数
 * @param p 待转换的十六进制字符串
 * @param u 保存转换结果 四个字节
 * @return
 */
static const char* pickle_parse_hex4(const char* p, unsigned* u){
    int i;
    *u = 0;
	//u是三十二位，可存放四位10进制数字
    //example:00A2
    for(i = 0; i < 4; i++){
        char ch = *p++;
        //相当于*16
        *u <<= 4;
        if(ch >= '0' && ch <= '9') *u |= ch - '0';
        else if(ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
        else if(ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
        else
            return NULL;
    }
    return p;
}

/**
 * 将码点转换成UTF-8形式存储
 * @param c
 * @param u
 */
static void pickle_encode_utf8(pickle_context* c,unsigned u){
    if(u <= 0x7F)/* 一个字节 */
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF){/* 两个字节 */
        PUTC(c, 0xc0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | ( u       & 0x3F));
    }else if(u <= 0xFFFF){/* 三个字节 */
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6 ) & 0x3F));
        PUTC(c, 0x80 | ((u        & 0x3F)));
    }else{/* 四个字节 */
        assert(u <= 0x10FFFF);
        PUTC(c,0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
}

/**
 * 解析字符串
 * unescaped = %x20-21 / %x23-5B / %x5D-10FFFF
 * @param c
 * @param v
 * @return
 */
static int pickle_parse_string(pickle_context* c, pickle_value* v){
    //备份栈顶，也就是一个待解析字符串最开始的位置
    size_t head = c->top,len;
    //存储码点的方式用UTF-8，UTF-8最大四个字节正好和unsigned int 大小一致，所以用unsigned存储码点
    unsigned u;
    //用于低代理的范围检测
    unsigned u2;
    const char* p;
    //   "开头
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
            case '\\':
                switch(*p++){
                    case '\"':  PUTC(c,'\"');   break;
                    case '\\':  PUTC(c,'\\');   break;
                    case '/':  PUTC(c,'/');     break;
                    case 'b':  PUTC(c,'\b');    break;
                    case 'f':  PUTC(c,'\f');    break;
                    case 'n':   PUTC(c,'\n');   break;
                    case 'r':  PUTC(c,'\r');    break;
                    case 't':  PUTC(c,'\t');    break;
                    case 'u':
                        if(!(p = pickle_parse_hex4(p, &u)))
                            STRING_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX);
                        if( u >= 0xD800 && u <= 0xDBFF){/* surrogate pair */
                            if(*p++ != '\\')
                                STRING_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE);
                            if(*p++ != 'u')
                                STRING_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE);
                            //解析低代理的四位二进制字符串
                            if(!(p = pickle_parse_hex4(p, &u2)))
                                STRING_ERROR(PICKLE_PARSE_INVALID_UNICODE_HEX);
                            if(u2 < 0xDC00 || u2 > 0xDFFF)/* 低代理范围不合法，直接返回 */
                                STRING_ERROR(PICKLE_PARSE_INVALID_UNICODE_SURROGATE);
                            //高低代理转换成码点公式
                            //codepoint = 0x10000 + (H − 0xD800) × 0x400 + (L − 0xDC00)
                            //因为高代理左移了十位，所以这里|代替+
                            u = (((u-0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        //码点转换成UTF-8存储
                        pickle_encode_utf8(c,u);
                        break;
                    default:
                        STRING_ERROR(PICKLE_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                //恢复栈顶
                STRING_ERROR(PICKLE_PARSE_MISS_QUOTATION_MARK);
            default:
                //小于0x20的都是非打印字符，非法字符串，“和\都已经处理了，空格字符将会被去空格函数删掉
                if ((unsigned char)ch < 0x20) {
                    STRING_ERROR(PICKLE_PARSE_INVALID_STRING_CHAR);
                }
                PUTC(c,ch);
        }
    }
}

//前向声明
static int pickle_parse_value(pickle_context* c, pickle_value* v);
/**
 * 递归实现数组解析
 * @param c 提供堆栈操作，和原始字符串
 * @param v 最后返回结果
 * @return
 */
static int pickle_parse_array(pickle_context* c, pickle_value* v){
    int i;
    //记录数组元素个数
    size_t size = 0;
    EXPECT(c,'[');
    //消除空格 防止[     ] 这种情况
    pickle_parse_whitespace(c);
    int ret;
    //遇到后括号直接返回
    if(*c -> json == ']'){
        c->json++;
        v->type = PICKLE_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return PICKLE_PARSE_OK;
    }
    //该循环负责不断解析数组中的元素
    for(;;){
        //存放下一层解析的单个元素
        pickle_value e;
        pickle_init(&e);
        //消除空格
        pickle_parse_whitespace(c);
        if((ret = pickle_parse_value(c, &e) != PICKLE_PARSE_OK))
            break;
        //将解析到的单个元素入栈
        // *(pickle_value*)pickle_context_push(c,sizeof(pickle_value)) = e;
        memcpy(pickle_context_push(c,sizeof(pickle_value)),&e,sizeof(pickle_value));

        //TODO 这里释放临时结点会导致字符串元素获取不到，因为释放会清空存放字符串的地址，导致之后访问不到字符串，只有确定该地址的字符串以后不用的时候再释放
        //这里的malloc的字符串空间应该保留到测试结束之后再释放掉
        //pickle_free(&e);

        size++;
        //防止元素后面由空格
        pickle_parse_whitespace(c);
        if(*c->json == ','){/* 跳过逗号 */
            c->json++;
        }else if(*c->json==']'){/* 检测到数组结束 */
            c->json++;
            v->type = PICKLE_ARRAY;
            //记录元素个数
            v->u.a.size = size;
            //计算所有结点所占的空间
            size *= sizeof(pickle_value);
            //分配空间并且堆栈中的元素出栈
            memcpy(v->u.a.e = (pickle_value*)malloc(size), pickle_context_pop(c,size),size);
            return PICKLE_PARSE_OK;
        }else{
            ret = PICKLE_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    //pop and free values on the stack
    for(i = 0; i < size; i++){
        pickle_free((pickle_value*)pickle_context_pop(c,sizeof(pickle_value)));
    }
    return ret;
}


/**
 * 解析json的逻辑判断
 * @param c
 * @param v
 * @return
 */
static int pickle_parse_value(pickle_context* c, pickle_value* v){
    switch (*c->json) {
        case 'n':   return pickle_parse_literal(c,v,"null",PICKLE_NULL);
        case 't':   return pickle_parse_literal(c,v,"true",PICKLE_TRUE);
        case 'f':   return pickle_parse_literal(c,v,"false",PICKLE_FALSE);
        case '\"':  return pickle_parse_string(c,v);
        case '[':   return pickle_parse_array(c,v);
        default:    return pickle_parse_number(c,v);
        case '\0':  return PICKLE_PARSE_EXPECT_VALUE;
    }
}
/**
 * 主解析函数
 * @param v
 * @param json
 * @return
 */
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





//**********************************************************************
/*
 * <结点>数据的对外访问接口
 */

/**
 * 获取当前的<结点>类型
 * @param v
 * @return
 */
pickle_type pickle_get_type(const pickle_value* v){
    assert(v != NULL);
    return v->type;
}

/**
 * 清空当前<结点>指向的字符空间，并申请新的内存空间，放入新的值，最后手动添加'\0'字符结束符
 * @param v 存储字符的结点
 * @param s 被保存的字符数组
 * @param len 字符数组长度，不包括结束符
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
/**
 * 如果当前<结点>是PICKLE_STRING类型，则获取当前<结点>指向的字符串；
 * @param v
 * @return
 */
const char* pickle_get_string(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_STRING);
    return v->u.s.s;
}
/**
 * 如果当前<结点>是PICKLE_STRING类型，获取当前<结点>指向的字符串的长度；
 * 不包括结束符
 * @param v
 * @return
 */
size_t pickle_get_string_len(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_STRING);
    return v->u.s.len;
}

/**
 * 设置当前<结点>boolean类型
 * PICKLE_FALSE--0
 * PICKLE_TRUE--1
 * @param v
 * @return
 */
int pickle_get_boolean(const pickle_value* v){
    assert(v != NULL && (v->type == PICKLE_TRUE || v->type == PICKLE_FALSE));
    return v->type == PICKLE_TRUE;
}
/**
 * 设置<结点>的boolean值
 * 0--PICKLE_FALSE
 * 非零--PICKLE_TRUE
 * @param v
 * @param b
 */
void pickle_set_boolean(pickle_value* v, int b){
    pickle_free(v);
    v->type = b ? PICKLE_TRUE : PICKLE_FALSE;
}


/**
 * 给当前<结点>设置实数值，并且type改为PICKLE_NUMBER
 * @param v
 * @param n
 */
void pickle_set_number(pickle_value* v, double n){
    pickle_free(v);
    v->u.n = n;
    v->type = PICKLE_NUMBER;
}
/**
 * 获取当前<结点>的实数值
 * @param v
 * @return
 */
double pickle_get_number(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_NUMBER);
    return v->u.n;
}

/**
 * @param v
 * @return 元素的个数
 */
size_t pickle_get_array_size(const pickle_value* v){
    assert(v != NULL && v->type == PICKLE_ARRAY);
    return v->u.a.size;
}
pickle_value* pickle_get_array_element(const pickle_value* v, size_t index){
    assert(v != NULL && v->type == PICKLE_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

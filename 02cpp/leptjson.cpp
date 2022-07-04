#include "leptjson.h"
#include <cassert>  /* assert() */
#include <cstdlib> /* exit() */
#include <cerrno> /* errno, ERANGE */

#define EXPECT(c, ch) \
    do {\
        assert(*c.json == (ch)); \
        ++c.json; \
    } while (0)

#define IS_DIGIT(ch) \
    ((ch) >= '0' && (ch) <= '9')
#define IS_DIGIT_NONZERO(ch) \
    ((ch) >= '1' && (ch) <= '9')

// JSON context; 
typedef struct 
{
    const char* json;
} LeptContext;

static void parseWs(LeptContext &c) 
{
    // cursor;
    const char *p = c.json;  
    // skip ws; 
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        ++p;
    // move cursor to the first non-ws char; 
    c.json = p;

    return; 
}
static int parseLiteral(LeptContext& c, Lept::Value *v, const char* stdtxt, Lept::Type type)
{
    EXPECT(c, stdtxt[0]); 
    unsigned int count = 1; 
    while (stdtxt[count] != '\0') 
    {
        if (c.json[count - 1] != stdtxt[count]) 
            return Lept::PARSE_INVALID_VALUE; 
            ++count; 
    }
    c.json += count - 1; 
    v->setType(type); 

    return Lept::PARSE_OK;
}
static int parseNumber(LeptContext& c, Lept::Value* v)
{
    const char* end = c.json;
    /* validate legal number */
#if 1 
    if (*end == '-')
        end += 1;

    if (!IS_DIGIT(*end))
        return Lept::PARSE_INVALID_VALUE;
    else if (IS_DIGIT_NONZERO(*end))
    {
        do
        {
            end += 1;
        } while (IS_DIGIT(*end));
    }
    else
        end += 1; 

    if (*end == '.')
    {
        if (!IS_DIGIT(*(end + 1)))
            return Lept::PARSE_INVALID_VALUE;
        do
        {
            end += 1;
        } while (IS_DIGIT(*end));
    }

    if (*end == 'e' || *end == 'E')
    {
        end += 1; 
        if (*end == '+' || *end == '-')
            end += 1; 
        if (!IS_DIGIT(*end))
            return Lept::PARSE_INVALID_VALUE;
        while (IS_DIGIT(*end))
            end += 1; 
    }
#endif
    errno = 0; 
    v->setType(Lept::Type::NUMBER); /* set type to make setNum available */
    v->setNum(strtod(c.json, nullptr)); 
    c.json = end; 

    if (errno = ERANGE && (v->getNum() == HUGE_VAL || v->getNum() == -HUGE_VAL))
    {
        v->setType(Lept::Type::NULLJSON);
        return Lept::PARSE_NUMBER_OVERFLOW;
    }
    return Lept::PARSE_OK;

    /* for debugging */
#if 0 
    return Lept::PARSE_INVALID_VALUE; 
#endif
}

// parse JSON value; 
static int parseValue(LeptContext &c, Lept::Value* v)
{
    switch (*c.json) 
    {
        case 'n': // meet NULL; 
            return parseLiteral(c, v, "null", Lept::Type::NULLJSON);
        case 'f': // meet FALSE; 
            return parseLiteral(c, v, "false", Lept::Type::FALSE);
        case 't': // meet TRUE; 
            return parseLiteral(c, v, "true", Lept::Type::TRUE);
        default: // number or invalid JSON context; 
            return parseNumber(c, v);
        case '\0': // reach the end of JSON context and have read nothing;
            return Lept::PARSE_EXPECT_VALUE; 
    }
}



/* Lept::Value */
// Constructor; 
Lept::Value::Value(void) : 
    m_type(Lept::Type::NULLJSON), 
    m_num(0.0)
{

}
// Destructor; 
Lept::Value::~Value(void)
{

}

Lept::Type Lept::Value::getType(void) const
{
    return this->m_type;
}
double Lept::Value::getNum(void) const
{
    assert(this->getType() == Lept::Type::NUMBER); 

    return this->m_num; 
}
void Lept::Value::setType(Lept::Type type)
{
    this->m_type = type; 

    return; 
}
void Lept::Value::setNum(double num)
{
    assert(this->getType() == Lept::Type::NUMBER);

    this->m_num = num; 

    return; 
}

// parse JSON context to tree context; 
int Lept::Value::parse(const char* json)
{
    LeptContext c; // create JSON context; 
    int ret = Lept::PARSE_OK; 

    assert(this != nullptr);

    c.json = json; // add to JSON context; 
    this->setType(Lept::Type::NULLJSON); // by default set NULL data; 
    parseWs(c); // skip the leading ws; 

    if ((ret = parseValue(c, this)) == Lept::PARSE_OK)
    {
        parseWs(c); 
        if (*c.json != '\0')
        {
            this->setType(Lept::Type::NULLJSON); 
            ret = Lept::PARSE_ROOT_NOT_SINGULAR;
        }
    }

    return ret;
}
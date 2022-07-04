#include "leptjson.h"
#include <cassert>  /* assert() */

#define EXPECT(c, ch) \
    do {\
        assert(*c.json == (ch)); \
        ++c.json; \
    } while (0)

#define parseLiteral(c, v, stdtxt, type) \
    do {\
        EXPECT(c, stdtxt[0]); \
        bool flag = true; \
        unsigned int count = 1; \
        while (stdtxt[count] != '\0') \
        {\
            if (c.json[count - 1] != stdtxt[count]) \
            {\
                flag = false; \
                break; \
            }\
            ++count; \
        }\
        \
        if (!flag)\
            return Lept::PARSE_INVALID_VALUE; \
        c.json += count - 1; \
        v->setType(type); \
        return Lept::PARSE_OK; \
    } while (0)

#define parseNull(c, v) \
    parseLiteral(c, v, "null", Lept::Type::NULLJSON)

#define parseFalse(c, v) \
    parseLiteral(c, v, "false", Lept::Type::FALSE)

#define parseTrue(c, v) \
    parseLiteral(c, v, "true", Lept::Type::TRUE)

// JSON context; 
typedef struct 
{
    const char* json;
}LeptContext;

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

#if 0
static int parseNull(LeptContext &c, Lept::Value* v)
{
    EXPECT(c, 'n'); // check if meet NULL; 
    if (c.json[0] != 'u' || c.json[1] != 'l' || c.json[2] != 'l')
        return Lept::PARSE_INVALID_VALUE;
    c.json += 3;
    v->setType(Lept::Type::NULLJSON); 

    return Lept::PARSE_OK;
}

static int parseFalse(LeptContext &c, Lept::Value* v)
{
    EXPECT(c, 'f'); // check if meet NULL; 
    if (c.json[0] != 'a' || c.json[1] != 'l' || c.json[2] != 's' || c.json[3] != 'e')
        return Lept::PARSE_INVALID_VALUE;
    c.json += 4;
    v->setType(Lept::Type::FALSE);

    return Lept::PARSE_OK; 
}

static int parseTrue(LeptContext &c, Lept::Value* v)
{
    EXPECT(c, 't'); // check if meet NULL; 
    if (c.json[0] != 'r' || c.json[1] != 'u' || c.json[2] != 'e')
        return Lept::PARSE_INVALID_VALUE;
    c.json += 3;
    v->setType(Lept::Type::TRUE);

    return Lept::PARSE_OK;
}
#endif

// parse JSON value; 
static int parseValue(LeptContext &c, Lept::Value* v)
{
    switch (*c.json) 
    {
        case 'n': // meet NULL; 
            parseNull(c, v);
        case 'f': // meet FALSE; 
            parseFalse(c, v);
        case 't': // meet TRUE; 
            parseTrue(c, v);
        case '\0': // reach the end of JSON context and have read nothing;
            return Lept::PARSE_EXPECT_VALUE; 
        default: // invalid JSON context; 
            return Lept::PARSE_INVALID_VALUE; 
    }
}



/* Lept::Value */
// Constructor; 
Lept::Value::Value(void) : 
    m_type(Lept::Type::NULLJSON)
{

}
// Destructor; 
Lept::Value::~Value(void)
{

}

Lept::Type Lept::Value::getType(void) const
{
    assert(this != nullptr);

    return this->m_type;
}
void Lept::Value::setType(Lept::Type type)
{
    assert(this != nullptr); 

    this->m_type = type; 

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
            ret = Lept::PARSE_ROOT_NOT_SINGULAR; 
    }

    return ret;
}
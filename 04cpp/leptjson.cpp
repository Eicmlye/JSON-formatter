#include "leptjson.h"
#include <cassert>  /* assert() */
#include <cstdlib> /* exit() */
#include <cerrno> /* errno, ERANGE */
#include <queue> /* std::queue<> */

#define EXPECT(c, ch) \
    do {\
        assert(*c.json == (ch)); \
        ++c.json; \
    } while (0)

#define IS_DIGIT(ch) \
    ((ch) >= '0' && (ch) <= '9')
#define IS_DIGIT_NONZERO(ch) \
    ((ch) >= '1' && (ch) <= '9')
#define IS_HEX(ch) \
    (IS_DIGIT(ch) || ((ch) >= 'A' && (ch) <= 'F') || ((ch) >= 'a' && (ch) <= 'f'))

// JSON context; 
typedef struct 
{
    const char* json = nullptr;
    std::queue<char>* que = nullptr; /* buffer for variable length type */
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
static unsigned long ch2hex(char ch)
{
    assert(IS_HEX(ch)); 

    if (IS_DIGIT(ch))
        return (unsigned long)(ch - '0');
    else if (ch <= 'F')
        return (unsigned long)(ch - 'A' + 10);
    else
        return (unsigned long)(ch - 'a' + 10);
}
static unsigned long str2hex(LeptContext& c, int &ret)
{
    unsigned long hex = 0;

    if (!IS_HEX(*c.json))
    {
        ret = Lept::PARSE_INVALID_UNICODE_HEX;
        return 0; 
    }
    hex += ch2hex(*c.json); 
    c.json += 1; 

    for (unsigned int index = 1; index < 4; ++index)
    {
        if (!IS_HEX(*c.json))
        {
            ret = Lept::PARSE_INVALID_UNICODE_HEX;
            return 0; 
        }

        hex *= 16;
        hex += ch2hex(*c.json);
        c.json += 1;
    }

    return hex; 
}
static int parseHex(LeptContext& c, int &ret)
{ 
    EXPECT(c, 'u');

    /* get code point */
    unsigned long hex = str2hex(c, ret);
    if (ret != Lept::PARSE_OK)
        return 0; 

    if (hex >= 0xD800 && hex <= 0xDBFF)
    {
        if (*c.json != '\\' || *(c.json + 1) != 'u')
        {
            ret = Lept::PARSE_INVALID_UNICODE_SURROGATE;
            return 0;
        }
        c.json += 2; 

        unsigned long lowhex = str2hex(c, ret);
        if (lowhex < 0xDC00 || lowhex > 0xDFFF)
        {
            ret = Lept::PARSE_INVALID_UNICODE_SURROGATE;
            return 0;
        }

        hex = 0x10000 + (hex - 0xD800) * 0x400 + (lowhex - 0xDC00); 
    }

    assert(hex >= 0x0000 && hex <= 0x10FFFF); 

    /* get unicode */
    if (hex < 0x0080) /* 0xxxxxxx */
    {
        c.que->push((uint8_t)hex); 
    }
    else if (hex < 0x0800) /* 110xxxxx 10xxxxxx */
    {
        /* 0xC0 = 11000000 */
        c.que->push(0xC0 | ((uint8_t)(hex >> 6) & 0xFF)); /* 0x80 = 10000000 */
        c.que->push(0x80 | ((uint8_t)(hex) & 0x3F)); /* 0x3F = 00111111 */
    }
    else if (hex < 0x10000) /* 1110xxxx 10xxxxxx 10xxxxxx */
    {
        /* 0xE0 = 11100000 */
        c.que->push(0xE0 | ((uint8_t)(hex >> 12) & 0xFF)); /* 0x80 = 10000000 */
        c.que->push(0x80 | ((uint8_t)(hex >> 6) & 0x3F)); /* 0x3F = 00111111 */
        c.que->push(0x80 | ((uint8_t)(hex) & 0x3F)); /* 0x3F = 00111111 */
    }
    else if (hex < 0x10FFFF) /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    {
        /* 0xF0 = 11110000 */
        c.que->push(0xF0 | ((uint8_t)(hex >> 18) & 0xFF)); /* 0x80 = 10000000 */
        c.que->push(0x80 | ((uint8_t)(hex >> 12) & 0x3F)); /* 0x3F = 00111111 */
        c.que->push(0x80 | ((uint8_t)(hex >> 6) & 0x3F)); /* 0x3F = 00111111 */
        c.que->push(0x80 | ((uint8_t)(hex) & 0x3F)); /* 0x3F = 00111111 */
    }

    return Lept::PARSE_OK; 
}
static int parseString(LeptContext& c, Lept::Value* v)
{
    EXPECT(c, '\"'); 

    /* validate string */
    while (*c.json != '\"')
    {
        int ret; /* for unicode parsing */

        if (*c.json == '\0') /* this is rational because \0 is represented by \u0000 in JSON context */
            return Lept::PARSE_MISSING_QUOTATION_MARK; 

        if (*c.json == '\\')
        { /* deal with escape characters */
            c.json += 1;
            
            switch (*c.json)
            {
            case '\"':
            case '/':
            case '\\':
                c.que->push(*c.json);
                c.json += 1;
                break;
            case 'b':
                c.que->push('\b');
                c.json += 1;
                break;
            case 'f':
                c.que->push('\f');
                c.json += 1;
                break;
            case 'n':
                c.que->push('\n');
                c.json += 1;
                break;
            case 'r':
                c.que->push('\r');
                c.json += 1;
                break;
            case 't':
                c.que->push('\t');
                c.json += 1;
                break;
            case 'u': /* Unicode UTF-8 */
                ret = Lept::PARSE_OK; 
                parseHex(c, ret);
                if (ret != Lept::PARSE_OK)
                    return ret; 
                break; 
            default:
                return Lept::PARSE_INVALID_STRING_ESCAPE;
            }
        }
        else if (*c.json >= '\x20') /* need NOT check " and \ here, because they have been blocked by if and while */
        { /* now for unescaped characters */
            c.que->push(*c.json);
            c.json += 1;
        }
        else
            return Lept::PARSE_INVALID_STRING_CHAR; 
    }
    c.json += 1; 

    /* load string to v */
    v->setType(Lept::Type::STRING); 
    while (!c.que->empty())
    {
        v->appendChar(c.que->front());
        c.que->pop(); 
    }

    return Lept::PARSE_OK; 
}

// parse JSON value; 
static int parseValue(LeptContext &c, Lept::Value* v)
{
    switch (*c.json) 
    {
        case 'n': // NULL; 
            return parseLiteral(c, v, "null", Lept::Type::NULLJSON);
        case 'f': // FALSE; 
            return parseLiteral(c, v, "false", Lept::Type::FALSE);
        case 't': // TRUE; 
            return parseLiteral(c, v, "true", Lept::Type::TRUE);
        default: // numbers or invalid JSON context; 
            return parseNumber(c, v);
        case '\"': // strings; 
            return parseString(c, v); 
        case '\0': // reach the end of JSON context and have read nothing;
            return Lept::PARSE_EXPECT_VALUE; 
    }
}



/* ------- Lept::Value -------- */
// Constructor; 
Lept::Value::Value(Lept::Type type) :
    m_type(type)
{
    switch (type)
    {
    case Lept::Type::STRING:
        this->m_str = new std::string;
        break; 
    default:
        this->m_num = 0.0; 
        break; 
    }
}
// Destructor; 
Lept::Value::~Value(void)
{
    if (this->getType() == Lept::Type::STRING)
        delete this->getStr(); 
}

/* get-Functions */
Lept::Type Lept::Value::getType(void) const
{
    return this->m_type;
}
bool Lept::Value::getBoolean(void) const
{
    assert(this->getType() == Lept::Type::TRUE || this->getType() == Lept::Type::FALSE); 

    return (this->getType() == Lept::Type::TRUE); 
}
double Lept::Value::getNum(void) const
{
    assert(this->getType() == Lept::Type::NUMBER); 

    return this->m_num; 
}
std::string* Lept::Value::getStr(void) const
{
    assert(this->getType() == Lept::Type::STRING); 

    return this->m_str; 
}

/* set-Functions */
void Lept::Value::setType(Lept::Type type)
{
    switch (this->m_type)
    {
    case Lept::Type::STRING:
        delete this->getStr();
        break; 
    default:
        break; 
    }

    this->m_type = type;
    switch (type)
    {
    case Lept::Type::STRING:
        this->m_str = new std::string; 
        break; 
    default:
        this->m_num = 0.0; 
        break; 
    }

    return; 
}
void Lept::Value::setNull(void)
{
    this->setType(Lept::Type::NULLJSON); 

    return; 
}
void Lept::Value::setBoolean(bool bln)
{
    this->setType(bln ? Lept::Type::TRUE : Lept::Type::FALSE);

    return;
}
void Lept::Value::setNum(double num)
{
    this->setType(Lept::Type::NUMBER);
    this->m_num = num; 

    return; 
}
void Lept::Value::setStr(std::string str)
{
    this->setType(Lept::Type::STRING); 
    *this->m_str = str; 

    return;
}
void Lept::Value::setStr(const char* str)
{
    this->setType(Lept::Type::STRING);
    *this->m_str = str; 

    return; 
}
void Lept::Value::appendChar(char ch)
{
    assert(this->getType() == Lept::Type::STRING); 

    this->m_str->push_back(ch); 

    return; 
}

// parse JSON context to tree context; 
int Lept::Value::parse(const char* json)
{
    LeptContext c; // create JSON context; 
    int ret = Lept::PARSE_OK; 

    c.json = json; // add to JSON context; 
    this->setType(Lept::Type::NULLJSON); // by default set NULL data; 
    c.que = new std::queue<char>; 
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

    delete c.que; 

    return ret;
}

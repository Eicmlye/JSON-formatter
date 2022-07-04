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
static int parseString(LeptContext& c, Lept::Value* v)
{
    EXPECT(c, '\"'); 

    /* validate string */
    while (*c.json != '\"')
    {
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
            case 'u': /* will deal with this later */
                return Lept::PARSE_INVALID_STRING_ESCAPE;
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

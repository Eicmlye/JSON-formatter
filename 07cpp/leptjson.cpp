#include "leptjson.h"
#include <iostream> /* std::fixed */
#include <cassert>  /* assert() */
#include <cstdlib> /* exit() */
#include <cerrno> /* errno, ERANGE */
#include <queue> /* std::queue<> */
#include <iomanip> /* std::setprecision() */
// #include <type_traits> /* std::is_same<>::value */

/* macros */
#if 1
#define EXPECT(c, ch) \
    do {\
        assert(*c->getTxt() == (ch)); \
        c->txtIncre(); \
    } while (0)

#define IS_DIGIT(ch) \
    ((ch) >= '0' && (ch) <= '9')
#define IS_DIGIT_NONZERO(ch) \
    ((ch) >= '1' && (ch) <= '9')
#define IS_HEX(ch) \
    (IS_DIGIT(ch) || ((ch) >= 'A' && (ch) <= 'F') || ((ch) >= 'a' && (ch) <= 'f'))

#define CH2HEX(ch) \
    (\
        (IS_DIGIT(ch)) ? \
        (\
            ((unsigned long)(ch - '0'))\
        ) : \
        (\
            (ch <= 'F') ? \
            (\
                ((unsigned long)(ch - 'A' + 10))\
            ) : \
            (\
                ((unsigned long)(ch - 'a' + 10))\
            )\
        )\
    )

#define INDENT(value, cache) \
    do {\
        for (unsigned int index = 0; index < value->getLevel(); ++index)\
            cache.append("\t"); \
    } while (0)
#endif


/* ------- Lept::Value -------- */
#if 1
// Constructor; 
Lept::Value::Value(Lept::Type type) :
    m_type(type), 
    m_level(0)
{
    switch (type)
    {
    case Lept::Type::STRING:
        this->m_str = new std::string;
        break; 
    case Lept::Type::ARRAY:
        this->m_arr = new std::vector<Lept::Value*>; 
        break; 
    default:
        this->m_num = 0.0; 
        break; 
    }
}
// Destructor; 
Lept::Value::~Value(void)
{
    Lept::Type type = this->getType(); 

    switch (type)
    {
    case Lept::Type::STRING:
        delete this->m_str; 
        break; 
    case Lept::Type::ARRAY:
        int len; 
        len = this->m_arr->size() - 1; 
        for (; len >= 0; --len)
            delete this->getArrElem(len); 
        delete this->m_arr; 
        break;
    case Lept::Type::OBJECT:
        len = this->m_obj->size() - 1;
        for (; len >= 0; --len)
        {
            delete this->getObjElem(len)->key;
            delete this->getObjElem(len)->value;
            delete this->getObjElem(len);
        }
        delete this->m_obj;
        break;
    default:
        break; 
    }
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
std::vector<Lept::Value*>* Lept::Value::getArr(void) const
{
    assert(this->getType() == Lept::Type::ARRAY);

    return this->m_arr;
}
Lept::Value* Lept::Value::getArrElem(void) const
{
    assert(this->getType() == Lept::Type::ARRAY);

    return this->m_arr->at(this->m_arr->size() - 1);
}
Lept::Value* Lept::Value::getArrElem(unsigned int index) const
{
    assert(this->getType() == Lept::Type::ARRAY);

    return this->m_arr->at(index);
}
std::vector<Lept::Member*>* Lept::Value::getObj(void) const
{
    assert(this->getType() == Lept::Type::OBJECT);

    return this->m_obj;
}
Lept::Member* Lept::Value::getObjElem(void) const
{
    assert(this->getType() == Lept::Type::OBJECT);

    return this->m_obj->at(this->m_obj->size() - 1);
}
Lept::Member* Lept::Value::getObjElem(unsigned int index) const
{
    assert(this->getType() == Lept::Type::OBJECT);

    return this->m_obj->at(index);
}
int Lept::Value::getLevel(void) const
{
    return this->m_level; 
}

/* set-Functions */
void Lept::Value::setType(Lept::Type type)
{
    int len = 0;

    switch (this->m_type)
    {
    case Lept::Type::STRING:
        delete this->m_str;
        break; 
    case Lept::Type::ARRAY:
        len = this->m_arr->size() - 1;
        for (; len >= 0; --len)
            delete this->getArrElem(len);
        delete this->m_arr; 
        break; 
    case Lept::Type::OBJECT:
        len = this->m_obj->size() - 1;
        for (; len >= 0; --len)
        {
            delete this->getObjElem(len)->key;
            delete this->getObjElem(len)->value;
            delete this->getObjElem(len);
        }
        delete this->m_obj;
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
    case Lept::Type::ARRAY:
        this->m_arr = new std::vector<Lept::Value*>; 
        break; 
    case Lept::Type::OBJECT:
        this->m_obj = new std::vector<Lept::Member*>; 
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
void Lept::Value::setStrNew(void)
{
    assert(this->getType() == Lept::Type::STRING); 

    this->m_str = new std::string; 

    return; 
}
void Lept::Value::setStr(std::string* str)
{
    assert(this->getType() == Lept::Type::STRING);

    this->m_str = str; 

    return; 
}
void Lept::Value::setStr(std::string str)
{
    assert(this->getType() == Lept::Type::STRING);

    *this->m_str = str;

    return;
}
void Lept::Value::appendChar(char ch)
{
    assert(this->getType() == Lept::Type::STRING); 

    this->m_str->push_back(ch); 

    return; 
}
void Lept::Value::appendArrElem(Lept::Value &elem)
{
    assert(this->getType() == Lept::Type::ARRAY);

    this->m_arr->push_back(&elem); 

    return; 
}
void Lept::Value::appendObjElem(Lept::Member& elem)
{
    assert(this->getType() == Lept::Type::OBJECT);

    this->m_obj->push_back(&elem);

    return;
}
void Lept::Value::setLevel(int level)
{
    this->m_level = level; 

    return; 
}
void Lept::Value::levelUp(void)
{
    ++this->m_level;

    return;
}
void Lept::Value::levelDown(void)
{
    --this->m_level;

    return;
}

/* parse JSON context to tree context */
int Lept::Value::parse(Lept::Context &c)
{
    int ret = Lept::PARSE_OK;

    this->setType(Lept::Type::NULLJSON); // by default set NULL data; 
    c.parseWs(); // skip the leading ws; 

    if ((ret = c.parseValue(*this)) == Lept::PARSE_OK)
    {
        c.parseWs();
        if (*c.getTxt() != '\0')
        {
            this->setType(Lept::Type::NULLJSON);
            ret = Lept::PARSE_ROOT_NOT_SINGULAR;
        }
    }

    return ret;
}
int Lept::Value::parse(const char* json)
{
    Lept::Context c(json); // create JSON context; 

    return this->parse(c); 
}
int Lept::Value::parse(std::string json)
{
    char* str = new char[json.size() + 1]; 
    strcpy(str, json.c_str()); 
    Lept::Context c(str); 
    int ret = this->parse(c); 
    delete[] str; 

    return ret; 
}

/* JSON stringifier components*/
#if 1
int Lept::Value::stringifyLiteral(std::string& JSONCache) const
{
    Lept::Type type = this->getType();
    assert(type == Lept::Type::NULLJSON || this->getType() == Lept::Type::FALSE || this->getType() == Lept::Type::TRUE); 
    
    switch (type)
    {
    case Lept::Type::NULLJSON:
        JSONCache.append("null"); 
        break;
    case Lept::Type::FALSE:
        JSONCache.append("false");
        break;
    case Lept::Type::TRUE:
        JSONCache.append("true");
        break;
    }

    return Lept::STRINGIFY_OK; 
}
int Lept::Value::stringifyNumber(std::string& JSONCache) const
{
    assert(this->getType() == Lept::Type::NUMBER); 

    char buffer[32]; 
    sprintf(buffer, "%.17g", this->getNum());
    JSONCache.append(buffer);

    return Lept::STRINGIFY_OK; 
}
/* CAUTION: \uxxxx escapes remain to be fixed */
int Lept::Value::stringifyString(std::string& JSONCache) const
{
    assert(this->getType() == Lept::Type::STRING);

    JSONCache.append("\"");
    
    const std::string* p = this->getStr();
    unsigned int len = p->size(), index = 0; 
    char cur; 
    while (index < len)
    {
        /* excape chars except \uxxxx */
        cur = p->at(index); 
        switch (cur)
        {
        case '\"': JSONCache.append("\\\""); break; 
        case '\\': JSONCache.append("\\\\"); break;
        case '/': JSONCache.append("\\/"); break;
        case '\b': JSONCache.append("\\b"); break;
        case '\f': JSONCache.append("\\f"); break;
        case '\n': JSONCache.append("\\n"); break;
        case '\r': JSONCache.append("\\r"); break;
        case '\t': JSONCache.append("\\t"); break;
        default:
            if (cur < 0x20) 
            {
                /* \uxxxx escapes remain to be fixed */
            }
            else 
                JSONCache.append(p->substr(index, 1)); 
            break;
        }
        ++index; 
    }

    JSONCache.append("\"");

    return Lept::STRINGIFY_OK;
}
int Lept::Value::stringifyArray(std::string& JSONCache)
{
    assert(this->getType() == Lept::Type::ARRAY);
    int ret = Lept::STRINGIFY_OK; 

    unsigned int len = this->getArr()->size();
    JSONCache.append("[\n");
    this->levelUp();
    if (len != 0) // beautifiy empty array; 
        INDENT(this, JSONCache);

    for (unsigned int index = 0; index < len; ++index)
    {
        this->getArr()->at(index)->setLevel(this->getLevel()); 
        ret = this->getArr()->at(index)->stringify(JSONCache);
        if (ret != Lept::STRINGIFY_OK)
            return ret;
        if (index != len - 1)
        {
            JSONCache.append(",\n");
            INDENT(this, JSONCache);
        }
        else
            JSONCache.append("\n");
    }
    this->levelDown();
    INDENT(this, JSONCache);
    JSONCache.append("]");

    return ret;
}
int Lept::Value::stringifyObject(std::string& JSONCache)
{
    assert(this->getType() == Lept::Type::OBJECT);
    int ret = Lept::STRINGIFY_OK;

    unsigned int len = this->getObj()->size();
    JSONCache.append("{\n");
    this->levelUp();
    if (len != 0) // beautifiy empty object; 
        INDENT(this, JSONCache);

    Lept::Value cache(Lept::Type::STRING);
    for (unsigned int index = 0; index < len; ++index)
    {
        cache.setStrNew(); 
        cache.setStr(*this->getObj()->at(index)->key);
        cache.stringifyString(JSONCache); 
        JSONCache.append(":");

        this->getObj()->at(index)->value->setLevel(this->getLevel()); 
        ret = this->getObj()->at(index)->value->stringify(JSONCache);
        if (ret != Lept::STRINGIFY_OK)
            return ret;
        if (index != len - 1)
        {
            JSONCache.append(",\n");
            INDENT(this, JSONCache);
        }
        else
            JSONCache.append("\n"); 
    }
    this->levelDown();
    INDENT(this, JSONCache);
    JSONCache.append("}");

    return ret;
}
int Lept::Value::stringify(std::string& JSONCache)
{
    Lept::Type type = this->getType();
    int ret = Lept::STRINGIFY_OK;
    std::string copy = JSONCache;
    if (this->getLevel() == 0)
        JSONCache = {}; /* avoid rebundance when reusing JSONCache for another stringification */

    switch (type)
    {
    case Lept::Type::NULLJSON:
    case Lept::Type::FALSE:
    case Lept::Type::TRUE:
        ret = this->stringifyLiteral(JSONCache);
        break;
    case Lept::Type::NUMBER:
        ret = this->stringifyNumber(JSONCache);
        break;
    case Lept::Type::STRING:
        ret = this->stringifyString(JSONCache);
        break;
    case Lept::Type::ARRAY:
        ret = this->stringifyArray(JSONCache);
        break;
    case Lept::Type::OBJECT:
        ret = this->stringifyObject(JSONCache);
        break;
    }

    /* stringification failure, JSONCache rewrite to original value */
    if (ret != Lept::STRINGIFY_OK)
        JSONCache = copy; 

    return ret;
}
#endif
#endif


/* -------- Lept::Context -------- */
#if 1
// Constructor; 
Lept::Context::Context(const char* txt) :
    m_txt(txt)
{

}
// Destructor; 
Lept::Context::~Context(void)
{

}

/* get-Functions */
const char* Lept::Context::getTxt(void) const
{
    return this->m_txt;
}

/* set-Functions */
void Lept::Context::setTxt(const char* txt)
{
    this->m_txt = txt;

    return;
}
void Lept::Context::txtIncre(unsigned int inc)
{
    this->setTxt(this->getTxt() + inc);

    return;
}

/* JSON parser components */
#if 1
/* parse single value */
void Lept::Context::parseWs(void)
{
    // cursor;
    const char* p = this->getTxt();
    // skip ws; 
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        ++p;
    // move cursor to the first non-ws char; 
    this->setTxt(p);

    return;
}
int Lept::Context::parseLiteral(Lept::Value& v, const char* stdtxt, Lept::Type type)
{
    EXPECT(this, stdtxt[0]);
    unsigned int count = 1;
    while (stdtxt[count] != '\0')
    {
        if (this->getTxt()[count - 1] != stdtxt[count])
            return Lept::PARSE_INVALID_VALUE;
        ++count;
    }
    this->txtIncre(count - 1);
    v.setType(type);

    return Lept::PARSE_OK;
}
int Lept::Context::parseNumber(Lept::Value& v)
{
    const char* end = this->getTxt();
    /* validate legal number */

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

    errno = 0;
    v.setType(Lept::Type::NUMBER); /* set type to make setNum available */
    v.setNum(strtod(this->getTxt(), nullptr));
    this->setTxt(end);

    /* overflow detection */
    if (errno = ERANGE && (v.getNum() == HUGE_VAL || v.getNum() == -HUGE_VAL))
    {
        v.setType(Lept::Type::NULLJSON);
        return Lept::PARSE_NUMBER_OVERFLOW;
    }
    return Lept::PARSE_OK;
}
/* parse strings and arrays */
unsigned long Lept::Context::str2hex(int& ret)
{
    unsigned long hex = 0;

    if (!IS_HEX(*this->getTxt()))
    {
        ret = Lept::PARSE_INVALID_UNICODE_HEX;
        return 0;
    }
    hex += CH2HEX(*this->getTxt());
    this->txtIncre();

    for (unsigned int index = 1; index < 4; ++index)
    {
        if (!IS_HEX(*this->getTxt()))
        {
            ret = Lept::PARSE_INVALID_UNICODE_HEX;
            return 0;
        }

        hex *= 16;
        hex += CH2HEX(*this->getTxt());
        this->txtIncre();
    }

    return hex;
}
int Lept::Context::parseHex(std::string* str, int& ret)
{
    EXPECT(this, 'u');

    /* get code point */
    unsigned long hex = this->str2hex(ret);
    if (ret != Lept::PARSE_OK)
        return 0;

    if (hex >= 0xD800 && hex <= 0xDBFF)
    {
        if (*this->getTxt() != '\\' || *(this->getTxt() + 1) != 'u')
        {
            ret = Lept::PARSE_INVALID_UNICODE_SURROGATE;
            return 0;
        }
        this->txtIncre(2);

        unsigned long lowhex = this->str2hex(ret);
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
        str->push_back((uint8_t)hex);
    }
    else if (hex < 0x0800) /* 110xxxxx 10xxxxxx */
    {
        /* 0xC0 = 11000000 */
        str->push_back(0xC0 | ((uint8_t)(hex >> 6) & 0xFF)); /* 0x80 = 10000000 */
        str->push_back(0x80 | ((uint8_t)(hex) & 0x3F)); /* 0x3F = 00111111 */
    }
    else if (hex < 0x10000) /* 1110xxxx 10xxxxxx 10xxxxxx */
    {
        /* 0xE0 = 11100000 */
        str->push_back(0xE0 | ((uint8_t)(hex >> 12) & 0xFF)); /* 0x80 = 10000000 */
        str->push_back(0x80 | ((uint8_t)(hex >> 6) & 0x3F)); /* 0x3F = 00111111 */
        str->push_back(0x80 | ((uint8_t)(hex) & 0x3F)); /* 0x3F = 00111111 */
    }
    else if (hex < 0x10FFFF) /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    {
        /* 0xF0 = 11110000 */
        str->push_back(0xF0 | ((uint8_t)(hex >> 18) & 0xFF)); /* 0x80 = 10000000 */
        str->push_back(0x80 | ((uint8_t)(hex >> 12) & 0x3F)); /* 0x3F = 00111111 */
        str->push_back(0x80 | ((uint8_t)(hex >> 6) & 0x3F)); /* 0x3F = 00111111 */
        str->push_back(0x80 | ((uint8_t)(hex) & 0x3F)); /* 0x3F = 00111111 */
    }

    return Lept::PARSE_OK;
}
int Lept::Context::parseString(std::string* str)
{
    EXPECT(this, '\"');

    /* validate string */
    while (*this->getTxt() != '\"')
    {
        int ret; /* for unicode parsing */

        if (*this->getTxt() == '\0') /* this is rational because \0 is represented by \u0000 in JSON context */
            return Lept::PARSE_MISSING_QUOTATION_MARK;

        if (*this->getTxt() == '\\')
        { /* deal with escape characters */
            this->txtIncre();

            switch (*this->getTxt())
            {
            case '\"':
            case '/':
            case '\\':
                str->push_back(*this->getTxt());
                this->txtIncre();
                break;
            case 'b':
                str->push_back('\b');
                this->txtIncre();
                break;
            case 'f':
                str->push_back('\f');
                this->txtIncre();
                break;
            case 'n':
                str->push_back('\n');
                this->txtIncre();
                break;
            case 'r':
                str->push_back('\r');
                this->txtIncre();
                break;
            case 't':
                str->push_back('\t');
                this->txtIncre();
                break;
            case 'u': /* Unicode UTF-8 */
                ret = Lept::PARSE_OK;
                this->parseHex(str, ret);
                if (ret != Lept::PARSE_OK)
                    return ret;
                break;
            default:
                return Lept::PARSE_INVALID_STRING_ESCAPE;
            }
        }
        else if (*this->getTxt() >= '\x20') /* need NOT check " and \ here, 'cuz they've been blocked by if and while */
        { /* now for unescaped characters */
            str->push_back(*this->getTxt());
            this->txtIncre();
        }
        else
            return Lept::PARSE_INVALID_STRING_CHAR;
    }
    this->txtIncre();

    return Lept::PARSE_OK;
}
int Lept::Context::parseString(Lept::Value& v)
{
    std::string* str = new std::string;
    int ret = Lept::PARSE_OK;

    if ((ret = this->parseString(str)) != Lept::PARSE_OK)
        return ret;

    v.setType(Lept::Type::STRING);
    v.setStr(str); 

    return ret;
}
int Lept::Context::parseArray(Lept::Value& v)
{
    EXPECT(this, '[');

    Lept::Value* cache; 
    int ret = Lept::PARSE_OK; 
    v.setType(Lept::Type::ARRAY); 
    this->parseWs(); 

    while (*this->getTxt() != ']')
    {
        cache = new Lept::Value; 
        ret = this->parseValue(*cache);
        if (ret != Lept::PARSE_OK)
        {
            v.setType(Lept::Type::NULLJSON);
            return ret;
        }
        v.appendArrElem(*cache);

        this->parseWs(); 
        if (*this->getTxt() == ']')
            continue; 
        else if (*this->getTxt() == ',')
        {
            this->txtIncre();
            this->parseWs();
        }
        else
        {
            v.setType(Lept::Type::NULLJSON);
            return Lept::PARSE_MISSING_COMMA_OR_BRACKET;
        }
    }
    this->txtIncre(); 

    return ret; 
}
/* parse objects */
int Lept::Context::parseObject(Lept::Value& v)
{
    EXPECT(this, '{');

    Lept::Member* cache;
    std::string* key; 
    Lept::Value* value; 
    int ret = Lept::PARSE_OK;
    v.setType(Lept::Type::OBJECT);
    this->parseWs();

    while (*this->getTxt() != '}')
    {
        cache = new Lept::Member;
        key = new std::string;
        if (*this->getTxt() != '\"')
        {
            v.setType(Lept::Type::NULLJSON);
            return Lept::PARSE_MISSING_KEY;
        }
        ret = this->parseString(key);
        if (ret != Lept::PARSE_OK)
        {
            v.setType(Lept::Type::NULLJSON);
            return ret;
        }
        cache->key = key; 

        this->parseWs(); 
        if (*this->getTxt() != ':')
        {
            v.setType(Lept::Type::NULLJSON);
            return Lept::PARSE_MISSING_COLON;
        }
        this->txtIncre(); 
        this->parseWs();

        value = new Lept::Value;
        ret = this->parseValue(*value);
        if (ret != Lept::PARSE_OK)
        {
            v.setType(Lept::Type::NULLJSON);
            return ret;
        }
        cache->value = value; 

        v.appendObjElem(*cache);

        this->parseWs();
        if (*this->getTxt() == '}')
            continue;
        else if (*this->getTxt() == ',')
        {
            this->txtIncre();
            this->parseWs();
        }
        else
        {
            v.setType(Lept::Type::NULLJSON);
            return Lept::PARSE_MISSING_COMMA_OR_BRACE;
        }
    }
    this->txtIncre();

    return ret;
}
/* parse JSON values */
int Lept::Context::parseValue(Lept::Value& v)
{
    char txt = *this->getTxt();

    switch (txt)
    {
    case 'n': // NULL; 
        return this->parseLiteral(v, "null", Lept::Type::NULLJSON);
    case 'f': // FALSE; 
        return this->parseLiteral(v, "false", Lept::Type::FALSE);
    case 't': // TRUE; 
        return this->parseLiteral(v, "true", Lept::Type::TRUE);
    default: // numbers or invalid JSON context; 
        return this->parseNumber(v);
    case '\"': // strings; 
        return this->parseString(v);
    case '[': // arrays; 
        return this->parseArray(v);
    case '{': // objects; 
        return this->parseObject(v); 
    case '\0': // reach the end of JSON context and have read nothing;
        return Lept::PARSE_EXPECT_VALUE;
    }
}
#endif
#endif
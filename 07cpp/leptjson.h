#ifndef _H_LEPTJSON
#define _H_LEPTJSON /* guard */

#include <string> /* std::string */
#include <vector> /* std::vector */
#include <fstream> /* std::ofstream */

namespace Lept
{
    /* JSON data structure */
    enum class Type
    {
        NULLJSON,   /* null */
        FALSE,      /* false */
        TRUE,       /* true */
        NUMBER,     /* -1.23e-45*/
        STRING,     /* "Hello123\n\"Eric\"" */
        ARRAY,      /* [1, "2", [3]] */
        OBJECT      /* {"level":1, "name":{"surname":"Monlye"}} */
    };

    /* JSON object member */
    class Value; /* JSON value */
    typedef struct
    {
        std::string* key;
        Lept::Value* value;
    } Member; 

    /* JSON tree node structure */
    class Context; 
    class Value
    {
    private:
        Lept::Type m_type; 
        /* data */
        union
        {
            double m_num; /* number */
            std::string* m_str; /* string */
            std::vector<Lept::Value*>* m_arr; /* array */
            std::vector<Lept::Member*>* m_obj; /* object */
        }; 
        int m_level; 

    public:
        // Constructor; 
        Value(Lept::Type type = Lept::Type::NULLJSON); 
        // Destructor; 
        virtual ~Value(void); 

        // get-Functions; 
        Lept::Type getType(void) const;
        bool getBoolean(void) const; 
        double getNum(void) const; 
        std::string* getStr(void) const;
        std::vector<Lept::Value*>* getArr(void) const; 
        Lept::Value* getArrElem(void) const; /* get last element */
        Lept::Value* getArrElem(unsigned int index) const;
        std::vector<Lept::Member*>* getObj(void) const; 
        Lept::Member* getObjElem(void) const; /* get last element */
        Lept::Member* getObjElem(unsigned int index) const; 
        int getLevel(void) const; 

        // set-Functions
        void setType(Lept::Type type); 
        void setNull(void); 
        void setBoolean(bool bln); 
        void setNum(double num);
        void setStrNew(void); 
        void setStr(std::string* str); 
        void setStr(std::string str); 
        void appendChar(char ch); 
        void appendArrElem(Lept::Value &elem); 
        void appendObjElem(Lept::Member& elem); 
        void setLevel(int level); 
        void levelUp(void); 
        void levelDown(void); 

        /* parse JSON context to tree structure */
        int parse(Lept::Context &c);
        int parse(const char* json);
        int parse(std::string json);

        /* stringify JSON value */
        int stringifyLiteral(std::string& JSONCache) const;
        int stringifyNumber(std::string& JSONCache) const;
        int stringifyString(std::string& JSONCache) const;
        int stringifyArray(std::string& JSONCache);
        int stringifyObject(std::string& JSONCache);
        int stringify(std::string& JSONCache);
    };

    /* JSON parser error info */
    enum
    {
        PARSE_OK = 0, /* successfully parsed */
        PARSE_EXPECT_VALUE, /* only whitespace detected */
        PARSE_INVALID_VALUE, /* invalid leading character or incomplete literal */
        PARSE_ROOT_NOT_SINGULAR, /* non-ws detected after ending ws */
        PARSE_NUMBER_OVERFLOW, /* number is greater than what double could hold */
        PARSE_MISSING_QUOTATION_MARK, /* ending quotation missing */
        PARSE_INVALID_STRING_ESCAPE, /* invalid excape character */
        PARSE_INVALID_STRING_CHAR, /* '\x00' to '\x1F' detected, which is invalid in JSON */
        PARSE_INVALID_UNICODE_HEX, /* invalid '\uxxxx' form detected */
        PARSE_INVALID_UNICODE_SURROGATE, /* higher or lower surrogate out of range, or missing lower surrogate */
        PARSE_MISSING_COMMA_OR_BRACKET, /* comma or ending bracket missing */
        PARSE_MISSING_KEY, /* key missing */
        PARSE_MISSING_COLON, /* colon missing */
        PARSE_MISSING_COMMA_OR_BRACE /* comma or ending brace missing */
    };
    /* JSON stringifier error info */
    enum
    {
        STRINGIFY_OK = 0, /* successfully stringified */
        STRINGIFY_FILE_OPEN_FAILURE /* output target file open error */
    };

    /* JSON context */
    class Context
    {
    private:
        const char* m_txt;

    public:
        // Constructor; 
        Context(const char* txt = nullptr);
        // Destructor; 
        ~Context(void);

        // get-Functions; 
        const char* getTxt(void) const;

        // set-Functions; 
        void setTxt(const char* txt);
        void txtIncre(unsigned int inc = 1);

        /* parse value */
        void parseWs(void);
        int parseLiteral(Lept::Value& v, const char* stdtxt, Lept::Type type);
        int parseNumber(Lept::Value& v);
        unsigned long str2hex(int& ret);
        int parseHex(std::string* str, int& ret); 
        int parseString(std::string* str); 
        int parseString(Lept::Value& v);
        int parseArray(Lept::Value& v);
        int parseObject(Lept::Value& v); 
        int parseValue(Lept::Value& v);
    }; 
}

#endif /* _H_LEPTJSON */
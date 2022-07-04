#ifndef _H_LEPTJSON
#define _H_LEPTJSON /* guard */

#include <string> /* std::string */

namespace Lept
{
    /* JSON data structure */
    enum class Type
    {
        NULLJSON, 
        FALSE, 
        TRUE, 
        NUMBER, 
        STRING, 
        ARRAY, 
        OBJECT
    };

    /* JSON tree node structure */
    class Value
    {
    private:
        Lept::Type m_type; 
        /* data */
        union
        {
            double m_num; /* number */
            std::string* m_str; /* string */
        }; 

    public:
        Value(Lept::Type type = Lept::Type::NULLJSON); 
        ~Value(void); 

        Lept::Type getType(void) const;
        bool getBoolean(void) const; 
        double getNum(void) const; 
        std::string* getStr(void) const; 

        void setType(Lept::Type type); 
        void setNull(void); 
        void setBoolean(bool bln); 
        void setNum(double num);
        void setStr(std::string str);
        void setStr(const char* str);
        void appendChar(char ch); 

        /* parse JSON context to tree structure */
        int parse(const char* json);
    };

    /* error info */
    enum
    {
        PARSE_OK = 0,
        PARSE_EXPECT_VALUE,
        PARSE_INVALID_VALUE,
        PARSE_ROOT_NOT_SINGULAR,
        PARSE_NUMBER_OVERFLOW,
        PARSE_MISSING_QUOTATION_MARK,
        PARSE_INVALID_STRING_ESCAPE,
        PARSE_INVALID_STRING_CHAR, 
        PARSE_INVALID_UNICODE_HEX, 
        PARSE_INVALID_UNICODE_SURROGATE
    };
}

#endif /* _H_LEPTJSON */
#ifndef _H_LEPTJSON
#define _H_LEPTJSON /* guard */

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

    public:
        Value(void); 
        ~Value(void); 

        Lept::Type getType(void) const;
        void setType(Lept::Type type); 

        /* parse JSON context to tree structure */
        int parse(const char* json);
    };

    /* error info */
    enum 
    {
        PARSE_OK = 0, 
        PARSE_EXPECT_VALUE, 
        PARSE_INVALID_VALUE, 
        PARSE_ROOT_NOT_SINGULAR
    };
}

#endif /* _H_LEPTJSON */
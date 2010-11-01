//
// $Id: parse.cpp 729 2010-10-31 07:00:15Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
//
#include <stab.h>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include "dharma/symbol_util.h"
#include "generic/temporary.h"
#include "zdk/check_ptr.h"
#include "zdk/debug_sym.h"
#include "zdk/type_system_util.h"
#include "typez/public/enum_type.h"
#include "typez/public/types.h"
#include "public/block.h"
#include "public/compile_unit.h"
#include "public/function.h"
#include "public/fwdtype.h"
#include "public/variable.h"
#include "private/parse_events.h"
#include "private/expect.h"
#include "private/token.h"
#include "private/throw.h"
#include "private/util.h"
#include "unmangle/unmangle.h"

#if defined(NDEBUG) && defined(DEBUG)
 #undef DEBUG
#endif

using namespace std;
using namespace Platform;
using namespace Stab;


////////////////////////////////////////////////////////////////
static bool inline debug_stabz()
{
    static bool flag = getenv("ZERO_DEBUG_STABZ") != 0;
    return flag;
}

// silence off compiler warning
template<typename X>
inline const X& no_effect_statement(const X& x) { return x; }

////////////////////////////////////////////////////////////////
#ifdef DEBUG_STATE
  #define START_STATE(name, ptr) \
    if (!stack_.empty()) stack_.back().freeze(); \
    stack_.push_back(ParseState(name, ptr));

  #define END_STATE(x) pop_state(x)
#else
  #define START_STATE(name, ptr)
  #define END_STATE(x) no_effect_statement(x)
#endif

#ifdef DEBUG
 #define TRACE() if (!debug_stabz()); else\
  clog << __func__ << '@' << __FILE__ << ':' << __LINE__ << endl
 #define TRACE_(x) TRACE() << (x) << endl;
#else
 #define TRACE()
 #define TRACE_(x)
#endif


ParseEvents::ParseEvents(TypeSystem& types, Descriptor& desc)
    : types_(types), desc_(desc)
{
}


ParseEvents::~ParseEvents() throw()
{
}


void ParseEvents::on_section(const char*)
{
}


void ParseEvents::on_begin(SharedString&, const char*, size_t)
{
}


void ParseEvents::on_done(size_t)
{
}


bool ParseEvents::on_stab( size_t          index,
                           const stab_t&   stab,
                           const char*     str,
                           size_t          strLen)
{
    assert(str);

    klass_.reset();
    klassStack_.clear();

    switch (stab.type())
    {
    case N_FUN:
        on_func(index, stab, str, strLen);
        break;

    case N_LSYM:
    case N_GSYM:
    case N_PSYM:
    case N_RSYM:
    case N_LCSYM:
    case N_STSYM:
        // parse the stab string to extract the type info
        if (!parse(stab, str, strLen))
        {
            cerr << str << endl;
            return false;
        }
        break;

    case N_SO:
        if (strLen)
        {
            unit_ = desc_.get_compile_unit(stab.value(), index);

            if (unit_.get())
            {
                assert(unit_->begin_addr() == stab.value());
                assert(unit_->begin_index() == index);
            }
        }
        else
        {
            if (func_.get())
            {
                finish_func();
            }
            unit_.reset();
        }
        break;

    case N_LBRAC:
        assert(!blocks_.empty());
        assert(func_.get());
        {
            if (!param_.empty())
            {
                func_->assign_variables(param_);
                param_.clear();
            }
            RefPtr<Block>& top = blocks_.back();

            // N_LBRAC and N_RBRAC stab values are the
            // addresses where the block starts and ends,
            // respectively.
            // For stabs in sections, values are relative
            // to the function in which they occur.

            const addr_t addr = func_->begin_addr() + stab.value();

            RefPtr<Block> block(new Block(addr, index, vars_));

            // vars_.clear();
            // the Block ctor is expected to clear the vector
            assert(vars_.empty());

            top->add_child(block);
            blocks_.push_back(block);
        }
        break;

    case N_RBRAC:
        assert(!blocks_.empty());
        {
            RefPtr<Block>& top = blocks_.back();

            top->set_end_addr(func_->begin_addr() + stab.value());
            top->set_end_index(index);

            assert(vars_.empty());
        }
        blocks_.pop_back();
        break;
    }
    return true;
}


void ParseEvents::on_func(size_t          index,
                          const stab_t&   stab,
                          const char*     str,
                          size_t          strLen)
{
    if (strLen == 0)
    {
        assert(*str == 0);
        finish_func();

        // all lexical blocks but the one corresponding
        // to the function itself should be popped off
        // the stack at this point
        assert(blocks_.size() == 1);
        assert(blocks_.front() == func_);

        blocks_.pop_back();
        func_.reset();
    }
    else
    {
        finish_func();
        func_ = current_unit()->lookup_function(stab.value(), true);

        // expect that the InitEvents have added this
        // function to the compilation unit.
        assert(!func_.is_null());

        if (!parse(stab, str, strLen))
        {
            cerr << str << endl;
            assert(false);
        }

        if (!blocks_.empty())
        {
            // a closing N_FUN is not mandatory, and that
            // is the only case that may leave a block on
            // the stack
            assert(blocks_.size() == 1);
            assert(interface_cast<Function*>(blocks_[0].get()));

            blocks_.clear();
        }
        blocks_.push_back(func_);
    }
    vars_.clear();
}


/**
 * Add function type to the type system
 */
void ParseEvents::finish_func()
{
    if (func_.is_null())
    {
        return; // nothing to do
    }
    if (!param_.empty())
    {
        vars_.insert(vars_.begin(), param_.begin(), param_.end());
        param_.clear();
    }
    if (!vars_.empty())
    {
        func_->assign_variables(vars_);
        vars_.clear();
    }

    SharedString& fname = func_->name();
    RefPtr<MethodImpl> method(current_unit()->lookup_method(fname));
    if (method.get())
    {
        const ParamTypes& param = func_->param_types();
        RefPtr<DataType> retType = func_->return_type();
        RefPtr<FunType> funType =
            get_function_type(types_, retType, param, false);
        method->set_type(funType);
    }
}


bool ParseEvents::pop_state(bool expr)
{
    if (expr)
    {
        stack_.pop_back();
    }
    return expr;
}


ostream& ParseEvents::dump_stack(ostream& outs) const
{
    StateStack::const_reverse_iterator i(stack_.rbegin());
    for (; i != stack_.rend(); ++i)
    {
        (*i).dump(outs);
        outs << endl;
    }
    return outs;
}


/**
 * Print error message to stderr.
 */
static bool error(int type, const char* begin, const char* end)
{
    assert(begin);
    cerr << "STABZ: error in " << Stab::get_name(type) << endl;

    static const int saneWidth = 78;

    //  The stab string can be quite long, especially in
    //  C++ programs that use templates and STL.
    if (distance(begin, end) >= saneWidth)
    {
        cerr << "STABZ: " << string(begin, end) << "\n\n";
    }
    else
    {
        int len = strlen(begin);
        if (len >= saneWidth)
        {
            len = saneWidth;
        }
        cerr << "STABZ: " << string(begin, begin + len) << endl;
        cerr << setw(end - begin + 9) << "^\n\n";
    }
    return false;
}


/**
 * Parse a '(number, number)' string and extract the pair
 * of numbers. This is the format in which GCC (at least
 * from 2.95 above) keyes the type id in stabs.
 */
static TypeID parse_type_id(const char* str)
{
    assert(*str == '(');

    char* ptr = 0;

    TypeID result(strtoul(++str, &ptr, 0), 0);

    if (*ptr == ',')
    {
        ++ptr;
        result.second = strtoul(ptr, &ptr, 0);
    }
    return result;
}


/**
 * Start parsing a stab string. We expect a variable or
 * type name (identifier), followed by a colon.
 */
static RefPtr<SharedString>
parse_start(const char*& begin, const char*& end)
{
    EXPECT(T_IDENT, next_token(begin, end));
    assert(end > begin);

    RefPtr<SharedString> ident(SharedStringImpl::create(begin, end));
    assert(ident->c_str());
    EXPECT(T_COLON, next_token(begin, end));

    return ident;
}


bool
ParseEvents::parse(const stab_t& stab, const char* str, size_t len)
{
    stack_.clear();

    bool result = true;

    const char* begin = str;
    const char* end = begin;

    START_STATE("parse", end);

    RefPtr<SharedString> name = parse_start(begin, end);

    if (name.is_null())
    {
        return error(stab.type(), begin, end);
    }

    TypeID typeID;

    switch (*end)
    {
    case T_TYPE_DEF:
    case T_TYPE_TAG:
        if (!expect(T_TYPE_DEF, T_TYPE_TAG,
                    next_token(begin, end),
                    "T_TYPE_DEF or T_TYPE_TAG")
         || !parse_type(end, typeID, name.get()))
        {
            dump_stack(clog);
            result = false;
        }
        break;

    case 'c':
        result = parse_constant(++end, name.get());
        break;

    case 'S':
    case 'G':
        result = parse_global(stab, end, typeID, name.get());
        break;

    case 'P':   // param by register
    case 'r':   // register variable
    case 'a':   // param passed by reference in register (AIX?)
        assert(stab.type() == N_RSYM);
        goto FALLTHRU;

    case 'v':   // parameter passed by reference
    case 'p':
        assert(stab.type() == N_PSYM);

    case 'V':
        if (stab.type() == N_STSYM)
        {
            result = parse_global(stab, end, typeID, name.get());
            break;
        }
    FALLTHRU:
        ++end;

        // assume GNU compiler/assembler: the type id
        // is expected in the form of '(' num, num ')'
        assert(*end == '(');

    case '(':
        begin = end;
        result = END_STATE(parse_type(end, typeID));

        if (result && name.get())
        {
            add_local_var(stab, typeID, *name);
        }
        break;

    case 'F':
    case 'f':
        ++end;

        assert(!func_.is_null());

        result = END_STATE(parse_type(end, typeID));
        if (result)
        {
            RefPtr<DataType> type = current_unit()->get_type(types_, typeID);
            func_->set_return_type(type);
        }
        break;

    default: result = false;
    }
    if (!result)
    {
        error(stab.type(), begin, end);
    }
#if 0
    if (*end)
    {
        clog << str << endl;
    }

    /*  Tokenize the remainder of the string, for debugging;
        helps me check for tokenization bugs. In case of a
        complete and correct parse, all characters should be
        eaten by here, and nothing should be printed. */

    for (Token tok = T_ERROR; *end; tok = next_token(begin, end))
    {
        if (tok == T_END)
        {
            break;
        }
        clog << setw(8) << tok << ' ';
        copy(begin, end, ostream_iterator<char>(clog));
        clog << endl;
    }
#endif
    return result;
}


/**
 * See: http://sources.redhat.com/gdb/current/onlinedocs/stabs_5.html
 * http://sources.redhat.com/gdb/current/onlinedocs/stabs_toc.html
 */
bool
ParseEvents::parse_constant(const char*& ptr, SharedString* id)
{
    START_STATE("parse_constant", ptr);
    const char* begin = ptr;
    EXPECT('=', *begin);
    int ival = 0;
    switch(*++begin)
    {
    case 'b':
        ival = strtol(++begin, 0, 0);
        // todo
        return END_STATE(true);

    case 'c': // todo
        break;

    case 'e': // todo
        break;
    case 'i':
        ival = strtol(++begin, 0, 0);
        // clog << id->c_str() << "=" << ival << endl;
        // todo
        return END_STATE(true);

    case 'r': // todo
        break;
    case 's': // todo
        break;
    case 'S':
        break;

    default:
        break;
    }
    return false;
}


/**
 * @return true if successful
 */
bool ParseEvents::parse_type(const char*&        ptr,
                             TypeID&             typeID,
                             RefPtr<DataType>&   type)
{
    const bool result = parse_type(ptr, typeID, 0);

    if (result)
    {
        type = current_unit()->get_type(types_, typeID);
        assert(!type.is_null());
    }
    return result;
}


/**
 * Parse type declaration in stab; `ptr' is the current pointer
 * in the stab string, `type' is filled out with the key of the
 * parsed type, and `name' is an optional type name, extracted
 * from the string by the caller.
 */
bool ParseEvents::parse_type(const char*&    ptr,
                             TypeID&         typeID,
                             SharedString*   name)
{
    START_STATE("parse_type", ptr);

    const char* begin = ptr;
    EXPECT(T_TYPE_KEY, next_token(begin, ptr));

    size_t size = 0;
    typeID = parse_type_id(begin);

    if (next_token(begin, ptr) != T_EQUALS)
    {
        ptr = begin; // done; put back
    }
    else if (*ptr == '(')
    {
        TypeID otherID;

        if (!parse_type(ptr, otherID, name))
        {
            return false;
        }

        if (typeID == otherID)
        {
            // the void type is represented as self-defined
            current_unit()->add_type(typeID, types_.get_void_type());
        }
        else
        {
            RefPtr<DataType> other =
                current_unit()->get_type(types_, otherID, false);
            current_unit()->add_type(typeID, other.get());
        }
    }
    else
    {
        for (; *ptr; )
        {
            Token tok = next_token(begin, ptr);

            switch (tok)
            {
            case T_ARRAY:
                return END_STATE(parse_array(ptr, typeID, name));

            case T_ENUM:
                return END_STATE(parse_enum(ptr, typeID, size, name));

            case T_FUN:
                return END_STATE(parse_fun_type(ptr, typeID, name));

            case T_IDENT:
                if (*begin == 'x')
                {
                    return END_STATE(parse_fwd_type(begin, ptr, typeID));
                }
                return false;

            case T_RANGE:
                return END_STATE(parse_range_type(ptr, typeID, name));

            case T_UNION:
                return END_STATE(parse_class(ptr, typeID, name, true));

            case T_STRUCT:
                return END_STATE(parse_class(ptr, typeID, name, false));

            case T_REFERENCE:
            case T_POINTER:
                return END_STATE(parse_pointer(begin, ptr, typeID, name));

            case T_MEM_FUN:
                return END_STATE(parse_fun_type(ptr, typeID, name, false));

            case T_MEM_TYPE:
                if (!parse_type(ptr, typeID, name)) return false;
                EXPECT(T_COMMA, next_token(begin, ptr));
                return END_STATE(parse_type(ptr, typeID, name));

            case T_TYPE_ATTR:
                {
                    switch (*++begin)
                    {
                    case 's':
                        size = strtoul(++begin, 0, 0);
                        break;

                    default:
                        break;
                    }
                }
                if (*ptr)
                {
                    EXPECT(T_SEMICOLON, next_token(begin, ptr));
                }
                break;

            case T_NUMBER:
                parse_negative_types(begin, typeID, name, size);
                if (*ptr)
                {
                    EXPECT(T_SEMICOLON, next_token(begin, ptr));
                }
                break;

            case T_BUILTIN_FP:
                return END_STATE(parse_builtin_fp_type(ptr, typeID, name));

            case T_CONST_QUAL:
            case T_VOLATILE_QUAL:
                return END_STATE(parse_qualified_type(ptr, typeID, name, tok));

            default:
                assert(begin);
                cerr << "STABZ: Unexpected: " << string(begin, ptr) << endl;
                return false;
            }
        }
    }
    return END_STATE(true);
}


/**
 * Parse array type definition
 * Assumption: the index is of integer type
 */
bool ParseEvents::parse_array(const char*&    str,
                              const TypeID&   typeID,
                              SharedString*   name)
{
    START_STATE("parse_array", str);

    const char* begin = str;

    TypeID indexType;

    if (!parse_type(str, indexType))
    {
        return false;
    }
    EXPECT(T_SEMICOLON, next_token(begin, str));

    // Get the lower and upper array bounds;
    // the assumption here is that the index
    // into the array is of an integral type
    // (which is true for C/C++).
    IntRange bounds;

    if (!parse_range(str, bounds)) return false;

    // Now parse the element type.
    TypeID elemTypeID;

    if (!parse_type(str, elemTypeID))
    {
        return false;
    }
    // Done parsing, make the type:
    RefPtr<DataType> elemType =
        current_unit()->get_type(types_, elemTypeID, false);

    RefPtr<DataType> type = types_.get_array_type(
        bounds.first, bounds.second, elemType.get());

    current_unit()->add_type(typeID, type.get());
    return END_STATE(true);
}



bool
ParseEvents::parse_base_classes(const char*&    str,
                                const TypeID&   classID,
                                SharedString*   className)
{
    START_STATE("parse_base_classes", str);

    const char* pos = str;

    // Extract the number of base classes we need to parse
    EXPECT(T_NUMBER, next_token(pos, str));
    const size_t count = strtoul(pos, 0, 10);

    EXPECT(T_COMMA, next_token(pos, str));

    for (size_t n = 0; n != count; ++n)
    {
        START_STATE("parse_base", str);

        const bool isVirtBase = (*str++ == '1');

        const Access access = static_cast<Access>(*str++ - '0');

        // Offset from the address of the derived
        // class that inherits from these bases.
        EXPECT(T_NUMBER, next_token(pos, str));

        const size_t bitOffs = strtoul(pos, 0, 10);

        EXPECT(T_COMMA, next_token(pos, str));

        TypeID baseClassID;
        if (!parse_type(str, baseClassID)) return false;

        EXPECT(T_SEMICOLON, next_token(pos, str));
        END_STATE(true);

        // Now aggregate this base class into
        // the derived class that we are parsing:
        assert(!klass_.is_null());

        RefPtr<DataType> type = current_unit()->get_type(types_, baseClassID);
        klass_->add_base(*type, bitOffs, access, isVirtBase);
    }
    return END_STATE(true);
}


/**
 * Parse C-structs, unions, and C++ classes
 */
bool
ParseEvents::parse_class(const char*&    str,
                         const TypeID&   typeID,
                         SharedString*   name,
                         bool            isUnion)
{
#ifdef DEBUG
    string stateName("parse_class");
    if (name)
    {
        stateName += '(';
        stateName += name->c_str();
        stateName += ')';
    }
    START_STATE(stateName, str);
#endif

    if (klass_)
    {
        klassStack_.push_back(klass_);
    }
    const char* pos = str;

    // Parse the size of the struct/union/class,
    // in bytes; expect a base-ten number
    EXPECT(T_NUMBER, next_token(pos, str));
    const size_t nbits = strtoul(pos, 0, 10) * byte_size;

    klass_ = new ClassTypeImpl(NULL, name, nbits, isUnion);

    types_.manage(klass_.get());

    if ((*str == T_INHERITANCE) && !parse_base_classes(++str, typeID, name))
    {
        return false;
    }

    bool result = true;

    for (bool isFunction = false; *str; )
    {
        START_STATE("parse_class_member", str);

        if (*str == T_SEMICOLON)
        {
            ++str;
            END_STATE(true);
            break;
        }
        else
        {
            assert(*str);
            if (!parse_member(str, typeID, isFunction))
            {
                result = false;
                break;
            }
            EXPECT(T_SEMICOLON, next_token(pos, str));

        }

        /* if (*str == T_SEMICOLON) // done?
        {
            ++str;
            END_STATE(true);
            break;
        } */

        END_STATE(true);
    }

    // Get the base class that contains the vptr
    // (if the class has virtual methods.)
    if (result && *str == '~')
    {
        START_STATE("parse_tilde", str);

        EXPECT(T_PERCENT, *++str);
        ++str;

        TypeID baseType; // first base class
        if (!parse_type(str, baseType)) return false;

        EXPECT(T_SEMICOLON, next_token(pos, str));
        END_STATE(true);
    }

    // Add the class to the current unit's type tables.
    assert(klass_);
    current_unit()->add_type(typeID, klass_.get());

    if (!klassStack_.empty())
    {
        klass_ = klassStack_.back();
        klassStack_.pop_back();
    }
    return END_STATE(true);
}


/**
 * Parse the definition of an enumerated type
 * and construct a map from values to names.
 */
bool
ParseEvents::parse_enum(const char*&    str,
                        const TypeID&   typeID,
                        unsigned int    nbytes,
                        SharedString*   name)
{
    START_STATE("parse_enum", str);

    /*** TODO ***/
    /* Is the maximum size of an enumerated type
       compiler-dependent? should I use a typedef
       from platform.h rather than int32_t? */
    map<int32_t, RefPtr<SharedString> > namesMap;

    for (const char* pos = str; *str; )
    {
        EXPECT(T_IDENT, next_token(pos, str));
        RefPtr<SharedString> valName(SharedStringImpl::create(pos, str));

        EXPECT(T_COLON, next_token(pos, str));
        EXPECT(T_NUMBER, next_token(pos, str));

        namesMap.insert(make_pair(atoi(pos), valName));

        EXPECT(T_COMMA, next_token(pos, str));

        if (*str == ';')
        {
            ++str;
            break;
        }
    }
    /*** see note above regarding int32_t ***/
    DataType* type =
        types_.manage(new EnumTypeImpl<>(name, namesMap));
    current_unit()->add_type(typeID, type);

    return END_STATE(true);
}


bool
ParseEvents::parse_global(const stab_t&   stab,
                          const char*&    ptr,
                          TypeID&         typeID,
                          SharedString*   name)
{
    START_STATE("parse_global", ptr);

    if (!parse_type(++ptr, typeID, name))
    {
        return false;
    }

    if (name)
    {
        RefPtr<DataType> type =
            current_unit()->get_type(types_, typeID, true);
        RefPtr<Variable> var(
            new GlobalVariable(*name, *type, stab.value()));

        current_unit()->globals().insert(make_pair(name, var));
    }
    return END_STATE(true);
}


/**
 * Deal with forward-declared structs, unions and enums
 */
bool
ParseEvents::parse_fwd_type(const char*     pos,
                            const char*&    str,
                            const TypeID&   typeID)
{
    START_STATE("parse_fwd_type", str);
    assert(*pos == 'x');
    ++pos;

    assert(*pos == 's' || *pos == 'u' || *pos == 'e');

    RefPtr<SharedString> ident(SharedStringImpl::create(++pos, str));
    ForwardType* type = new ForwardType(ident.get(), typeID);
    types_.manage(type);
    current_unit()->add_type(typeID, type);

    if (*str == T_COLON)
    {
        ++str;
    }
    return END_STATE(true);
}


/**
 * Parse function types
 */
bool
ParseEvents::parse_fun_type(const char*&    str,
                            const TypeID&   typeID,
                            SharedString*   ident,
                            bool            isStatic)
{
    // return type and params
    TypeID              tmp;
    RefPtr<DataType>    retType;
    ParamTypes          argTypes;

    string linkageName;

    const char* pos = str;

    START_STATE("parse_fun_type", str);

    if (*str == '#')
    {
        TRACE();

        // C++ method, parse the return type
        if (!parse_type(++str, tmp, retType))
        {
            return false;
        }
        EXPECT(T_SEMICOLON, next_token(pos, str));
    }
    else if (isStatic)
    {
        if (!parse_type(str, tmp, retType))
        {
            return false;
        }
        if (*str == T_SEMICOLON && isdigit(str[1]))
        {
            ++str;
        }
    }
    else
    {
        TRACE();

        // parse parameter types and return type
        START_STATE("parse_param", str);

        RefPtr<DataType> klass;

        for (; *str; )
        {
            RefPtr<DataType> type;
            if (!parse_type(str, tmp, type))
            {
                return false;
            }
            if (klass.is_null())
            {
                klass = type;
            }
            else if (retType.is_null())
            {
                retType = type;
            }
            else if (!interface_cast<VoidType*>(type.get()))
            {
                argTypes.push_back(WeakDataTypePtr(type.get()));
            }
            if (*str == T_COMMA)
            {
                ++str;
                continue;
            }

            EXPECT(T_SEMICOLON, next_token(pos, str));
            break;
        }
        END_STATE(true);
    }

    if (*str == T_COLON) // mangled C++ params -- not used
    {
        EXPECT(T_COLON, next_token(pos, str));

        if (*str != T_SEMICOLON)
        {
            if ((str = strchr(pos = str , ';')) == NULL)
            {
                EXPECT(T_SEMICOLON, 0);
            }
            else
            {
                assert(str >= pos);
                assert(pos);

                linkageName = string(pos, str);
            }
        }
        if (*str == T_SEMICOLON && isdigit(str[1]))
        {
            ++str;
        }
    }
    Access access = ACCESS_PUBLIC;

    if (isdigit(*str))
    {
        switch (*str++)
        {
        case '0': access = ACCESS_PRIVATE; break;
        case '1': access = ACCESS_PROTECTED; break;
        case '9': // optimized out

        case '2': // public (the default)
            break;
        }
    }
    else
    {
        add_fun_type(&typeID, ident, linkageName.c_str(), retType, &argTypes);
        return END_STATE(true);
    }

    Qualifier qual = QUALIFIER_NONE;
    switch (*str++)
    {
    case 'A': break;
    case 'B': qual = QUALIFIER_CONST; break;
    case 'C': qual = QUALIFIER_VOLATILE; break;
    case 'D': qual = QUALIFIER_CONST_VOLATILE; break;
        break;

    default: return false; // unexpected
    }

    bool isVirtual = false;
    long virtFunIndex = 0;

    switch (*str)
    {
    case '*':
        isVirtual = true; // virtual method

        // expect the index of the virtual method
        EXPECT(T_NUMBER, next_token(pos, ++str));
        virtFunIndex = strtol(pos, 0, 0);

        EXPECT(T_SEMICOLON, next_token(pos, str));

        // expect the type of the first base class in the
        // inheritance hierarchy defining the virtual member
        // function
        if (!parse_type(str, tmp))
        {
            return false;
        }
        // fallthru
    case '.':
    case '?':
        ++str;
        break;
    }

    // FIXME: I don't know not exactly why this happens for
    //   __comp_ctor:: (functions generated by g++ 3.2.2)
    //  or maybe it is a bug in my parsing?
    // if (!interface_cast<FunType*>(retType.get()))
    {
        add_fun_type(&typeID,
                    ident,
                    linkageName.c_str(),
                    retType,
                    &argTypes,
                    access,
                    isVirtual,
                    qual,
                    virtFunIndex);
    }
    if (*str != ';')
    {
        START_STATE("parse_memfun", str);

        TRACE();
        END_STATE(parse_mem_fun(str, tmp, ident));
    }
    return END_STATE(true);
}



/**
 * Parse a floating-point type
 */
bool
ParseEvents::parse_builtin_fp_type(const char*&    str,
                                   const TypeID&   typeID,
                                   SharedString*   name)
{
    const char* pos = str;

    START_STATE("parse_builtin_fp_type", str);
    EXPECT(T_NUMBER, next_token(pos, str));

    EXPECT(T_SEMICOLON, next_token(pos, str));
    EXPECT(T_NUMBER, next_token(pos, str));

    size_t nbytes = strtoul(pos, 0, 0);

    EXPECT(T_SEMICOLON, next_token(pos, str));
    EXPECT(T_NUMBER, next_token(pos, str));
    EXPECT(T_SEMICOLON, next_token(pos, str));

    DataType* type = types_.get_float_type(name, nbytes);
    current_unit()->add_type(typeID, type);

    return END_STATE(true);
}


bool
ParseEvents::parse_qualified_type(const char*&    str,
                                  TypeID&         typeID,
                                  SharedString*   name,
                                  int             token)
{
    START_STATE("parse_type_qualifier", str);

    TypeID qualifiedID;
    if (!parse_type(str, qualifiedID))
    {
        return false;
    }

    RefPtr<DataType> qualifiedType =
        current_unit()->get_type(types_, qualifiedID);

    DataType* type = NULL;
    switch (token)
    {
    case T_CONST_QUAL:
        type = types_.get_qualified_type(qualifiedType.get(), QUALIFIER_CONST);
        break;

    case T_VOLATILE_QUAL:
        type = types_.get_qualified_type(qualifiedType.get(), QUALIFIER_VOLATILE);
        break;

    default:
        assert(false);
    }
    assert(type);
    current_unit()->add_type(typeID, type);
    return END_STATE(true);
}


/**
 * Helper called by parse_range, convert string to 64-bit integer.
 */
static inline int64_t strtoint(const char* str)
{
    int base = 0; // let strtoull guess the base

    if (*str == '0') // octal
    {
        base = 8;

        if (*++str == '0')
        {
            return (*++str == '2')
                ?  -strtoull(str, 0, 8) : strtol(str, 0, 8);
        }
    }
    return strtoull(str, 0, base);
}


bool ParseEvents::parse_range(const char*& str, IntRange& range)
{
    const char* pos = str;

    EXPECT(T_NUMBER, next_token(pos, str));
    range.first = strtoint(pos);

    EXPECT(T_SEMICOLON, next_token(pos, str));
    EXPECT(T_NUMBER, next_token(pos, str));

    range.second = strtoull(pos, 0, 0);

    // When debugging 32-bit apps, interpret -1 as (uint32_t)-1
    // rather than (uint64_t)-1
    if ((pos[0] == '-') && (pos[1] == '1')
     && (pos[2] == 0 || pos[2] == ';')
     && (types_.word_size() == 32))
    {
        range.second &= 0xffffffff;
    }

    EXPECT(T_SEMICOLON, next_token(pos, str));
    return true;
}


bool
ParseEvents::parse_range_type(const char*&    str,
                              const TypeID&   typeID,
                              SharedString*   name)
{
    START_STATE("parse_range_type", str);

    const char* pos = str;

    EXPECT(T_TYPE_KEY, next_token(pos, str));

    EXPECT(T_SEMICOLON, next_token(pos, str));

    IntRange range;

    if (!parse_range(str, range)) return false;

    // make new type and add it to the current compilation unit
    DataType* type = NULL;

    if ((range.first > 0) && (range.second == 0))
    {
        type = types_.get_float_type(name, range.first);
    }
    else
    {
        // todo: make sure is integral, by examining rangeType
        size_t nbits = 0;
        uint64_t n = range.second - range.first;
        if ((range.second != 0) && (n == 0))
        {
            nbits = 64;
        }
        for (; n; n >>= 1, ++nbits)
        {}
        assert(nbits || n == 0);
        bool isSigned = (range.first < 0);

        // hack around STABS reporting the char type
        // as an unsigned value in the 0-127 range.
        if (name && name->is_equal("char") && nbits == 7)
        {
            ++nbits;
            isSigned = true;
        }
        type = types_.get_int_type(name, nbits, isSigned);
    }
    current_unit()->add_type(typeID, CHKPTR(type));
    return END_STATE(true);
}


bool
ParseEvents::parse_mem_fun(const char*&    str,
                           TypeID&         type,
                           SharedString*   name)
{
    START_STATE("parse_mem_fun", str);

    const char* pos = str;

    EXPECT(T_TYPE_KEY, next_token(pos, str));
    type = parse_type_id(pos);

    const bool isStatic = (*str == T_COLON);
    str = pos;

    if (isStatic)
    {
        TRACE_(name);
        return END_STATE(parse_fun_type(str, type, name, isStatic));
    }

    return END_STATE(parse_type(str, type, name));
}



/**
 * Parse a class member (can be data or member function)
 */
bool
ParseEvents::parse_member(const char*&    str,
                          const TypeID&   classID,
                          bool&           isFunction)
{
    const char* pos = str;

    START_STATE("parse_member", str);

    RefPtr<SharedString> ident; // member name

    if (*pos == T_COLON)
    {
        //
        // unnamed
        //
    }
    else
    {
        EXPECT(T_IDENT, next_token(pos, str));

        // for GCC 3.2
        if (strncmp(pos, "_vptr.", 6) == 0)
        {
            ident = shared_string(".vptr");
        }
        else
        {
            const char* tmp = str;
            while ((tmp > pos) && (*(tmp - 1) == ':'))
            {
                --tmp;
            }
            // make sure that there are no trailing spaces
            while ((tmp > pos) && (*(tmp - 1) == ' '))
            {
                --tmp;
            }
            ident.reset(SharedStringImpl::create(pos, tmp));
        }
    }

    TypeID typeID;
    bool oldvptr = false;

    if (*str == '(') // is function or .vf or .vb?
    {
        // GCC 2.95? The GCC 3 series emit _vptr.the_klass_name
        if (*pos == '.' && pos[1] == 'v')
        {
            if (pos[2] == 'f')
            {
                ident = shared_string(".vptr");
            }
            else if (pos[2] == 'b')
            {
                ident = shared_string(".vbase");
            }
            oldvptr = true;
            EXPECT(T_TYPE_KEY, next_token(pos, str));
            pos = str;
        }
        else
        {
            isFunction = true;
            return END_STATE(parse_mem_fun(str, typeID, ident.get()));
        }
    }

    assert(!isFunction);
    EXPECT(T_COLON, next_token(pos, str));

    Access access = ACCESS_PUBLIC;

    if (*str == '/') // access specifer?
    {
        switch (*++str)
        {
        case '0': access = ACCESS_PRIVATE; break;
        case '1': access = ACCESS_PROTECTED; break;
        case '9': // optimized out

        case '2': // public (the default)
        default:
            break;
        }
        ++str;
    }

    if (!parse_type(str, typeID))
    {
        return false;
    }

    RefPtr<DataType> type = current_unit()->get_type(types_, typeID);

    if (*str == T_COLON) // static member?
    {
        ++str;
        EXPECT(T_IDENT, next_token(pos, str));

        RefPtr<SharedString> linkName = SharedStringImpl::create(pos, str);

        // add static member
        if (klass_.is_null())
        {
            THROW(runtime_error("no class"));
        }
        klass_->add_member(ident, linkName, 0, 0, *type, true);
        current_unit()->add_type(klass_->name(), klass_);
    }
    else if (*str == T_COMMA)
    {
        ++str;

        EXPECT(T_NUMBER, next_token(pos, str));
        const size_t bitOffs = strtoul(pos, 0, 10);

        size_t bitSize = 0;
        if (oldvptr)
        {
            // implicitly assume the size of an address
            bitSize = types_.word_size();
        }
        else
        {
            EXPECT(T_COMMA, next_token(pos, str));
            EXPECT(T_NUMBER, next_token(pos, str));

            bitSize = strtoul(pos, 0, 10);
        }

        // add member data to the current class
        if (ClassTypeImpl* klass = klass_.get())
        {
            if (ident && ident->is_equal2(type->name()))
            {
                // Workaround GCC STABS representing bases as
                // aggregated parts -- if one names the member
                // the same as the class, then this is not going
                // to work; I guess I'll just take the chance.
                //
                // Also, it seems that g++ -g means by default
                // stabs+ for gcc 2.95, so it is a low risk
                //
                klass->add_base(*type, bitOffs, access, false);
            }
            else
            {
                /*
                clog << __func__ << ": " << klass->name() << ": "
                     << ident->c_str() << " bitsize=" << bitSize
                     << " bitOffs=" << bitOffs << endl;  */

                // if compiled with -gstabs rather than -gstabs+
                const bool isStatic = (bitSize == 0);
                klass->add_member(ident, 0, bitOffs, bitSize, *type, isStatic);
            }
        }
        else
        {
            TRACE_("NULL class");
        }
    }
    return END_STATE(true);
}


/**
 * Negative types: "[...] is the method used in XCOFF for defining
 * builtin types. Since the debugger knows about the builtin types
 * anyway, the idea of negative type numbers is simply to give a special
 * type number which indicates the builtin type. There is no stab defining
 * these types."
 * http://sources.redhat.com/gdb/current/onlinedocs/stabs_5.html#SEC35
 *
 * I have tested with GCC 2.95 thru 4.1.x and it seems that the boolean
 * type is the only one for which a negative type is emitted (at least
 * for C/C++).
 * Given that a) I only care about ELF, and
 * b) STAB is fading away (Intel Compiler 9 does not bother supporting it)
 * I don't think it is worth supporting negative types.
 */
void ParseEvents::parse_negative_types( const char*     pos,
                                        const TypeID&   typeID,
                                        SharedString*   name,
                                        size_t          size)
{
    const int n = atoi(pos);
    switch (n)
    {
    case -16:
        {
            DataType* type = types_.get_bool_type(size);
            current_unit()->add_type(typeID, type);
        }
        break;

    // just for reference:
    case -1: // int, 32 bit signed integral type.
    case -2: // char, 8 bit type holding a character.
    case -3: // short, 16 bit signed integral type.
    case -4: // long, 32 bit signed integral type.
    case -5: //  unsigned char, 8 bit unsigned integral type.
    case -6: // signed char, 8 bit signed integral type.
    case -7: // unsigned short, 16 bit unsigned integral type.
    case -8: //  unsigned int, 32 bit unsigned integral type.
    case -9: // unsigned, 32 bit unsigned integral type.
    case -10:// unsigned long, 32 bit unsigned integral type.
    case -11:// void, type indicating the lack of a value.
    case -12:// float, IEEE single precision.
    case -13:// double, IEEE double precision.
    case -14:// long double, IEEE double precision. The compiler claims
             // the size will increase in a future release, and for binary
             // compatibility you have to avoid using long double. I hope when
             // they increase it they use a new negative type number.

    case -15:// integer. 32 bit signed integral type.

    case -17:// short real. IEEE single precision.

    case -18:// real. IEEE double precision.

    case -19:// stringptr. See section 5.6 Strings.

    case -20:// character, 8 bit unsigned character type.

    case -21:// logical*1, 8 bit type. This Fortran type has a
             // split personality in that it is used for boolean
             // variables, but can also be used for unsigned integers.
             // 0 is false, 1 is true, and other values are non-boolean.

    case -22:// logical*2, 16 bit type.
    case -23:// logical*4, 32 bit type.
    case -24:// logical, 32 bit type.
    case -25:// complex. A complex type consisting of two IEEE
             // single-precision floating point values.

    case -26:// complex. A complex type consisting of two IEEE double-precision
             //floating point values.

    case -27:// integer*1, 8 bit signed integral type.

    case -28:// integer*2, 16 bit signed integral type.
    case -29:// integer*4, 32 bit signed integral type.
    case -30:// wchar. Wide character, 16 bits wide, unsigned (what format? Unicode?).
    case -31:// long long, 64 bit signed integral type.
    case -32:// unsigned long long, 64 bit unsigned integral type.
    case -33:// logical*8, 64 bit unsigned integral type.
    case -34:// integer*8, 64 bit signed integral type.
        {
            ostringstream err;
            err << "STAB: unhandled negative type: " << n;

            throw runtime_error(err.str());
        }
        break;

    default: // out of the specified range
        {
            ostringstream err;
            err << "STAB: unspecified negative type: " << n;

            throw range_error(err.str());
        }
        break;
    }
}


/**
 * Parse pointer or reference type (assuming that,
 * under the hood, C++ compilers implement references
 * as pointers).
 */
bool ParseEvents::parse_pointer(
    const char*     pos,
    const char*&    str,
    const TypeID&   typeID,
    SharedString*   ident)
{
    START_STATE("parse_pointer", str);

    // parse pointed-to (or referred) type
    TypeID refTypeID;

    if (!parse_type(str, refTypeID))
    {
        return false;
    }

    RefPtr<DataType> type = current_unit()->get_type(types_, refTypeID);

    DataType* ptrType = NULL;
    if (*pos == '*')
    {
        ptrType = types_.get_pointer_type(type.get());
    }
    else
    {
        assert(*pos == '&');
        ptrType = types_.get_reference_type(type.get());
    }
    assert(ptrType);
    current_unit()->add_type(typeID, ptrType);
    return END_STATE(true);
}



void ParseEvents::add_local_var(const stab_t&   stab,
                                const TypeID&   typeID,
                                SharedString&   name)
{
    RefPtr<DataType> type = current_unit()->get_type(types_, typeID, true);
    RefPtr<Variable> var;

    switch (stab.type())
    {
    case N_LCSYM:
    case N_STSYM:
        // variable has global lifetime, its name has local scope
        var = new GlobalVariable(name, *type, stab.value());
        break;

    case N_PSYM:
        var = new Parameter(name, *type, stab.value());
        param_.push_back(var);
        return;

    case N_RSYM:
    case N_LSYM:
        {
            VarList::reverse_iterator i = param_.rbegin();
            for (; i != param_.rend(); ++i)
            {
                assert(interface_cast<Parameter*>(i->get()));
                if ((*i)->name() == name)
                {
                    *i = new Parameter(name, *type, (*i)->offset());
                    goto BREAK;
                }
            }
        }
        var.reset(new Variable(name, *type, stab.value()));

    BREAK:
        break;

    default: assert(false);
    }

    if (!var.is_null())
    {
        vars_.push_back(var);
    }
}



static bool inline needs_mangling(const char* linkName)
{
    return !linkName || linkName[0] != '_'
        || (linkName[1] != 'Z' && linkName[1] != '_');
}


RefPtr<FunType> ParseEvents::add_fun_type(
    const TypeID*           typeID,
    SharedString*           name,       // short name
    const char*             linkName,   // mangled name
    const RefPtr<DataType>& retType,
    const ParamTypes*       argTypes,
    Access                  access,
    bool                    isVirtual,
    Qualifier               qual,
    long                    virtFunIndex)
{
    ParamTypes paramTypes;

    if (argTypes && !argTypes->empty())
    {
        paramTypes.assign(argTypes->begin(), argTypes->end());
    }
    // accepts variable number of arg?
    bool varArgsOk = !linkName || linkName[0] == 0;
 /*
    // note: reading the STABS format is slow because of the linear
    // (non-indexed) structure, unmangling here worsens performance;
    // any better ideas? The main place where this information is
    // needed is in the expression interpreter
    if (char* fn = unmangle(linkName, 0, 0, UNMANGLE_NOFUNARGS))
    {
        varArgsOk = strstr(fn, "...");
        free(fn);
    }
    else // looks like an extern "C" function
    {
        varArgsOk = true;
    }
 */
    FunType* funType =
        get_function_type(types_, retType, paramTypes, varArgsOk);

    assert(funType);

    if (!typeID)
    {
        assert(false); // should never happen
    }
    else
    {
        current_unit()->add_type(*typeID, funType);

        if (klass_.get()) // currently parsing a class?
        {
            ostringstream mangledName;

            // normally, the mangled name should contain the "plain"
            // name as a substring -- otherwise, it means the compiler
            // optimized the length of the string;
            // (GNU C++ 2.95 seems to be doing it, but not 3.2.2 nor 3.4.0
            // todo: investigate other compilers

            if (name && needs_mangling(linkName))
            {
                // hack around and manually mangle the function name
                assert(!name->is_equal("__base_ctor"));
                assert(!name->is_equal("__comp_ctor"));

                const char* klassName =
                    CHKPTR(klass_->unqualified_name())->c_str();

                if (!name->is_equal(klassName))
                {
                    mangledName << CHKPTR(name->c_str());
                }
                mangledName << "__";

                if (qual & QUALIFIER_CONST)
                {
                    mangledName << 'C';
                }
                if (qual & QUALIFIER_VOLATILE)
                {
                    mangledName << 'V';
                }
                if (linkName && strstr(linkName, klassName))
                {
                    mangledName << linkName;
                }
                else
                {
                    mangledName << klass_->unqualified_name()->length();
                    mangledName << klassName;

                    if (linkName)
                    {
                        mangledName << linkName;
                    }
                }
            }
            else if (linkName)
            {
                mangledName << linkName;
            }

            RefPtr<MethodImpl> method = klass_->add_method(
                name,
                shared_string(mangledName.str()),  // linkage name
                funType,
                access,
                isVirtual,
                qual);

            method->set_vtable_offset(virtFunIndex);
            if (method->linkage_name())
            {
                current_unit()->add_method(*method->linkage_name(), method);
            }
        }
    }
    return funType;
}

// Copyright (c) 2004, 2005, 2006, 2007 Cristian L. Vlasceanu
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

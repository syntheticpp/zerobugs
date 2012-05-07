#ifndef DECODER_H__7B3B1118_B516_411D_B449_95D7E5C32248
#define DECODER_H__7B3B1118_B516_411D_B449_95D7E5C32248
//
// $Id$
//
// Decoder for C++ Itanium ABI (aka V3 ABI) mangled names.
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include "config.h"
#include <boost/static_assert.hpp>
#include <sstream>
#ifdef USE_STD_CONTAINERS
 #include <stack>
#endif
#include "nuts/assert.h"
#include "nuts/fixed_stack.h"
#include "nuts/string.h"
#include "nuts/substring.h"
#include "nuts/temporary.h"
#include "decoder_macros.h"
#include "dictionary.h"
#include "unmangle.h"

#define USE_CXXABI  1

#if USE_CXXABI
 #include <cxxabi.h>
 using namespace __cxxabiv1;
#endif



#if defined(CPLUS_DEMANGLE_COMPAT) && !defined(USE_SHORT_STD_SUBST)
 #define USE_SHORT_STD_SUBST
#endif
#ifdef DEBUG_SUBST
#define add_substitution(o,p,f)                                     \
 clog << '(' << __LINE__ << "): " << (o).substr(p) << endl;         \
 add_substitution_(o,p,f)
#else
 #define add_substitution(o,p,f) add_substitution_(o,p,f)
#endif
/**
 * Decodes a name mangled according to the C++ Itanium ABI
 * specification. Implemented as a template, so that we don't have
 * to do a separate conversion in cases where the output has to be
 * a wide-char string.
 * @note: the parse() method is NOT idempotent. You need
 * one Decoder instance for each input (mangled) string.
 *
 * For C++ mangling info:
 * @see http://www.codesourcery.com/cxx-abi/abi.html
 */
template<typename CharT, size_t MAX_TEMP = 512>
class Decoder
{
public:
    typedef nuts::string<CharT> string_type;

    typedef nuts::substring<string_type> substring_type;

    typedef typename DictBase<substring_type>::container_type container_type;

    typedef Dictionary<substring_type, SubstPolicy<container_type> > Substitutions;
    typedef Dictionary<substring_type> TemplateArgs;

#include "output.h"

#if defined (USE_STD_CONTAINERS)
    typedef std::vector<Output*> TempOutputs;
#else
    typedef nuts::fixed_stack<Output*, MAX_TEMP> TempOutputs;
#endif

    /// The purpose of these flags is to indicate whether
    /// a sequence can be a substitution candidate -- not reliable
    /// for any other purposes.
    struct NameFlags
    {
        bool isTemplate_: 1;
        bool isCtor_    : 1;
        bool isDtor_    : 1;
        bool isOperator_: 1;
        bool isConvOper_: 1;    // conversion operator?
        bool isSource_  : 1;    // is a <source-name>?

        NameFlags()
            : isTemplate_(false)
            , isCtor_(false)
            , isDtor_(false)
            , isOperator_(false)
            , isConvOper_(false)
            , isSource_(false)
        { }
        bool may_have_return_type() const
        {
            return isTemplate_ && !isCtor_ && !isDtor_ && !isConvOper_;
        }
        bool may_substitute(CharT c = 0) const
        {
            return (c == 'I' || (!isOperator_ && !isDtor_ && !isCtor_));
        }
    };

public:
    Decoder(const char* name, size_t size, int flags)
        : name_(name)
        , size_(size ? size : (name ? strlen(name) : 0))
        , flags_(flags)
        , pos_(0)
        , sourceNamePos_(-1)
        , sourceNameSize_(0)
        , cvQualifiers_(0)
        // It takes at least 2 chars for a substitution (S_);
        // we can estimate the maximum size of the dictionary.
        , subst_((size_ + 1) / 2)
        //ditto for the template args dictionary
        , templateArgs_((size_ + 1) / 2)
        , templateDepth_(0)
        , inPrototype_(0)
        , inPrefix_(0)
        , anonymous_(false)
        , status_(0)
    {
    }
    ~Decoder()
    {
        std::for_each(temp_.begin(), temp_.end(), boost::checked_deleter<Output>());
    }
    /// demangler entry point
    CharT* parse(int* status = 0, size_t* len = 0)
    {
        CharT* result = 0;
        if (name_[0] == '_' && name_[1] == 'Z')
        {
            pos_ = 2;
            Output output;
    #ifdef CPLUS_DEMANGLE_BUG_COMPAT
            string_type qualifiers;
            Temporary<string_type*>__(cvQualifiers_, &qualifiers);
    #endif
            parse_encoding(output); // see <encoding> below
    #ifdef CPLUS_DEMANGLE_BUG_COMPAT
            output.append(qualifiers);
    #endif
            if (status_ == UNMANGLE_STATUS_SUCCESS)
            {
                result = output.detach(len);
            }
        }
        else if ((strncmp("_GLOBAL_", name_, 8) == 0)
	        && (name_[8] == '.' || name_[8] == '_' || name_[8] == '$')
	        && (name_[9] == 'D' || name_[9] == 'I')
	        && name_[10] == '_')
        {
            return parse_global_ctor_dtor(status, len);
        }
        else
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        if (status)
        {
            *status = status_;
        }
        return result;
    }

    /// parse _GLOBAL__I_[...], etc.
    CharT* parse_global_ctor_dtor(int* status, size_t* len)
    {
        Output output;
        if (name_[9] == 'I')
        {
            output.append("global constructors keyed to ", 29);
        }
        else
        {
            output.append("global destructors keyed to ", 28);
        }
        pos_ = 11;
#ifndef CPLUS_DEMANGLE_COMPAT
        if (name_[pos_] == '_' && name_[pos_ + 1] == 'Z')
        {
            pos_ += 2;
            parse_encoding(output);
        }
        else
#endif // CPLUS_DEMANGLE_COMPAT
        {
            output.append(name_ + 11, size_ - 11);
        }
        if (status) { *status = status_; }
        return status_ ? 0 : output.detach(len);
    }
private:
#ifdef DEBUG
    void parse_error() { } // for setting debugger breakpoints
#endif
    void parse_encoding(Output& out)
    {
        if (pos_ >= size_)
        {
            DECODER_ERROR__(INVALID_NAME); // unexpected end of input
        }
        else if (!parse_special_name(out))
        {
            parse_data_or_func_name(out);

            if ((pos_ < size_) && (name_[pos_] != 'E'))
            {
                //DECODER_ERROR__(INVALID_NAME);
                // copy the remainder of string to output,
                // rather than issuing an error
                out.append(name_ + pos_, size_ - pos_);
                pos_ = size_;
            }
        }
    }
    /// <encoding>   ::= <function name><bare-function-type>
    ///              ::= <data name>
    ///              ::= <special-name>
    /// This method deals with the first 2 productions
    void parse_data_or_func_name(Output& out)
    {
        if (pos_ >= size_)
        {
            DECODER_ERROR__(INVALID_NAME);
            return;
        }
#ifndef CPLUS_DEMANGLE_BUG_COMPAT
        string_type qualifiers;
        Temporary<string_type*>__(cvQualifiers_, &qualifiers);
#endif
        NameFlags nameFlags;
        Output& temp = temp_output();
        if (!parse_name(temp, nameFlags))
        {
            DECODER_ERROR__(INVALID_NAME);
            return;
        }
        else
        {
            bool isFuncName = false;
            if (nameFlags.may_have_return_type())
            {
                Temporary<size_t>__(inPrototype_, inPrototype_ + 1);
                Output* outp = &out;
                if (flags_ & UNMANGLE_NOFUNARGS)
                {
                    outp = &temp_output();
                }
                if (parse_type(*outp, true, 0))
                {
                    outp->append(" ", 1);
                    isFuncName = true;
                }
            }
            out.append(temp);
            if (isFuncName || (pos_ < size_ && name_[pos_] != 'E'))
            {
                if (emit_function_params(out))
                {
                    //subst_.insert(out.substr(pos), true);
                }
                else
                {
                    return;
                }
            }
        }
#ifndef CPLUS_DEMANGLE_BUG_COMPAT
        if ((flags_ & UNMANGLE_NOFUNARGS) == 0)
        {
            out.append(qualifiers);
        }
#endif
    }
    /// <name>  ::= <nested-name>
    ///         ::= <unscoped-name>
    ///         ::= <unscoped-template-name> <template-args>
    ///         ::= <local-name>
    ///
    /// <unscoped-template-name> ::= <unscoped-name>
    ///                          ::= <substitution>
    /// @return true when a NAME is recognized.
    bool parse_name(Output& out, NameFlags& nameFlags)
    {
        if (pos_ < size_)
        {
            typename Output::Position pos(out);

            nameFlags.isSource_ = false;
            switch (name_[pos_])
            {
            case 'N':
                parse_nested_name(out, nameFlags);
                return true;
            case 'Z':
                parse_local_name(out);
                return true;
            case 'S':
                {   // may be an unscoped-template-name, or
                    // an unscoped-name starting with St
                    bool isStd = (name_[pos_ + 1] == 't');
                    parse_substitution(out, nameFlags);

                    if (isStd)
                    {
                        Temporary<const CharT*>__(out.delim_, "::");
                        if (!parse_unscoped_name(out, nameFlags))
                        {
                            return false;
                        }
                    }
                    if (name_[pos_] == 'I')
                    {
                        add_substitution(out, pos, nameFlags);
                        parse_template_args(out, nameFlags);
                    }
                }
                return (status_ == UNMANGLE_STATUS_SUCCESS);

            default:
                if (parse_unscoped_name(out, nameFlags))
                {
                    if ((pos_ < size_) && (name_[pos_] == 'I'))
                    {
                        add_substitution(out, pos, nameFlags);
                        //unscoped-template-name
                        parse_template_args(out, nameFlags);
                    }
                    return true;
                }
                break;
            }
        }
        return false;
    }
    ///<unscoped-name> ::= <unqualified-name>
    ///                ::= St <unqualified-name> # ::std::
    /// @return true when an UNSCOPED-NAME is recognized.
    bool parse_unscoped_name(Output& out, NameFlags& nameFlags)
    {
    #if 0
        NUTS_ASSERT(pos_ < size_);
        // the name production above has already checked for this case:
        if ((pos_ + 1 < size_) && name_[pos_] == 'S' && name_[pos_+1] == 't')
        {
            out.append("std", 3);
            pos_ += 2;
        }
    #endif
        return (pos_ < size_) ? parse_unqualified_name(out, nameFlags) : false;
    }

    ///<lambda-sig> ::= <parameter type>+  # Parameter types or "v" if the lambda has no parameters
    void parse_lambda_sig(Output& out)
    {
        NUTS_ASSERT(name_[pos_] == 'l');
//
// TODO
//
    }

    ///<local-name> ::= Z <function encoding> E <entity name> [<discriminator>]
    ///             ::= Z <function encoding> E s [<discriminator>]
    ///<discriminator> ::= _ <non-negative number>
    void parse_local_name(Output& out)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'Z');
        ++pos_;
        parse_encoding(out);
        if ((pos_ + 1 >= size_) || (name_[pos_] != 'E'))
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else
        {
            ++pos_;
            out.append("::", 2);
            size_t i = 0;
            NameFlags nameFlags;
            if (name_[pos_] == 's')
            {
                out.append("string literal", 14);
                ++pos_;
            }
            else if (parse_name(out, nameFlags))
            {
                ++i;
            }
            else
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            if ((pos_ < size_) && (name_[pos_] == '_'))
            {
                ++pos_;
                size_t n = parse_unsigned() + i;
                out.append(" [#", 3);
                out.append(n);
                out.append("]", 1);
            }
        }
    }
    ///<unqualified-name>::= <operator-name>
    ///                  ::= <ctor-dtor-name>
    ///                  ::= <source-name>
    ///                  ::= <local source-name>
    ///                  ::= <unnamed-type-name>
    /// <local-source-name> ::= L <source-name> <discriminator>
    /// <unnamed-type-name> ::= <closure-type-name>
    /// <closure-type-name> ::= <Ul <lambda-sig> E [ <nonnegative number> ] _
    ///
    bool parse_unqualified_name(Output& out, NameFlags& nameFlags)
    {
        NUTS_ASSERT(pos_ < size_);
        bool result = false;
        CharT c = name_[pos_];
        if (c == 'C')
        {
            parse_ctor_dtor_name(out);
            nameFlags.isCtor_ = true;
            result = true;
        }
        else if (c == 'D')
        {
            parse_ctor_dtor_name(out);
            nameFlags.isDtor_ = true;
            result = true;
        }
        /* else if (c == 'U')
        {
            c = name_[++pos_];
            if (c == 't')
            {
                NUTS_ASSERT(!"unhandled");
            }
            else if (c == 'l')
            {
                parse_lambda_sig(out);
                result = true;
            }
        } */
        else if (c != 'E')
        {
            if (c == 'L')
            {
                c = name_[++pos_];
            }
            if (isdigit(c))
            {
                if (templateDepth_ == 0)
                {
                    sourceNamePos_ = out.size() + (out.delim_ ? 2 : 0);
                    sourceNameSize_ = 0;
                }
                parse_source_name(out);
                if (status_ == UNMANGLE_STATUS_SUCCESS)
                {
                    result = true;
                    nameFlags.isSource_ = true;
                }
            }
            else if ((result = parse_operator_name(out)))
            {
                nameFlags.isSource_ = false;
                nameFlags.isOperator_ = true;
            }
        }
        return result;
    }
    ///<source-name> ::= <positive length number> <identifier>
    void parse_source_name(Output& out)
    {
        NUTS_ASSERT(pos_ < size_);
        const size_t len = parse_unsigned();
        if (pos_ + len <= size_)
        {
            if ((len >= 8) && memcmp(name_ + pos_, "_GLOBAL_", 8) == 0)
            {
                register char c = name_[pos_ + 8];
                if ((c == '.' || c == '_' || c == '$')
                    && name_[pos_ + 9] == 'N')
                {
                    typename Output::Position pos(out);

                    BOOST_STATIC_ASSERT(sizeof(ANONYMOUS_NS_PREFIX) == 22);
                    out.append(ANONYMOUS_NS_PREFIX, sizeof(ANONYMOUS_NS_PREFIX) - 1);
                    if (inPrefix_)
                    {
                        anonymous_ = true;
                    }
                    else
                    {
                        subst_.insert(out.substr(pos, sizeof(ANONYMOUS_NS_PREFIX) - 1), true);
                    }
                    pos_ += len;
                    Temporary<const CharT*>(out.delim_, (CharT*)0);
                    out.append("::", 2);
                    if (templateDepth_ == 0)
                    {
                        sourceNamePos_ = out.size();
                        sourceNameSize_ = 0;
                    }
                    parse_source_name(out);
                    return;
                }
            }
            out.append(name_ + pos_, len);
            pos_ += len; // advance read position
        }
        else
        {
            DECODER_ERROR__(INVALID_NAME);
        }
    }
    // <operator-name> ::=
    //   nw             # new
    //   na             # new[]
    //   dl             # delete
    //   da             # delete[]
    //   ng             # - (unary)
    //   ad             # & (unary)
    //   de             # * (unary)
    //   co             # ~
    //   pl             # +
    //   mi             # -
    //   ml             # *
    //   dv             # /
    //   rm             # %
    //   an             # &
    //   or             # |
    //   eo             # ^
    //   aS             # =
    //   pL             # +=
    //   mI             # -=
    //   mL             # *=
    //   dV             # /=
    //   rM             # %=
    //   aN             # &=
    //   oR             # |=
    //   eO             # ^=
    //   ls             # <<
    //   rs             # >>
    //   lS             # <<=
    //   rS             # >>=
    //   eq             # ==
    //   ne             # !=
    //   lt             # <
    //   gt             # >
    //   le             # <=
    //   ge             # >=
    //   nt             # !
    //   aa             # &&
    //   oo             # ||
    //   pp             # ++
    //   mm             # --
    //   cm             # ,
    //   pm             # ->*
    //   pt             # ->
    //   cl             # ()
    //   ix             # []
    //   qu             # ?
    //   sz             # sizeof
    //   sr             # scope resolution (::)
    //   cv <type>      # (cast)
    //   v <digit> <source-name>    # vendor extended operator
    enum
    {
        OPF_NONE        = 0,
        OPF_SPACE_ONCE  = 1,
        OPF_CAST        = 2,
        OPF_VENDOR      = 4,
        OPF_UNARY       = 8,
        OPF_BINARY      = 16,
        OPF_TRINARY     = 24,
        OPF_CALL        = 32,
    };
    struct Operator
    {
        const CharT* demangled_;
        int flags_;
        size_t len_;
    };
    typedef Operator OperatorTable['z' - 'a' + 1]['z' - 'A' + 1];
    /**
     * Operator descriptor, for initializing the OperatorTable
     */
    struct OperatorDesc
    {
        char mangled_[3];
        Operator oper_;
    };
    ///@return true when an operator is recognized
    bool parse_operator_name(Output& out, int* flags = 0)
    {
        NUTS_ASSERT(pos_ < size_);
        //char n = name_[pos_];
        //char m = (pos_ + 1 < size_) ? name_[pos_ + 1] : 0;
        int n = name_[pos_];
        int m = (pos_ + 1 < size_) ? name_[pos_ + 1] : 0;

        if (n >= 'a' && n <= 'z' && m >= 'A' && m <= 'z')
        {
            n -= 'a';
            m -= 'A';
            const Operator& op = operator_table()[n][m];
            if (op.demangled_)
            {
                out.append(op.demangled_, op.len_);
                pos_ += 2;
                if (op.flags_ & OPF_CAST) //conversion
                {
                    if (!parse_type(out, false, 0))
                    {
                        DECODER_ERROR__(INVALID_NAME);
                    }
                }
                else if (op.flags_ & OPF_VENDOR)
                {
                    TODO;
                }
                out.spaceOnce_ = (op.flags_ & OPF_SPACE_ONCE);
                if (flags)
                {
                    *flags = op.flags_;
                }
                return true;
            }
        }
        return false;
    }
#define STR(x) #x
#define OP__(name,flags) { "operator" name, (flags), sizeof("operator" name)-1 }
    static const OperatorTable& operator_table()
    {
        static OperatorTable table;
        static bool initialized = false;
        static const OperatorDesc oper[] =
        {
            { "nw", OP__(" new", 0) },
            { "na", OP__(" new[]", 0) },
            { "dl", OP__(" delete", 0) },
            { "da", OP__(" delete[]", 0) },
            { "ps", OP__("+", OPF_UNARY) },
            { "ng", OP__("-", OPF_UNARY) },
            { "ad", OP__("&", OPF_UNARY) },
            { "de", OP__("*", OPF_UNARY) },
            { "co", OP__("~", OPF_UNARY) },
            { "pl", OP__("+", OPF_BINARY) },
            { "mi", OP__("-", OPF_BINARY) },
            { "ml", OP__("*", OPF_BINARY) },
            { "dv", OP__("/", OPF_BINARY) },
            { "rm", OP__("%", OPF_BINARY) },
            { "an", OP__("&", OPF_BINARY) },
            { "or", OP__("|", OPF_BINARY) },
            { "eo", OP__("^", OPF_BINARY) },
            { "aS", OP__("=", OPF_BINARY) },
            { "pL", OP__("+=", OPF_BINARY) },
            { "mI", OP__("-=", OPF_BINARY) },
            { "mL", OP__("*=", OPF_BINARY) },
            { "dV", OP__("/=", OPF_BINARY) },
            { "rM", OP__("%=", OPF_BINARY) },
            { "aN", OP__("&=", OPF_BINARY) },
            { "oR", OP__("|=", OPF_BINARY) },
            { "eO", OP__("^=", OPF_BINARY) },
            { "ls", OP__("<<", OPF_SPACE_ONCE | OPF_BINARY) },
            { "rs", OP__(">>", OPF_BINARY) },
            { "lS", OP__("<<=", OPF_BINARY) },
            { "rS", OP__(">>=", OPF_BINARY) },
            { "eq", OP__("==", OPF_BINARY) },
            { "ne", OP__("!=", OPF_BINARY) },
            { "lt", OP__("<", OPF_SPACE_ONCE | OPF_BINARY) },
            { "gt", OP__(">", OPF_BINARY) },
            { "le", OP__("<=", OPF_BINARY) },
            { "ge", OP__(">=", OPF_BINARY) },
            { "nt", OP__("!", OPF_UNARY) },
            { "aa", OP__("&&", OPF_BINARY) },
            { "oo", OP__("||", OPF_BINARY) },
            { "pp", OP__("++", 0) },
            { "mm", OP__("--", 0) },
            { "cm", OP__(",", 0) },
            { "pm", OP__("->*", 0) },
            { "pt", OP__("->", 0) },
            { "cl", OP__("()", OPF_CALL) },
            { "ix", OP__("[]", 0) },
            { "qu", OP__("?", OPF_TRINARY) },
            { "st", { "", OPF_UNARY, 0 } },
            { "sz", { "sizeof", OPF_UNARY, 6 } },
            { "sr", { "::", 0, 2 } },
            { "cv", { "operator ", OPF_CAST, 9 } },
            { "v",  { "", OPF_VENDOR, 0 } },
        };
        static const size_t size = sizeof(oper)/sizeof(oper[0]);
        if (!initialized)
        {
            memset(table, 0, sizeof(table));
            for (size_t i = 0; i != size; ++i)
            {
                const OperatorDesc& op = oper[i];
                int n = op.mangled_[0] - 'a';
                int m = op.mangled_[1] - 'A';
                NUTS_ASSERT('z' - 'A' + 1 > m);

                table[n][m] = op.oper_;
            }
            initialized = true;
        }
        return table;
    }
#undef OP__
    /// <prefix> ::= <prefix> <unqualified-name>
    ///          ::= <template-prefix> <template-args>
    ///          ::= <template-param> # starts with a T
    ///          ::= # empty
    ///          ::= <substitution>
    /// <template-prefix>::= <prefix> <template unqualified-name>
    ///                  ::= <template-param> # stats with a T
    ///                  ::= <substitution>
    void parse_prefix(Output& out, NameFlags& nameFlags)
    {
        if (pos_ == size_)
        {
            DECODER_ERROR__(INVALID_NAME); // unexpected end of input
        }
        else
        {
            NUTS_ASSERT(pos_ < size_);
        }
        NUTS_ASSERT(nameFlags.isTemplate_ == false);
        bool initial = true;
        bool havePrefix = false;
        // empty string is a prefix, but not a template-prefix
        bool maybeTemplatePrefix = false;
        typename Output::Position pos(out);

        for (bool canSubstitute = false; pos_ < size_;)
        {
            switch (name_[pos_])
            {
            case 'S':
                if (!initial)
                {
                    return;
                }
                parse_substitution(out, nameFlags);
                havePrefix = maybeTemplatePrefix = true;
                break;
            case 'T':
                if (!initial)
                {
                    return;
                }
                nameFlags.isSource_ = false;
                parse_template_param(out);
                havePrefix = maybeTemplatePrefix = true;
                break;
            case 'I':
                nameFlags.isSource_ = false;
                if (maybeTemplatePrefix)
                {
                    add_substitution(out, pos, nameFlags);
                    const size_t argPos = templateArgs_.size();
                    parse_template_args(out, nameFlags);
                    if (name_[pos_] != 'E')
                    {
                        add_substitution(out, pos, nameFlags);
                        canSubstitute = false;
                    }
                    if ((templateDepth_ == 0) && (inPrototype_ == 0))
                    {
                        TemplateArgs args(templateArgs_, argPos);
                        templateArgs_.swap(args);
                    }
                }
                else
                {
                    DECODER_ERROR__(INVALID_NAME);
                    return;
                }
                break;
            default:
                {
                    Temporary<bool>__(inPrefix_, true);
                    const bool isTemplate = nameFlags.isTemplate_;
                    size_t len = out.size() - pos;
                    if (havePrefix)
                    {
                        out.delim_ = "::";
                        nameFlags.isTemplate_ = false;
                    }
                    if (parse_unqualified_name(out, nameFlags))
                    {
                        havePrefix = maybeTemplatePrefix = true;
                        initial = false;
                        if (canSubstitute)
                        {
                            subst_.insert(out.substr(pos, len), true);
                        }
                        if (anonymous_)
                        {
                            anonymous_ = false;
                            if (out[len + 2] == ':')
                            {
                                ++len;
                            }
                            subst_.insert(out.substr(len + 2, sizeof(ANONYMOUS_NS_PREFIX) - 1), true);
                        }
                        // defer adding substitution to dictionary until
                        // the next iteration, since it may be a function name
                        canSubstitute = nameFlags.may_substitute();
                        break;
                    }
                    nameFlags.isTemplate_ = isTemplate;
                }
                return; // break out the for loop
            }
        }
    }
    /// <nested-name>::= N [<CV-qualifiers>] <prefix> <unqualified-name> E
    ///              ::= N [<CV-qualifiers>] <template-prefix> <template-args> E
    void parse_nested_name(Output& out, NameFlags& nameFlags)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'N');
        ++pos_;

        if (cvQualifiers_)
        {
            parse_cv_qualifiers(*cvQualifiers_);
        }
        if (status_ == UNMANGLE_STATUS_SUCCESS)
        {
            NUTS_ASSERT(pos_ < size_);
            parse_prefix(out, nameFlags);
            //
            // TODO: deal with lambdas
            //
            if ((pos_ >= size_) || (name_[pos_] != 'E'))
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            ++pos_;
        }
    }
    /// ctors and dtors are special cases of <unqualified-name>,
    /// where the final <unqualified-name> of a nested name is
    /// replaced by one of the following:
    /// <ctor-dtor-name> ::= C1 # complete object ctor
    ///                  ::= C2 # base object ctor
    ///                  ::= C3 # complete object allocating ctor
    ///                  ::= C9 # ? generated by Intel Compiler 8
    ///                  ::= D0 # deleting dtor
    ///                  ::= D1 # complete object dtor
    ///                  ::= D2 # complete object dtor
    ///                  ::= D9 # ? generated by Intel Compiler 8
    void parse_ctor_dtor_name(Output& out)
    {
        static const struct Descr
        {
            const char* tilde_; char kind_[5];
        } descr[] = { { 0, "1239" }, { "~", "0129" } };

        NUTS_ASSERT(pos_ < size_);
        const int n = name_[pos_] - 'C';

        NUTS_ASSERT(n == 0 || n == 1);
        if (++pos_ == size_)
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else if (!strchr(descr[n].kind_, name_[pos_]))
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else
        {
        #ifdef DEBUG
            if (sourceNamePos_ == (size_t)-1)
            {
                std::clog << name_ << std::endl;
                std::clog << std::string(pos_, ' ') << '^' << std::endl;
            }
        #endif
            NUTS_ASSERT(sourceNamePos_ != (size_t)-1);

            ++pos_;
            const size_t size = sourceNameSize_
                ? sourceNameSize_ : out.size() - sourceNamePos_;

            out.append(descr[n].tilde_, descr[n].tilde_ ? 1 : 0);
            out.append(out.data() + sourceNamePos_, size);

            sourceNamePos_ = (size_t)-1;
            sourceNameSize_ = 0;
        }
    }
    /// <special-name>
    /// ::= TV <type>  # virtual table
    /// ::= TT <type>  # VTT structure (construction vtable index)
    /// ::= TI <type>  # typeinfo structure
    /// ::= TS <type>  # typeinfo name (null-terminated byte string)
    /// ::= GV <object name> # Guard variable for one-time initialization
    /// ::= Tc <call-offset> <call-offset> <base encoding>
    ///          # base is the nominal target function of thunk
    ///          # first call-offset is 'this' adjustment
    ///          # second call-offset is result adjustment
    /// ::= T <call-offset> <base encoding>
    ///          # base is the nominal target function of thunk
    /// <call-offset> ::= h <nv-offset> _
    ///               ::= v <v-offset> _
    /// <nv-offset> ::= <offset number> # non-virtual base override
    /// <v-offset>  ::= <offset number> _ <virtual offset number>
    ///          # virtual base override, with vcall offset
    /// @return true when special name is recognized
    bool parse_special_name(Output& out)
    {
        bool result = false;
        if (pos_ >= size_)
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else
        {
            size_t register next = pos_ + 1;
            if (next < size_)
            {
                NameFlags nameFlags;
                switch (name_[pos_])
                {
                case 'G':
                    if (name_[next] == 'V')
                    {
                        out.append("guard variable for ", 19);
                        pos_ += 2;
                        if (!(result = parse_name(out, nameFlags)))
                        {
                            DECODER_ERROR__(INVALID_NAME);
                        }
                    }
                    break;
                case 'T':
                    switch (name_[next])
                    {
                    case 'C':
                        out.append("construction vtable for ", 24);
                        pos_ += 2;
                        if (pos_ < size_)
                        {
                            Output& temp = temp_output();
                            if (!parse_type(temp, false, 0)
                             || parse_unsigned() == Output::npos
                             || (name_[pos_++] != '_')
                             || !parse_type(out, false, 0))
                            {
                                DECODER_ERROR__(INVALID_NAME);
                            }
                            result = true;
                            out.append("-in-", 4);
                            out.append(temp);
                        }
                        break;
                    case 'T':
                        out.append("VTT for ", 8);
                        goto read_type;
                    case 'I':
                        out.append("typeinfo for ", 13);
                        goto read_type;
                    case 'S':
                        out.append("typeinfo name for ", 18);
                        goto read_type;
                    case 'V':
                        out.append("vtable for ", 11);
                    read_type:
                        pos_ += 2;
                        if (!(result = parse_type(out, false, 0)))
                        {
                            DECODER_ERROR__(INVALID_NAME);
                        }
                        break;
                    case 'c':
                        pos_ += 2;
                        out.append("covariant return thunk ", 23);
                        if (pos_ >= size_)
                        {
                            DECODER_ERROR__(INVALID_NAME);
                        }
                        for (size_t i = 0; i < 2; ++i)
                        {
                            if (name_[pos_] == 'h')
                            {
                                ++pos_;
                                parse_non_virtual_offset(out);
                            }
                            else if (name_[pos_] == 'v')
                            {
                                ++pos_;
                                parse_virtual_offset(out);
                            }
                            else
                            {
                                DECODER_ERROR__(INVALID_NAME);
                            }
                        }
                        goto _read_encoding;
                    case 'h':
                        pos_ += 2;
                        out.append("non-virtual thunk ", 18);
                        parse_non_virtual_offset(out);
                        goto _read_encoding;
                    case 'v':
                        pos_ += 2;
                        out.append("virtual thunk ", 14);
                        parse_virtual_offset(out);
                    _read_encoding:
                        out.append("to ", 3);
                        parse_encoding(out);
                        result = (status_ == UNMANGLE_STATUS_SUCCESS);
                        break;
                    }
                    break;
                }
            }
        }
        return result;
    }
    void parse_virtual_offset(Output& out)
    {
#if defined(CPLUS_DEMANGLE_COMPAT)
        parse_signed_uscore();
        parse_signed_uscore();
#else
        out.append("[v:", 3);
        parse_signed_uscore(&out);
        out.append(",", 1);
        parse_signed_uscore(&out);
        out.append("] ", 2);
#endif
    }
    void parse_non_virtual_offset(Output& out)
    {
#if defined(CPLUS_DEMANGLE_COMPAT)
        parse_signed_uscore();
#else
        out.append("[nv:", 4);
        parse_signed_uscore(&out);
        out.append("] ", 2);
#endif
    }
    /// <template-args>  ::= I <template-arg>+ E
    /// <template-args>  ::= I <template-arg>+ E
    void parse_template_args(Output& out, NameFlags& flags)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'I');
        Temporary<size_t>__(inPrototype_, 0);
        if ((templateDepth_ == 0) && (sourceNamePos_ != (size_t)-1))
        {
            //NUTS_ASSERT(out.size() >= sourceNamePos_);
            if (out.size() >= sourceNamePos_)
            {
                sourceNameSize_ = out.size() - sourceNamePos_;
            }
        }
        flags.isSource_ = false;
        flags.isTemplate_ = true;
        Temporary<const CharT*>delim(out.delim_, (CharT*)0);
        out.append("<", 1);
        Temporary<size_t> depth(templateDepth_);
        ++templateDepth_;
        ++pos_;
        for (size_t p = out.size(); parse_template_arg(out, p);)
        {
            out.delim_ = ", ";
            p = out.size() + 2; // skip ", "
        }
        out.delim_ = 0;
        out.append(">", 1);
        if ((pos_ >= size_) || (name_[pos_] != 'E'))
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        ++pos_;
    }
    /// <template-arg>   ::= <type>
    ///                  ::= X <expression> E
    ///                  :: <expr-primary>
    bool parse_template_arg(Output& out, size_t pos)
    {
        bool result = false;
        if (pos_ < size_)
        {
            switch (name_[pos_])
            {
            case 'X':
                parse_expression(out);
                if ((pos_ < size_) && (name_[pos_] == 'E'))
                {
                    result = true;
                }
                else
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                break;
            case 'L':
                parse_expr_primary(out);
                result = (status_ == UNMANGLE_STATUS_SUCCESS);
                break;
            default:
                result = parse_type(out, false, 0);
                break;
            }
            if (result && (templateDepth_ == 1))
            {
                substring_type arg = out.substr(pos);
                templateArgs_.insert(arg);
            }
        }
        return result;
    }
    /// <expression>
    /// ::= <unary operator-name> <expression>
    /// ::= <binary operator-name> <expression> <expression>
    /// ::= <trinary operator-name> <expression> <expression> <expression>
    /// ::= st <type>
    /// ::= <template-param>
    /// ::= sr <type> <unqualified-name>                   # dependent name
    /// ::= sr <type> <unqualified-name> <template-args>   # dependent template-id
    /// ::= <expr-primary>
    void parse_expression(Output& out)
    {
        NUTS_ASSERT(pos_ < size_);
        if (name_[pos_] != 'X')
        {
            std::clog << "name_=" << name_ << ", pos_" << pos_ << std::endl;
            std::clog << "name_[pos_]=" << name_[pos_] << std::endl;
        }
        NUTS_ASSERT(name_[pos_] == 'X');
        ++pos_;
        parse_expression_(out, true);
        if (name_[pos_] == 'E')
        {
            ++pos_;
        }
        else
        {
            DECODER_ERROR__(INVALID_NAME);
        }
    }
    ///@return true if an expression is recognized
    ///and parsed successfully
    bool parse_expression_(Output& out, bool top = false)
    {
        NUTS_ASSERT(pos_ < size_);
        if (name_[pos_] == 'L')
        {
            if (!top) out.append("(", 1);
            parse_expr_primary(out);
            if (!top) out.append(")", 1);
            return (status_ == UNMANGLE_STATUS_SUCCESS);
        }
        else if (name_[pos_] == 's')
        {
            char c = name_[pos_ + 1];
            if (c == 't')
            {
                out.append("sizeof", 6);
                top = false; // force paranthesis
            }
            else if (c != 'r')
            {
                goto _read_operator;
            }
            if (!top) out.append("(", 1);
            pos_ += 2;
            if (!parse_type(out, false, 0))
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else if (c == 'r')
            {
                out.append("::", 2);
                NameFlags nameFlags;
                if (!parse_unqualified_name(out, nameFlags))
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                else if (name_[pos_] == 'I')
                {
                    parse_template_args(out, nameFlags);
                }
            }
            if (!top) out.append(")", 1);
            return (status_ == UNMANGLE_STATUS_SUCCESS);
        }
    _read_operator:
        int opFlags = 0;
        Output& temp = temp_output();
        if (!parse_operator_name(temp, &opFlags))
        {
            return false;
        }
        if (!top) out.append("(", 1);
        size_t i = 0, n = (opFlags >> 3) & 3;
        if (n == 1) // unary operator?
        {
            if (temp.size() <= 8)
            {
                out.append(temp);
            }
            else
            {
                out.append(temp.data() + 8, temp.size() - 8);
            }
        }
        for (; (i < n) && parse_expression_(out); ++i)
        {
            if ((i == 0) && (n > 1))
            {
                NUTS_ASSERT(temp.size() > 8);
                out.append(temp.data() + 8, temp.size() - 8);
            }
            else if ((i == 1) && (n > 2))
            {
                out.append(":", 1);
            }
        }
        if (i < n)
        {
            DECODER_ERROR__(INVALID_NAME);
            return false;
        }
        if (!top) out.append(")", 1);
        return (status_ == UNMANGLE_STATUS_SUCCESS);
    }
    /// <expr-primary> ::= L <type> <number> E # integer literal
    ///                ::= L <type> <float>  E # floating literal
    ///                ::= L <mangled-name>  E # external name
    void parse_expr_primary(Output& out)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'L');
        Output& temp = temp_output();
        if (++pos_ >= size_)
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else if (name_[pos_] == '_')
        {
            ++pos_;
        }
        if ((pos_ < size_) && (name_[pos_] == 'Z'))
        {
            ++pos_;
            parse_encoding(out);
            // check post-condition:
            if ((pos_ >= size_) || (name_[pos_] != 'E'))
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                ++pos_;
            }
        }
        else if (!parse_type(temp, false, 0))
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else
        {
        #if defined(CPLUS_DEMANGLE_COMPAT)
            if (std::char_traits<CharT>::compare(temp.data(), "bool", 4) == 0)
            {
                out.boolAlpha_ = true;
            }
            else if (name_[pos_] != 'n' && !isdigit(name_[pos_]))
            {
                out.append("(", 1); out.append(temp); out.append(")", 1);
            }
            else if (out.delim_)
            {
                Temporary<const CharT*> delim(out.delim_, (CharT*)0);
                out.append(delim, 2);
            }
        #else
            out.append("(", 1); out.append(temp); out.append(")", 1);
        #endif
            register char c = name_[pos_];
            if (temp.size()
                && (strcmp(temp.data(), "float") == 0
                 || strcmp(temp.data(), "double") == 0
                 || strcmp(temp.data(), "__float128") == 0))
            {
                parse_double(&out);
            }
            else if ((c == 'n') || isdigit(c))
            {
                parse_signed(&out);
            }
            if (name_[pos_] == 'E')
            {
                ++pos_;
            }
            else
            {
                DECODER_ERROR__(INVALID_NAME);
            }
        }
    }
    /// <template-param> ::= T_ # first template parameter
    ///                  ::= T <parameter-2 non-negative number > _
    void parse_template_param(Output& out)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'T');
        size_t i = 0;
        if (name_[++pos_] != '_')
        {
            i = parse_unsigned() + 1;
        }
        if (name_[pos_] != '_')
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        if (templateArgs_.output_elem(out, i))
        {
            subst_.insert(templateArgs_[i], true);
        }
        else
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        ++pos_;
    }
    /// <substitution>   ::= S <seq-id> _
    ///                  ::= S_
    void parse_substitution(Output& out, NameFlags& flags)
    {
#define EMIT(abbr) do {                     \
    if (templateDepth_ == 0)                \
    {                                       \
        sourceNamePos_ = out.size() + 5;    \
        sourceNameSize_ = 0;                \
    }                                       \
    out.append(abbr, sizeof(abbr) - 1);     \
    out.substIndex_ = (size_t)-2; } while(0)

        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'S');
        flags.isSource_ = false;

        switch (name_[++pos_])
        {
        case 't': EMIT("std");
            break;
        case 'a': EMIT("std::allocator");
            break;
        case 'b': EMIT("std::basic_string");
            break;
        case 's':
    #ifdef USE_SHORT_STD_SUBST
            EMIT("std::string");
    #else
            EMIT("std::basic_string<char, std::char_traits<char>, std::allocator<char> >");
    #endif
            break;
        case 'i':
    #ifdef USE_SHORT_STD_SUBST
            EMIT("std::istream");
    #else
            EMIT("std::basic_istream<char, std::char_traits<char> >");
    #endif
            break;
        case 'o':
    #ifdef USE_SHORT_STD_SUBST
            EMIT("std::ostream");
    #else
            EMIT("std::basic_ostream<char, std::char_traits<char> >");
    #endif
            break;
        case 'd':
    #ifdef USE_SHORT_STD_SUBST
            EMIT("std::iostream");
    #else
            EMIT("std::basic_iostream<char, std::char_traits<char> >");
    #endif
            break;
        default:
            {
                size_t i = 0;
                char c = name_[pos_];
                if (c != '_')
                {
                    do
                    {
                        i *= 36;
                        if (isdigit(c))
                        {
                            i += c - '0';
                        }
                        else if (isupper(c))
                        {
                            i += c - 'A' + 10;
                        }
                        else
                        {
                            DECODER_ERROR__(INVALID_NAME);
                            break;
                        }
                    }
                    while ((c = name_[++pos_]) != '_');
                    ++i;
                }
                NUTS_ASSERT(pos_ <= size_);
                if (status_ == UNMANGLE_STATUS_SUCCESS)
                {
                    if (subst_.output_elem(out, i))
                    {
                #if defined(WORKAROUND_GNU_ABI_BUG)
                    // work around a bug in g++ 3.2.2 --
                    // use carefully (for example if you want to
                    // compare with other demanglers output).
                        if ((i > 10) && (name_[pos_ + 1] == 'S'))
                        {
                            subst_.insert(subst_[i], true);
                        }
                #endif // WORKAROUND_GNU_ABI_BUG
                    }
                    else
                    {
                        DECODER_ERROR__(INVALID_NAME);
                    }
                    out.substIndex_ = i;
                }
            }
            break;
        }
        ++pos_;
#undef EMIT
        NUTS_ASSERT(!flags.isSource_);
    }
    /// <CV-qualifiers>  ::= [r] [V] [K] # C99 restrict, volatile, const
    bool parse_cv_qualifiers(string_type& out)
    {
        NUTS_ASSERT(pos_ < size_);
        size_t pos = pos_;
        for (;; ++pos_)
        {
            char c = name_[pos_];
            if (c != 'r' && c != 'V' && c != 'K')
            {
                break;
            }
        }
        for (register size_t i = pos_ - 1; i >= pos; --i)
        {
            switch (name_[i])
            {
            case 'r': out.prepend(" restricted", 11);
                break;
            case 'V': out.prepend(" volatile", 9);
                break;
            case 'K': out.prepend(" const", 6);
                break;
            default: NUTS_ASSERT(false);
            }
        }
        return pos_ > pos;
    }
    /// <bare-function-type> ::= <signature type>+
    ///    # types are possible return type, then parameter types
    void parse_bare_function_type(Output& out, string_type& quals, Output* klass)
    {
        typename Output::Position pos(out);
        if (parse_type(out, true, 0)) // output the return type
        {
            // avoid putting a comma after the return type
            Temporary<const CharT*>__(out.delim_, (CharT*)0);
            Output& temp = temp_output();
            temp.append(out.substr(pos));
            temp.append(" ()", 3);

            out.append(" (", 2);
            if (klass)
            {
                out.append(*klass); // for member function types
            }
            apply_qualifiers(out, quals);
            out.append(")", 1);

            const size_t outPos = out.size();
            emit_function_params(out);
            temp.append(out.substr(outPos));
            subst_.insert(temp.substr(0));
        }
    }
    bool emit_function_params(Output& out)
    {
        Output* outp = &out;
        if ((templateDepth_ == 0) && (flags_ & UNMANGLE_NOFUNARGS))
        {
            outp = &temp_output();
        }
        Temporary<size_t>__(inPrototype_, inPrototype_ + 1);
        outp->append("(", 1);
        size_t i = 0;
        for (; parse_type(*outp, false, i); ++i)
        { }
        outp->append(")", 1);
        if (i == 0) outp->rewind(2);
        return (status_ == UNMANGLE_STATUS_SUCCESS);
    }
    /// Types are qualified (optionally) by single-character prefixes
    /// encoding cv-qualifiers and/or pointer, reference, complex, or
    /// imaginary types:
    ///  <type> ::= <CV-qualifiers> <type>
    ///     ::= P <type>   # pointer-to
    ///     ::= R <type>   # reference-to
    ///     ::= C <type>   # complex pair (C 2000)
    ///     ::= G <type>   # imaginary (C 2000)
    ///     ::= U <source-name> <type> # vendor extended type qualifier
    bool parse_type(Output& out, bool ret, size_t indx)
    {
        string_type qualifiers;
        return parse_type(out, ret, indx, qualifiers);
    }
    void apply_qualifiers(Output& out, string_type& qual, size_t pos = string_type::npos)
    {
        if (qual.size() == 0)
        {
            return;
        }
        Temporary<const CharT*>__(out.delim_, (CharT*)0);
        size_t i = out.size();
        out.append(qual);
        const size_t n = out.size();
        if ((pos != string_type::npos) && (i > pos))
        {
            for (++i; i < n; ++i)
            {
                switch (out[i])
                {
                /*case ' ':
                    subst_.insert(out.substr(pos, i - pos));
                    break; */
                case '*':
                case '&':
                    subst_.insert(out.substr(pos, i - pos));
                    subst_.insert(out.substr(pos, ++i - pos));
                    break;
                }
            }
            subst_.insert(out.substr(pos, n - pos));
        }
        qual.clear();
    }
    ///Types are encoded as follows:
    ///  <type> ::= <builtin-type>
    ///     ::= <function-type>
    ///     ::= <class-enum-type>
    ///     ::= <array-type>
    ///     ::= <pointer-to-member-type>
    ///     ::= <template-param>
    ///     ::= <template-template-param> <template-args>
    ///     ::= <substitution>
    ///@return true when a type is recognized and decoded
    bool parse_type(Output& out, bool ret, size_t indx, string_type& quals)
    {
        bool result = false;
        BuiltinType builtin = B_NONE;
        if (pos_ < size_)
        {
            typename Output::Position outPtr(out);
            Temporary<const CharT*>__(out.delim_);
            if (indx && !ret)
            {
                out.delim_ = ", ";
            }
            NameFlags nameFlags;
            size_t haveQual = 0;
            switch (name_[pos_])
            {
            case 'A':
                parse_array_type(out, quals);
                result = true;
                break;
            case 'C':
                TODO; // complex pair (C 2000)
                break;
            case 'F':
                parse_function_type(out, quals);
                result = true;
                break;
            case 'G':
                TODO; // imaginary (C 2000)
                break;
            case 'M':
                parse_pointer_to_member_type(out, quals);
                result = true;
                break;
            case 'P':
                quals.prepend("*", 1); ++pos_; ++haveQual;
                break;
            case 'R':
                quals.prepend("&", 1); ++pos_; ++haveQual;
                break;
            case 'S':
                if (name_[pos_ + 1] != 't')
                {
                    parse_substitution(out, nameFlags);
                }
                //<class-enum-type> ::= <name>
                else if (!parse_name(out, nameFlags))
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                goto check_for_template_args;
            case 'T':
                nameFlags.isSource_ = false;
                parse_template_param(out);
            check_for_template_args:
                // check for template-template-param
                // <template-template-param> ::= <template-param><substitution>
                add_substitution(out, outPtr, nameFlags);
                if ((pos_ < size_) && (name_[pos_] == 'I'))
                {
                    NUTS_ASSERT(!builtin);
                    parse_template_args(out, nameFlags);
                    add_substitution(out, outPtr, nameFlags);
                }
                apply_qualifiers(out, quals, outPtr);
                result = true;
                break;
            case 'U':
                TODO; // vendor extended type qualifier
                break;
            default:
                if (parse_cv_qualifiers(quals))
                {
                    ++haveQual;
                }
                else
                {
                    NUTS_ASSERT(!haveQual);
                    NUTS_ASSERT(!result);
                    if (quals.size())
                    {
                        // show void* regardless if ret type or not:
                        ret = true;
                    }
                    builtin = parse_builtin_type(out, ret);
                    if (builtin == B_VENDOR)
                    {
                        nameFlags.isSource_ = true; // force to dict
                    }
                    //<class-enum-type> ::= <name>
                    if ((builtin != B_NONE) || parse_name(out, nameFlags))
                    {
                        if ((builtin == B_NONE) || (builtin == B_VENDOR))
                        {
                            if (templateDepth_)
                            {
                                nameFlags.isSource_ = true;
                            }
                            add_substitution(out, outPtr, nameFlags);
                        }
                        apply_qualifiers(out, quals, outPtr);
                        return true;
                    }
                }
                break;
            }
            if (haveQual)
            {
                NUTS_ASSERT(!result);
                return parse_type(out, ret, indx, quals);
            }
            if (result && (builtin == B_NONE || builtin == B_VENDOR))
            {
                add_substitution(out, outPtr, nameFlags);
            }
        }
        return result;
    }
    /// <array-type> ::= A <positive dimension number> _ <element type>
    ///              ::= A [<dimension expression>] _ <element type>
    void parse_array_type(Output& out, const string_type& quals)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'A');
        Output* temp = NULL;
        if (++pos_ >= size_)
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else
        {
            size_t nelem = (size_t)-1;
            if (isdigit(name_[pos_]))
            {
                nelem = parse_unsigned();
            }
            else if (name_[pos_] == 'X')
            {
                parse_expression(out);
            }
            else if (name_[pos_] == 'T')
            {
                temp = &temp_output();
                parse_template_param(*temp);
            }

            if (name_[pos_] != '_')
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                ++pos_;
                if (parse_type(out, false, 0))
                {
                    if (!quals.empty())
                    {
                        out.append(" (", 2);
                        out.append(quals);
                        out.append(") ", 2);
                    }
                    out.append("[", 1);
                    if (nelem != (size_t)-1)
                    {
                        out.append(nelem);
                    }
                    if (temp)
                    {
                        out.append(*temp);
                    }
                    out.append("]", 1);
                }
                else
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
            }
        }
    }
    /// <function-type> ::= F [Y] <bare-function-type> E
    void parse_function_type(Output& out, string_type& quals, Output* klass = 0)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'F');
        if (name_[++pos_] == 'Y')
        {
            out.append("extern C ", 9);
            ++pos_;
        }
        parse_bare_function_type(out, quals, klass);
        if ((pos_ >= size_) || (name_[pos_] != 'E'))
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        ++pos_;
    }
    /// <pointer-to-member-type> :: M <class type> <member type>
    void parse_pointer_to_member_type(Output& out, string_type& quals)
    {
        NUTS_ASSERT(pos_ < size_);
        NUTS_ASSERT(name_[pos_] == 'M');
        ++pos_;
        Output& temp = temp_output();
        if (!parse_type(temp, false, 0))
        {
            DECODER_ERROR__(INVALID_NAME);
        }
        else
        {
            temp.append("::*", 3);
            if (name_[pos_] == 'F')
            {
                parse_function_type(out, quals, &temp);
            }
            else if (!parse_type(out, false, 0))
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                out.append(" ", 1);
                out.append(temp);
                apply_qualifiers(out, quals);
            }
        }
    }
    struct TypeEntry { const CharT* name; size_t len; };
    typedef TypeEntry BuiltinTable['z' - 'a' + 1];
#define TYPE_ENTRY(n) { n, sizeof(n) - 1 }
    /// <builtin-type> ::= v    # void
    ///     ::= w  # wchar_t
    ///     ::= b  # bool
    ///     ::= c  # char
    ///     ::= a  # signed char
    ///     ::= h  # unsigned char
    ///     ::= s  # short
    ///     ::= t  # unsigned short
    ///     ::= i  # int
    ///     ::= j  # unsigned int
    ///     ::= l  # long
    ///     ::= m  # unsigned long
    ///     ::= x  # long long, __int64
    ///     ::= y  # unsigned long long, __int64
    ///     ::= n  # __int128
    ///     ::= o  # unsigned __int128
    ///     ::= f  # float
    ///     ::= d  # double
    ///     ::= e  # long double, __float80
    ///     ::= g  # __float128
    ///     ::= z  # ellipsis
    ///     ::= u <source-name>    # vendor extended type
    static const BuiltinTable& builtin_table()
    {
        static BuiltinTable table =
        {
            /* a */TYPE_ENTRY("signed char"),
            /* b */TYPE_ENTRY("bool"),
            /* c */TYPE_ENTRY("char"),
            /* d */TYPE_ENTRY("double"),
            /* e */TYPE_ENTRY("long double"), // __float80
            /* f */TYPE_ENTRY("float"),
            /* g */TYPE_ENTRY("__float128"),
            /* h */TYPE_ENTRY("unsigned char"),
            /* i */TYPE_ENTRY("int"),
#if defined(CPLUS_DEMANGLE_COMPAT)
            /* j */TYPE_ENTRY("unsigned"),
#else
            /* j */TYPE_ENTRY("unsigned int"),
#endif
            /* k */ { NULL, 0 },
            /* l */TYPE_ENTRY("long"),
            /* m */TYPE_ENTRY("unsigned long"),
            /* n */TYPE_ENTRY("__int128"),
            /* o */TYPE_ENTRY("unsigned __int128"),
            /* p */ { NULL, 0 },
            /* q */ { NULL, 0 },
            /* r */ { NULL, 0 },
            /* s */ TYPE_ENTRY("short"),
            /* t */ TYPE_ENTRY("unsigned short"),
            /* u */ { NULL, 0 },
            /* v */ TYPE_ENTRY("void"),
            /* w */ TYPE_ENTRY("wchar_t"),
            /* x */ TYPE_ENTRY("long long"), // __int64
            /* y */ TYPE_ENTRY("unsigned long long"), // unsigned __int64
            /* z */ TYPE_ENTRY("..."),
        };
        return table;
#undef TYPE_ENTRY
    }
    enum BuiltinType { B_NONE, B_NORMAL, B_VOID, B_VENDOR };
    BuiltinType parse_builtin_type(Output& out, bool ret)
    {
        register CharT c = pos_ < size_ ? name_[pos_] : 0;
        if (c >= 'a' && c <= 'z')
        {
            switch (c)
            {
            case 'u': // vendor extension
                if (++pos_ >= size_)
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                else
                {
                    parse_source_name(out);
                    return B_VENDOR;
                }
                break;
            case 'v': // void
                if (!ret && inPrototype_)
                {
                    ++pos_;
                    return B_VOID;
                }
                // fallthru
            default:
                {
                    const TypeEntry& entry = builtin_table()[c - 'a'];
                    if (entry.name)
                    {
                        out.append(entry.name, entry.len);
                        ++pos_;
                        return c == 'v' ? B_VOID : B_NORMAL;
                    }
                }
                break;
            }
        }
        return B_NONE;
    }
    /// add subst entry to dictionary
    void add_substitution_(Output& out, size_t pos, NameFlags& flags)
    {
        const bool overridePolicy = flags.isSource_;
        if ((out.substIndex_ == Output::npos)
            && (out.size() > pos)
            && flags.may_substitute(name_[pos_]))
        {
            subst_.insert(out.substr(pos), overridePolicy);
        }
        flags.isSource_ = false;
    }
    /// make a temp output object
    Output& temp_output()
    {
        temp_.push_back(new Output());
        return *temp_.back();
    }
    size_t parse_unsigned(Output* output = 0)
    {
        if (!isdigit(name_[pos_]))
        {
            DECODER_ERROR__(INVALID_NAME);
            return 0;
        }
        size_t n = 0;
        for (register char c; isdigit(c = name_[pos_]); ++pos_)
        {
            n = n * 10 + c - '0';
        }
        NUTS_ASSERT(pos_ <= size_);
        if (output) output->append(n);
        return n;
    }
    int parse_signed(Output* output = 0)
    {
        int sign = 1;
        if (name_[pos_] == 'n')
        {
            sign = -1;
            if (output) output->append("-", 1);
            if (++pos_ >= size_) DECODER_ERROR__(INVALID_NAME);
        }
        return sign * parse_unsigned(output);
    }
    void parse_signed_uscore(Output* output = 0)
    {
        parse_signed(output);
        if (name_[pos_] == '_')
        {
            ++pos_;
        }
        else
        {
            DECODER_ERROR__(INVALID_NAME);
        }
    }
    void parse_double(Output* output = 0)
    {
        char* p = 0;
        long double f = strtold(name_ + pos_, &p);
        pos_ = p - name_;
        if (output)
        {
            output->append("[", 1);
            std::ostringstream out;
            out << f;
            output->append(out.str());
            output->append("]", 1);
        }
    }
private:
    Decoder(const Decoder&);
    Decoder& operator==(const Decoder&);

    const char*             name_;
    size_t                  size_;
    int                     flags_;
    size_t                  pos_;           // current position in input string
    size_t                  sourceNamePos_; // for ctor and dtor (C1, C2, etc)
    size_t                  sourceNameSize_;
    nuts::string<CharT>*    cvQualifiers_;
    Substitutions           subst_;         // substitutions dictionary
    TemplateArgs            templateArgs_;
    size_t                  templateDepth_;
    size_t                  inPrototype_;   // parsing ret type or param types?
    bool                    inPrefix_;
    bool                    anonymous_;     // pending anonymous namespace?
    int                     status_;
    // need to keep temps around because there might be substring
    // references to them; store pointers and not values because
    // nuts::string's copy ctor transfers ownership (like auto_ptr).
    TempOutputs             temp_;
};
#endif // DECODER_H__7B3B1118_B516_411D_B449_95D7E5C32248
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

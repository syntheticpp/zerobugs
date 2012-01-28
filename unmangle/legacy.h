#ifndef LEGACY_H__0A821E67_C5BE_44C5_B0B4_EA56BF947779
#define LEGACY_H__0A821E67_C5BE_44C5_B0B4_EA56BF947779
//
// $Id$
//
// Demangler for gcc 2.95 mangling scheme
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cctype>
#include "nuts/assert.h"
#include <set>
#if defined(USE_STD_CONTAINERS)
 #include <stack>
 #include <vector>
#else
 #include "nuts/fixed_stack.h"
 #include "nuts/static_vector.h"
#endif
#include "nuts/string.h"
#include "nuts/substring.h"
#include "nuts/temporary.h"
#include "decoder_macros.h"
#include "dictionary.h"
#include "unmangle.h"       // for enumerated flags & status

#define APPEND(o,s)     (o).append(s, sizeof(s)-1)
#define PREPEND(o,s)    (o).prepend(s, sizeof(s)-1)
#if defined(DEBUG_UNMANGLE) && !defined (CXXFILT)
 static size_t indent = 0;
 struct Indent { Indent() { ++indent; } ~Indent() { --indent; } };
 #define TRACE Indent __indent__;                               \
    std::cout << std::string(indent, ' ')  << __func__;         \
    std::cout << "(" << __FILE__ << ':' << __LINE__ << "): ";   \
    std::cout << name_ + pos_ << std::endl;
#else
 #define TRACE // as nothing
#endif

template<typename C>
struct SquanglePolicy
{
    typedef C container_type;
    typedef typename C::value_type value_type;

    void insert(C& c, const value_type& v, bool override = false)
    {
        if (set_.insert(v).second || override)
        {
#ifdef DEBUG_DICT
            std::clog << c.size() << ": " << v << std::endl;
#endif
            c.push_back(v);
            NUTS_ASSERT(v[0] != ',');
            NUTS_ASSERT(v[0] != ':');
        }
    }
    void swap(SquanglePolicy& other) throw()
    {
        set_.swap(other.set_);
    }
    void clear() { set_.clear(); }

private:
    std::set<value_type> set_;
};
namespace legacy
{
    /**
     * Demangler for the old g++ mangling scheme.
     */
    template<typename CharT, size_t MAX_TEMP = 512> class Decoder
    {
    public:
        typedef nuts::string<CharT> string_type;
        typedef nuts::substring<string_type> substring_type;
        typedef typename DictBase<substring_type>::container_type container_type;
        typedef Dictionary<substring_type> dict_type;
        typedef Dictionary< substring_type,
                            SquanglePolicy<container_type> > squangle_dict;
#include "output.h"
#if defined (USE_STD_CONTAINERS)
        typedef std::vector<Output*> TempOutputs;
#else
        typedef nuts::fixed_stack<Output*, MAX_TEMP> TempOutputs;
#endif
#ifdef CPLUS_DEMANGLE_COMPAT
        static const size_t ANONYMOUS_NS_PREFIX_LEN = sizeof ("{anonymous}") - 1;
#else
        static const size_t ANONYMOUS_NS_PREFIX_LEN = sizeof ANONYMOUS_NS_PREFIX - 1;
#endif

        explicit Decoder(const char* name, int flags = UNMANGLE_DEFAULT)
            : name_(name)
            , size_(name ? strlen(name) : 0)
            , pos_(0)
            , isCtor_(false)
            , gotKlassName_(false)
            , flags_(flags)
            , status_(UNMANGLE_STATUS_SUCCESS)
            , templateArgs_((size_ + 2) / 3)
            , types_((size_ + 1) / 2)
            , squangle_((size_ + 1) / 2)
        { }
        ~Decoder()
        {
            using namespace boost;
            std::for_each(temp_.begin(), temp_.end(), checked_deleter<Output>());
        }

        /// entry point
        CharT* parse(int* status = 0, size_t* outlen = 0)
        {
            TRACE;
            CharT* result = 0;
            if (name_)
            {
                Output out;
                if (parse(out) && out.data())
                {
                    NUTS_ASSERT(status_ == UNMANGLE_STATUS_SUCCESS);
                    if (pos_ == 0) // nothing parsed?
                    {
                        DECODER_ERROR__(INVALID_NAME);
                    }
                    else
                    {
                        //result = out.detach(outlen);
                        return out.detach(outlen);
                    }
                }
                if (status)
                {
                    *status = status_;
                }
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
            NUTS_ASSERT(size_ >= 11);
            name_ += 11;
            size_ -= 11;
            parse(output);
            if (status)
            {
                *status = status_;
            }
            return status_ ? 0 : output.detach(len);
        }

    private:
        /// @return true if a special case is recognized
        bool parse_special_cases(Output& out)
        {
            TRACE;
            if (name_[pos_] == '_')
            {
                if (maybe_name(name_[pos_ + 1]))
                {
                    parse_static_name(out);
                }
                else if (BEGINS_WITH("_._")) // dtor?
                {
                    parse_dtor(out);
                }
                else if (BEGINS_WITH("__vt_"))
                {
                    while ((pos_ < size_) && (status_ == 0))
                    {
                        if (name_[pos_] == '.')
                        {
                            out.append("::", 2);
                            ++pos_;
                        }
                        parse_name(out);
                    }
                    APPEND(out, " virtual table");
                }
                else if (BEGINS_WITH("__thunk_"))
                {
                    APPEND(out, "virtual function thunk (delta:-");
                    parse_signed(&out);
                    APPEND(out, ") for ");
                    if (name_[pos_] == '_')
                    {
                        ++pos_;
                        parse(out);
                    }
                    else
                    {
                        DECODER_ERROR__(INVALID_NAME);
                        return false;
                    }
                }
                else if (BEGINS_WITH("__ti"))
                {
                    parse_type(out);
                    APPEND(out, " type_info node");
                }
                else if (BEGINS_WITH("__tf"))
                {
                    parse_type(out);
                    APPEND(out, " type_info function");
                }
              /*
                else if (BEGINS_WITH("_t"))
                {
                    TODO;
                }*/
                else if (name_[pos_ + 1] == '_')
                {
                    isCtor_ = true;
                    parse(out, pos_);
                }
                else
                {
                    return false;
                }
                return (status_ == UNMANGLE_STATUS_SUCCESS);
            }
            return false;
        }
        bool begins_with(const char* str, size_t n)
        {
            if ((size_ > n) && strncmp(name_ + pos_, str, n) == 0)
            {
                pos_ += n;
                return true;
            }
            return false;
        }
        void parse_name(Output& out)
        {
            TRACE;
            const size_t p = out.size();
            if (pos_ >= size_)
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else if (name_[pos_] == 'Q')
            {
                ++pos_;
                parse_qualified_name(out);
            }
            else
            {
                parse_unqualified_name(out);
            }
            if (klass_.empty())
            {
                klass_ = out.substr(p); // possible class name
            }
            gotKlassName_ = true;
        }
        /// First the letter 'Q' indicates a qualified name; that is followed
        /// by the number parts in the qualified name. If that number is 9 or
        /// less, it is emitted without delimiters. Otherwise, an underscore
        /// is written before and after the count.
        void parse_qualified_name(Output& out)
        {
            TRACE;
            const size_t nameCount = parse_integer();
            for (size_t i = 0; (i != nameCount) && (status_ == 0); ++i)
            {
                if (i)
                {
                    out.append("::", 2);
                }
                Temporary<bool>__(gotKlassName_);
                parse_type(out);
            }
        }
        /// A simple class, template or namespace name is encoded as the
        /// number of characters in the name, followed by the actual
        /// characters.
        void parse_unqualified_name(Output& out)
        {
            TRACE;
            const size_t p = out.size();
            if (name_[pos_] == 't')
            {
                ++pos_;
                parse_template_instance(out);
            }
            else if (size_t len = parse_unsigned())
            {
                if (pos_ + len > size_)
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                else
                {
                    bool anonymous = false;
                    // todo: revisit
                    if ((len >= 8) && memcmp(name_ + pos_, "_GLOBAL_", 8) == 0)
                    {
                        register char c = name_[pos_ + 8];
                        if ((c == '.' || c == '_' || c == '$')
                            && name_[pos_ + 9] == 'N')
                        {
                        #ifdef CPLUS_DEMANGLE_COMPAT
                            out.append("{anonymous}", 11);
                        #else
                            out.append(ANONYMOUS_NS_PREFIX,
                                sizeof(ANONYMOUS_NS_PREFIX) - 1);
                        #endif
                            pos_ += len;
                            anonymous = true;
                        }
                    }
                    if (!anonymous)
                    {
                        out.append(name_ + pos_, len);
                        pos_ += len;
                    }
                }
            }
            if (!gotKlassName_)
            {
                klass_ = out.substr(p); // possible class name
            }
        }
        void parse_template_instance(Output& out)
        {
            TRACE;
            parse_name(out);
            parse_template_param_list(out);
        }
        void parse_template_param_list(Output& out)
        {
            TRACE;
            if (pos_ >= size_)
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                out.append("<", 1);
                const size_t paramCount = parse_unsigned();
                for (size_t i = 0; (i != paramCount) && (status_ == 0); ++i)
                {
                    if (i)
                    {
                        out.append(", ", 2);
                    }
                    parse_template_param(out);
                }
                out.append(">", 1);
            }
        }
        void parse_template_param(Output& out, bool isUnsigned = false)
        {
            TRACE;
            if (pos_ >= size_)
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                switch (name_[pos_])
                {
                case 'Z':
                    ++pos_; parse_type(out);
                    break;
                case 'z':
                    ++pos_;
                    parse_template_template_param(out);
                    break;
                case 'i':
                case 'l':
                case 's':
                case 'x':
                    ++pos_;
                    parse_integer(&out);
                    break;
                case 'b':
                    ++pos_;
                    switch (name_[pos_])
                    {
                    case '0': APPEND(out, "false"); break;
                    case '1': APPEND(out, "true"); break;
                    default: DECODER_ERROR__(INVALID_NAME); break;
                    }
                    ++pos_;
                    break;
                case 'c':
                    ++pos_;
                    APPEND(out, "(char)");
                    parse_integer(&out);
                    break;
                case 'f':
                case 'd':
                case 'r':
                    TODO;  // parse_float
                    break;
                case 'U':
                    ++pos_;
                    parse_template_param(out, true);
                    break;

                default: DECODER_ERROR__(INVALID_NAME); break;
                }
            }
        }
        void parse_type(Output& out)
        {
            string_type qualifiers; // const, volatile, pointer, etc.
            parse_type(out, qualifiers);
        }
        void parse_type(Output& out, string_type& qual, size_t ptrDepth = 0)
        {
            TRACE;
            const size_t p = out.size();
            bool haveQual = false; // have CV-qualifiers, ptr, ref ?
            char c = pos_ < size_ ? name_[pos_] : -1;
            if (isdigit(c))
            {
                parse_unqualified_name(out);
            }
            else
            {
                switch (c)
                {
                case 'd': APPEND(out, "double"); ++pos_; break;
                case 'f': APPEND(out, "float");  ++pos_; break;
                case 'J': APPEND(out, "__complex"); ++pos_; break;
                case 'v': APPEND(out, "void"); ++pos_; break;
                case 'r': APPEND(out, "long double"); ++pos_; break;
                case 'e': APPEND(out, "..."); ++pos_; break;
                case 'i': APPEND(out, "int"); ++pos_; break;
                case 'l': APPEND(out, "long"); ++pos_; break;
                case 's': APPEND(out, "short"); ++pos_; break;
                case 'c': APPEND(out, "char"); ++pos_; break;
                case 'x': APPEND(out, "long long"); ++pos_; break;
                case 'b': APPEND(out, "bool"); ++pos_; break;
                case 'w': APPEND(out, "wchar_t"); ++pos_; break;
                case 'I': TODO; break;
                case 'A':
                    ++pos_; parse_array_type(out);
                    break;
                case 'B':
                    ++pos_;
                    {
                        int n = parse_integer();
                        if (!squangle_.output_elem(out, n))
                        {
                            DECODER_ERROR__(INVALID_NAME);
                        }
                    }
                    break; // squangle
                case 'F':
                    ++pos_;
                    parse_fun_type(out, qual);
                    break;
                case 'P':
                    ++pos_; ++ptrDepth; haveQual = true;
                    PREPEND(qual, "*");
                    break;
                case 'R':
                    ++pos_; ++ptrDepth; haveQual = true;
                    PREPEND(qual, "&");
                    break;
                case 'Q':
                    ++pos_;
                    parse_qualified_name(out);
                    break;
                case 't':
                    if ((name_[++pos_] == 'z') && (name_[pos_] == 'X'))
                    {
                        parse_template_template_param(out);
                    }
                    else
                    {
                        parse_template_instance(out);
                    }
                    break;
                case 'X':
                case 'Y':
                    ++pos_;
                    {
                        size_t i = parse_integer();
                        parse_integer(); // eat template ptrDepth
                        if (!templateArgs_.output_elem(out, i))
                        {
                            DECODER_ERROR__(INVALID_NAME);
                            return;
                        }
                    }
                    break;
                case 'Z': // decoding template template param?
                    TODO;
                    //++pos_; APPEND(out, "class ");
                    break;
                case 'U':
                    ++pos_; APPEND(out, "unsigned "); parse_type(out);
                    break;
                case 'S':
                    ++pos_; APPEND(out, "signed "); parse_type(out);
                    break;
                default:
                    if ((haveQual = parse_type_qualifiers(qual)) == false)
                    {
                        DECODER_ERROR__(INVALID_NAME);
                        return;
                    }
                }
            }
            if (haveQual)
            {
                parse_type(out, qual, ptrDepth);
            }
            else
            {
                apply_qualifiers(out, qual, p);
                if (ptrDepth > 1)
                {
                    substring_type t = out.substr(p);
                    types_.insert(t);
                    squangle_.insert(t);
                }
            }
        }
        bool parse_type_qualifiers(string_type& qual)
        {
            TRACE;
            bool result = false;
            for (; pos_ < size_; ++pos_)
            {
                switch (name_[pos_])
                {
                case 'u':
                    PREPEND(qual, " __restrict "); // C99
                    result = true;
                    break;
                case 'C':
                    PREPEND(qual, " const");
                    result = true;
                    break;
                case 'V':
                    PREPEND(qual, " volatile");
                    result = true;
                    break;
                case 'G': // ignore --fixme
                    result = true;
                    break;
                default:
                    return result;
                }
            }
            return result;
        }
        // "qualifiers" here encompasses pointer, reference, CV-quals, C99 restricted,
        // and C 2000's imaginary (G)
        void
        apply_qualifiers(Output& out, string_type& qual, size_t pos = string_type::npos)
        {
            TRACE;
            out.append(qual);
            qual.clear();
        }
        unsigned long long parse_unsigned(Output* out = NULL)
        {
            unsigned long long n = 0;
            if ((pos_ >= size_) || !isdigit(name_[pos_]))
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                char c;
                while (isdigit((c = name_[pos_])))
                {
                    n = n * 10 + (c - '0');
                    if (out) { out->append(&c, 1); }
                    ++pos_;
                }
            }
            return n;
        }
        long long parse_signed(Output* out = 0)
        {
            TRACE;
            int sign = 1;
            if (name_[pos_] == 'm')
            {
                ++pos_;
                sign = -1;
                if (out) { out->append("-", 1); }
            }
            return sign * parse_unsigned(out);
        }
        // fixme: need to handle unsigned long long?
        long long parse_integer(Output* out = 0)
        {
            //assert(pos_ < size_);
            char c = name_[pos_++];
            if (c == '_')
            {
                long long result = parse_signed(out);
                assert(pos_ < size_);
                if (name_[pos_++] != '_')
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                return result;
            }
            int sign = 1;
            if (c == 'm')
            {
                if (out) { out->append("-", 1); }
                assert(pos_ < size_);
                c = name_[pos_++];
                sign = -1;
            }
            if (out) { out->append(&c, 1); }
            return sign * (c - '0');
        }
        void parse_fun_qualifiers(string_type& qual)
        {
            for (;; ++pos_)
            {
                register int c = name_[pos_];
                switch (c)
                {
                case 'C': APPEND(qual, " const"); break;
                case 'V': APPEND(qual, " volatile"); break;
                case 'S': APPEND(qual, " static"); break;
                default: return;
                }
            }
        }
        bool parse(Output& out)
        {
            if (parse_special_cases(out) || parse(out, 0))
            {
                return true;
            }
            else if (pos_ < size_)
            {
                //copy remainder to output rather than error-ing
                out.append(name_ + pos_, size_ - pos_);
            }
            return false;
        }
        bool parse(Output& out, size_t pos)
        {
            TRACE;
            bool result = false;
            const size_t n = size_ ? size_ - 1 : 0;
            string_type qual;
            for (size_t i = pos; (i < n) && (status_ == 0); ++i)
            {
                if ((name_[i] == '_') && (name_[i + 1] == '_'))
                {
                    pos_  = i + 2;
                    result = true;
                    parse_fun_qualifiers(qual);

                    switch (name_[pos_])
                    {
                    case 'F': // func name
                        if (i == pos)
                        {
                            return false;
                        }
                        if (!parse_operator(out, pos, i))
                        {
                            out.append(name_ + pos, i - pos);
                        }
                        ++pos_;
                        parse_fun_param(out);
                        break;
                    case 'H': // func template inst or specialization
                        ++pos_;
                        parse_template_fun_instance(out, pos, i - pos);
                        if (status_ != UNMANGLE_STATUS_SUCCESS)
                        {
                            return false;
                        }
                        break;

                    default:
                        {
                            const size_t p = out.size();
                            parse_name(out);

                            if (status_ != UNMANGLE_STATUS_SUCCESS)
                            {
                                isCtor_ = gotKlassName_ = false;
                                status_ = UNMANGLE_STATUS_SUCCESS;
                                klass_.clear();
                                continue;
                            }
                            substring_type t = out.substr(p);
                            types_.insert(t);
                            squangle_.insert(t);
                            out.append("::", 2);
                            if (isCtor_)
                            {
                                NUTS_ASSERT(i == pos);
                                out.append(klass_);
                            }
                            else if (!parse_operator(out, pos, i))
                            {
                                out.append(name_ + pos, i - pos);
                            }
                            parse_fun_param(out);
                            if ((flags_ & UNMANGLE_NOFUNARGS) == 0)
                            {
                                out.append(qual);
                            }
                        }
                        break;
                    }
                    i = pos_ - 1;
                }
            }
            if (status_ != UNMANGLE_STATUS_SUCCESS) { result = false; }
            return result;
        }
        void parse_fun_type(Output& out, string_type& qual)
        {
            Output& temp = temp_output();
            temp.append(" (", 2);
            apply_qualifiers(temp, qual);
            temp.append(")", 1);
            // In the context of a function type, show the function
            // parameters regardless of the option flags.
            Temporary<int>__(flags_);
            flags_ &= ~UNMANGLE_NOFUNARGS;
            parse_fun_param(temp);
            if (name_[pos_] == '_')
            {
                ++pos_;
                parse_type(out);// the return type
            }
            out.append(temp);
        }
        void parse_fun_param(Output& out)
        {
            TRACE;
            if (flags_ & UNMANGLE_NOFUNARGS)
            {
                Output& discard = temp_output();
                Temporary<int>__(flags_, (flags_ & ~UNMANGLE_NOFUNARGS));
                parse_fun_param(discard);
                return;
            }
            out.append("(", 1);
            if (pos_ == size_)
            {
                out.append("void", 4);
            }
            else
            {
                substring_type last; // last seen type
                for (size_t i = 0; (pos_ < size_) && (status_ == 0); ++i)
                {
                    if (name_[pos_] == '_')
                    {
                        break;
                    }
                    if (i)
                    {
                        out.append(", ", 2);
                    }
                    switch (name_[pos_])
                    {
                    case 'T':
                        ++pos_;
                        {
                            size_t n = parse_unsigned();
                            if (!types_.output_elem(out, n))
                            {
                                DECODER_ERROR__(INVALID_NAME);
                            }
                        }
                        break;
                    case 'N': // repeat a memorized type
                        ++pos_;
                        {
                            size_t repeat = parse_integer();
                            const size_t n = parse_integer();
                            while (repeat--)
                            {
                                if (!types_.output_elem(out, n))
                                {
                                    DECODER_ERROR__(INVALID_NAME);
                                    break;
                                }
                                if (repeat)
                                {
                                    out.append(", ", 2);
                                }
                            }
                        }
                        break;
                    case 'n':  // repeat last seen type
                        ++pos_;
                        {
                            size_t repeat = parse_integer();
                            while (repeat--)
                            {
                                if (last.empty())
                                {
                                    DECODER_ERROR__(INVALID_NAME);
                                    break;
                                }
                                out.append(last);
                                if (repeat)
                                {
                                    out.append(", ", 2);
                                }
                            }
                        }
                        break;
                    default:
                        {
                            size_t p = out.size();
                            parse_type(out);
                            last = out.substr(p);
                            types_.insert(last);
                        }
                        break;
                    }
                }
            }
            out.append(")", 1);
        }
        /// A function template specialization (either an instantiation or
        /// an explicit specialization) is encoded by an `H' followed by the
        /// encoding of the template parameters, followed by an `_', the
        /// encoding of the argument types to the template function (not the
        /// specialization), another `_', and the return type.
        /// (Like the argument types, the return type is the return type of the
        /// function template, not the specialization.) Template parameters in
        /// the argument and return types are encoded by an `X' for type parameters,
        /// `zX' for template parameters, or a `Y' for constant parameters, an index
        /// indicating their position in the template parameter list declaration,
        /// and their template ptrDepth.
        void parse_template_fun_instance(Output& out, size_t pos, size_t len)
        {
            TRACE;
            templateArgs_.clear();
            if (pos_ >= size_)
            {
                DECODER_ERROR__(INVALID_NAME);
                return;
            }
            string_type qual;
            Output& temp = temp_output();
            temp.append("<", 1);
            const size_t paramCount = parse_unsigned();
            for (size_t i = 0; (i != paramCount) && (status_ == 0); ++i)
            {
                if (i)
                {
                    temp.append(", ", 2);
                }
                // If a template parameter is a type, it is written as a `Z'
                // followed by the encoding of the type. If it is a template,
                // it is encoded as `z' followed by the parameter of the
                // template template parameter and the template name
                if (name_[pos_] == 'Z')
                {
                    size_t pos = temp.size();
                    parse_template_param(temp);
                    templateArgs_.insert(temp.substr(pos));
                }
                else if (name_[pos_] == 'z')
                {
                    ++pos_;
                    parse_template_template_param(temp);
                }
                else
                {
                    TODO;
                }
            }
            temp.append(">", 1);
            Output& fname = temp_output();
            if (pos_ < size_)
            {
                if (name_[pos_] != '_')
                {
                    DECODER_ERROR__(INVALID_NAME);
                    return;
                }
                ++pos_;
                // parse qualifiers if any
                parse_fun_qualifiers(qual);
                if (maybe_name(name_[pos_]) || (name_[pos_] == 't'))
                {
                    parse_name(fname);
                    substring_type t = fname.substr(0);
                    types_.insert(t);
                    squangle_.insert(t);
                    fname.append("::", 2);
                }
            }
            if (isCtor_)
            {
                NUTS_ASSERT(len == 0);
                if (!klass_.empty())
                {
#if defined(CPLUS_DEMANGLE_LEGACY_COMPAT) // mimic cplus_demangle bug
                                          // to keep some tests happy
                    fname.append(temp);
#endif
                    //fname.append("::", 2);
                    fname.append(klass_);
                }
#if !defined(CPLUS_DEMANGLE_LEGACY_COMPAT)
                // uncomment for verbose ctors
                // fname.append(temp);
#endif
            }
            else if (parse_operator(fname, pos, pos + len))
            {
                fname.append(temp);
            }
            else
            {
                fname.append(name_ + pos, len);
                fname.append(temp);
            }
            parse_fun_param(fname);
            if (pos_ < size_)
            {
                if (flags_ & UNMANGLE_NOFUNARGS)
                {
                    // nothing to do
                }
                else if (name_[pos_] != '_')
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
                else
                {   // parse the return type
                    ++pos_;
                    parse_type(out);
                    out.append(" ", 1);
                }
            }
            out.append(fname);
            if ((flags_ & UNMANGLE_NOFUNARGS) == 0)
            {
                out.append(qual);
            }
        }
        void parse_static_name(Output& out)
        {
            TRACE;
            ++pos_;
            parse_name(out);
            if ((name_[pos_] != '.') && (name_[pos_] != '$'))
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                ++pos_;
                out.append("::", 2);
                out.append(name_ + pos_);
            }
        }
        void parse_dtor(Output& out)
        {
            TRACE;
            //const size_t p = out.size();
            parse_name(out);
            //substring_type k = out.substr(p);
            //types_.insert(k);
            //squangle_.insert(k);
            out.append("::~", 3);
            if (klass_.empty())
            {
                DECODER_ERROR__(INVALID_NAME);
            }
            else
            {
                out.append(klass_);
                parse_fun_param(out);
            }
        }
        void parse_template_template_param(Output& out)
        {
            size_t paramCount = parse_integer();
            APPEND(out, "template <");
            for (size_t i = 0; i < paramCount; ++i)
            {
                if (i)
                {
                    out.append(", ", 2);
                }
                parse_type(out);
            }
            APPEND(out, "> class");
            if ((pos_ < size_) && (name_[pos_] != '_'))
            {
                out.append(" ", 1);
                parse_unqualified_name(out);
            }
        }
        void parse_array_type(Output& out)
        {
            size_t size = (size_t)-1;
            if (name_[pos_] != '_')
            {
                size = parse_unsigned(); // todo: can be 64-bit?
                if (name_[pos_] != '_')
                {
                    DECODER_ERROR__(INVALID_NAME);
                }
            }
            ++pos_;
            parse_type(out);
            out.append(" [", 2);
            if (size != (size_t)-1)
            {
                out.append(size);
            }
            out.append("]", 1);
        }
        /// make a temp output object
        Output& temp_output()
        {
            temp_.push_back(new Output());
            return *temp_.back();
        }
#if DEBUG
        // hook for setting debugger breakpoints
        void parse_error() { }
#endif
        struct Operator
        {
            const char* mangled_;
            const char* name_;
            size_t mangledSize_;
            size_t size_;
        };
        /// @return true if an operator name is recognized
        bool parse_operator(Output& out, size_t pos, size_t end)
        {
            TRACE;
            if ((name_[pos++] != '_') || (name_[pos++] != '_'))
            {
                return false;
            }
            // user-defined conversion operator?
            if ((name_[pos] == 'o') && (name_[pos + 1] == 'p'))
            {
                APPEND(out, "operator ");
                Temporary<size_t>__(pos_, pos + 2);
                parse_type(out);
                return true;
            }
    #define OP__(m,n) { m, n, sizeof(m) - 1, sizeof(n) - 1 }
            static const Operator optable[] =
            {
                OP__("nw", " new"),
                OP__("dl", " delete"),
                OP__("new", " new"),
                OP__("delete", " delete"),
                OP__("ne", "!="),
                OP__("eq", "=="),
                OP__("ge", ">="),
                OP__("gt", ">"),
                OP__("le", "<="),
                OP__("lt", "<"),
                OP__("plus", "+"),
                OP__("pl", "+"),
                OP__("apl", "+="),
                OP__("minus", "-"),
                OP__("mi", "-"),
                OP__("ami", "-="),
                OP__("mult", "*"),
                OP__("ml", "*"),
                OP__("aml", "*="),
                OP__("convert", "+"),
                OP__("negate", "-"),
                OP__("trunc_mod", "%"),
                OP__("md", "%"),
                OP__("amd", "%="),
                OP__("trunc_div", "/"),
                OP__("dv", "/"),
                OP__("adv", "/="),
                OP__("truth_andif", "&&"),
                OP__("aa", "&&"),
                OP__("truth_orif", "||"),
                OP__("oo", "||"),
                OP__("truth_not", "!"),
                OP__("nt", "!"),
                OP__("postincrement", "++"),
                OP__("pp", "++"),
                OP__("postdecrement", "--"),
                OP__("mm", "--"),
                OP__("bit_ior", "|"),
                OP__("or", "|"),
                OP__("aor", "|="),
                OP__("bit_xor", "^"),
                OP__("er", "^"),
                OP__("aer", "^="),
                OP__("bit_and", "&"),
                OP__("ad", "&"),
                OP__("aad", "&="),
                OP__("bit_not", "~"),
                OP__("co", "~"),
                OP__("call", "()"),
                OP__("cl", "()"),
                OP__("cond", "?:"),
                OP__("alshift", "<<"),
                OP__("ls", "<<"),
                OP__("als", "<<="),
                OP__("arshift", ">>"),
                OP__("rs", ">>"),
                OP__("ars", ">>="),
                OP__("component", "->"),
                OP__("rf", "->"),
                OP__("indirect", "*"),
                OP__("method_call", "->()"),
                OP__("addr", "&"),
                OP__("array", "[]"),
                OP__("vc", "[]"),
                OP__("compound", ","),
                OP__("cm", ","),
                OP__("nop", ""),
                OP__("as", "="),
                OP__("cond", "?:"),
                OP__("cn", "?:"),
                OP__("max", ">?"),
                OP__("mx", ">?"),
                OP__("min", "<?"),
                OP__("mn", "<?"),
            };
#undef OP__
            // todo: improve algorithm
            for (size_t i = 0; i != sizeof(optable)/sizeof(optable[0]); ++i)
            {
                const Operator& op = optable[i];
                if (end - pos != op.mangledSize_)
                {
                    continue;
                }
                if (strncmp(name_ + pos, op.mangled_, op.mangledSize_) == 0)
                {
                    APPEND(out, "operator");
                    out.append(op.name_, op.size_);
                    return true;
                }
            }
            return false;
        }
        static bool maybe_name(char c) { return (c == 'Q' || isdigit(c)); }

    // non-assignable, non-copyable
        Decoder(const Decoder&);
        Decoder& operator=(const Decoder&);

        const char*     name_;
        size_t          size_;
        size_t          pos_;
        bool            isCtor_;
        bool            gotKlassName_;
        int             flags_;
        int             status_;
        dict_type       templateArgs_;
        dict_type       types_;
        squangle_dict   squangle_;
        TempOutputs     temp_;
        substring_type  klass_;
    };
}
#endif // LEGACY_H__0A821E67_C5BE_44C5_B0B4_EA56BF947779
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

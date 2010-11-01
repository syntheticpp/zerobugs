#ifndef ENCODER_H__1C552AE1_2014_4B9A_A25C_D6A5CCE47F7D
#define ENCODER_H__1C552AE1_2014_4B9A_A25C_D6A5CCE47F7D
//
// $Id: encoder.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <iosfwd>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace Mangle
{
    typedef std::map<std::string, size_t> Dictionary;

    /**
     * c++ mangling schemes
     */
    enum Scheme { kV3Abi, kOldGcc };

    enum ComponentType { kNormal, kCtor, kDtor, kTemplateArg };

    /**
     * c++ name component
     */
    class Component
    {
    public:
        explicit Component(const std::string&, ComponentType = kNormal);

        ~Component();

        const std::string& name() const { return name_; }

        std::string substr() const;

        void add_child(Component*);

        const std::vector<Component*>& children() const { return children_; }

        const Component* parent() const { return parent_; }

        ComponentType type() const { return type_; }

        bool is_template() const { return isTemplate_; }

        void set_template() { isTemplate_ = true; }

    private:
        Component(const Component&);
        Component& operator=(const Component&);

        std::string             name_; // unqualified name
        std::vector<Component*> children_;
        const Component*        parent_;
        ComponentType           type_;
        bool                    isTemplate_;
    };


    /**
     * Partially mangle a C++ name -- the result is intended to be
     * used as a search key on a sorted collection of mangled names;
     * Calling lower_bound() and upper_bound() on the collection
     * should narrow down the search quite a bit -- we can then
     * demangle all the names in that range and do an exact match.
     * @note if AGGRESSIVE_MANGLE is defined at compile-time, the
     * encoder will try to perform a more complete mangling.
     */
    class PartialEncoder
    {
    public:
        explicit PartialEncoder(Scheme scheme = kV3Abi)
            : scheme_(scheme), hasTemplate_(false), depth_(0)
        { }

        virtual ~PartialEncoder();

        std::string run(const char*, const char* qualifiers = NULL);

        bool has_template() const { return hasTemplate_; }

        /**
         * @return depth of qualified name -- for example, depth of Foo::bar is 2,
         * depth of Foo::Bar::fun is 3, Foo<Bar>::fun is 1, and so on.
         */
        size_t depth() const { return depth_; }

    protected:
        void clear();

        /**
         * Break the name down into components
         */
        void tokenize(const char*);

        Component* add_component(const char*, const char*, Component* parent);
        Component* add_template_arg(std::stack<Component*>&, Component& parent);

        std::string encode_name(const Component&);

        bool encode(const Component*, std::ostream&);

#if defined (AGGRESSIVE_MANGLE)
        Dictionary::const_iterator find_best_subst(const Component*, size_t&) const;

        void print_dictionary(std::ostream&) const;
#endif // AGGRESSIVE_MANGLE

    private:
        PartialEncoder(const PartialEncoder&);
        PartialEncoder& operator=(const PartialEncoder&);

        Scheme                  scheme_; // which encoding scheme to use
        std::vector<Component*> components_;
#if defined (AGGRESSIVE_MANGLE)
        Dictionary              subst_;
#endif
        bool                    hasTemplate_;
        size_t                  depth_;
    };

} // namespace Mangle
#endif // ENCODER_H__1C552AE1_2014_4B9A_A25C_D6A5CCE47F7D
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

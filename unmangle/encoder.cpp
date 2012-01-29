//
// $Id$
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include "encoder.h"

using namespace std;
using namespace Mangle;


static const map<string, string>& builtin_subst()
{
    static map<string, string> m;
    if (m.empty())
    {
    /* built-in types */
        m.insert(m.end(), make_pair("__float128", "g"));
        m.insert(m.end(), make_pair("char", "c"));
        m.insert(m.end(), make_pair("double", "d"));
        m.insert(m.end(), make_pair("float", "f"));
        m.insert(m.end(), make_pair("int", "i"));
    /* compression -- standard substitutions */
        m.insert(m.end(), make_pair("std", "St"));
        m.insert(m.end(), make_pair("allocator", "Sa"));
        m.insert(m.end(), make_pair("basic_string", "Sb"));
        m.insert(m.end(), make_pair(
            "basic_string<char, char_traits<char>, allocator<char> >", "Ss"));
        m.insert(m.end(), make_pair("basic_istream<char, char_traits<char> >", "Si"));
        m.insert(m.end(), make_pair("basic_ostream<char, char_traits<char> >", "So"));
        m.insert(m.end(), make_pair("basic_iostream<char, char_traits<char> >", "Sd"));
    }
    return m;
}

#if defined(AGGRESSIVE_MANGLE)
static bool is_substitutable(const string& name)
{
    return !name.empty()
        //&& name != "const"
        //&& name != "volatile"
        && (builtin_subst().find(name) == builtin_subst().end());
}
#endif


Component::Component(const string& name, ComponentType type)
    : name_(name)
    , parent_(NULL)
    , type_(type)
    , isTemplate_(false)
{
}


Component::~Component()
{
}


void Component::add_child(Component* comp)
{
    assert(comp);
    assert(!comp->parent());

    children_.push_back(comp);
    comp->parent_ = this;
}

#ifdef AGGRESSIVE_MANGLE
string Component::substr() const
{
    if (type() == kCtor
        || type() == kDtor
        || name() == "const"
        || name() == "volatile")
    {
        return "";
    }
    string result;
    for (size_t i = 0; i != children_.size(); ++i)
    {
        if (!result.empty())
        {
            if (type() == kTemplateArg)
            {
                result += ' ';
            }
            else
            {
                result += ", ";
            }
        }
        string temp = children_[i]->substr();

        if ((type() == kTemplateArg) && !is_substitutable(temp))
        {
            return "";
        }
        result += temp;
    }
    if (is_template())
    {
        result = "<" + result + ">";
    }
    return name() + result;
}
#endif // AGGRESSIVE_MANGLE


string PartialEncoder::encode_name(const Component& comp)
{
    const string& name = comp.name();

    assert(!name.empty());

    if (isdigit(name[0])) // numeric template param?
    {
        char* p = NULL;
        long num = strtol(name.c_str(), &p, 0);
        num = num; // silence compiler warning

        if ((p != NULL) && (*p == '.'))
        {
            return "Ld" + name + "E";
        }
        else
        {
            return "Li" + name + "E";
        }
    }
    else if (name == "true") // boolean template param?
    {
        return "Lb1E";
    }
    else if (name == "false")
    {
        return "Lb0E";
    }
    else
    {
        const map<string, string>& m = builtin_subst();
        map<string, string>::const_iterator j = m.find(name);
        if (j != m.end())
        {
            return j->second;
        }
        ostringstream tmp;

#if !defined(AGGRESSIVE_MANGLE)
        tmp << name.size() << name;
        return tmp.str();
#else
        Dictionary::iterator i = subst_.find(name);
        if (i == subst_.end())
        {
            subst_.insert(i, make_pair(name, subst_.size()));
            tmp << name.size();
            return tmp.str() + name;
        }

        tmp << 'S';
        if (i->second)
        {
            tmp << i->second - 1;
        }
        else
        {
            tmp << '_';
        }
#endif // AGGRESSIVE_MANGLE
        return tmp.str();
    }
}


PartialEncoder::~PartialEncoder()
{
    clear();
}


string PartialEncoder::run(const char* symbol, const char* qual)
{
    clear();
    ostringstream mangled;
    tokenize(symbol);

    switch (scheme_)
    {
    case kV3Abi:
        {
            mangled << "_Z";

            if (depth() > 1)
            {
                mangled << 'N';
            }
            const size_t nameCount = components_.size();
            for (size_t i = 0; i != nameCount; ++i)
            {
                const Component* comp = components_[i];
                assert(comp);

                if (comp->parent())
                {
                    continue;
                }
                if (strncmp("operator", comp->name().c_str(), 8) == 0)
                {
                    depth_ = 1; // hack -- prevent 'E' endings
                    break;
                }
                encode(comp, mangled);
#if !defined( AGGRESSIVE_MANGLE)
                if (comp->is_template())
                {
                    return mangled.str();
                }
#endif
            }

            if (depth() > 1)
            {
                mangled << "E";
            }
        }
        break;

    case kOldGcc:
        if (size_t n = components_.size())
        {
            // find the last top-level component
            for (;; --n)
            {
                Component* comp = components_[n - 1];
                assert(comp);

                if (!comp->parent())
                {
                    mangled << comp->name();
                    break;
                }
            }
            if (n > 1)
            {
                mangled << "__";

                if (qual)
                {
                    mangled << qual;
                }
                if (depth() > 2)
                {
                    mangled << 'Q';
                    if (depth() > 10)
                    {
                        mangled << '_';
                    }
                    mangled << depth() - 1;

                    ostringstream tmp;
                }
                Component* klass = components_[0];
                assert(klass);
                if (klass->is_template())
                {
                    mangled << 't';
                }
                mangled << klass->name().size() << klass->name();
                if (klass->is_template())
                {
                    mangled << klass->children().size();
                }
            }
        }
        break;
    }
    return mangled.str();
}


bool PartialEncoder::encode(const Component* comp, ostream& outs)
{
    assert(comp);
    if ((comp->type() == kCtor) || (comp->type() == kDtor))
    {
       outs << comp->name();
    }
    else
    {
        assert(comp->name() != "const"
            && comp->name() != "volatile"
            && comp->name() != "*"
            && comp->name() != "&");

        if (comp->type() != kTemplateArg)
        {
            outs << encode_name(*comp);
        }
    }
    const size_t numChildren = comp->children().size();

    if (numChildren && (comp->type() != kTemplateArg))
    {
        outs << 'I';
#if !defined (AGGRESSIVE_MANGLE)
        return false;
#endif
    }
    string prevEnc, name;

    size_t n = 0;
#if defined(AGGRESSIVE_MANGLE)
    Dictionary::const_iterator iter = find_best_subst(comp, n);
    assert(n <= numChildren);

    if (iter != subst_.end())
    {
        ostringstream tmp;
        tmp << 'S';
        if (iter->second)
        {
            tmp << iter->second - 1;
        }
        else
        {
            tmp << '_';
        }
        ++n;
        prevEnc = tmp.str();
        name = iter->first;
    }
    bool canSubst = true;
#endif // AGGRESSIVE_MANGLE

    bool cvQual = false;

    for (size_t i = n; i != numChildren; ++i)
    {
        const Component* child = comp->children()[i];

        if (child->name() == "const")
        {
            prevEnc.insert(0, scheme_ == kV3Abi ? "K" : "C");
            cvQual = true;
        }
        else if (child->name() == "volatile")
        {
            prevEnc.insert(0, "V");
            cvQual = true;
        }
        else if (child->name() == "*")
        {
            assert(!prevEnc.empty());
            prevEnc.insert(0, "P");
            cvQual = false;
        }
        else if (child->name() == "&")
        {
            assert(!prevEnc.empty());
            prevEnc.insert(0, "R");
            cvQual = false;
        }
        else
        {
            if (!cvQual)
            {
                outs << prevEnc; // flush previous encoding
                prevEnc.clear();
            }
            ostringstream tmp;
            if (!encode(child, tmp))
            {
                return false;
            }

            prevEnc += tmp.str();
        }
#if defined (AGGRESSIVE_MANGLE)
        if (!is_substitutable(child->name()))
        {
            canSubst = false;
        }
        if (canSubst)
        {
            if (!name.empty())
            {
                name += ' ';
            }
            name += child->name();
            assert(is_substitutable(name));
            subst_.insert(make_pair(name, subst_.size()));
            print_dictionary(clog);
        }
#endif // AGGRESSIVE_MANGLE
    }
    outs << prevEnc;

    if (numChildren && (comp->type() != kTemplateArg))
    {
        outs << 'E';
    }
#if defined (AGGRESSIVE_MANGLE)
    string substr = comp->substr();
    if (is_substitutable(substr))
    {
        subst_.insert(make_pair(substr, subst_.size()));
        print_dictionary(clog);
    }
#endif // AGGRESSIVE_MANGLE
    return true;
}


void PartialEncoder::clear()
{
    hasTemplate_ = false;
    depth_ = 0;

    vector<Component*>::iterator i = components_.begin();
    for (; i != components_.end(); ++i)
    {
        delete *i;
    }
    components_.clear();
#if defined (AGGRESSIVE_MANGLE)
    subst_.clear();
#endif
}


void PartialEncoder::tokenize(const char* symbol)
{
    assert(symbol);
    stack<Component*> templateStack; // for nested template args
    Component* parent = NULL;
    const char* name = symbol;
    const char* p = name;

    for (; *p; ++p)
    {
        switch (*p)
        {
        case ':':
            if (p > name)
            {
                add_component(name, p, parent);
            }
            name = (p[1] == ':') ? p + 1 : p;
            break;

        case '*':
        case '&':
            add_component(name, p, parent);
            add_component(p, p + 1, parent);
            name = p + 1;
            break;

        case '<':
            if (Component* comp = add_component(name, p, parent))
            {
                comp->set_template();
                templateStack.push(comp);
                parent = templateStack.top();
                assert(parent);

                // intermediary node, holds all components of
                // a template argument
                parent = add_template_arg(templateStack, *parent);

                name = p + 1;
                hasTemplate_ = true;
            }
            break;

        case '>':
            assert(has_template());
            while ((*name == '>') || (*name == ' '))
            {
                ++name;
            }
            if (p > name)
            {
                add_component(name, p, parent);
            }
            // pop the intermediary node of the last argument
            assert(!templateStack.empty());
            templateStack.pop();
            // pop the template node
            assert(!templateStack.empty());
            templateStack.pop();
            parent = templateStack.empty() ? NULL : templateStack.top();
            name = p + 1;
            break;

        case ',':
            add_component(name, p, parent);
            name = p + 1;
            if (has_template())
            {
                // pop the intermediary node of the last argument
                assert(!templateStack.empty());
                templateStack.pop();
                // the template node should be on the stack
                assert(!templateStack.empty());
                parent = templateStack.top();
                assert(parent);
                // make an intermediary node for the next arg
                parent = add_template_arg(templateStack, *parent);
            }
            break;

        case ' ':
            if ((*name != ' ') && (*name != ',') && (*name != '>'))
            {
                add_component(name, p, parent);
            }
            name = p + 1;
            break;

        case '(':
            if (p > name)
            {
                add_component(name, p, parent);
            }
            return;
        }
    }
    if (*name)
    {
        add_component(name, p, parent);
    }
}


Component*
PartialEncoder::add_component(const char* first, const char* last, Component* parent)
{
    assert(last >= first);
    if (last <= first)
    {
        return NULL;
    }

    if ((*first == ':') || (*first == ' '))
    {
        ++first;
    }
    ComponentType type = kNormal;
    string name;
    if (*first == '~')
    {
        if (scheme_ == kOldGcc)
        {
            name.assign(first, last);
        }
        else
        {
            name = "D1";
        }
        type = kDtor;
    }
    else
    {
        assert(isalnum(*first) || (*first == '_')
            || (*first == '*') || (*first == '&'));
        name.assign(first, last);

        // check for ctor name
        if (!components_.empty() && (!parent || parent->type() != kTemplateArg))
        {
            const Component* prev = components_.back();
            while (prev->parent())
            {
                prev = prev->parent();
            }
            assert(prev);

            if (prev->name() == name)
            {
                if (scheme_ == kOldGcc)
                {
                    name = "";
                }
                else
                {
                    name = "C1";
                }
                type = kCtor;
            }
        }
    }
    auto_ptr<Component> comp (new Component(name, type));

    if (parent)
    {
        parent->add_child(comp.get());
    }
    else
    {
        ++depth_;
    }

    components_.push_back(comp.get());
    return comp.release();
}


Component*
PartialEncoder::add_template_arg(stack<Component*>& templateStack, Component& parent)
{
    auto_ptr<Component> comp(new Component("", kTemplateArg));
    parent.add_child(comp.get());
    components_.push_back(comp.get());

    templateStack.push(comp.get());
    return comp.release();
}


#ifdef AGGRESSIVE_MANGLE
Dictionary::const_iterator
PartialEncoder::find_best_subst(const Component* comp, size_t& childIndex) const
{
    Dictionary::const_iterator iter = subst_.end();

    string name;
    const size_t numChildren = comp->children().size();
    for (size_t i = 0; i != numChildren; ++i)
    {
        const Component* child = comp->children()[i];
        string substr = child->substr();
        if (!substr.empty())
        {
            if (!name.empty())
            {
                if (comp->type() == kTemplateArg)
                {
                    name += ' ';
                }
                else
                {
                    name += ", ";
                }
            }
            name += substr;
        }

        Dictionary::const_iterator tmp = subst_.find(name);
        if (tmp == subst_.end())
        {
            if (iter != subst_.end())
            {
//#if defined(_DEBUG) || defined(DEBUG)
//                    clog << iter->first << endl;
//#endif
                break;
            }
        }
        else
        {
            childIndex = i;
        }
        iter = tmp;
    }
    return iter;
}


void PartialEncoder::print_dictionary(ostream& outs) const
{
/* #if defined(_DEBUG) || defined(DEBUG)
    for (Dictionary::const_iterator i = subst_.begin(); i != subst_.end(); ++i)
    {
        outs << i->second << ": " << i->first << endl;
    }
#endif */
}
#endif // AGGRESSIVE_MANGLE
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4

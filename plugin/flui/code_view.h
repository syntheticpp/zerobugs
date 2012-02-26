#ifndef CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
#define CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89
//
// -*- tab-width: 4; indent-tabs-mode: nil;  -*-
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
//
// $Id: $
//
// Toolkit-independent logic for displaying source and assembly code.
//
#include "view.h"
#include "zdk/platform.h"
#include "zdk/shared_string.h"
#include "listing.h"
#include <unordered_map>


namespace ui
{
    using Platform::addr_t;

    class Controller;
    class MultiCodeView;


    /**
     * Base (abstract) class for displaying some sort of CODE
     * listing (such as source code, assembly, etc.)
     */
    class CodeView : public View
    {
    public:
        /**
         * Make the symbol (and the code surrounding it) visible.
         */
        virtual void show(RefPtr<Thread>, RefPtr<Symbol>) = 0;

        /**
         * update view to show breakpoint, if necessary
         */
        virtual void update_breakpoint(BreakPoint&) = 0;

        virtual const CodeListing* get_listing() = 0;

        MultiCodeView* parent() const {
            return parent_;
        }

        void set_parent(MultiCodeView* parent) {
            parent_ = parent;
        }

    protected:
        explicit CodeView(Controller&);
        ~CodeView() throw();

        virtual ViewType type() const { return VIEW_Code; }

        const RefPtr<Symbol>& current_symbol() const {
            return current_;
        }

        void set_current_symbol(const RefPtr<Symbol>& sym);

    private:
        RefPtr<Symbol>  current_;
        MultiCodeView*  parent_;
    };

    typedef RefPtr<CodeView> CodeViewPtr;


    ////////////////////////////////////////////////////////////
    /**
     * Map source code line numbers to addresses in the code.
     */
    class LineAddrMap
    {
    public:
        void add(int line, addr_t addr);

        int addr_to_line(addr_t) const;

        addr_t line_to_addr(int) const;

        void clear() {
            addrToLine_.clear();
            lineToAddr_.clear();
        }

    private:
        // maintain mapping (direct and reverse) between
        // addresses in the code and lines in the listing
        typedef std::unordered_map<addr_t, int> AddrToLine;
        typedef std::unordered_multimap<int, addr_t> LineToAddr;

        AddrToLine  addrToLine_;
        LineToAddr  lineToAddr_;
    };


    ////////////////////////////////////////////////////////////
    class SourceView : public CodeView, protected CodeListing
    {
    public:
        explicit SourceView(Controller&);

    protected:
        ~SourceView() throw();

        void read_file(Thread*, const char* filename);

        const CodeListing* get_listing() {
            return this;
        }

        //
        // CodeListing interface
        //
        virtual bool refresh(
            const RefPtr<Thread>&,
            const RefPtr<Symbol>&
            );

        virtual const char* current_file() const;
        virtual size_t current_line() const;

        virtual size_t line_count() const {
            return lines_.size();
        }

        virtual const std::string& line(size_t index) const {
            assert(index < lines_.size());
            return lines_[index];
        }

        virtual addr_t selected_addr() const;

    private:
        // Store the file as a vector of lines of text.
        std::vector<std::string>    lines_;
        std::string                 filename_;

        LineAddrMap                 lineAddrMap_;
    };


    ////////////////////////////////////////////////////////////
    class AsmView : public CodeView, protected CodeListing
    {
    public:
        explicit AsmView(Controller&);

        /**
         * add a listing line to the view.
         * @param addr address in the target's code that
         *  the listing line corresponds to
         * @param line listing line may be (assembly or source)
         */
        void add_listing_line(
            addr_t              addr,
            const std::string&  line);

        const CodeListing* get_listing() {
            return this;
        }

        virtual void set_row_count(int) = 0;

    protected:
        virtual ~AsmView() throw();

        //
        // CodeListing interface
        //
        int addr_to_line(addr_t) const;

        virtual bool refresh(
            const RefPtr<Thread>&,
            const RefPtr<Symbol>&
            );

        virtual const char* current_file() const;
        virtual size_t current_line() const;

        virtual const std::string& line(size_t index) const;

        virtual size_t line_count() const {
            return lines_.size();
        }

        virtual addr_t selected_addr() const;

    private:
        typedef std::vector<std::string> Lines;

        Lines       lines_;
        LineAddrMap lineAddrMap_;
    };


    ////////////////////////////////////////////////////////////
    /**
     * Composite code view.
     */
    class MultiCodeView : public CodeView
    {
    public:
        explicit MultiCodeView(Controller&);

        void set_current_view(const RefPtr<CodeView>& v) {
            currentView_ = v;
        }

    protected:
        ~MultiCodeView() throw();

        virtual void clear();

        virtual const CodeListing* get_listing();

        virtual ViewType type() const { return VIEW_Code; }

        virtual void update(const State&);

        /**
         * update view to show breakpoint, if necessary
         */
        void update_breakpoint(BreakPoint&);

        virtual void make_visible(CodeViewPtr) = 0;

    private:
        virtual Layout::CallbackPtr make_callback() = 0;

        virtual CodeViewPtr make_view(const Symbol&) = 0;

    protected:
        // map code views by file name
        std::unordered_map<SharedStringPtr, CodeViewPtr> views_;

    private:
        CodeViewPtr currentView_;
    };
};

#endif // CODE_VIEW_H__12269830_C69E_4A94_96F8_368E54F08F89


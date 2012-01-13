#!/usr/bin/env python
#
# Graphical installer script for Zero Debugger
# Version 0.13
# -------------------------------------------------------------------------
# This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# -------------------------------------------------------------------------

import glob
import gnome.ui
import gobject
import gtk
import os
import subprocess

try:
    import platform
    arch = str(platform.machine())
except:
    f = os.popen("arch -m")
    arch = f.readline()
    f.close()
import sys

GDK_INPUT_READ = 1 #for portability across pygtk versions

#---------------------------------------------------------------
#
# Strings
#
#---------------------------------------------------------------
intro_text = """
Zero is a source-level symbolic debugger for user-mode
Linux applications written in C and C++. 

Installing on: """ + arch

try:
    intro_text = intro_text + "-" + "-".join(platform.dist())
except:
    pass

dest_text = """
This is the directory where all files will be installed. 
The installer creates the directory structure to hold 
binaries, plug-ins, help files, dynamic libraries, etc.  
"""

log_text = """
Enter the name of the file where installation details are recorded.
"""

complete_text = """
Installation complete. You may now invoke the debugger like this:
%s/bin/zero
"""

#---------------------------------------------------------------
#
# Wrapper script that will be installed (sets environment 
# before invoking the debugger proper).
#
#---------------------------------------------------------------
script_text = """#! /usr/bin/env bash
# Wrapper script for the Zero Debugger
# Copyright (c) 2010 Cristian Vlasceanu
#
# Sets up the environment for running zero
#
# Workaround broken __mt_alloc
export GLIBCXX_FORCE_NEW=1
export GLIBCPP_FORCE_NEW=1

# Gotta be careful with empty LD_LIBRARY_PATH, because of
# this bug: http://sourceware.org/bugzilla/show_bug.cgi?id=4776
#
if test -n "$LD_LIBRARY_PATH"; then
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PREFIX/zero/lib
else
  export LD_LIBRARY_PATH=$PREFIX/zero/lib
fi

if test -n "$LD_LIBRARY_PATH"; then
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PREFIX/zero/lib
else
  export LD_LIBRARY_PATH=$PREFIX/zero/lib
fi

#save settings and history in current user's HOME by default
if [ -z "$ZERO_CONFIG_PATH" ]
    then export ZERO_CONFIG_PATH=~
fi

if [ -z "$ZERO_PLUGIN_PATH" ]
    then export ZERO_PLUGIN_PATH=$PREFIX/zero/plugin
fi

export ZERO_HELP_PATH=$PREFIX/zero/help

if [ -z "$ZERO_START_MAIN" ]
#start debugging at main() rather than at CRT start;
#comment this out if your debugged programs don't have a 
# main() function
    then export ZERO_START_MAIN=1
fi 

#set this to use stack frame information from plugins
#note: if not set, it defaults to true on x86_64
#export ZERO_USE_FRAME_HANDLERS=1

#use hardware breakpoints whenever available (default);
#it is a good idea to turn it off on Virtual PC or VMware
if [ -z "$ZERO_HARDWARE_BREAKPOINTS" ]
    then export ZERO_HARDWARE_BREAKPOINTS=1
fi

#Use expensive type lookups with DWARF
# If set, attempt to find the full type definition, rather than just
# being happy with a forward declaration that matches.
# If a full definition is not found in the current module, lookup all
# program modules.
export ZERO_USE_EXPENSIVE_LOOKUPS=false

#do not dive into assembly when stepping from a portion
#where we do have C/C++ source code, into another one
#where source is not available
if [ -z "$ZERO_SOURCE_STEP" ]
    then export ZERO_SOURCE_STEP=1
fi

if [ -z "$ZERO_QUICK_SHUTDOWN" ]
    then export ZERO_QUICK_SHUTDOWN=1
fi

$PREFIX/bin/zero-bin $@
"""


#---------------------------------------------------------------
#
# Files (and directories) to install
#
#---------------------------------------------------------------
_files = {
    "bin/zero"          : "bin/zero-bin",
    "bin/zserver"       : "bin/zserver",
    "plugin/zdisasm.so" : "zero/plugin/zdisasm.so",
    "plugin/zdwarf.so"  : "zero/plugin/zdwarf.so",
    "plugin/zstabs.so"  : "zero/plugin/zstabs.so",
    "plugin/zgui.so"    : "zero/plugin/zgui.so",
    "plugin/zpython.so" : "zero/plugin/zpython.so",
    "plugin/zremote-proxy.so" : "zero/plugin/zremote-proxy.so",
    "plugin/.zero.py"   : "zero/plugin/.zero.py",
    "plugin/update.py"  : "zero/plugin/update.py",
    "help/"             : "zero/help",
    "lib/libdwarf.so"   : "zero/lib/libdwarf.so",
    "lib/libdemangle_d.so" : "zero/lib/libdemangle_d.so",
    "zeroicon.png"      : "zero/zeroicon.png",
    "LIBDWARFCOPYRIGHT" : "zero/LIBDWARFCOPYRIGHT",
}
_numFiles = 0


#---------------------------------------------------------------
#
# Event handlers shared by Wizard (Druid) pages
#
#---------------------------------------------------------------
class PageEvents(object):
    """
    Handle common signal events from druid pages
    """
    def __init__(self, druidPage):
        druidPage.set_title("ZeroBUGS Setup")
        druidPage.connect("cancel", self.__on_cancel)
        pixbuf = gtk.gdk.pixbuf_new_from_file("500_setup.png")
        try:
            druidPage.set_watermark(pixbuf)
        except:
            pass

    def __on_cancel(self, page, druid):
        druid.quit()


#---------------------------------------------------------------
#
# Base Wizard class
#
#---------------------------------------------------------------
class Wizard(gnome.ui.Druid):
    def __init__(self):
        gnome.ui.Druid.__init__(self)
        self.completed = False
        self.__children = []

    def append(self, process):
        self.__children.append(process)

    def quit(self):
        if self.completed:
            self.__quit()
        #Prompt user for confirmation
        dlg = gtk.MessageDialog(None,   #parent
                                0,      #flags
                                gtk.MESSAGE_QUESTION, 
                                gtk.BUTTONS_YES_NO,
                                "Do you really want to cancel?")
        dlg.set_transient_for(self.get_toplevel())
        dlg.connect("response", self.__on_response)
        dlg.run()
    
    def __on_response(self, dialog, response):
        dialog.destroy()
        if response == gtk.RESPONSE_YES:
            self.__quit()

    def __quit(self):
        gtk.main_quit()
        #reap child processes:
        for process in self.__children:
            process.poll()
        sys.exit(0)
    

#---------------------------------------------------------------
#
# Introductory page
#
#---------------------------------------------------------------
class IntroPage(gnome.ui.DruidPageEdge):
    """
    First page of our install druid
    todo: each page should be a singleton
    """
    def __init__(self):
        gnome.ui.DruidPageEdge.__init__(self, gnome.ui.EDGE_START)
        self.set_text(intro_text)
        self.__events = PageEvents(self)
        self.show_all()
    

#---------------------------------------------------------------
#
# main Setup page, handles most of the user interaction
#
#---------------------------------------------------------------
class MainPage(gnome.ui.DruidPageStandard):
    def __init__(self):
        gnome.ui.DruidPageStandard.__init__(self)
        self.__events = PageEvents(self)
        prefix=""
        logname="install.log"
        for arg in sys.argv:
            if arg.find("--prefix=") == 0:
                prefix = arg[9:]
            elif arg.find("--log=") == 0:
                logname = arg[6:]
        if not prefix:
            prefix="/usr/local/"
        box = gtk.HBox()
        self.__prefix = gtk.Entry()
        self.__prefix.set_size_request(300, -1)
        self.__prefix.set_text(prefix)
        box.add(self.__prefix)
        btn = gtk.Button("Browse...")
        box.pack_end(btn)
        btn.connect("clicked", self.__browse)
        self.append_item("Destination Path", box, dest_text)
        self.append_item("", gtk.HSeparator(), "")
        self.__logname = gtk.Entry()
        self.__logname.set_text(logname)
        self.append_item("Log Filename", self.__logname, log_text)
        self.show_all()

    def __browse(self, w):
        """
        Begin browsing for the destination directory
        """
        fileSel = gtk.FileSelection("Destination")
        #fileSel.hide_fileop_buttons()
        fileSel.set_select_multiple(False)
        fileSel.set_filename(self.__prefix.get_text())
        fileSel.connect("response", self.__on_response)
        fileSel.run()

    def __on_response(self, dlg, response):
        """
        Handle responses from the FileSelection dialog
        """
        if response == gtk.RESPONSE_OK:
            filename = dlg.get_filename()
            self.__prefix.set_text(filename)
        dlg.hide()

    def dest_dir(self):
        return self.__prefix.get_text()

    def log_name(self):
        return self.__logname.get_text()


#---------------------------------------------------------------
#
#
#
#---------------------------------------------------------------
class FinishPage(gnome.ui.DruidPageEdge):
    def __init__(self):
        gnome.ui.DruidPageEdge.__init__(self, gnome.ui.EDGE_FINISH)
        self.__events = PageEvents(self)
        self.srcFiles = []
        for f in _files:
            if os.path.isdir(f):
                f = f + "/*"
            self.srcFiles += glob.glob(f)
        global _numFiles
        _numFiles = len(self.srcFiles) + 1 # srcFiles + wrapper script
        self.set_text(
            "Ready to install %d files:\n\n%s" % (_numFiles, "\n".join(_files)))
        self.show_all()
    

#---------------------------------------------------------------
#
# Shows file copy progress
#
#---------------------------------------------------------------
class LogWindow(gtk.Dialog):
    def __init__(self):
        gtk.Dialog.__init__(self, "Install Log")
        vbox = gtk.VBox()
        self.vbox.pack_start(vbox, False)
        frame = gtk.Frame("Percentage completed: ")
        frame.set_border_width(5)
        vbox.pack_start(frame, False)
        hbox = gtk.HBox()
        frame.add(hbox)
        self.__progress = gtk.ProgressBar()
        hbox.add(self.__progress)
        frame = gtk.Frame("Detail: ")
        frame.set_border_width(5)
        self.vbox.pack_end(frame, False)    
        sw = gtk.ScrolledWindow()
        sw.set_border_width(5)
        sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        frame.add(sw)
        text = gtk.TextView()
        text.set_editable(False)
        text.set_cursor_visible(False)
        self.__buf = text.get_buffer()
        try:
            self.__errTag = self.__buf.create_tag(foreground="red")
        except:
            #handle older pygtk (2.0)
            self.__errTag = self.__buf.create_tag("fg")
            self.__errTag.set_property("foreground", "red")
        sw.add(text)
        self.set_size_request(460, 240)
        self.add_button("Close", gtk.RESPONSE_OK)
        self.connect("response", self.__on_response)

    def __on_response(self, dlg, response):
        self.hide()

    def add(self, line):
        iter = self.__buf.get_iter_at_mark(self.__buf.get_insert())
        self.__buf.insert(iter, line)

    def add_error(self, line):
        iter = self.__buf.get_iter_at_mark(self.__buf.get_insert())
        self.__buf.insert_with_tags(iter, line, self.__errTag)

    def set_fraction(self, fract):
        if fract <= 1.:
            self.__progress.set_fraction(fract)
            percentage = str(int(fract * 100)) + "%"
            self.__progress.set_text(percentage)


#---------------------------------------------------------------
#
# The Setup Wizard
#
#---------------------------------------------------------------
class InstallWizard(gtk.Window):
    """
    The toplevel window that holds the druid
    """
    def __init__(self):
        gtk.Window.__init__(self, "toplevel")
        self.connect("delete_event", self.__on_delete)
        self.set_position(gtk.WIN_POS_CENTER_ALWAYS)
        self.set_title("Setup")
        self.set_size_request(500, 400)
        self.__logFile = None
        self.__passwd = None
        self.__druid = Wizard()
        self.__druid.append_page(IntroPage())
        self.__main = MainPage()
        self.__finish = FinishPage()
        self.__finish.connect("finish", self.__install)
        self.__druid.append_page(self.__main)
        self.__druid.append_page(self.__finish)
        self.__druid.show()
        self.add(self.__druid)

    def __on_delete(self, w, event):
        self.__druid.quit()
        return True

    def __handle_stdout(self, source, condition):
        line = source.readline()
        if line:
            self.__logWin.add(line)
        else:
            return False
        self.__logFile.write(line)
        self.__completed +=1.
        assert(_numFiles)
        fractionDone = self.__completed / _numFiles
        self.__logWin.set_fraction(fractionDone)
        if not self.__errorCount and (fractionDone >= 1.):
            self.__logWin.hide()
            self.__druid.completed = True
            self.__druid.set_show_finish(False)
            self.__druid.set_buttons_sensitive(False, False, True, False)
            self.__finish.set_text(complete_text % self.__main.dest_dir())
        return True

    def __handle_stderr(self, source, condition):
        line = source.readline()
        if line:
            self.__errorCount += 1 
            self.__logWin.add_error(line)
        else:
            return False
        return True

    def __check_uid(self, destDir):
        d = destDir
        while True:
            try:
                statResult = os.stat(d)
                break
            except:
                d = os.path.dirname(d)

        if statResult.st_uid != os.getuid():
            dlg = gtk.Dialog("Password Required")
            dlg.set_transient_for(self)
            dlg.set_size_request(300, 120)
            dlg.vbox.add(gtk.Label("Enter sudo password:"))
            self.__passwdEntry = gtk.Entry()
            self.__passwdEntry.set_visibility(False)
            dlg.vbox.add(self.__passwdEntry)
            dlg.add_button("Cancel", gtk.RESPONSE_CANCEL)
            dlg.add_button("OK", gtk.RESPONSE_OK)
            dlg.connect("response", self.__on_passwd_response)
            dlg.show_all()
            self.__passwd = None
            while True:
                dlg.run()
                if self.__passwd != None:
                    break;
            return True
        return False # no sudo necessary

    def __on_passwd_response(self, dlg, response):
        self.__passwd = ""
        if response == gtk.RESPONSE_OK:
            self.__passwd = self.__passwdEntry.get_text()
            #test the password:
            p = os.popen("sudo -k && sudo -S test 0 2>/dev/null", "w")
            p.write(self.__passwd + "\n")
            status = p.close()
            if status:
                self.__passwd = None
        dlg.hide()

    def __system(self, command, synchronous = False):
        p = subprocess.Popen(command, bufsize=2048, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        gobject.io_add_watch(p.stdout,
            GDK_INPUT_READ, 
            self.__handle_stdout)
        gobject.io_add_watch(p.stderr,
            GDK_INPUT_READ, 
            self.__handle_stderr)
        if self.__passwd:
            p.tochild.write(self.__passwd + "\n")
            p.tochild.flush()
        if synchronous:
            status = -1
            while status == -1:
                status = p.poll()
                gtk.main_iteration()
            print command,"status:",status
            return status
        else:
            self.__druid.append(p)

    def __install(self, page, druid):
        self.__errorCount = 0
        try:
            self.__logFile.close()
            self.__logWin.destroy()
        except:
            pass
        destDir = self.__main.dest_dir()
        destDir = os.path.abspath(destDir)
        if self.__check_uid(destDir) and not self.__passwd:
            return
        self.__logWin = LogWindow()
        self.__logWin.set_transient_for(self)
        self.__logWin.show_all()
        self.__logFile = open(self.__main.log_name(), "w")
        self.__completed = 0
        prefix = ""
        if self.__passwd:
            prefix = "sudo -S -p '' "
        command = prefix + 'bash -c "echo Copying files...'
        for f in _files:
            dest = destDir + "/" + _files[f]
            d = os.path.dirname(dest)
            command += " && mkdir -p " + d + " && cp -r -v " + f + " " + dest
        
        command += ' "'
        self.__system(command)
        self.__install_wrapper_script(prefix, destDir)


    def __install_wrapper_script(self, prefix, destDir):
        script = script_text.replace("$PREFIX", destDir)
        f = open("wrapper", "w")
        f.write(script)
        f.close()

        target = "%s/bin/zero" % destDir
        cmd = prefix + 'bash -c "mkdir -p ' + \
                os.path.dirname(target) + \
              ' && mv wrapper ' + target + \
              ' && chmod +x ' + target + '"'
        self.__system(cmd)
        self.__logFile.write("zero -> " + target + "\n")


#---------------------------------------------------------------
#
# main
#
#---------------------------------------------------------------
if __name__ == "__main__":
    wiz = InstallWizard()
    wiz.show()
    gtk.main()


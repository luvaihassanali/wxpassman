prefix = /usr/local
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
INSTALL = /usr/bin/install -c
EXEEXT = 
WINDRES = 
NM = nm
BK_DEPS = /Users/luv/wxWidgets-3.1.4/build-cocoa-release/bk-deps
srcdir = ../src
cryptoppsrcdir = ../cryptopp
top_srcdir = /Users/luv/wxWidgets-3.1.4/
LIBS = -framework Security -lpthread -liconv
LDFLAGS_GUI = 
CXX = g++ -std=gnu++11 -mmacosx-version-min=10.10
CXXFLAGS = 
CPPFLAGS = 
LDFLAGS = 
USE_DPI_AWARE_MANIFEST = 2
WX_LIB_FLAVOUR = 
TOOLKIT = OSX_COCOA
TOOLKIT_LOWERCASE = osx_cocoa
TOOLKIT_VERSION = 
TOOLCHAIN_FULLNAME = osx_cocoa-unicode-3.1
EXTRALIBS =   -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL  -framework Security -lpthread -liconv -lcryptopp -L${cryptoppsrcdir} -lsqlite3
EXTRALIBS_XML =  -lexpat
EXTRALIBS_GUI = -framework WebKit
WX_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -I${wx_top_builddir}/lib/wx/include/osx_cocoa-unicode-3.1 -I${top_srcdir}/include -I${srcdir}/sqlite3-3.35.2 -I${cryptoppsrcdir} 
WX_CXXFLAGS = -Wall -Wundef -Wunused-parameter -Wno-ctor-dtor-privacy -Woverloaded-virtual -Wno-deprecated-declarations -O2 -fno-common  -fvisibility=hidden -fvisibility-inlines-hidden
WX_LDFLAGS =  -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL 
HOST_SUFFIX = 
SAMPLES_RPATH_FLAG = 
SAMPLES_CXXFLAGS = 
wx_top_builddir = /Users/luv/wxWidgets-3.1.4/build-cocoa-release

### Variables: ###

DESTDIR = 
WX_RELEASE = 3.1
WX_VERSION = $(WX_RELEASE).4
LIBDIRNAME = $(wx_top_builddir)/lib
wxpassman_CXXFLAGS = $(WX_CPPFLAGS) -D__WX$(TOOLKIT)__ $(__WXUNIV_DEFINE_p) \
	$(__DEBUG_DEFINE_p) $(__EXCEPTIONS_DEFINE_p) $(__RTTI_DEFINE_p) \
	$(__THREAD_DEFINE_p) -I$(srcdir) $(__DLLFLAG_p) -I$(srcdir) \
	$(WX_CXXFLAGS) $(SAMPLES_CXXFLAGS) $(CPPFLAGS) $(CXXFLAGS)
wxpassman_OBJECTS =  \
	$(__wxpassman___win32rc) \
	wxpassman_wxpassman.o

### Conditionally set variables: ###

#CXXC = $(CXX)
CXXC = $(BK_DEPS) $(CXX)
#PORTNAME = base
PORTNAME = $(TOOLKIT_LOWERCASE)$(TOOLKIT_VERSION)
#WXBASEPORT = _carbon
#WXDEBUGFLAG = d
WXUNICODEFLAG = u
#WXUNIVNAME = univ
EXTRALIBS_FOR_BASE = $(EXTRALIBS)
#EXTRALIBS_FOR_BASE = $(EXTRALIBS) \
#	$(EXTRALIBS_XML) $(EXTRALIBS_GUI)
EXTRALIBS_FOR_GUI = $(EXTRALIBS_GUI)
#EXTRALIBS_FOR_GUI = 
#__WIN32_DPI_MANIFEST_p = \
#	--define \
#	wxUSE_DPI_AWARE_MANIFEST=$(USE_DPI_AWARE_MANIFEST)
#__WXUNIV_DEFINE_p = -D__WXUNIVERSAL__
#__WXUNIV_DEFINE_p_1 = --define __WXUNIVERSAL__
#__DEBUG_DEFINE_p = -DwxDEBUG_LEVEL=0
#__DEBUG_DEFINE_p_1 = --define wxDEBUG_LEVEL=0
#__EXCEPTIONS_DEFINE_p = -DwxNO_EXCEPTIONS
#__EXCEPTIONS_DEFINE_p_1 = --define wxNO_EXCEPTIONS
#__RTTI_DEFINE_p = -DwxNO_RTTI
#__RTTI_DEFINE_p_1 = --define wxNO_RTTI
#__THREAD_DEFINE_p = -DwxNO_THREADS
#__THREAD_DEFINE_p_1 = --define wxNO_THREADS
__DLLFLAG_p = -DWXUSINGDLL
__DLLFLAG_p_1 = --define WXUSINGDLL
COND_PLATFORM_OS2_1___wxpassman___os2_emxbindcmd = $(NM) wxpassman$(EXEEXT) | if \
	grep -q pmwin.763 ; then emxbind -ep wxpassman$(EXEEXT) ; fi
#__wxpassman___os2_emxbindcmd = $(COND_PLATFORM_OS2_1___wxpassman___os2_emxbindcmd)
#__RCDEFDIR_p = --include-dir \
#	$(LIBDIRNAME)/wx/include/$(TOOLCHAIN_FULLNAME)
#__wxpassman___win32rc = wxpassman_sample_rc.o
__wxpassman_app_Contents_PkgInfo___depname \
	= wxpassman.app/Contents/PkgInfo
__wxpassman_bundle___depname = wxpassman_bundle
#____wxpassman_BUNDLE_TGT_REF_DEP = \
#	$(__wxpassman_app_Contents_PkgInfo___depname)
#____wxpassman_BUNDLE_TGT_REF_DEP \
#	= $(__wxpassman_app_Contents_PkgInfo___depname)
____wxpassman_BUNDLE_TGT_REF_DEP \
	= $(__wxpassman_app_Contents_PkgInfo___depname)
#____wxpassman_BUNDLE_TGT_REF_DEP \
#	= $(__wxpassman_app_Contents_PkgInfo___depname)
#____wxpassman_BUNDLE_TGT_REF_DEP = \
#	$(__wxpassman_app_Contents_PkgInfo___depname)
COND_MONOLITHIC_0___WXLIB_CORE_p = \
	-lwx_$(PORTNAME)$(WXUNIVNAME)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)_core-$(WX_RELEASE)$(HOST_SUFFIX)
__WXLIB_CORE_p = $(COND_MONOLITHIC_0___WXLIB_CORE_p)
COND_MONOLITHIC_0___WXLIB_BASE_p = \
	-lwx_base$(WXBASEPORT)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__WXLIB_BASE_p = $(COND_MONOLITHIC_0___WXLIB_BASE_p)
COND_MONOLITHIC_1___WXLIB_MONO_p = \
	-lwx_$(PORTNAME)$(WXUNIVNAME)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
#__WXLIB_MONO_p = $(COND_MONOLITHIC_1___WXLIB_MONO_p)
#__LIB_SCINTILLA_IF_MONO_p \
#	= \
#	-lwxscintilla$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_TIFF_p \
	= \
	-lwxtiff$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_JPEG_p \
	= \
	-lwxjpeg$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_PNG_p \
	= \
	-lwxpng$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_ZLIB_p = \
	-lwxzlib$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
COND_wxUSE_REGEX_builtin___LIB_REGEX_p = \
	-lwxregex$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_REGEX_p = $(COND_wxUSE_REGEX_builtin___LIB_REGEX_p)
#__LIB_EXPAT_p = \
#	-lwxexpat$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)

### Targets: ###

all: wxpassman$(EXEEXT) $(__wxpassman_bundle___depname)

install: 

uninstall: 

install-strip: install

clean: 
	rm -rf ./.deps ./.pch
	rm -f ./*.o
	rm -f wxpassman$(EXEEXT)
	rm -rf wxpassman.app

distclean: clean
	rm -f config.cache config.log config.status bk-deps bk-make-pch shared-ld-sh Makefile

wxpassman$(EXEEXT): $(wxpassman_OBJECTS) $(__wxpassman___win32rc)
	$(CXX) -o $@ $(wxpassman_OBJECTS)     -L$(LIBDIRNAME) $(LDFLAGS_GUI) $(SAMPLES_RPATH_FLAG) $(LDFLAGS)  $(WX_LDFLAGS) $(__WXLIB_CORE_p)  $(__WXLIB_BASE_p)  $(__WXLIB_MONO_p) $(__LIB_SCINTILLA_IF_MONO_p) $(__LIB_TIFF_p) $(__LIB_JPEG_p) $(__LIB_PNG_p)  $(EXTRALIBS_FOR_GUI) $(__LIB_ZLIB_p) $(__LIB_REGEX_p) $(__LIB_EXPAT_p) $(EXTRALIBS_FOR_BASE) $(LIBS)
	$(__wxpassman___os2_emxbindcmd)

wxpassman.app/Contents/PkgInfo: wxpassman$(EXEEXT) $(top_srcdir)/src/osx/carbon/Info.plist.in $(srcdir)/wxmac.icns
	mkdir -p wxpassman.app/Contents
	mkdir -p wxpassman.app/Contents/MacOS
	mkdir -p wxpassman.app/Contents/Resources
	
	
	sed -e "s/IDENTIFIER/`echo $(srcdir) | sed -e 's,\.\./,,g' | sed -e 's,/,.,g'`/" \
	-e "s/EXECUTABLE/wxpassman/" \
	-e "s/VERSION/$(WX_VERSION)/" \
	$(top_srcdir)/src/osx/carbon/Info.plist.in >wxpassman.app/Contents/Info.plist
	
	
	/bin/echo "APPL????" >wxpassman.app/Contents/PkgInfo
	
	
	ln -f wxpassman$(EXEEXT) wxpassman.app/Contents/MacOS/wxpassman
	
	
	cp -f $(srcdir)/wxmac.icns wxpassman.app/Contents/Resources/wxmac.icns

wxpassman_bundle: $(____wxpassman_BUNDLE_TGT_REF_DEP)

wxpassman_wxpassman.o: $(srcdir)/wxpassman.cpp
	$(CXXC) -c -o $@ $(wxpassman_CXXFLAGS) $(srcdir)/wxpassman.cpp


# Include dependency info, if present:
-include ./.deps/*.d

.PHONY: all install uninstall clean distclean wxpassman_bundle

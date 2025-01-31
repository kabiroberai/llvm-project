DYLIB_ONLY := YES
DYLIB_NAME := $(BASENAME)
DYLIB_SWIFT_SOURCES := $(DYLIB_NAME).swift
DISABLE_SWIFT_INTERFACE := YES
SWIFTFLAGS_EXTRAS = \
            -Xcc -I$(BOTDIR)/Foo -emit-objc-header-path Foo.h \
	    -Xcc -iquote -Xcc ./buildbot/iquote-path \
	    -Xcc -I -Xcc ./buildbot/I-double \
	    -Xcc -I./buildbot/I-single \
	    -Xcc -F./buildbot/Frameworks \
	    -Xcc -F -Xcc buildbot/Frameworks \
	    -Xcc -iquote -Xcc /nonexisting-rootdir \
	    -import-objc-header $(BOTDIR)/Foo/bridge.h \
	    -disable-bridging-pch \
	    -Xcc -ivfsoverlay -Xcc buildbot/Foo/overlay.yaml

include Makefile.rules

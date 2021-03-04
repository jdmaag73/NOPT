app = champsim

srcExt = cc
srcDir = src branch replacement prefetcher
objDir = obj
binDir = bin
inc = inc

debug = 0

ifeq ($(debug),1)
	CFlags = -Wall -O0 -std=c++11
else
	CFlags = -Wall -O3 -std=c++11
endif
LDFlags =
libs =
libDir =


#************************ DO NOT EDIT BELOW THIS LINE! ************************

ifeq ($(debug),1)
	debug=-gdwarf-2
else
	debug=
endif
defines := $(addprefix -D,${DEFINES})
inc := $(addprefix -I,$(inc))
libs := $(addprefix -l,$(libs))
libDir := $(addprefix -L,$(libDir))
CFlags += -c $(debug) $(inc) $(libDir) $(libs)
sources := $(shell find $(srcDir) -name '*.$(srcExt)')
srcDirs := $(shell find . -name '*.$(srcExt)' -exec dirname {} \; | uniq)
objects := $(patsubst %.$(srcExt),$(objDir)/%.o,$(sources))

ifeq ($(srcExt),cc)
	CC = $(CXX)
else
	CFlags += -std=gnu99
endif

.phony: all clean distclean lib


all: $(binDir)/$(app)

$(binDir)/$(app): buildrepo $(objects)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CC) $(objects) $(LDFlags) -o $@

$(objDir)/%.o: %.$(srcExt)
	@echo "Generating dependencies for $<..."
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CC) $(CFlags) $(defines) $< -o $@

lib: $(binDir)/$(app).a

$(binDir)/$(app).a: buildrepo $(objects)
	@mkdir -p `dirname $@`
	@echo "Linkando libreria $@..."
	$(AR) r $@ $(objects)
	
clean:
	$(RM) -r $(objDir)

distclean: clean
	$(RM) -r $(binDir)/$(app)

buildrepo:
	@$(call make-repo)

define make-repo
   for dir in $(srcDirs); \
   do \
	mkdir -p $(objDir)/$$dir; \
   done
endef


# usage: $(call make-depend,source-file,object-file,depend-file)
define make-depend
  $(CC) -MM       \
        -MF $3    \
        -MP       \
        -MT $2    \
        $(CFlags) \
        $1
endef

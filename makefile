PROJECT = query
CC = gcc
AS = as
KERNEL_NAME = $(shell uname -s)
LIB_LOC_DIR = /usr/local/lib

OSX_DEPLOY_VERS = 10.5
SDK = /Developer/SDKs/MacOSX$(OSX_DEPLOY_VERS).sdk

ifeq ($(DEBUG),1)

MDEBUG_FLAGS = -g
DEBUG_FLAGS = -g3 -O0

ifeq ($(KERNEL_NAME),Darwin)
DEBUG_CMD = dsymutil --out="$(DLL_FINALE_NAME).dSYM" $@
DEBUG_CMD_EXE = dsymutil --out="$(EXE_DST)/$@.dSYM" $@
endif

else

MDEBUG_FLAGS = -O
DEBUG_FLAGS = 
DEBUG_CMD = 

endif

define INSTALL
	$(INSTALL_CMD) $1 $2
endef

Darwin_DLL_FLAGS = -dynamiclib
Darwin_LN_FLAGS = -isysroot $(SDK) -Wl,-syslibroot,$(SDK) -mmacosx-version-min=$(OSX_DEPLOY_VERS) -macosx_version_min=$(OSX_DEPLOY_VERS) -F/Library/Frameworks -framework Lasso9
Darwin_DLL_EXT = .dylib
Linux_DLL_FLAGS = -shared
Linux_LN_FLAGS = -llasso9_runtime
Linux_DLL_EXT = .so

DLL_EXT = $($(KERNEL_NAME)_DLL_EXT)
LINK_DST = $(LASSO9_HOME)/LassoLibraries
LASSOAPP_DST = $(LASSO9_HOME)/LassoApps
EXE_DST = $(LASSO9_HOME)

LASSO9_MODULE_CMD = /usr/bin/lassoc
INSTALL_CMD = cp -f 

C_FLAGS = $(DEBUG_FLAGS) -fPIC
LN_FLAGS = $(DEBUG_FLAGS) $($(KERNEL_NAME)_LN_FLAGS)
DLL_FLAGS = $($(KERNEL_NAME)_DLL_FLAGS) 

#CC_ERR = #> /dev/null 2>&1

BC_EXT = .bc
LASSOAPP_EXT = .lassoapp
INTERMEDIATE_EXT = o

MODULE_DLL_FLAGS = $(MDEBUG_FLAGS) -dll -n -obj
MODULE_LASSOAPP_FLAGS = $(MDEBUG_FLAGS) -dll -n -obj -lassoapp
MODULE_APP_FLAGS = $(MDEBUG_FLAGS) -app -n -obj

DLL_FINALE_NAME = $(LINK_DST)/$(basename $@)$(DLL_EXT)
BC_FINALE_NAME = $(LINK_DST)/$(basename $@)$(BC_EXT)
LASSOAPP_FINALE_NAME = $(LASSOAPP_DST)/$(basename $@)$(LASSOAPP_EXT)
EXE_FINAL_NAME = $(EXE_DST)/$(basename $@)

COMPAT_DEPS_LIST = compat/compat.bytes compat/compat.compare \
					compat/compat.database compat/compat.date \
					compat/compat.define compat/compat.encoding \
					compat/compat.error compat/compat.file \
					compat/compat.global compat/compat.map \
					compat/compat.match compat/compat.math \
					compat/compat.misc compat/compat.null \
					compat/compat.priorityqueue compat/compat.sessions \
					compat/compat.string compat/compat.var \
					compat/compat.valid compat/compat.cache

STD_LIST = lasso9-core support_paths locale \
				file dir net regexp sqlite curl database \
				queue date duration list random ljapi pdf \
				inline zip set xml serialization bom json \
				encrypt sys_process worker_pool dbgp_server $(COMPAT_DEPS_LIST)
DLL_LIST = $(addsuffix $(DLL_EXT), $(STD_LIST) $(COMPAT_DEPS_LIST))
BC_LIST = $(addsuffix $(BC_EXT), $(STD_LIST) $(COMPAT_DEPS_LIST))

#all: base lasso9
all: $(PROJECT)$(DLL_EXT)

debug:
	$(MAKE) DEBUG=1 -j2

#base: $(DLL_LIST)
	
#bc: $(BC_LIST)
	

makefile: ;

%.d.$(INTERMEDIATE_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" $(MODULE_DLL_FLAGS) -o $@ $<
%.ap.$(INTERMEDIATE_EXT): %
	"$(LASSO9_MODULE_CMD)" $(MODULE_LASSOAPP_FLAGS) -o $@ $<
%.a.$(INTERMEDIATE_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" $(MODULE_APP_FLAGS) -o $@ $<
%.d.i386.$(INTERMEDIATE_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" $(MODULE_DLL_FLAGS) -arch i386 -o $@ $<
%.ap.i386.$(INTERMEDIATE_EXT): %
	"$(LASSO9_MODULE_CMD)" $(MODULE_LASSOAPP_FLAGS) -arch i386 -o $@ $<
%.a.i386.$(INTERMEDIATE_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" $(MODULE_APP_FLAGS) -arch i386 -o $@ $<
%.d.x86_64.$(INTERMEDIATE_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" $(MODULE_DLL_FLAGS) -arch x86_64 -o $@ $<
%.ap.x86_64.$(INTERMEDIATE_EXT): %
	"$(LASSO9_MODULE_CMD)" $(MODULE_LASSOAPP_FLAGS) -arch x86_64 -o $@ $<
%.a.x86_64.$(INTERMEDIATE_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" $(MODULE_APP_FLAGS) -arch x86_64 -o $@ $<

%$(BC_EXT): %.inc
	"$(LASSO9_MODULE_CMD)" -dll -o $@ $<
	@mkdir -p "$(LINK_DST)"
	@mkdir -p "$(LINK_DST)/compat"
	@$(INSTALL_CMD) "`pwd`/$@" "$(BC_FINALE_NAME)"

%$(DLL_EXT): %.d.o
	$(CC) $(ARCH_PART) $(DLL_FLAGS) -o $@ $< $(LN_FLAGS) $(CC_ERR)
	@mkdir -p "$(LINK_DST)"
	@mkdir -p "$(LINK_DST)/compat"
	$(DEBUG_CMD)
	@$(call INSTALL, $@, "$(DLL_FINALE_NAME)")

%$(LASSOAPP_EXT): %.ap.o
	$(CC) $(ARCH_PART) $(DLL_FLAGS) -o $@ $< $(LN_FLAGS) $(CC_ERR)
	@mkdir -p "$(LASSOAPP_DST)"
	$(DEBUG_CMD)
	@$(call INSTALL, $@, "$(LASSOAPP_FINALE_NAME)")

%: %.a.o
	$(CC) $(ARCH_PART) -o $@ $@.a.o $(LN_FLAGS) $(CC_ERR)
	$(DEBUG_CMD_EXE)
	@$(call INSTALL, $@, "$(EXE_FINAL_NAME)")

install:
	-mv -f "$(PROJECT)$(DLL_EXT)" "$(DLL_FINALE_NAME)"
	
clean:
	@rm *.$(INTERMEDIATE_EXT) *$(DLL_EXT) *$(BC_EXT) *$(LASSOAPP_EXT) *.o 2>/dev/null || :
	@rm -rf *.dSYM 2>/dev/null  || :
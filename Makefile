##
## EPITECH PROJECT, 2020
## Makefile
## File description:
## Makefile for c++20
##

# DEBUG_MAKEFILE	:=	true

BINDIR			:=	binaries
SRCDIR			:=	sources
INCDIR			:=	includes
TSTDIR			:=	tests
LIBDIR			:=	libraries
EXTERNDIR		:=	externs
EXTERNBINDIR	:=	$(BINDIR)/$(EXTERNDIR)

BUILDDIR		:=	.build
OBJDIR			:=	objects
DEPDIR			:=	dependencies
OBJEXTERNDIR	:=	$(EXTERNDIR)/$(OBJDIR)
DEPEXTERNDIR	:=	$(EXTERNDIR)/$(DEPDIR)

ARGV1			:=	127.0.0.1 8080
ARGV2			:=	8080
NAME1			:=	$(BINDIR)/client
NAME2			:=	$(BINDIR)/server
TNAME			:=	$(BINDIR)/unit

C_SRCEXT		:=	.c
CPP_SRCEXT		:=	.cpp
CPPM_SRCEXT		:=	.cppm

C_HDREXT		:=	.h
CPP_HDREXT		:=	.hpp

OBJEXT			:=	.o
DEPEXT			:=	.d
PCMEXT			:=	.pcm
PCHEXT			:=	.gch

## wflags
COMMON_WFLAGS	:=	pedantic all extra missing-field-initializers
C_WFLAGS		:=	
CPP_WFLAGS		:=	no-volatile no-address no-nonnull-compare no-unused-variable no-unused-parameter no-class-memaccess no-pointer-arith
CPPM_WFLAGS		:=

## flags
COMMON_FLAGS	:=	-fmax-errors=10
C_FLAGS			:=	
CPP_FLAGS		:=	-std=c++20
CPPM_FLAGS		:=	-std=c++20
PCH_FLAGS		:=	-std=c++20

## -I
COMMON_CPPFLAGS	:=	$(INCDIR) $(SRCDIR) $(EXTERNDIR) $(EXTERNDIR)/HdrOnly $(LIBDIR)
C_CPPFLAGS		:=
CXX_CPPFLAGS	:=

## -L
LIBLOCATION		:=

## -l
LIBBIN			:=	boost_system boost_thread pthread sodium


# ============================================================================= Mode debug
ifeq (debug,$(findstring debug,$(MAKECMDGOALS)))

MODE_EXT		:=	_debug
MODE_FLAGS		:=	-g3 -Og -pipe -DDEBUG=2
MOD_BUILDDIR	:=	$(addsuffix /debug,$(BUILDDIR))


# ============================================================================= Mode valgrind
else ifeq (auto_valgrind,$(findstring auto_valgrind,$(MAKECMDGOALS)))

MODE_EXT		:=	_valgrind
MODE_FLAGS		:=	-g3 -Og -pipe -DDEBUG=1
MOD_BUILDDIR	:=	$(addsuffix /valgrind,$(BUILDDIR))


# ============================================================================= Mode gdb
else ifeq (auto_gdb,$(findstring auto_gdb,$(MAKECMDGOALS)))

MODE_EXT		:=	_gdb
MODE_FLAGS		:=	-g3 -Og -pipe
MOD_BUILDDIR	:=	$(addsuffix /gdb,$(BUILDDIR))


# ============================================================================= Mode test
else ifeq (auto_tests,$(findstring auto_tests,$(MAKECMDGOALS)))

MODE_EXT		:=	_tests
MODE_FLAGS		:=	-g3 -Og -pipe -DTEST=1
MOD_BUILDDIR	:=	$(addsuffix /tests,$(BUILDDIR))
NAME			:=	$(TNAME)



# ============================================================================= Mode release
else

MODE_EXT		:=
MODE_FLAGS		:=	-Ofast -pipe
MOD_BUILDDIR	:=	$(addsuffix /release,$(BUILDDIR))

endif

## ============================================================================

## DIR
OBJDIR			:=	$(MOD_BUILDDIR)/$(OBJDIR)
DEPDIR			:=	$(MOD_BUILDDIR)/$(DEPDIR)
OBJEXTERNDIR	:=	$(BUILDDIR)/$(OBJEXTERNDIR)
DEPEXTERNDIR	:=	$(BUILDDIR)/$(DEPEXTERNDIR)

CC				:=	gcc
CXX				:=	g++

MAIN1			:=	$(SRCDIR)/Client/main$(CPP_SRCEXT)
MAIN2			:=	$(SRCDIR)/Server/main$(CPP_SRCEXT)

# find
C_SRC1			!=	find $(SRCDIR) -type f -name \*$(C_SRCEXT) ! -path \*/Server/\*
C_SRC2			!=	find $(SRCDIR) -type f -name \*$(C_SRCEXT) ! -path \*/Client/\*
CPP_SRC1		!=	find $(SRCDIR) -type f -name \*$(CPP_SRCEXT) ! -path $(MAIN1) ! -path \*/Server/\*
CPP_SRC2		!=	find $(SRCDIR) -type f -name \*$(CPP_SRCEXT) ! -path $(MAIN2) ! -path \*/Client/\*
# CPPM_SRC		!=	find $(SRCDIR) -type f -name \*$(CPPM_SRCEXT) ! -path $(MAIN)

TST_C_SRC		!=	find $(TSTDIR) -type f -name \*$(C_SRCEXT)
TST_CPP_SRC		!=	find $(TSTDIR) -type f -name \*$(CPP_SRCEXT)
# TEST_CPPM_SRC		!=	find $(TSTDIR) -type f -name \*$(CPPM_SRCEXT)

FOUNDLIBS		!=	find $(LIBDIR) -maxdepth 1 -type d ! -path $(LIBDIR)
FOUNDEXTERN		!=	find $(EXTERNDIR) -maxdepth 1 -type d ! -path $(EXTERNDIR) ! -path $(EXTERNDIR)/HdrOnly

CPP_PCH			:=	$(INCDIR)/pch$(CPP_HDREXT)

# .x=.o

ifeq (auto_tests,$(findstring auto_tests,$(MAKECMDGOALS)))
C_OBJ1			:=	$(patsubst %$(C_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(C_SRC1) $(TST_C_SRC))
C_OBJ2			:=	$(patsubst %$(C_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(C_SRC2) $(TST_C_SRC))
CPP_OBJ1		:=	$(patsubst %$(CPP_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(CPP_SRC1) $(TST_CPP_SRC))
CPP_OBJ2		:=	$(patsubst %$(CPP_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(CPP_SRC2) $(TST_CPP_SRC))
CPPM_OBJ		+=	$(patsubst %$(CPPM_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(CPPM_SRC) $(TST_CPPM_SRC))
else
C_OBJ1			:=	$(patsubst %$(C_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(C_SRC1))
C_OBJ2			:=	$(patsubst %$(C_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(C_SRC2))
CPP_OBJ1		:=	$(patsubst %$(CPP_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(CPP_SRC1) $(MAIN1))
CPP_OBJ2		:=	$(patsubst %$(CPP_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(CPP_SRC2) $(MAIN2))
CPPM_OBJ		+=	$(patsubst %$(CPPM_SRCEXT),$(OBJDIR)/%$(OBJEXT),$(CPPM_SRC))
endif

# .x=.x.gch
C_PCH_OBJ		+=	$(patsubst %,%$(PCHEXT),$(C_HDR))
CPP_PCH_OBJ		+=	$(patsubst %,%$(PCHEXT),$(CPP_PCH))

## c flags
CFLAGS			+=	$(foreach flag, $(COMMON_WFLAGS),$(addprefix -W,$(flag)))
CFLAGS			+=	$(foreach flag, $(C_WFLAGS),$(addprefix -W,$(flag)))
CFLAGS			+=	$(C_FLAGS) $(COMMON_FLAGS)

## cpp flags
CXXFLAGS		+=	$(foreach flag, $(COMMON_WFLAGS),$(addprefix -W,$(flag)))
CXXFLAGS		+=	$(foreach flag, $(CPP_WFLAGS),$(addprefix -W,$(flag)))
CXXFLAGS		+=	$(CPP_FLAGS) $(COMMON_FLAGS)
# CXXFLAGS		+=	-fmodules-ts $(foreach objdir, $(dir $(CPPM_OBJ)),-fprebuilt-module-path=$(objdir))

## cppm flags
CXXMFLAGS		+=	$(foreach flag, $(COMMON_WFLAGS),$(addprefix -W,$(flag)))
CXXMFLAGS		+=	$(foreach flag, $(CPPM_WFLAGS),$(addprefix -W,$(flag)))
# CXXMFLAGS		+=	$(CPPM_FLAGS) -fmodules-ts --precompile $(COMMON_FLAGS)

## c.gch flags
C_PCHFLAGS		+=	-x c-header
C_PCHFLAGS		+=	$(foreach flag, $(COMMON_WFLAGS),$(addprefix -W,$(flag)))
C_PCHFLAGS		+=	$(foreach flag, $(CPP_WFLAGS),$(addprefix -W,$(flag)))

## cpp.gch flags
CPP_PCHFLAGS	+=	-x c++-header
CPP_PCHFLAGS	+=	$(foreach flag, $(COMMON_WFLAGS),$(addprefix -W,$(flag)))
CPP_PCHFLAGS	+=	$(foreach flag, $(CPP_WFLAGS),$(addprefix -W,$(flag)))
CPP_PCHFLAGS	+=	$(CPP_FLAGS)

## mode flags
CFLAGS			+=	$(MODE_FLAGS)
CXXFLAGS		+=	$(MODE_FLAGS)
CXXMFLAGS		+=	$(MODE_FLAGS)
C_PCHFLAGS		+=	$(MODE_FLAGS)
CPP_PCHFLAGS	+=	$(MODE_FLAGS)

## includes
CPPFLAGS		+=	$(foreach inc, $(COMMON_CPPFLAGS),$(addprefix -I,$(inc)))
CPPFLAGS		+=	$(foreach inc, $(C_CPPFLAGS),$(addprefix -I,$(inc)))
CPPFLAGS		+=	$(foreach inc, $(CXX_CPPFLAGS),$(addprefix -I,$(inc)))

## libraries
FOUNDLIBS		:=	$(patsubst $(LIBDIR)/%,%,$(FOUNDLIBS))
FOUNDLIBSBIN	:=	$(patsubst %,./$(BINDIR)/lib%.a,$(FOUNDLIBS))

## externs
FOUNDEXTERN		:=	$(patsubst $(EXTERNDIR)/%,%,$(FOUNDEXTERN))
FOUNDEXTERNBIN	:=	$(patsubst %,./$(EXTERNBINDIR)/lib%.a,$(FOUNDEXTERN))

## -L
LDFLAGS			:=	$(foreach lib, $(LIBLOCATION),$(addprefix -L,$(lib)))
LDFLAGS			+=	$(addprefix -L,$(BINDIR))
LDFLAGS			+=	$(addprefix -L,$(EXTERNBINDIR))

## -l
LDLIBS			:=	$(foreach lib, $(LIBBIN),$(addprefix -l,$(lib)))
LDLIBS			+=	$(foreach lib, $(FOUNDLIBS),$(addprefix -l,$(lib)$(MODE_EXT)))
LDLIBS			+=	$(foreach lib, $(FOUNDEXTERN),$(addprefix -l,$(lib)))

## BINDIR
BINDIR			:=	$(patsubst /%,,$(BINDIR))
BINDIR			:=	$(patsubst ~%,,$(BINDIR))
ifeq "$(BINDIR)" ""
$(error invalid bindir)
endif
NAME			:=	$(BINDIR)/$(NAME)

## ============================================================================

PRINTF				:=	@printf

NORMAL				:=	\e[0m
BLACK				:=	\e[0;30m
DARKGRAY			:=	\e[1;30m
RED					:=	\e[0;31m
LRED				:=	\e[1;31m
GREEN				:=	\e[0;32m
LGREEN				:=	\e[1;32m
BROWN				:=	\e[0;33m
YELLOW				:=	\e[1;33m
BLUE				:=	\e[0;34m
LBLUE				:=	\e[1;34m
PURPLE				:=	\e[0;35m
LPURPLE				:=	\e[1;35m
CYAN				:=	\e[0;36m
LCYAN				:=	\e[1;36m
LIGHT				:=	\e[0;37m
WHITE				:=	\e[1;37m

ifneq "$(DEBUG_MAKEFILE)" "true"
MAKEFLAGS		+=	--silent --no-print-directory
else
.NOTPARALLEL:
endif

## ============================================================================

all: linkage1 linkage2

precompilation : $(C_PCH_OBJ) $(CPP_PCH_OBJ)
	$(PRINTF) "$(LCYAN)[Precompilation]$(NORMAL) done\n"

libraries : $(FOUNDLIBSBIN)
	$(PRINTF) "$(LCYAN)[Libraries]$(NORMAL) done\n"

externs : $(FOUNDEXTERNBIN)
	$(PRINTF) "$(LCYAN)[Externs]$(NORMAL) done\n"

compilation1 : $(C_OBJ1) $(CPP_OBJ1)
	$(PRINTF) "$(LCYAN)[Compilation]$(NORMAL) done\n"

compilation2 : $(C_OBJ2) $(CPP_OBJ2)
	$(PRINTF) "$(LCYAN)[Compilation]$(NORMAL) done\n"

linkage1 : $(NAME1)$(MODE_EXT)
	$(PRINTF) "$(LCYAN)[Linkage]$(NORMAL) done\n"

linkage2 : $(NAME2)$(MODE_EXT)
	$(PRINTF) "$(LCYAN)[Linkage]$(NORMAL) done\n"

test_linkage : $(TEST_NAME)$(MODE_EXT)
	$(PRINTF) "$(LCYAN)[Linkage]$(NORMAL) done\n"

## ============================================================================

$(NAME1)$(MODE_EXT): compilation1 | libraries externs $(BINDIR)
	$(CXX) $(OUTPUT_OPTION) $(CPP_OBJ1) $(CPPM_OBJ) $(LDFLAGS) $(LDLIBS)

$(NAME2)$(MODE_EXT): compilation2 | libraries externs $(BINDIR)
	$(CXX) $(OUTPUT_OPTION) $(CPP_OBJ2) $(CPPM_OBJ) $(LDFLAGS) $(LDLIBS)

$(TEST_NAME)$(MODE_EXT): compilation1 compilation2 | libraries externs $(BINDIR)
	$(CXX) $(OUTPUT_OPTION) $(CPP_OBJ) $(CPPM_OBJ) $(LDFLAGS) $(LDLIBS)

force :;
# libX.a
./$(BINDIR)/lib%.a : force
	$(eval DIRNAME = $(patsubst $(BINDIR)/lib%.a,%,$(patsubst ./%,%,$@)))
ifneq "$(DEBUG_MAKEFILE)" "true"
	$(MAKE) -C $(LIBDIR)/$(DIRNAME) \
		BINDIR=../../$(BINDIR) \
		NAME=$(DIRNAME)$(MODE_EXT) \
		OBJDIR=../../$(OBJDIR)/$(LIBDIR)/$(DIRNAME) \
		DEPDIR=../../$(DEPDIR)/$(LIBDIR)/$(DIRNAME) \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		CXXMFLAGS="$(CXXMFLAGS)" \
		CPPFLAGS="-I.." \
		--silent --no-print-directory
else
	$(MAKE) -C $(LIBDIR)/$(DIRNAME) \
		BINDIR=../../$(BINDIR) \
		NAME=$(DIRNAME)$(MODE_EXT) \
		OBJDIR=../../$(OBJDIR)/$(LIBDIR)/$(DIRNAME) \
		DEPDIR=../../$(DEPDIR)/$(LIBDIR)/$(DIRNAME) \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		CXXMFLAGS="$(CXXMFLAGS)" \
		CPPFLAGS="-I.."
endif

# externs/libX.a
./$(EXTERNBINDIR)/lib%.a : force
	$(eval DIRNAME = $(patsubst $(EXTERNBINDIR)/lib%.a,%,$(patsubst ./%,%,$@)))
ifneq "$(DEBUG_MAKEFILE)" "true"
	$(MAKE) -C $(EXTERNDIR)/$(DIRNAME) \
		BINDIR=../../$(EXTERNBINDIR) \
		NAME=$(DIRNAME) \
		OBJDIR=../../$(OBJEXTERNDIR)/$(DIRNAME) \
		DEPDIR=../../$(DEPEXTERNDIR)/$(DIRNAME) \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		CXXMFLAGS="$(CXXMFLAGS)" \
		CPPFLAGS="-I.." \
		--silent --no-print-directory
else
	$(MAKE) -C $(EXTERNDIR)/$(DIRNAME) \
		BINDIR=../../$(EXTERNBINDIR) \
		NAME=$(DIRNAME) \
		OBJDIR=../../$(OBJEXTERNDIR)/$(DIRNAME) \
		DEPDIR=../../$(DEPEXTERNDIR)/$(DIRNAME) \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		CXXMFLAGS="$(CXXMFLAGS)" \
		CPPFLAGS="-I.."
endif

# .c
$(OBJDIR)/%$(OBJEXT): %$(C_SRCEXT) | $(C_PCH_OBJ)
	mkdir -p $(@D) $(patsubst $(OBJDIR)%,./$(DEPDIR)%,$(@D))
	$(CC) -c $(OUTPUT_OPTION) -c $(CPPFLAGS) $(CFLAGS) $< \
		-MT $@ -MMD -MP -MF $(patsubst $(OBJDIR)%$(OBJEXT),$(DEPDIR)/%$(DEPEXT),$(patsubst ./%,%,$@))
	$(PRINTF) "$(LCYAN)[Compilation]$(NORMAL) $<\n"

# .cpp
$(OBJDIR)/%$(OBJEXT): %$(CPP_SRCEXT) | $(CPPM_OBJ) precompilation
	mkdir -p $(@D) $(patsubst $(OBJDIR)%,./$(DEPDIR)%,$(@D))
	$(CXX) -c $(OUTPUT_OPTION) $(CPPFLAGS) $(CXXFLAGS) $< \
		-MT $@ -MMD -MP -MF $(patsubst $(OBJDIR)%$(OBJEXT),$(DEPDIR)/%$(DEPEXT),$(patsubst ./%,%,$@))
	$(PRINTF) "$(LCYAN)[Compilation]$(NORMAL) $<\n"

# .cppm
$(OBJDIR)/%$(OBJEXT): %$(CPPM_SRCEXT) | precompilation
	mkdir -p $(@D) $(patsubst $(OBJDIR)%,./$(DEPDIR)%,$(@D))
	$(CXX) -o $(patsubst %$(OBJEXT),%$(PCMEXT),$@) $(CPPFLAGS) $(CXXMFLAGS) $< \
		-MT $(patsubst %$(OBJEXT),%$(PCMEXT),$@) -MMD -MP -MF $(patsubst $(OBJDIR)%$(OBJEXT),$(DEPDIR)/%$(DEPEXT),$(patsubst ./%,%,$@))
	$(CXX) $(OUTPUT_OPTION) -c $(CXXFLAGS) $(patsubst %$(OBJEXT),%$(PCMEXT),$@)
	$(PRINTF) "$(LCYAN)[Compilation][Module]$(NORMAL) $<\n"

# .h

%$(C_HDREXT)$(PCHEXT): %$(C_HDREXT)
	$(CC) $(C_PCHFLAGS) $(OUTPUT_OPTION) $(CPPFLAGS) $<
	$(PRINTF) "$(LCYAN)[Precompilation]$(NORMAL) $<\n"
.PRECIOUS: %$(C_HDREXT)$(PCHEXT)


%$(CPP_HDREXT)$(PCHEXT): %$(CPP_HDREXT)
	$(CXX) $(CPP_PCHFLAGS) $(OUTPUT_OPTION) $(CPPFLAGS) $<
	$(PRINTF) "$(LCYAN)[Precompilation]$(NORMAL) $<\n"
.PRECIOUS: %$(CPP_HDREXT)$(PCHEXT)

$(BINDIR) :; mkdir -p $(BINDIR)

ifneq "$(wildcard $(DEPDIR))" ""
include $(shell find $(DEPDIR) -type f -name \*$(DEPEXT))
endif

## clean

re :
	$(MAKE) ffclean
	$(MAKE) auto

clean :
	rm -rf $(OBJDIR) $(DEPDIR)
	rm -f vgcore.*
	$(PRINTF) "$(DARKGRAY)[Clean]$(NORMAL) done\n"

fclean : clean
	rm -f $(NAME1) $(NAME1)_debug $(NAME1)_valgrind $(NAME1)_gdb $(NAME2) $(NAME2)_debug $(NAME2)_valgrind $(NAME2)_gdb
	$(foreach libbin, $(FOUNDLIBS),rm -f $(BINDIR)/lib$(libbin).a $(BINDIR)/lib$(libbin)_debug.a $(BINDIR)/lib$(libbin)_valgrind.a $(BINDIR)/lib$(libbin)_gdb.a)
	$(PRINTF) "$(DARKGRAY)[FClean]$(NORMAL) done\n"

ffclean : fclean
	find . -name \*.gch -delete
	$(PRINTF) "$(DARKGRAY)[FFClean]$(NORMAL) done\n"

fffclean : ffclean
	rm -rf $(BUILDDIR)
	rm -rf $(BINDIR)
	$(PRINTF) "$(DARKGRAY)[FFFClean]$(NORMAL) done\n"

## auto

auto: linkage1
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto $(ARGV1)\n"
	./$(NAME1)$(MODE_EXT) $(ARGV1)

auto2: linkage2
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto $(ARGV2)\n"
	./$(NAME2)$(MODE_EXT) $(ARGV2)

debug : linkage1
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) debug $(ARGV1)\n"
	./$(NAME1)$(MODE_EXT) $(ARGV2)

debug2 : linkage2
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) debug $(ARGV2)\n"
	./$(NAME2)$(MODE_EXT) $(ARGV2)

auto_valgrind : linkage1
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto_valgrind $(ARGV)\n"
	valgrind --track-origins=yes ./$(NAME1)$(MODE_EXT) $(ARGV1) && cat clientValgrind.log

auto_valgrind2 : linkage2
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto_valgrind $(ARGV)\n"
	valgrind --track-origins=yes ./$(NAME2)$(MODE_EXT) $(ARGV2) && cat serverValgrind.log

auto_gdb : linkage1
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto_gdb $(ARGV)\n"
	gdb --args ./$(NAME1)$(MODE_EXT) $(ARGV1)

auto_gdb2 : linkage2
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto_gdb $(ARGV)\n"
	gdb --args ./$(NAME2)$(MODE_EXT) $(ARGV2)

auto_tests : all
	$(PRINTF) "$(YELLOW)[Binary]$(NORMAL) auto_gdb $(ARGV)\n"
	./$(NAME)$(MODE_EXT)

## phony

.PHONY: all clean fclean re auto debug auto_valgrind auto_gdb auto_tests

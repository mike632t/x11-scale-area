#
# makefile
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Note - Shell commands must start with a tab character at the beginning
# of each line NOT spaces..!
#
#  30 Jul 23   0.1   - Initial version - MT
#   4 Aug 23         - Added backup files to tar archive - MT
#  11 Nov 24   0.2   - Project name derived from current folder - MT
#  24 Sep 25   0.3   - Added ability to include common files - MT
#  15 Oct 25   0.4   - Evaluate date and archive name before use (and allow
#                      branch to be optional) - MT
#  28 Dec 25   0.5   - Added a rule to prevent make from using the built‑in
#                      rule when linking object files since the latest make
#                      versions are stricter - MT
#                      
#

PROJECT	= $(strip $(notdir $(abspath $(CURDIR)/.)))

COMMON	=  x11-scale-area 
#SOURCE	=  $(wildcard *.c)  # Compile all source files
SOURCE	=  $(filter-out $(COMMON:=.c), $(wildcard *.c))  # Compile all source files other than common files
INCLUDE	=  $(wildcard *.h)  # Automatically get all include files
BACKUP	=  $(wildcard *.c.[0-9])
OBJECT	=  $(SOURCE:.c=.o)
PROGRAM	=  $(SOURCE:.c=)

FILES		=  $(SOURCE) $(COMMON:=.c) $(BACKUP) $(INCLUDE) makefile LICENSE $(wildcard *.png) # README.md .gitignore .gitattributes
LANG		=  LANG_$(shell (echo $$LANG | cut -f 1 -d '_'))
UNAME		=  $(shell uname)
BRANCH	=  $(shell git rev-parse --abbrev-ref HEAD > /dev/null 2>&1 && echo `git rev-parse --abbrev-ref HEAD 2>/dev/null` || true)
COMPILER	= `$(CC) -v 2>&1 | grep '$(CC) version' | sed -e 's/([^()]*)//g' | sed -e 's/[ \t]*$$//g'`

LIBS	=  -lm -lX11 -lpng -lrt
FLAGS	=  -fcommon -Wall -pedantic -std=gnu99
FLAGS	+= -Wno-comment -Wno-unused-function #-Wno-deprecated-declarations -Wno-builtin-macro-redefined
FLAGS	+= -D $(LANG)

_date	= `date +'%Y%m%d%H%M'`

ifeq ($(BRANCH),)
_archive= $(PROJECT)-$(_date).tar.gz
else
_archive= $(PROJECT)-$(BRANCH)-$(_date).tar.gz
endif

# SDL Specific settings
ifneq ($(shell pkg-config --exists sdl2 > /dev/null 2>&1 && echo "SDL2" || echo ""),)
SFLAGS	= -D__SDL__ `pkg-config --cflags sdl2`
SLIBS	= `pkg-config --libs sdl2`
endif

# Operating system specific settings
ifeq ($(UNAME), NetBSD) 	# NetBSD..
LIBS	+=  -lcompat
#FLAGS	+=  -I /usr/X11R7/include/ -L /usr/X11R7/lib/ -R /usr/X11R7/lib
endif

# Compiler specific settings
ifeq ($(CC), cc)
#LIBS	+=  
FLAGS	+=  -no-pie
endif

ifdef DEBUG
FLAGS	+=  -g
endif

make:common $(PROGRAM) $(OBJECT)  # Build common sources, then all programs and their object files

all:clean common $(PROGRAM) $(OBJECT)  # Build common sources, programs, and objects

common:  # Compile common sources
ifneq ($(COMMON),)
ifdef DEBUG
	@echo $(CC) $(FLAGS) -c $(COMMON:=.c) 
endif
	@$(CC) $(FLAGS) -c $(COMMON:=.c) 
endif

$(PROGRAM): %: %.o  # Prevents make from falling back to the built‑in rule

%.o: %.c  # Compile other  sources (won't include common - already compiled)
ifdef DEBUG
	@echo $(CC) $(FLAGS) $(SFLAGS) -c $<
endif
	@$(CC) $(FLAGS) -c $<

%: %.o  # Link and display executable file name to indecate progress
ifdef DEBUG
	@echo $(CC) $(FLAGS) $(COMMON:=.o) -o $@ $<  $(LIBS) $(SLIBS) 
endif
	@$(CC) $(FLAGS) $(COMMON:=.o) -o $@ $< $(LIBS) $(SLIBS) 
	@ls --color $@

list:
	@echo $(COMMON:=.c) 
	@echo $(SOURCE) 
	@echo $(OBJECT) 

clean:
ifdef DEBUG
	@rm -f $(COMMON:=.o) -v  2>&1 | sed 's/removed/rm/g' | sed 's/'\''//g'
	@rm -f $(OBJECT) -v  2>&1 | sed 's/removed/rm/g' | sed 's/'\''//g'
	@rm -f $(PROGRAM) -v 2>&1 | sed 's/removed/rm/g' | sed 's/'\''//g'
else
	@rm -f $(COMMON:=.o) # -v
	@rm -f $(OBJECT) # -v
	@rm -f $(PROGRAM) # -v
endif

backup: clean
ifdef DEBUG
		@tar -czvpf ..\/$(_archive) $(FILES);cd .. && ls --color $(_archive) 2>/dev/null || ls $(_archive) || true
else
		@tar -czpf ..\/$(_archive) $(FILES);cd .. && ls --color $(_archive) 2>/dev/null || ls $(_archive) || true
endif

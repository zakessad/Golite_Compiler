CC := gcc

ifdef NDEBUG
CFLAGS := -std=c99 -Wall -Wextra -Werror -O3 -march=native -mtune=native -DNDEBUG
else
CFLAGS := -std=c99 -Wall -Wextra -Werror -O0 -g -ggdb
endif

CFLAGS += -D_POSIX_C_SOURCE=200809L


LEX := flex
YACC := bison


EXE := golite

LIBS := -lfl

HDRS := preproc_codegen.h python_templates.h codegen.h typecheck.h symbol.h pretty.h weeder.h tree.h y.tab.h
SRCS := preproc_codegen.c codegen.c typecheck.c symbol.c pretty.c weeder.c tree.c y.tab.c lex.yy.c main.c
OBJS := $(SRCS:.c=.o)


$(EXE):  $(HDRS) $(OBJS) Makefile
	$(CC) $(CFLAGS) $(LIBS) -o $@ $(OBJS)

$(OBJS): $(HDRS) $(SRCS) Makefile

lex.yy.c: golite.l Makefile
	$(LEX)  -o $@ $<

y.tab.h:
y.tab.c:  golite.y Makefile
	$(YACC) -o $@ $< -d --verbose


.PHONY: clean
clean:
	$(RM) y.output y.tab.h y.tab.c lex.yy.c $(OBJS) $(EXE)

# Makefile to build sema
#

CXX = gcc
LIBS = -lpthread
CPP_FLAGS = -g -Wall -Wextra -pedantic
EXEC = sema
SRC = sema.c
EXEC_INSTALL_DIR = /usr/bin
DOC_INSTALL_DIR = /usr/share/doc/sema

OBJS = $(SRC:.c=.o)
DOC_OBJS = AUTHORS INSTALL LICENSE README

%.o: %.cc
	$(CXX)  $(CPP_FLAGS) -c $< -o $@

.PHONY: all
all: $(EXEC)


$(EXEC): $(OBJS)
	$(CXX)  -o $(EXEC) $(LIBS) $(OBJS)


install: all
	install $(EXEC) $(EXEC_INSTALL_DIR)
	install -d $(DOC_INSTALL_DIR)
	for f in $(DOC_OBJS); \
	do \
	 install $${f} $(DOC_INSTALL_DIR); \
	done

test: all
	$(EXEC) -c sema-test;\
	$(EXEC) -r sema-test;\
	$(EXEC) -w sema-test;\
	$(EXEC) -d sema-test;\
	if [ $$? -ne 0 ];\
		then echo "Unit test failed";\
	else \
		echo "All test(s) passed";\
	fi 	

.PHONY: clean
clean:
	rm -f $(OBJS) $(EXEC)


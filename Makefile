CC 				:=	g++
CARGS 			:=	-g -Wall

DEV_OBJ			:=	$(patsubst %.c, %.o, $(shell find dev.d -name "*.c"))
SERV_OBJ		:=	$(patsubst %.c, %.o, $(shell find serv.d -name "*.c"))
DEV_HEADERS		:=	$(shell find dev.d -name "*.h")
SERV_HEADERS	:=	$(shell find serv.d -name "*.h")
ALL_OBJ			:=	$(shell find ./ -name "*.o")
THIS			:=	./Makefile

DEVICE			:=	dev
SERVER			:=	serv

all: $(DEVICE) $(SERVER)

$(DEVICE): $(DEV_OBJ)
	@echo Linking $@
	@$(CC) $(DEV_OBJ) -o $@ $(CARGS)

$(SERVER): $(SERV_OBJ)
	@echo Linking $@
	@$(CC) $(SERV_OBJ) -o $@ $(CARGS)

dev.d/%.o: dev.d/%.c $(DEV_HEADERS) $(THIS)
	@echo Compiling $@
	@$(CC) -c $< -o $@ $(CARGS)

serv.d/%.o: serv.d/%.c $(SERV_HEADERS) $(THIS)
	@echo Compiling $@
	@$(CC) -c $< -o $@ $(CARGS)

.PHONY: clean test
clean:
	@echo Removing object files and executables
	@rm -f $(ALL_OBJ) $(SERVER) $(DEVICE)
test: $(DEVICE) $(SERVER)
	@echo Test session started
	@./test.sh
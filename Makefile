CC 				:=	gcc

MODE			:=	DEBUG

DEFINE_LIST		:=	$(MODE)
CARGS			:=	-Wall $(addprefix -D, $(DEFINE_LIST)) -std=c99

ifeq ($(MODE),DEBUG)
	CARGS		+=	-ggdb -Wextra
else ifeq ($(MODE),RELEASE)
	CARGS		+=	-O3
endif

DEV_CFILES		:= 	$(shell find dev.d -name "*.c")
SERV_CFILES		:=	$(shell find serv.d -name "*.c")
GLOBAL_CFILES	:=	$(shell find global.d -name "*.c")
DEV_OBJ			:=	$(patsubst %.c, %.o, $(DEV_CFILES))
SERV_OBJ		:=	$(patsubst %.c, %.o, $(SERV_CFILES))
GLOBAL_OBJ		:=	$(patsubst %.c, %.o, $(GLOBAL_CFILES))
DEV_HEADERS		:=	$(shell find dev.d -name "*.h")
SERV_HEADERS	:=	$(shell find serv.d -name "*.h")
GLOBAL_HEADERS	:=	$(shell find global.d -name "*.h")
ALL_OBJ			:=	$(shell find ./ -name "*.o")
THIS			:=	./Makefile

DEVICE			:=	dev
SERVER			:=	serv

all: $(DEVICE) $(SERVER)

$(DEVICE): $(DEV_OBJ) $(GLOBAL_OBJ)
	@echo Linking $@
	@$(CC) $(DEV_OBJ) $(GLOBAL_OBJ) -o $@ $(CARGS)

$(SERVER): $(SERV_OBJ) $(GLOBAL_OBJ)
	@echo Linking $@
	@$(CC) $(SERV_OBJ) $(GLOBAL_OBJ) -o $@ $(CARGS)

dev.d/%.o: dev.d/%.c $(DEV_HEADERS) $(GLOBAL_HEADERS) $(THIS)
	@echo Compiling $@
	@$(CC) -c $< -o $@ $(CARGS)

serv.d/%.o: serv.d/%.c $(SERV_HEADERS) $(GLOBAL_HEADERS) $(THIS)
	@echo Compiling $@
	@$(CC) -c $< -o $@ $(CARGS)

global.d/%.o: global.d/%.c $(GLOBAL_HEADERS) $(THIS)
	@echo Compiling $@
	@$(CC) -c $< -o $@ $(CARGS)

.PHONY: clean clean-data dry-run
clean:
	@echo Removing object files and executables
	@rm -f $(ALL_OBJ) $(SERVER) $(DEVICE)

clean-data:
	@echo Removing program execution related data
	@rm -frd Chat File Relay.lst Syncread.lst Auth.lst

dry-run: 
	@$(CC) $(DEV_CFILES) $(GLOBAL_CFILES) -o /dev/null $(CARGS)
	@$(CC) $(SERV_CFILES) $(GLOBAL_CFILES) -o /dev/null $(CARGS)
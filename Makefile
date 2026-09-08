all: build clean run
C_FLAGS =  -g -Wall -Wextra -std=c23 
HEADER_DIR = -I/$(HOME)/Bureau/static_web_server/lib/http_parser -I/$(HOME)/Bureau/static_web_server/lib/utility -I/$(HOME)/Bureau/static_web_server/lib/http_handler
main.o: src/main.c
	gcc -g -c  $(C_FLAGS) $(HEADER_DIR) src/main.c

http_parser.o: lib/http_parser/http_parser.c lib/http_parser/http_parser.h
	gcc -g -c $(C_FLAGS)  $(HEADER_DIR) lib/http_parser/http_parser.c

utils.o: lib/utility/utils.c lib/utility/utils.h
	gcc -g -c $(C_FLAGS) -I/home/sidy-sow/Bureau/static_web_server/lib/utility lib/utility/utils.c

http_handler.o: lib/http_handler/http_handler.c lib/http_handler/http_handler.h
	gcc -g -c $(C_FLAGS) $(HEADER_DIR) lib/http_handler/http_handler.c
	
build: main.o http_parser.o utils.o http_handler.o
	gcc -o servix main.o http_parser.o utils.o http_handler.o
clean:
	rm -rf main.o http_parser.o utils.o http_handler.o
run:
	./servix
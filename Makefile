CC = gcc 
CFLAGS = -Wall -Wextra -g
NC = -lncurses
TARGET = client server

normal: $(TARGET)

client: 
	$(CC) $(CFLAGS) client_template.c -o client $(NC)

server:
	$(CC) $(CFLAGS) server.c -o server $(NC)
	
clean:
	rm -rf $(TARGET)

restart: clean normal
	@tput setaf 2; 
	@echo "restarted successfully";
	@tput sgr0;

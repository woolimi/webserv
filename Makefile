NAME = webserv
CC = clang++

# FLAG = -Wall -Wextra -Werror
# FLAG = -g3 -fsanitize=address
FLAG =

SRC_NAME = ConfigParser.cpp \
	handle_get.cpp \
	handle_put.cpp \
	handle_trace.cpp \
	HTTP.cpp \
	main.cpp \
	mimetype.cpp \
	req_interpreter.cpp \
	res_cgi.cpp \
	res_generator.cpp \
	res_make.cpp \
	res_send.cpp \
	utils.cpp
SRC_PATH = ./srcs

SRC = $(addprefix $(SRC_PATH)/, $(SRC_NAME))

OBJ_NAME = $(SRC_NAME:.cpp=.o)
OBJ_PATH = ./obj
OBJ = $(addprefix $(OBJ_PATH)/, $(OBJ_NAME))

INC_LINK = -I./lib
LIBFT = -L./lib -lft

all: $(NAME)
	
$(NAME): $(OBJ) libft
	@$(CC) $(FLAG) $(INC_LINK) $(OBJ) $(LIBFT) -o $(NAME)

libft :
	@$(MAKE) -C ./lib all

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	@mkdir $(OBJ_PATH) 2> /dev/null || true
	$(CC) $(FLAG) $(INC_LINK) -o $@ -c $<

clean:
	@$(MAKE) -C ./lib clean
	@rm -rf ./obj

fclean: clean
	@$(MAKE) -C ./lib fclean
	@rm -rf ./obj $(NAME)

re: fclean all

.PHONY: all clean fclean re
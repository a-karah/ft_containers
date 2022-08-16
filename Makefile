NAME = ft_containers
STD_NAME = std_containers

# control codes
RESET	=	\033[0m
GREEN	=	\033[32m
YELLOW	=	\033[33m
BLUE	=	\033[34m
RED		=	\033[31m
UP		=	\033[A
CUT		=	\033[K

all: 
	@echo "$(YELLOW)Compiling [$(NAME)]..."
	@c++ -Wall -Werror -Wextra -std=c++98 *.cpp -o $(NAME)
	@c++ -Wall -Werror -Wextra -std=c++98 *.cpp -o $(STD_NAME)
	@printf "$(UP)$(CUT)"
	@echo "$(GREEN)Finished [$(NAME)]$(RESET)"

clean:
	rm -rf *.o

fclean: clean
	rm -rf $(NAME) $(STD_NAME)

re: fclean all

.PHONY: all clean fclean re

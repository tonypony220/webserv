NAME			=	webserv

OBJS_DIR		=	./objs

#CXX_HEADERS		=	Request.hpp Response.hpp Utils.hpp Server.hpp SocketTCP.hpp ParseConf.hpp HttpStatusCodes.hpp SmartPtr.hpp
CXX_HEADERS		=	.

CXX_FILES		=	poll.cpp Request.cpp Response.cpp Utils.cpp Server.cpp SocketTCP.cpp ParseConf.cpp

CXX_DIRS		=	$(dir $(CXX_FILES))
vpath %.cpp $(CXX_DIRS)

SRCS			=	$(CXX_FILES)
OBJS			=	$(addprefix $(OBJS_DIR)/, $(notdir $(CXX_FILES:.cpp=.o)))
DEPENDENCIES	=	$(OBJS:.o=.d)

CC				=	clang++
CFLAGS			=	-std=c++98 -O0 -g -fsanitize=address -Wall -Werror -Wextra -Wshadow -I$(CXX_HEADERS)

.PHONY : all clean fclean re bonus
.DEFAULT_GOAL := all

-include $(DEPENDENCIES)

$(OBJS_DIR)/%.o : %.cpp Makefile
	$(CC) $(CFLAGS) -c $< -o $@ -MD

$(OBJS_DIR) :
	@mkdir -p $(OBJS_DIR)

$(NAME) : $(SRCS) $(OBJS_DIR) $(OBJS) Makefile
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

all : $(NAME)

bonus : $(NAME)

clean :
	rm -rf $(OBJS) $(DEPENDENCIES)

fclean : clean
	rm -rf $(NAME) $(OBJS_DIR)

val : $(NAME)
	valgrind --leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=valgrind-out.txt \
		./$(NAME)

re: fclean all







#SRCS	=	poll.cpp Request.cpp Response.cpp Utils.cpp Server.cpp SocketTCP.cpp ParseConf.cpp
#HEAD	=		 	 Request.hpp Response.hpp Utils.hpp Server.hpp SocketTCP.hpp ParseConf.hpp HttpStatusCodes.hpp SmartPtr.hpp
#OBJS	=	${SRCS:.cpp=.o}
#CFLAGS	=   -std=c++98  -O0 -g -fsanitize=address -Wall -Wextra -Werror #-fsanitize=leak
#NAME	=	webserv
#CC		=  	clang++
#
## %.o: %.cpp $(HEAD)
## 	clang++ ${CFLAGS} -c $< -o ${<:.cpp=.o}
#
#${NAME}: ${OBJS} $(HEAD)
#	$(CC) ${CFLAGS} ${OBJS} -o ${NAME}
#
#all: ${NAME}
#
#clean:
#	-rm	-f ${OBJS}
#
#fclean: clean
#	-rm	-f ${NAME}
#
#re: fclean all
#
#.PHONY:	re all clean fclean $(NAME)
#

#SRCS	=	poll.cpp Request.cpp Response.cpp Utils.cpp Server.cpp SocketTCP.cpp ParseConf.cpp
#HEAD	=		 	 Request.hpp Response.hpp Utils.hpp Server.hpp SocketTCP.hpp ParseConf.hpp HttpStatusCodes.hpp SmartPtr.hpp
#OBJS	=	${SRCS:.cpp=.o}
#CFLAGS	=   -std=c++98  -O0 -g -fsanitize=address -Wall -Wextra -Werror #-fsanitize=leak
#NAME	=	webserv
#CC		=  	clang++
#
## %.o: %.cpp $(HEAD)
## 	clang++ ${CFLAGS} -c $< -o ${<:.cpp=.o}
#
#${NAME}: ${OBJS} $(HEAD)
#	$(CC) ${CFLAGS} ${OBJS} -o ${NAME}
#
#all: ${NAME}
#
#clean:
#	-rm	-f ${OBJS}
#
#fclean: clean
#	-rm	-f ${NAME}
#
#re: fclean all
#
#.PHONY:	re all clean fclean $(NAME)
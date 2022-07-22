SRCS	=	poll.cpp Request.cpp Response.cpp Utils.cpp Server.cpp SocketTCP.cpp ParseConf.cpp
HEAD	=		 	 Request.hpp Response.hpp Utils.hpp Server.hpp SocketTCP.hpp ParseConf.hpp HttpStatusCodes.hpp SmartPtr.hpp
OBJS	=	${SRCS:.cpp=.o}
CFLAGS	=   -std=c++98  -O0 -g -fsanitize=address -Wall -Wextra -Werror #-fsanitize=leak
NAME	=	webserv
CC		=  	clang++

# %.o: %.cpp $(HEAD)
# 	clang++ ${CFLAGS} -c $< -o ${<:.cpp=.o}

${NAME}: ${OBJS} $(HEAD)
	$(CC) ${CFLAGS} ${OBJS} -o ${NAME}

all: ${NAME}

clean:
	-rm	-f ${OBJS}
 
fclean: clean
	-rm	-f ${NAME}
		
re: fclean all

.PHONY:	re all clean fclean $(NAME) 



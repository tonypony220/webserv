SRCS	=	poll.cpp Request.cpp Response.cpp Utils.cpp Server.cpp SocketTCP.cpp ParseConf.cpp
HEAD	=		 	 Request.hpp Response.hpp Utils.hpp Server.hpp SocketTCP.hpp ParseConf.hpp
NAME	=	webserv
OBJS	=	${SRCS:.cpp=.o}
CFLAGS	=   -std=c++98 -O0 -g -fsanitize=address -Wall -Wextra -Werror #-fsanitize=leak

${NAME}: all

%.o: %.cpp $(HEAD)
	@g++ -I ${CFLAGS} -c $< -o ${<:.cpp=.o}

all: ${OBJS}
	g++ ${CFLAGS} ${OBJS} -o ${NAME}

clean:
	rm ${OBJS}
 
fclean: clean
		rm ${NAME}
		
re: fclean
	all

.PHONY:	re all clean fclean $(NAME) 



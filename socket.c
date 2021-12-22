#include <sys/socket.h>
#include <netinet/in.h>

void new_sock(unsigned int port) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = INADDR_ANY; // 0

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bind(sockfd, struct sockaddr_in *addr, int addrlen) && err("bind");

}

int main() {
	new_sock();
}

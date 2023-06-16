#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

#define N 128
#define PORT_NUM 8888

typedef struct sockaddr SA;

void fun_ls(int connfd)
{
	char buf[N];
	DIR *mydir;
	struct dirent *myitem;

	mydir = opendir(".");
	while ((myitem = readdir(mydir)) != NULL)
	{
		if ((strcmp(myitem->d_name, ".") == 0) || (strcmp(myitem->d_name, "..") == 0))
			continue;
		strcpy(buf, myitem->d_name);
		send(connfd, buf, N, 0);
	}
	printf("The current list of server has read by client.\n");
	close(connfd);

	return;
}

void fun_pwd(int connfd)
{
	char buf[N];

	getcwd(buf, sizeof(buf));
	send(connfd, buf, N, 0);
	printf("The current direction of server has read by client.\n");

	close(connfd);

	return;
}

void fun_cd(int connfd, char buf[])
{
	chdir(buf + 1);

	getcwd(buf + 1, sizeof(buf + 1));
	send(connfd, buf + 1, N, 0);
	printf("The current direction of server has changed by client.\n");

	close(connfd);
	return;
}
void fun_get(int connfd, char buf[])
{
	int fd, nbyte;

	if ((fd = open(buf + 1, O_RDONLY)) < 0)
	{
		fprintf(stderr, "fail to open %s: %s\n", buf + 1, strerror(errno));
		buf[0] = 'N';
		send(connfd, buf, N, 0);
		return;
	}
	buf[0] = 'Y';
	send(connfd, buf, N, 0);
	while ((nbyte = read(fd, buf, N)) > 0)
	{
		send(connfd, buf, nbyte, 0);
	}

	printf("Client has got a file from server.\n");
	close(connfd);

	return;
}

void fun_put(int connfd, char buf[])
{
	int fd, nbyte;

	if ((fd = open(buf + 1, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
	{
		printf("fail to create %s on server\n", buf + 1);
		return;
	}

	while ((nbyte = recv(connfd, buf, N, 0)) > 0)
	{
		write(fd, buf, nbyte);
	}

	printf("Server has got a file from client.\n");

	close(fd);

	return;
}

void fun_mkdir(int connfd, char buf[])
{
	if (mkdir(buf + 1, 00700) < 0)
	{
		printf("目录创建失败");
		goto ERROR;
	}
	getcwd(buf + 1, sizeof(buf + 1));
	printf("The current direction of server has changed by client.\n");
ERROR:
	close(connfd);
	return;
}
int fun_rmdir(const char *path)
{
	if (access(path, F_OK) == -1)
	{
		fprintf(stderr, "Directory %s does not exist\n", path);
		return -1;
	}
	if (rmdir(path) == -1)
	{
		fprintf(stderr, "Failed to delete directory %s: %s\n", path, strerror(errno));
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int listenfd, connfd;
	int optval = 1;
	char buf[N];
	char ip[N];
	int flag = 1;
	struct sockaddr_in server_addr;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "fail to socket: %s\n", strerror(errno));
		exit(-1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if (bind(listenfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		perror("fail to bind");
		exit(-1);
	}
	listen(listenfd, 5);
	printf("...Welcome to server terminal...\n");
	while (1)
	{

		if ((connfd = accept(listenfd, NULL, NULL)) < 0)
		{
			perror("fail to accept");
			break;
		}

		recv(connfd, buf, N, 0);
		switch (buf[0])
		{
		case 'L':
			fun_ls(connfd);
			break;
		case 'G':
			fun_get(connfd, buf);
			break;
		case 'P':
			fun_put(connfd, buf);
			break;
		case 'D':
			fun_pwd(connfd);
			break;
		case 'C':
			fun_cd(connfd, buf);
			break;
		case 'M':
			fun_mkdir(connfd, buf);
			break;
		case 'R':
			fun_rmdir(buf + 1);
			break;
		default:
			printf("Command error!\n");
		}

		close(connfd);
	}
	return 0;
}

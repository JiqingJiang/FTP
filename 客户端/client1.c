#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define N 128
#define PORT_NUM 21

typedef struct sockaddr SA;

void PrintHelp()
{
	printf("============================Command List============================\n");
	printf("     get : get <file> from server              					\n");
	printf("     put : put <file> to server                					\n");
	printf("     pwd : get current path of server \n");
	printf("     dir : get current file list of server\n");
	printf("     ls  : get current file list of client\n");
	printf("     cd  : change current path of server      					\n");
	printf("	mkdir: create a dir					\n");
	printf("      ?  : display list of commands\n");
	printf("     quit: quit the client\n");
	printf("===================================================================\n");
	printf("\n");

	return;
}

void fun_pwd(struct sockaddr_in server_addr)
{
	int sockfd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to list\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		close(sockfd);
	}

	strcpy(buf, "D");
	send(sockfd, buf, N, 0);

	while ((nbyte = recv(sockfd, buf, N, 0)) != 0)
	{
		printf("%s\n", buf);
	}

	close(sockfd);

	return;
}

void fun_dir(struct sockaddr_in server_addr)
{
	int sockfd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to show\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		close(sockfd);
		return;
	}

	strcpy(buf, "L");
	send(sockfd, buf, N, 0);

	while ((nbyte = recv(sockfd, buf, N, 0)) != 0)
	{
		printf("%s\n", buf);
	}

	close(sockfd);

	return;
}

void fun_ls(struct sockaddr_in server_addr)
{
	// int sockfd;
	// char buf[N];
	// sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// DIR *mydir;
	// struct dirent *myitem;

	// mydir = opendir(".");
	// while ((myitem = readdir(mydir)) != NULL)
	// {
	// 	if ((strcmp(myitem->d_name, ".") == 0) || (strcmp(myitem->d_name, "..") == 0))
	// 		continue;
	// 	strcpy(buf, myitem->d_name);
	// 	printf("%s\n", buf);
	// }
	int sockfd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to ls\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		close(sockfd);
	}

	strcpy(buf, "L");
	send(sockfd, buf, N, 0);

	while ((nbyte = recv(sockfd, buf, N, 0)) != 0)
	{
		printf("%s\n", buf);
	}

	close(sockfd);
}

void fun_cd(struct sockaddr_in server_addr, char command[])
{
	int sockfd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to change\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		goto ERROR_4;
	}

	sprintf(buf, "C%s", command + 3);
	send(sockfd, buf, N, 0);

	while ((nbyte = recv(sockfd, buf, N, 0)) != 0)
	{
		printf("Now the current path of server has changed to: %s\n", buf);
	}

ERROR_4:
	close(sockfd);

	return;
}

void fun_get(struct sockaddr_in server_addr, char command[])
{
	int sockfd, nbyte, fd;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to get\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		goto ERROR_2;
	}

	sprintf(buf, "G%s", command + 4);
	send(sockfd, buf, N, 0);
	recv(sockfd, buf, N, 0);

	if (buf[0] == 'N')
	{
		printf("No such file on server\n");
		goto ERROR_2;
	}

	if ((fd = open(command + 4, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
	{
		printf("fail to create local file %s\n", command + 4);
		goto ERROR_2;
	}

	while ((nbyte = recv(sockfd, buf, N, 0)) > 0)
	{
		write(fd, buf, nbyte);
	}
	printf("%s has got from server.\n", command + 4);
	close(fd);

ERROR_2:
	close(sockfd);

	return;
}

void fun_put(struct sockaddr_in server_addr, char command[])
{
	int sockfd, fd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to get\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		goto ERROR_3;
	}

	if ((fd = open(command + 4, O_RDONLY)) < 0)
	{
		printf("fail to open %s\n", command + 4);
		goto ERROR_3;
	}
	sprintf(buf, "P%s", command + 4);
	send(sockfd, buf, N, 0);

	while ((nbyte = read(fd, buf, N)) > 0)
	{
		send(sockfd, buf, nbyte, 0);
	}
	printf("%s has put to server.\n", command + 4);
	close(fd);

ERROR_3:
	close(sockfd);

	return;
}

void fun_mkdir(struct sockaddr_in server_addr, char command[])
{

	int sockfd, fd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to mkdir\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		goto ERROR_3;
	}

	sprintf(buf, "M%s", command + 6);
	send(sockfd, buf, strlen(buf), 0);
	close(fd);

ERROR_3:
	close(sockfd);

	return;
}
void fun_rmdir(struct sockaddr_in server_addr, char command[])
{
	int sockfd, fd, nbyte;
	char buf[N];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("fail to mkdir\n");
		return;
	}

	if (connect(sockfd, (SA *)(&server_addr), sizeof(server_addr)) < 0)
	{
		printf("fail to connect server\n");
		goto ERROR_3;
	}

	sprintf(buf, "R%s", command + 6);
	send(sockfd, buf, strlen(buf), 0);
	close(fd);

ERROR_3:
	close(sockfd);

	return;
}
int main(int argc, char *argv[])
{
	char NAME[N], PSWD[N];
	strcpy(NAME, "root");
	strcpy(PSWD, "1234");
	int sockfd, fd, nbyte;
	char command[32];
	struct sockaddr_in server_addr;
	int n = 0;
	char buf[N];
	char ip[N];

	if (argc < 3)
	{
		printf("Usage: %s <server_ip> <port>\n", argv[0]);
		exit(-1);
	}

	char username[N];
	char userpswd[N];
	printf("Please input user name:");
	scanf("%s", username);
	printf("Please input user password:");
	scanf("%s", userpswd);
	getchar();
	if ((strcmp(username, NAME) == 0) && (strcmp(userpswd, PSWD) == 0))
		printf("Login successfully!\n");
	else
		return 0;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "fail to socket: %s\n", strerror(errno));
		exit(-1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 0)
	{
		printf("invalid ip address\n!");
		return 1;
	}
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	// printf("%s connected successfully! \n",argv[1]);
	PrintHelp();
	while (1)
	{
		printf("ftp > ");
		fgets(command, 32, stdin);
		// scanf("%s",command);
		command[strlen(command) - 1] = '\0';
		if (strcmp(command, "?") == 0)
		{
			PrintHelp();
		}
		else if (strcmp(command, "dir") == 0)
		{
			fun_dir(server_addr);
		}
		else if (strcmp(command, "pwd") == 0)
		{
			fun_pwd(server_addr);
		}
		else if (strcmp(command, "ls") == 0)
		{
			fun_ls(server_addr);
		}
		else if (strncmp(command, "cd", 2) == 0)
		{
			fun_cd(server_addr, command);
		}
		else if (strncmp(command, "get", 3) == 0)
		{
			fun_get(server_addr, command);
		}
		else if (strncmp(command, "put", 3) == 0)
		{
			fun_put(server_addr, command);
		}
		else if (strncmp(command, "mkdir", 5) == 0)
		{
			fun_mkdir(server_addr, command);
		}
		else if (strncmp(command, "rmdir", 5) == 0)
		{
			fun_rmdir(server_addr, command);
		}
		else if (strcmp(command, "quit") == 0)
		{
			printf("Thanks for using , goodbye\n");
			break;
		}

		else
		{
			printf("请输入正确的命令，输入?查看命令列表\n");
		}
	}

	return 0;
}

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr

struct user_info { 
	char operation[2];
	char username[128];
	char password[128];
	char ip[20];
}; 

char *USER_IP = "0.0.0.13";
bool is_blocked = false;

void registration(int sockfd) { 
	char buff[MAX]; 
	char username[128];
	char password[128];
	
	struct user_info user;
	
	printf("Enter the username : "); 
	scanf("%s", username);
	printf("Enter the passowrd : ");
	scanf("%s", password);
	strcpy(user.operation, "r");
	strcpy(user.username, username);
	strcpy(user.password, password);
	strcpy(user.ip, USER_IP); 
	
	write(sockfd, &user, sizeof(struct user_info));
	
	bzero(buff, sizeof(buff)); 
	read(sockfd, buff, sizeof(buff));
	printf("From Server : %s", buff);
	bzero(buff, MAX); 
} 

void login(int sockfd) {	
	char buff[MAX]; 
	char username[128];
	char password[128];
	
	struct user_info user;
	
	printf("Enter the username: "); 
	scanf("%s", username);
	printf("Enter the passowrd : "); 
	scanf("%s", password);
	
	strcpy(user.operation, "l"); 
	strcpy(user.username, username);
	strcpy(user.password, password);
	strcpy(user.ip, USER_IP); 
	
	write(sockfd, &user, sizeof(struct user_info));
	
	bzero(buff, sizeof(buff)); 
	read(sockfd, buff, sizeof(buff));
	
	if (strcmp(buff, "BLOCKED") == 0) {
		printf("NOW YOU ARE BLOCKED\n");
		is_blocked = true;
		exit(1);
	} else {
		printf("From Server : %s", buff); 
	}
}

void check_ip(int sockfd) {
	char buff[MAX];
	
	struct user_info user;
	
	strcpy(user.operation, "c");
	strcpy(user.username, " ");
	strcpy(user.password, " ");
	strcpy(user.ip, USER_IP);
	
	write(sockfd, &user, sizeof(struct user_info));
	
	bzero(buff, sizeof(buff)); 
	read(sockfd, buff, sizeof(buff));
	
	if (strcmp(buff, "BLOCKED") == 0) {
		printf("NOW YOU ARE BLOCKED\n");
		is_blocked = true;
	}
}

void delete_user(int sockfd) {
	char buff[MAX];
	char username[128];
	struct user_info user;
	
	printf("Enter the username : ");
	scanf("%s", username);
	strcpy(user.operation, "d"); 
	strcpy(user.username, username); 
	strcpy(user.password, " ");
	strcpy(user.ip, USER_IP); 
	write(sockfd, &user, sizeof(struct user_info));
	
	bzero(buff, sizeof(buff)); 
	read(sockfd, buff, sizeof(buff));
	printf("From Server : %s", buff);
	bzero(buff, sizeof(buff)); 
}


void all_users() {
	struct user_info_reg { 
		char username[128]; 
		char password[128];
	}; 
	struct user_info_reg user; 
	

	int fd = open("username_list.dat", O_RDONLY);
	if (fd == -1) {
		printf("Cannot open the file\n");
	}

	printf("\n");
	printf("-----------------------------------------------------------\n");
	while(read(fd, &user, sizeof(struct user_info_reg)) != 0) {
		printf("Username: %s\n", user.username);
	}
	printf("-----------------------------------------------------------\n");

	close(fd);
	printf("\n");
}

void read_log_file() {
	struct log_file { 
		char status[2]; 
		char username[128]; 
		char ip[20];
	};
	struct log_file log;
	
	int fd = open("log.dat", O_RDONLY);
	if (fd == -1) {
		printf("Cannot open the file\n");
	}

	printf("\n");
	printf("-----------------------------------------------------------\n");
	while(read(fd, &log, sizeof(struct log_file)) != 0) {
		printf("Status: %s, Username: %s, IP: %s\n", log.status, log.username, log.ip);
	}
	printf("-----------------------------------------------------------\n");

	close(fd);
	printf("\n");
}

void read_black_list() {
	struct black_list {
		char ip[20];
	};
	struct black_list record;
	
	int fd = open("black-list.dat", O_RDONLY);
	if (fd == -1) {
		printf("Cannot open the file\n");
	}
	
	printf("\n");
	printf("-----------------------------------------------------------\n");
	while(read(fd, &record, sizeof(struct black_list)) != 0) {
		printf("IP: %s\n", record.ip);
	}
	printf("-----------------------------------------------------------\n");

	close(fd);
	printf("\n");
}
 
int main() { 
	int sockfd, connfd; 
	struct sockaddr_in servaddr; 
	int menu_option;
	
	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} else {
		printf("Socket successfully created..\n"); 
	} 
	bzero(&servaddr, sizeof(servaddr)); 
	
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
	
	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} else {
		printf("connected to the server..\n"); 
	}
	
	check_ip(sockfd);
	if(is_blocked) {
		printf("Your IP is blocked.\n");
		exit(1);
	}
	
	do {
		printf("Main Menu\n");
		printf("1. Registration\n");
		printf("2. Log In\n");
		printf("0. Exit\n");
		printf("\n");
		printf("!!!ONLY for System Administrators\n"); 
		printf("3. Delete user\n");
		printf("4. View all users\n");
		printf("5. Read log file\n");
		printf("6. Read black list\n");
		printf("Enter your choice: ");
		scanf("%d", &menu_option);
		
		switch (menu_option) {
			case 1:
				registration(sockfd); 
				break;
			case 2:
				login(sockfd);
				break;
			case 3:
				delete_user(sockfd);
				break;
			case 4:
				all_users();
				break;
			case 5:
				read_log_file();
				break;
			case 6:
				read_black_list();
				break;
			case 0:
				break;
			default:
				printf("Invalid Input\n");
		}
	} while (menu_option != 0);
	
	// close the socket 
	close(sockfd); 
} 
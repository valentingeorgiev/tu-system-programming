// Write CPP code here 
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

void registration(int sockfd) 
{ 
	char buff[MAX]; 

	char username[60];
	char password[100];
	char username_pasword[255];
	
	printf("Enter the username : "); 
	scanf("%s", username);
	printf("Enter the passowrd : "); 
	scanf("%s", password);
  strcpy(username_pasword, "1");
	strcat(username_pasword, " ");
	strcat(username_pasword, username);
	strcat(username_pasword, "/@");
  strcat(username_pasword, password);
	strcat(username_pasword, "\n");
	
	write(sockfd, username_pasword, strlen(username_pasword)); 
	bzero(buff, sizeof(buff)); 
	read(sockfd, buff, sizeof(buff)); 
	printf("From Server : %s", buff); 
} 

void login(int sockfd) 
{	
	char buff[MAX]; 
	char username[60];
	char password[100];
	char username_pasword[255];
	
	printf("Enter the username : "); 
	scanf("%s", username);
	printf("Enter the passowrd : "); 
	scanf("%s", password);
  strcpy(username_pasword, "2");
	strcat(username_pasword, " ");
	strcat(username_pasword, username);
	strcat(username_pasword, "/@");
  strcat(username_pasword, password);
	strcat(username_pasword, "~ip");
	strcat(username_pasword, "0.0.0.1");
	
	write(sockfd, username_pasword, strlen(username_pasword)); 

	bzero(buff, sizeof(buff)); 
	read(sockfd, buff, sizeof(buff)); 
	printf("From Server : %s", buff); 

}

int main() 
{ 
	int sockfd, connfd; 
	struct sockaddr_in servaddr; 
	int menu_option;


	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	servaddr.sin_port = htons(PORT); 

	// connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} 
	else
		printf("connected to the server..\n"); 

	do
	{
		printf("Main Menu\n");
		printf("1. Registration\n");
		printf("2. Log In\n");
		printf("0. Exit\n");
		printf("Enter your choice: ");
		scanf("%d", &menu_option);

		switch (menu_option)
		{
		case 1:
			registration(sockfd); 
			break;
		case 2:
			login(sockfd);
			break;
		case 0:
			break;
		default:
			printf("Invalid Input");
		}
	} while (menu_option != 0);


	// close the socket 
	close(sockfd); 
} 
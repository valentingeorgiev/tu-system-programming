#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include<unistd.h> 
#include<fcntl.h> 
#include <arpa/inet.h>
#include <errno.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

void add_blacl_list() {
	int fdread;
	char buffer[1000];
	int bytes_read;	
	int k = 0;
	fdread = open("log.txt", O_RDONLY); 

	do {
		char t = 0;
		bytes_read = read(fdread, &t, 1); 
		buffer[k++] = t;  
	} while (bytes_read != 0); 
	close(fdread);
}

void func(int sockfd) 
{ 
	char buff[MAX];
	char *operation;
	char *message;
	char *op1 = "1";
	char *op2 = "2";
	
	for (;;) { 
		bzero(buff, MAX); 
		
		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		
		operation = strtok(buff, " ");
		message = strtok(NULL, " ");
		char slog[255];
		if(strcmp(operation, op1) == 0) {
			int	fd= open("username_password.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			if(fd==-1){
				printf("No such file or directory");
			}
			write(fd, message, strlen(message));          
			close(fd);  
			
			bzero(buff, MAX); 
			strcpy(buff, "Successful Registration\n");
			write(sockfd, buff, sizeof(buff));
		}
		
		if(strcmp(operation, op2) == 0) {
			int fdread;
			char buffer[200];
			int bytes_read;	
			int k = 0;
			
			fdread = open("username_password.txt", O_RDONLY); 
			
			do {
				char t = 0;
				bytes_read = read(fdread, &t, 1); 
				buffer[k++] = t;  
			} while (bytes_read != 0); 
			
			char * username_password;
			char * ip;

			
			int	fdwritelog= open("log.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			if(fdwritelog ==-1){
				printf("No such file or directory");
			}
			
			username_password = strtok(message, "~ip");
			ip = strtok(NULL, "~ip");
			
			int is_contained = 0;
			char *findu = strstr(buffer, username_password);
			
			if (findu != NULL)
			{
				is_contained = 1;
			}
			
			if(is_contained == 1) {
				strcpy(slog, "s");
				strcat(slog, " ");
				strcat(slog, username_password);
				strcat(slog, "~ip ");
				strcat(slog, ip);
				strcat(slog, "\n");
				write(fdwritelog, slog, strlen(slog));
				
				bzero(buff, MAX); 
				strcpy(buff, "Successful Login\n");
				write(sockfd, buff, sizeof(buff));
			} else {

				strcpy(slog, "f");
				strcat(slog, " ");
				strcat(slog, username_password);
				strcat(slog, "~ip ");
				strcat(slog, ip);
				strcat(slog, "\n");
				write(fdwritelog, slog, strlen(slog)); 
				bzero(buff, MAX); 
				strcpy(buff, "Try again\n");
				write(sockfd, buff, sizeof(buff));
			}
			
			close(fdwritelog);
			close(fdread);
			add_blacl_list();
			bzero(buff, MAX); 
			strcpy(buff, "Login\n");
			write(sockfd, buff, sizeof(buff));

		
		}
	} 
} 

int main() 
{ 
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

	// Now server is ready to listen and verification 
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} 
	else
		printf("Server listening..\n"); 
	len = sizeof(cli); 

	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, &len); 
	if (connfd < 0) { 
		printf("server acccept failed...\n"); 
		exit(0); 
	} 
	else
		printf("server acccept the client...\n"); 

	func(connfd); 

	close(sockfd); 
} 

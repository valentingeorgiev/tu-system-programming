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
#include <stdbool.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

int count_lines(char *file) {
	int i = 0;
	int count = 0;
	while (file[i] != '\0') {
		if (file[i] == '\n') {
			count++;
		}
		i++;
	};
	return count;
}

void split_to_lines(char **array, char *line) {
	int i = 0;
	char *p = strtok(line, "\n");
	
	while (p != NULL) {
		*(array + i) = p;
		i++;
		p = strtok(NULL, "\n");
	}
}

int get_tries (char **file_rows, int rows_count, char *ip) {
	int count = 0;
	int i = 0;
	for (i = 0; i < rows_count; i++) {
		char *current_line = strdup(file_rows[i]);
		char *f_login = strtok(current_line, "~ip");
		char *f_login_ip = strtok(NULL, "~ip");
		if (strcmp(f_login_ip, ip) == 0 ) {
			count ++;
		}
		free(current_line);
	}
	return count;
}

bool does_it_exist_in_black_list (char *ip) {
	int fp = open("black-list.txt", O_RDONLY);
	off_t size = lseek(fp, 0, SEEK_END);
	lseek(fp, 0, SEEK_SET);
	// Allocate enough to hold the whole contents plus a '\0' char.
	
	char *buffer = malloc(size+ 1);
	read(fp, buffer, size);
	
	int lines_count = count_lines(buffer);
	char **lines=malloc(sizeof(char*)*lines_count);
	split_to_lines(lines, buffer);   
	
	close(fp); 
	
	int i;
	if (lines_count>=1) {
		for (i = 0; i < lines_count; i++) {
			if (strcmp(lines[i], ip) == 0) {
				return true;
			}
		}
	}
	free(lines);
	return false;
}

void add_to_black_list(char *ip) {
	int fd = open("black-list.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
	if (fd == -1){
		printf("No such file or directory");
	}
	write(fd, ip, strlen(ip));
	write(fd, "\n", strlen("\n"));        
	close(fd);  
}

bool check_in_black_list() {
	int fp = open("log.txt", O_RDONLY);
	off_t size = lseek(fp, 0, SEEK_END);
	lseek(fp, 0, SEEK_SET);
	// Allocate enough to hold the whole contents plus a '\0' char.
	char *buffer = malloc(size+ 1);
	
	read(fp, buffer, size);
	
	int lines_count = count_lines(buffer);
	char **lines=malloc(sizeof(char*)*lines_count);
	split_to_lines(lines, buffer);
	
	int i;
	char *f_login;
	char *f_login_username;
	char *f_login_ip;
	int count = 0;
	
	for (i = 0; i < lines_count; i++) {
		char *current_line = strdup(lines[i]);
		f_login = strtok(current_line, " ");
		
		if (strcmp(f_login, "f") == 0) {
			f_login_username = strtok(NULL, "f");
			strtok(f_login_username, "~ip");
			f_login_ip = strtok(NULL, "~ip");
			if (get_tries(lines, lines_count, f_login_ip) >= 20) {
				if (!does_it_exist_in_black_list(f_login_ip)) {
					add_to_black_list(f_login_ip);
					return true;
				} 
			}
		}
		free(current_line);
	}
	
	close(fp);
	free(buffer);
	free(lines);
	return false;
}

void func(int sockfd) {
	char buff[MAX];
	char *operation;
	char *message;
	char slog[255];
	char reg[255];
	char * username_password;
	char * username;
	char * ip;
	char * password;
	int countip = 0;

	for (;;) { 
		bzero(buff, MAX); 
		read(sockfd, buff, sizeof(buff)); 
		
		operation = strtok(buff, " ");
		message = strtok(NULL, " ");
	

		if (strcmp(operation, "0") == 0) {
			if (does_it_exist_in_black_list(message) == 1) {
				bzero(buff, MAX); 
				strcpy(buff, "BLOCKED");
				write(sockfd, buff, sizeof(buff));
				return;
			} else {
				bzero(buff, MAX); 
				strcpy(buff, "OK");
				write(sockfd, buff, sizeof(buff));
			}
		}
		
		//Registration
		if (strcmp(operation, "1") == 0) {
			username_password = strtok(message, "~ip");
			ip = strtok(NULL, "~ip");
			username = strtok(username_password, "/@");
			password = strtok(NULL, "/@");

			int	fd= open("username_password.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			write(fd, username, strlen(username));
			write(fd, "/@", strlen("/@"));
			write(fd, password, strlen(password));
			write(fd, "\n", strlen("\n"));          
			close(fd); 
			
			strcpy(slog, "r");
			strcat(slog, " ");
			strcat(slog, username);
			strcat(slog, "~ip");
			strcat(slog, ip);
			strcat(slog, "\n");
			
			int	fdwritelog= open("log.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			write(fdwritelog, slog, strlen(slog));
			close(fdwritelog);

			bzero(buff, MAX);
			strcpy(buff, "Successful Registration\n");
			write(sockfd, buff, sizeof(buff));
		}
		
		
		// Login
		if (strcmp(operation, "2") == 0) {
			int fdread;
			char buffer[200];
			int bytes_read;	
			int k = 0;
			char buff[MAX];
			int is_contained = 0;
			char *findu;
			
			username_password = strtok(message, "~ip");
			ip = strtok(NULL, "~ip");
			username = strtok(username_password, "/@");
			password = strtok(NULL, "/@");
			
			fdread = open("username_password.txt", O_RDONLY); 
			do {
				char t = 0;
				bytes_read = read(fdread, &t, 1);
				buffer[k++] = t;  
			} while (bytes_read != 0); 
			
			findu = strstr(buffer, username_password);
			if (findu != NULL) {
				is_contained = 1;
			}
			
			if (is_contained == 1) {
				strcpy(slog, "s");
				strcat(slog, " ");
				strcat(slog, username);
				strcat(slog, "~ip");
				strcat(slog, ip);
				strcat(slog, "\n");
				
				int	fdwritelog= open("log.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
				write(fdwritelog, slog, strlen(slog));
				close(fdwritelog);
				
				bzero(buff, MAX);
				strcpy(buff, "Successful Login\n");
				write(sockfd, buff, sizeof(buff));
			} else {
				strcpy(slog, "f");
				strcat(slog, " ");
				strcat(slog, username);
				strcat(slog, "~ip");
				strcat(slog, ip);
				strcat(slog, "\n");
				
				int	fdwritelog= open("log.txt", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
				write(fdwritelog, slog, strlen(slog)); 
				close(fdwritelog);
				
				int fp = open("log.txt", O_RDONLY);
				off_t size = lseek(fp, 0, SEEK_END);
				lseek(fp, 0, SEEK_SET);
				// Allocate enough to hold the whole contents plus a '\0' char.
				
				char *buffer = malloc(size+ 1);
				read(fp, buffer, size);
				
				int lines_count = count_lines(buffer);
				char **lines=malloc(sizeof(char*)*lines_count);
				
				if (lines_count>=1) {
					split_to_lines(lines, buffer);
					char * previous_username;
					char * previous_ip;
					strtok(lines[lines_count-2], " ");
					previous_username = strtok(strtok(NULL, " "), "~ip");
					previous_ip = strtok(NULL, "~ip");

					if ((strcmp(previous_username, username) == 0) && (strcmp(previous_ip, ip) == 0)) {
						countip++;
					}
					
					if (countip == 10) {
						printf("IP %s has tried to enter 10 times with user profile: %s\n", ip, username);
					}
				}
				
				close(fp);
				free(buffer);
				free(lines);
				
				int is_in_black_list = check_in_black_list();
				
				if (is_in_black_list==1) {
					printf("USER IS BLOCKED\n");
					bzero(buff, MAX); 
					strcpy(buff, "BLOCKED");
					write(sockfd, buff, sizeof(buff));
					return;	
				} else {
					bzero(buff, MAX); 
					strcpy(buff, "Try again\n");
					write(sockfd, buff, sizeof(buff));
				}
			}
			close(fdread);
		}
		
		if (strcmp(operation, "3") == 0) {
			int	usersfd = open("username_password.txt", O_RDONLY);
			int temp = open("temp.txt", O_CREAT | O_WRONLY, S_IRWXU);
			off_t size = lseek(usersfd, 0, SEEK_END);
			lseek(usersfd, 0, SEEK_SET);
			// Allocate enough to hold the whole contents plus a '\0' char.
			
			char *buffer = malloc(size+ 1);
			read(usersfd, buffer, size);
			
			int lines_count = count_lines(buffer);
			char **lines=malloc(sizeof(char*)*lines_count);
			split_to_lines(lines, buffer);
			
			int i;
			for (i = 0; i < lines_count; i++) {
				char *current_line = strdup(lines[i]);
				char* username = strtok(current_line, "/");
				if (strcmp(username, message) != 0) {
					write(temp, lines[i], strlen(lines[i]));
					write(temp, "\n", strlen("\n"));
				}
				free(current_line);
			}
			close(usersfd);
			close(temp);
			free(lines);
			
			remove("username_password.txt");
			rename("temp.txt", "username_password.txt");
		}
	} 
}

int main() {
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 
	
	// socket create and verification 
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 
	
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} else {
		printf("Socket successfully binded..\n"); 
	}
	
	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0); 
	} else {
		printf("Server listening..\n"); 
	}
	len = sizeof(cli); 
	
	// Accept the data packet from client and verification 
	connfd = accept(sockfd, (SA*)&cli, (unsigned int*)(&len)); 
	if (connfd < 0) {
		printf("server acccept failed...\n"); 
		exit(0);
	} else {
		printf("server acccept the client...\n"); 
	}
	
	func(connfd); 
	close(sockfd); 
} 
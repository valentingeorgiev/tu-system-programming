#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 

struct from_user { 
	char operation[2]; 
	char username[128]; 
	char password[128];
	char ip[20];
}; 

struct user_info_reg { 
	char username[128]; 
	char password[128];
};

struct log_file { 
	char status[2]; 
	char username[128]; 
	char ip[20];
}; 

struct black_list {
	char ip[20];
};

int get_tries (char *ip) {
	struct log_file log;
	int count = 0;
	
	int fdlog = open("log.dat", O_RDONLY);
	if (fdlog == -1) {
		printf("Cannot open log file\n");
	}
  while(read(fdlog, &log, sizeof(struct log_file)) != 0) {
		if (strcmp(log.status, "f") == 0) {
			if (strcmp(log.ip, ip) == 0 ) {
				count ++;
			}
		}
	}
	
	close(fdlog);
	return count - 1;
}

bool does_it_exist_in_black_list (char *ip) {
	struct black_list record;
	
	int fdread = open("black-list.dat", O_RDONLY);
	if (fdread == -1) {
		return false;
	}
	
  while(read(fdread, &record, sizeof(struct black_list)) != 0) {
		if (strcmp(record.ip, ip) == 0) {
			return true;
		}
	}

	close(fdread);
	
	return false;
}

void add_to_black_list(char *ip) {
	int fdwrite = open("black-list.dat", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
	if (fdwrite == -1) {
		printf("Cannot open black list\n");
	}
	
	struct black_list record;
	strcpy(record.ip, ip); 
	write(fdwrite, &record, sizeof(struct black_list));
	close(fdwrite);  
}

bool check_in_black_list() {
	struct log_file log;
	int fdlog = open("log.dat", O_RDONLY);
	if (fdlog == -1) {
		printf("Cannot open log file\n");
	}
  while(read(fdlog, &log, sizeof(struct log_file)) != 0) {
		if (strcmp(log.status, "f") == 0) {
			if (get_tries(log.ip) >= 20) {
				if (!does_it_exist_in_black_list(log.ip)) {
					add_to_black_list(log.ip);
					return true;
				}
			}
		}
	}

	close(fdlog);
	
	return false;
}

void server_operations(int sockfd) {
	char buff[MAX];
	int count_ip = 0;
	int fdread;
	int fdwrite;
	int is_contained = 0;
	
	struct from_user user;
	struct user_info_reg info_reg;
	struct log_file log;
	
	while(read(sockfd, &user, sizeof(struct from_user)) != 0) {

		// Check IP
		if (strcmp(user.operation, "c") == 0) {
			if (does_it_exist_in_black_list(user.ip) == 1) {
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
		
		if (strcmp(user.operation, "r") == 0) {
			strcpy(info_reg.username, user.username);
			strcpy(info_reg.password, user.password); 
			fdwrite = open("username_list.dat", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			if (fdwrite == -1) {
				printf("Cannot open list with users\n");
			}
			write(fdwrite, &info_reg, sizeof(struct user_info_reg));
			close(fdwrite);  
			
			bzero(buff, MAX);
			strcpy(buff, "Successful Registration\n");
			write(sockfd, buff, sizeof(buff));
			bzero(buff, MAX);
		}
		
		if (strcmp(user.operation, "l") == 0) {
			fdread = open("username_list.dat", O_RDONLY);
			if (fdread == -1) {
				printf("Cannot open list with users\n");
			}
			fdwrite = open("log.dat", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			if (fdwrite == -1) {
				printf("Cannot open log file\n");
			}
  		while(read(fdread, &info_reg, sizeof(struct user_info_reg)) != 0) {
				if ((strcmp(info_reg.username, user.username) == 0) && (strcmp(info_reg.password, user.password) == 0)) {
					is_contained = 1;
				}
			}
			
			if (is_contained == 1) {
				strcpy(log.status, "s");
				strcpy(log.username, user.username);
				strcpy(log.ip, user.ip);
				write(fdwrite, &log, sizeof(struct log_file));
				
				bzero(buff, MAX);
				strcpy(buff, "Successful Login\n");
				write(sockfd, buff, sizeof(buff));
				is_contained = 0;
			} else {
				int count_logs = 0;
				int fdreadlog = open("log.dat", O_RDONLY);
				if (fdreadlog == -1) {
					printf("Cannot open log file\n");
				}
				char previous_username[128];
				char previous_ip[20];
				
				//Counting the records in the file
				while(read(fdreadlog, &log, sizeof(struct log_file)) != 0) {
					count_logs++;
				}
				
				//Get the last record from the file
				for (int i = 0; i < count_logs; i++) {
					if (i == count_logs-1) {
						read(fdreadlog, &log, sizeof(struct log_file));
						strcpy(previous_username, log.username);
						strcpy(previous_ip, log.ip);
					}
				}
				
				if ((strcmp(previous_username, user.username) == 0) && (strcmp(previous_ip, user.ip) == 0)) {
					count_ip++;
				}

				if (count_ip == 9) {
					printf("IP %s has tried to enter 10 times with user profile: %s\n", user.ip, user.username);
					count_ip = 0;
				}
				
				strcpy(log.status, "f");
				strcpy(log.username, user.username);
				strcpy(log.ip, user.ip);
				write(fdwrite, &log, sizeof(struct log_file));
				
				int is_in_black_list = check_in_black_list();
				
				bzero(buff, MAX); 

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
			close(fdwrite);
		}
		
		if (strcmp(user.operation, "d") == 0) {
			int	usersfd = open("username_list.dat", O_RDONLY);
			if (usersfd == -1) {
				printf("Cannot open the file\n");
			}
			int	tempfd= open("tmp.dat", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			if (tempfd == -1) {
				printf("Cannot open the file\n");
			}

			while(read(usersfd, &info_reg, sizeof(struct user_info_reg)) != 0) {
				if ((strcmp(info_reg.username, user.username) != 0)) {
					write(tempfd, &info_reg, sizeof(struct user_info_reg));
				}
			}
			
			remove("username_list.dat");
			rename("tmp.dat", "username_list.dat");
			
			fdwrite = open("log.dat", O_APPEND | O_CREAT | O_WRONLY, S_IRWXU);
			if (fdwrite == -1) {
				printf("Cannot open log file\n");
			}
			strcpy(log.status, "d");
			strcpy(log.username, user.username);
			strcpy(log.ip, user.ip);
			write(fdwrite, &log, sizeof(struct log_file));
			
			bzero(buff, MAX);
			strcpy(buff, "Successful Delete\n");
			write(sockfd, buff, sizeof(buff));
			
			close(usersfd);
			close(tempfd);
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
	
	server_operations(connfd); 
	close(sockfd); 
} 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "tands.h"
#include <chrono>
#include <iomanip>
#include <sys/time.h>

using namespace std;

int main(int argc, char *argv[]){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	char hostname[100];
    char username[100];
	char *p;
	long int port_num;
	int sent_transactions = 0;
	port_num = strtol(argv[1], &p, 10);
	struct timeval current_time;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port_num);
	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_addr.s_addr = inet_addr(argv[2]);

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");

	// Variables needed to send and recieve data to and from a server
    char buf[4096];
	string user_input;

	// Variables to read input from a file
	std::string line;
    vector <string> lines;
    char cmd;
    int par;

	gethostname(hostname, 100);
	string space = " ";
	string host_name(hostname);
	int pid = getpid();
	string pid_int = to_string(pid);
	string dot = ".";

	string test_send = space + host_name + dot + pid_int;

	string toSend;

	
	// Get the number from the file
	printf("Using port %ld\n", port_num);
	printf("Using server address %s\n", argv[2]);
	printf("Host %s\n", test_send.c_str());

	while (getline(cin, line)){
		std::istringstream iss(line);
		iss >> cmd >> user_input;
		lines.push_back(line);
		if(cmd == 'T'){
			toSend = user_input + test_send;
			
			gettimeofday(&current_time, NULL); 
			printf("%ld.%d\t", current_time.tv_sec, int(current_time.tv_usec/10000));
			printf("Send (T %s)\n", user_input.c_str());
			int send_result = send(clientSocket, toSend.c_str(), toSend.size() + 1, 0);
			sent_transactions++;
			if (send_result != -1){
				// ECHO THE RESPOSE BACK
				memset(buf, 0, 4096);
				int bytes_recieved = recv(clientSocket, buf, 4096, 0);
				if (bytes_recieved > 0){
					
					gettimeofday(&current_time, NULL);
					printf("%ld.%d\t", current_time.tv_sec, int(current_time.tv_usec/10000));
					printf("Recv (d %s)\n", buf);
				}
				
			}
		}
		else if (cmd == 'S'){
			printf("Sleeping for %s units\n", user_input.c_str());
			char *pend;
			long int l1;
			l1 = strtol (user_input.c_str(),&pend,10);
			Sleep(l1);
		}
	}

	printf("Sent %d transactions\n", sent_transactions);



	close(clientSocket);
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "tands.h"
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <time.h>
#include <cstdlib>
#include <map>


using namespace std;


#define TRUE 1
#define FALSE 0

typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float> fsec;

map <string, int> connections_table;


int main(int argc, char *argv[]){
    struct timeval current_time;
    int    len, rc, on = 1;
    int    listen_sd = -1, new_sd = -1;
    int    desc_ready, end_server = FALSE, compress_array = FALSE;
    int    close_conn;
    char   buffer[80];
    char buffer2[100];
    struct sockaddr_in addr;
    int    timeout;
    struct pollfd fds[200];
    int    nfds = 1, current_size = 0, i, j;
    char hostname[100];
    char username[100];
    FILE *fp;
    char *p;
	long int port_num;
	port_num = strtol(argv[1], &p, 10);
    int total_trans = 0;
    fsec fs_global;
    int num_spec_global = 0;

    char file_name[100] = "server.log";
    int result = remove(file_name);
    fp = fopen(file_name, "a+");

    // Create connection

    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket() failed");
        exit(-1);
    }

    // Set socket to be reusable
    rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                  (char *)&on, sizeof(on));
    if (rc < 0)
    {
        perror("setsockopt() failed");
        close(listen_sd);
        exit(-1);
    } 

    // Set socket to be nonblocking

    rc = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (rc < 0)
    {
        perror("ioctl() failed");
        close(listen_sd);
        exit(-1);
    }

    // Bind the socket

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    //addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //addr.sin_addr.s_addr = inet_addr("129.128.29.41");
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port_num);
    rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0)
    {
        perror("bind() failed");
        close(listen_sd);
        exit(-1);
    }

    // Set the listen back log
    rc = listen(listen_sd, 32);
    if (rc < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        exit(-1);
    }

    // Initialize the pollfd structure
    memset(fds, 0 , sizeof(fds));

    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;

    timeout = 30000;
    //auto t0 = Time::now();
    do {
        printf("Waiting on poll()...Server will wait for 30 seconds for any incoming clients\n");
        printf("Please wait 30 seconds for displaying Summary Statistics\n");
        rc = poll(fds, nfds, timeout);

        if (rc < 0)
        {
            perror("poll() failed");
            break;
        }

        //Check to see if 30 seconds has expired
        if (rc == 0)
        {
            printf("30 Seconds are now over. Ending the Server.\n");
            fprintf(fp, "SUMMARY STATISTICS: \n");
            fprintf(fp, "Total Number of transaction from all clients: %d\n", total_trans);
            map<string, int>::iterator itr;
            for (itr = connections_table.begin(); itr != connections_table.end(); itr++){
                //fp << "Number of transactions from " << itr->first << '=' << itr->second << '\n';
                fprintf(fp, "Number of connections from %s = %d\n", itr->first.c_str(), itr->second);
            }
            //cout <<endl;
            fprintf(fp, "Transactions per Second = %.1f  (%d transactions/%.2f seconds)", total_trans/fs_global.count(), total_trans, fs_global.count());
            
            
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {
            if(fds[i].revents == 0)
                continue;

            if(fds[i].revents != POLLIN)
            {
                printf("Error!\n");
                end_server = TRUE;
                break;
            
            }

            if (fds[i].fd == listen_sd)
            {
                //printf("Listening socket is readable\n");
                do{
                    new_sd = accept(listen_sd, NULL, NULL);
                    // Error checking for accept command
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("accept() failed");
                            end_server = TRUE;
                        }
                        break;
                    }
                    
                    //printf("New Incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    //recv(fds[i].fd, buffer2, sizeof(buffer2), 0);
                    //printf("Test message: %s\n", buffer2);
                    
                } while (new_sd != -1);
            }

            else 
            {
                //printf("Descriptor %d is readable\n", fds[i].fd);
                int num_trans_specific = 0;  // Num is the number of transactions
                close_conn = FALSE;
                printf("Using port %ld\n", port_num);
                auto t0 = Time::now();
                
                do
                {
                    // Recieve data from the client

                    rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    string s(buffer);
                    // Split the string into the number and the machine.pid
                    istringstream is(s);
                    int n;
                    string machineName;
                    while (is >> n >> machineName){
                        total_trans++;
                        num_trans_specific++;
                        //std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
                        //std::cout << fixed << setprecision(2) << (ms.count()) << "\t";
                        gettimeofday(&current_time, NULL); 
                        fprintf(fp, "%ld.%d\t", current_time.tv_sec, int(current_time.tv_usec/10000));
                        fprintf(fp,"#  %d\t", total_trans);
                        strcpy(buffer2, machineName.c_str());
                        fprintf(fp,"(T %d) from %s\n", n, machineName.c_str());
                        Trans(n);
                        //std::cout << fixed << setprecision(2) << float(ms.count()/1000) << "\t";
                        gettimeofday(&current_time, NULL);
                        fprintf(fp, "%ld.%d\t", current_time.tv_sec, int(current_time.tv_usec/10000));
                        fprintf(fp, "#  %d \t(Done) from %s\n", total_trans, machineName.c_str());
                        string trans_num = to_string(total_trans);
                        rc = send(fds[i].fd, trans_num.c_str(), trans_num.size() + 1, 0);
                    }
                    
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("recv() failed");
                            close_conn = TRUE;
                        }
                        break;
                    }
                    if(rc == 0)
                    {
                        //printf("Connection closed\n");
                        close_conn = TRUE;
                        break;
                    }
                    len = rc; // length of data recieved

                    // Echo data back to the client
                    
                    bzero(buffer, sizeof(buffer));
                    
                    // Error check for this rc

                } while(TRUE);
               
                string connection_str(buffer2);
                connections_table.insert(pair<string, int>(connection_str, num_trans_specific));
                num_spec_global = num_trans_specific;
                auto t1 = Time::now();
                fsec fs = t1-t0;
                fs_global = fs;
                if (close_conn)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = TRUE;
                }
            }
        }

        if (compress_array)
        {
            compress_array = FALSE;
            for (i = 0; i<nfds;i++)
            {
                if (fds[i].fd == -1)
                {
                    for (j = 1; j<nfds; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }
    } while (end_server == FALSE);
    

    for (i = 0; i < nfds; i++)
    {
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }


    fclose(fp);
    return 0;

}
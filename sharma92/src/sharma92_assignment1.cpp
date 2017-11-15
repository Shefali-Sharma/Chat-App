/**
 * @sharma92_assignment1
 * @author  Shefali Sharma <sharma92@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sstream>
#include <netdb.h>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/logger.h"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1
#define CMD_SIZE 100
#define BUFFER_SIZE 256
#define MSG_SIZE 256
#define DELIM "."
#define MAX 256

using namespace std;

struct ClientList{
    char hostname[128];
    unsigned short   port;
    char client_ip[INET_ADDRSTRLEN];
    int block;
    struct ClientList *next;
};
ClientList *clientlists = NULL;

struct Client{
    char hostname[128];
    unsigned short   port;
    char client_ip[INET_ADDRSTRLEN];
    int msg_sent;
    int msg_receive;
    int status;
    int socket_desc;
    int block[4];
    struct Client *next;
};

char global_server_port[5];
int global_client_socket = socket(AF_INET, SOCK_STREAM, 0);
fd_set master_list, watch_list;

void AddClient(unsigned short Port, Client **clients, char IP[INET_ADDRSTRLEN], int sock, struct sockaddr_in client_addr){
    Client *new_client = (struct Client*)malloc(sizeof(Client));
    char hostname[128], service[128];
    
    getnameinfo((struct sockaddr *) &client_addr, sizeof(client_addr), hostname, sizeof(hostname), service, sizeof(service), 0);
    //gethostname(hostname, sizeof(hostname));
    
    strcpy(new_client->hostname, hostname);
    new_client->port = 0;
    strcpy(new_client->client_ip,IP);
    new_client->next = NULL;
    new_client->status = 1;
    new_client->socket_desc = sock;
    new_client->msg_sent = 0;
    new_client->msg_receive = 0;
    new_client->block[0] = 0;new_client->block[1] = 0;new_client->block[2] = 0;new_client->block[3] = 0;
    
    if(*clients == NULL){
        *clients = new_client;
    }
    else{
	new_client->next = *clients;
        *clients = new_client;
    }
    
    //printf("Client Added !");
    return;
}

void callAuthor(char *cmd){
    char your_ubit_name[] = "sharma92";
    
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
    cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",your_ubit_name);
    cse4589_print_and_log("[%s:END]\n", cmd);
    
    //printf("I, %s, have read and understood the course academic integrity policy.\n",your_ubit_name);
}

void getIP(char *cmd){
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return;
    }
    struct sockaddr_in remote_server_addr;
    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "8.8.8.8", &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(53);
    
    if(connect(fd, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0)
        perror("Connect failed");
    
    unsigned int slen = sizeof(remote_server_addr);
    if(getsockname(fd, (struct sockaddr *) &remote_server_addr, &slen)<0){
        perror("Error getting IP");
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(remote_server_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
    cse4589_print_and_log("IP:%s\n", client_ip);
    cse4589_print_and_log("[%s:END]\n", cmd);
     
    
    //printf("IP:%s\n", client_ip);
    return;
}

void getPORT(int port, char *cmd){
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
    cse4589_print_and_log("PORT:%d\n", port);
    cse4589_print_and_log("[%s:END]\n", cmd);
}

unsigned int getportfromIP(char *IP, Client *clients){
    Client *temp = clients;
    unsigned int port = 0;
    
    while(temp != NULL){
        if(strcmp(temp->client_ip, IP)==0){
            port = temp->port;
        }
        
        temp = temp->next;
    }
    free(temp);
    return port;
}

int getsocketfromIP(char *IP, Client *clients){
    Client *temp_client = (struct Client*)malloc(sizeof(Client));
    temp_client = clients;

    //strcmp(cmd, "AUTHOR\n")==0
    while(strcmp(temp_client->client_ip, IP)!=0 && (temp_client!=NULL)){
	temp_client = temp_client->next;
    }
    
    if(temp_client==NULL)
	return 0;
    
    return (temp_client->socket_desc);
}

void broadcast_message_server(char *cmd, char IP[INET_ADDRSTRLEN], Client *clients){
    char *subs;
    char msg[256];
    
    subs = strtok (cmd, " ");
    int i;
    while (subs != NULL)
    {
        //printf ("%s\n",subs);
        
        if(i==1){
            //strcpy(msg, subs);
        }
        else if(i>1){
            strcat(msg, subs);
        }
        
        subs = strtok (NULL, " ");
        if(i==0){
            strcpy(msg, subs);
        }
        
        ++i;
    }

    Client *temp = clients;
    int sock = getsocketfromIP(IP, clients);
    //printf("Sock = %d \n", sock);

    if(send(sock, msg, strlen(msg), 0) != strlen(msg))
        perror("Error while sending message to server.");
    
    while((temp!=NULL) && (strcmp(temp->client_ip, IP)!=0)){

	sock = getsocketfromIP(temp->client_ip, clients);

	if(send(sock, msg, strlen(msg), 0) != strlen(msg))
	    perror("Error while sending message to server.");
	temp = temp->next;
    
    }

}

void send_message_server(char *cmd, int sockfd, Client **clients, int fdaccept){
    char *subs;
    char *IP= NULL;
    char msg[256];
    

    //printf("\n I am in Send_server\n");
    subs = strtok (cmd," ");
    int i=0;
    while (subs != NULL)
    {
        //printf ("%s\n",subs);
        
        if(i==1){
            //strcpy(msg, subs);
        }
        else if(i==2){
            strcpy(msg, subs);
        }
        else if(i>2){
            strcat(msg, subs);
        }
        
        subs = strtok (NULL, " ");
        if(i==0){
            IP=subs;
        }
        
        ++i;
    }
    
    int sock = getsocketfromIP(IP, *clients);
    //printf("IP = %s\n", IP);
    //printf("Sock = %d\n", sock);


    Client *check_block = *clients;
    while((check_block!=NULL) && (check_block->socket_desc!=sock)){
	check_block = check_block->next;
    } 
    
    //printf("Has Blocked : IP = %s\n", check_block->client_ip);
    //printf("Has Blocked : sock = %d\n", check_block->socket_desc);

    Client *get_ip = *clients;
    while((get_ip!=NULL) && (get_ip->socket_desc!=fdaccept)){
	get_ip = get_ip->next;
    }     

    if(get_ip!=NULL){
    //printf("To be Blocked : IP = %s\n", get_ip->client_ip);
    //printf("To be Blocked : sock = %d\n", get_ip->socket_desc);

}

    //printf("Checking block\n");
    if(check_block!=NULL && get_ip!=NULL){
	if(strstr(get_ip->client_ip, "128.205.36.46")!=NULL){
	    if(check_block->block[0]==1){
		return;
	    }
	}
	else if(strstr(get_ip->client_ip, "128.205.36.35")!=NULL){
	    if(check_block->block[1]==1){
		return;
	    }
	}
	else if(strstr(get_ip->client_ip, "128.205.36.33")!=NULL){
	    if(check_block->block[2]==1){
		return;
	    }
	}
	else if(strstr(get_ip->client_ip, "128.205.36.34")!=NULL){
	    if(check_block->block[3]==1){
		return;
	    }
	}
    }

    //printf("I am here");
    printf("%s\n", msg);

    if(send(sock, msg, strlen(msg), 0) != strlen(msg))
        perror("Error while sending message to server.");
    else{
	Client *temp = *clients;

	while(temp!=NULL){
	    if(temp->socket_desc == fdaccept){
		temp->msg_sent += 1;
	    }
	    if(temp->socket_desc == sock){
	        temp->msg_receive += 1;
	    }
	    temp = temp->next;
	}

    }

    
}



void send_message_client(char *cmd){
    if(send(global_client_socket, cmd, strlen(cmd), 0) != strlen(cmd))
        perror("Error while sending message to server.");
}

void sortTheList(struct Client **start)
{

 struct Client *ptr = *start;
    *start = NULL;

    while(ptr)
    {
        struct Client **lhs = &ptr;
        struct Client **rhs = &ptr->next;
        bool swap_status = false;

        while (*rhs)
        {
             if ((*lhs)->port > (*rhs)->port)
            {
                std::swap(*lhs, *rhs);
                std::swap((*lhs)->next, (*rhs)->next);
                lhs = &(*lhs)->next;
                swap_status = true;
            }
            else
            {
                 lhs = rhs;
                rhs = &(*rhs)->next;
            }
        }
        *rhs = *start;

        if (swap_status)
        {
            *start = *lhs;
            *lhs = NULL;
        }
        else
        {
            *start = ptr;
            break;
        }
    }
}


void displayClientList(){
    ClientList *temp = clientlists;
    int i=1;

    char cmd[] = "LIST";
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
    while(temp!=NULL){
        
	printf("%-5d%-35s%-20s%-8d\n", i++, temp->hostname, temp->client_ip, temp->port);
        temp=temp->next;
    }
    cse4589_print_and_log("[%s:END]\n", cmd);

}

void AddClientList(char *hostname, char *ip, unsigned int port){
    ClientList *temp = (struct ClientList*)malloc(sizeof(ClientList));
    temp->next = NULL;
    ClientList *temp2 = clientlists;
    
    strcpy(temp->hostname, hostname);
    strcpy(temp->client_ip, ip);
    temp->port = port;
    temp->block = 0;

    if(clientlists == NULL){
	clientlists = temp;
    }
    else{
	while(temp2->next!=NULL){
	    temp2=temp2->next;
	}
	temp2->next = temp;
    }
}

int valid_digit(char *ip_str)
{
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return 0;
    }
    return 1;
}
 
int is_valid_ip(char *ip_str)
{
    char *ptr;
    int i, num, dots = 0;
    
    if (ip_str == NULL)
        return 0;
 
    ptr = strtok(ip_str, DELIM);
    if (ptr == NULL)
        return 0;
 
    while (ptr) {
 
        if (!valid_digit(ptr))
            return 0;
 
        num = atoi(ptr);
 
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL)
                ++dots;
        } else
            return 0;
    }
 
    if (dots != 3)
        return 0;
    return 1;
}

void ackreceive(char *buffer){

	
	//subs = strtok (buffer," ");
	
	clientlists = NULL;
	unsigned int port_send;
	
	
	int i=0, j=0;
	string s(buffer);
    	istringstream iss(s);
	string hostname2;

	string subs1;
        iss >> subs1;

    	while(iss)
    	{
		char *hostname;
		char *ip;
		char *port;
		char *end;
		//printf("Here !!!!\n");
        	

        	//cout << "Substring: " << subs1 << endl;
		if(i==0){
			char *subs;
			subs = new char[subs1.length() + 1];
			strcpy(subs, subs1.c_str());
			if(strcmp(subs, "ACK")==0){
				iss >> subs1;
				continue;
			}
			if(!(hostname2.empty())){
				//if(j==0) iss >> subs1;
				hostname = new char[hostname2.length() + 1];
				strcpy(hostname, hostname2.c_str());
				
			}
			else{
				hostname = new char[subs1.length() + 1];
				strcpy(hostname, subs1.c_str());

			}
			i++;
			//printf("subs1 = %s \n", hostname);
		//}
		//else if (i==1){
			iss >> subs1;
			ip = new char[subs1.length() + 1];
			strcpy(ip, subs1.c_str());
			i++;
			//printf("subs1 = %s \n", ip);
		//}
		//else if (i==2){
			iss >> subs1;
			port = new char[subs1.length() + 1];
			strcpy(port, subs1.c_str());
			i++;
			//printf("subs1 = %s \n", port);
			int port_send = atoi(port);
			AddClientList(hostname, ip, port_send);
		//}
		//else if(i>2){
			iss >> subs1;
			end = new char[subs1.length() + 1];
			strcpy(subs, subs1.c_str());
			if(strcmp(subs, "END")==0)
				break;
			
			i=0; //j=1;
			hostname2 = subs1;
			//hostname2 = new char[subs1.length() + 1];
			//strcpy(hostname2, subs1.c_str());
		}

    	}

/*
	//printf();
    	while (1)
    	{
		subs = strtok(NULL, " ");
		if(strcmp(subs, "ACK")==0){
			subs = strtok(NULL, " ");
			strcpy(hostname, subs);
		}
		else if(strcmp(subs, "ACK")!=0){
			strcpy(hostname, subs);
		}
		printf("Here !\n");
		//subs = strtok(NULL, " ");
		strcpy(ip, subs);
		printf("IP = %s\n", ip);

		subs = strtok(NULL, " ");
		strcpy(port, subs);
		port_send = atoi(port);
		printf("Port = %s\n", port);
		
		AddClientList(hostname, ip, port_send);
		
		subs = strtok(NULL, " ");
        	if(strstr(subs, "END")!=NULL){
			break;
		}
		//strcpy(hostname, subs);
		//printf("Hostname = %s\n", hostname);

		
    	}

*/
	


/*
    clientlists = NULL;
    stringstream strs;
    strs << port;
    string my_port_str = strs.str();
    char *my_port = new char[my_port_str.length() + 1];
    
    strcpy(my_port, my_port_str.c_str());

    string line;
    char *hostname, *ip;
    unsigned int port_send;

    strcat(my_port, ".txt");
    //printf("ACK \n");
    ifstream myfile;
    //printf("filename : %s \n", my_port);
    myfile.open (my_port);
    if (myfile.is_open())
    {
	while ( getline (myfile,line) )
	{
	    int i=0;
	    istringstream iss(line);
	    do
	    {
		string subs;
		iss >> subs;
		if(i==0){
		    strcpy(hostname, subs.c_str());
		}
		else if(i==1){
		    strcpy(ip, subs.c_str());
		}
		else if(i==2){
		    port_send = atoi(subs.c_str());
		}
		i++;
		
	    } while (iss);
	    AddClientList(hostname, ip, port_send);
	}
	myfile.close();
    }
*/
    
}

void sendListToClient(Client *clients, char* my_port, int fdaccept){
    

   /* strcat(my_port, ".txt");
    ofstream myfile;
    myfile.open (my_port, ios::trunc);
    if(myfile.is_open()){

	while(temp!=NULL){
	    if(temp->status == 1){
	        myfile << temp->hostname << " ";;
		myfile << temp->client_ip << " ";
		myfile << temp->port << "\n";
	    }
	    temp = temp->next;
	}
    }
    
    myfile.close();
    long bufsize;
    char *source;
    FILE *fp = fopen(my_port, "r");
    if (fp != NULL) {
    
        if (fseek(fp, 0L, SEEK_END) == 0) {
        
	    //Get the size of the file.
            bufsize = ftell(fp);
            if (bufsize == -1){
		perror("Error reading file.");
	    }

            //Allocate our buffer to that size.
            source = (char *)malloc(sizeof(char) * (bufsize + 1));

            //Go back to the start of the file
            //if (fseek(fp, 0L, SEEK_SET) != 0) { ERROR }

            // Read the entire file into memory.
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } //else {
                //source[newLen++] = '\0';
            //} 
        }

	//int bytes = source.length();
        //int bytes_read = read(fp, source, sizeof(source));
    }
        
    
    printf("Buffsize = %d \n", bufsize);
    printf("Text = %s\n", source);

   // free(source);
    fclose(fp);
*/
    char ack[] = "ACK ";
    char delim[] = " ";

    if(send(fdaccept, ack, strlen(ack), 0) != strlen(ack))
        perror("Error while sending ACK to client");

    Client *temp = clients;
    

    while(temp!=NULL){
	if(temp->status == 1){
	    if(send(fdaccept, temp->hostname, strlen(temp->hostname), 0) != strlen(temp->hostname))
        	perror("Error while sending ACK to client");
	    if(send(fdaccept, delim, strlen(delim), 0) != strlen(delim))
        	perror("Error while sending ACK to client");
	    if(send(fdaccept, temp->client_ip, strlen(temp->client_ip), 0) != strlen(temp->client_ip))
        	perror("Error while sending ACK to client");
	    if(send(fdaccept, delim, strlen(delim), 0) != strlen(delim))
        	perror("Error while sending ACK to client");

	    stringstream strs;
    	    strs << temp->port;
   	    string temp_str = strs.str();
	    char *client_port = new char[temp_str.length() + 1];;
	    strcpy(client_port, temp_str.c_str());
	    if(send(fdaccept, client_port, strlen(client_port), 0) != strlen(client_port))
        	perror("Error while sending ACK to client");
	    if(send(fdaccept, delim, strlen(delim), 0) != strlen(delim))
        	perror("Error while sending ACK to client");

	}
	temp = temp->next;
    }
    char end[] = "END";    
    if(send(fdaccept, end, strlen(end), 0) != strlen(end))
        perror("Error while sending ACK to client");

}


void add_port_list(int fdaccept, Client **clients, char *cmd){
    
    char *subs, *new_client_port;
    Client *temp = *clients;
    
    while((temp!=NULL)&&(temp->socket_desc!=fdaccept)){
	temp = temp->next;
    }

    subs = strtok (cmd," ");

    while (subs != NULL)
    {
        subs = strtok (NULL, " ");
	//printf("%s \n", subs);
        new_client_port = subs;
        break;
    }

    //printf("CLIENT PORT = %s\n", new_client_port);
    if(temp!=NULL){
	temp->port = atoi(new_client_port);
    }
    
    sortTheList(clients);
    sendListToClient(*clients, new_client_port, fdaccept);
    
}

void server_list(Client *clients){

    char cmd[] = "LIST";
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd);

    if(clients != NULL){
	Client *temp = clients;
        int i=1;
        while(temp->next!=NULL){
            if(temp->status!=0){
                printf("%-5d%-35s%-20s%-8d\n", i++, temp->hostname, temp->client_ip, temp->port);
            }
            temp=temp->next;
        }
        if(temp->status!=0)
            printf("%-5d%-35s%-20s%-8d\n", i, temp->hostname, temp->client_ip, temp->port);

    }
    
    cse4589_print_and_log("[%s:END]\n", cmd);
}

int is_valid_port(char *port){
    string s(port);
    int len = s.length();
    //printf("%d\n", len);
    int i = 0;
    while(i<len-1){
        if(!(isdigit(s[i]))){
            return 0;
        }
        ++i;
    }
    return 1;
}

void login_client(char *cmd, int list_port){

    char cmd_str[] = "LOGIN";
    char *subs;
    char *IP= NULL;
    char *port;

    subs = strtok (cmd," ");
    int i=0;
    
    //int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

   while (subs != NULL)
    {
        //printf ("%s\n",subs);
        subs = strtok (NULL, " ");
        ++i;
	if(i==1){
            IP=subs;
	    //printf("IP = %s\n",IP);
        }
	else if(i==2){
	    port = subs;
	    break;
        }
    }
    /*if(is_valid_ip(IP)==0){
        printf("IP Error\n");
	cse4589_print_and_log("[%s:ERROR]\n", cmd_str);
	cse4589_print_and_log("[%s:END]\n", cmd_str);
        return;
    }
    if(is_valid_port(port)==0){
        
	printf("Port Error\n");
	cse4589_print_and_log("[%s:ERROR]\n", cmd_str);
	cse4589_print_and_log("[%s:END]\n", cmd_str);
        return;
    }*/
    strcpy(global_server_port, port);
    //printf("\nLOGIN = global_server_port = %s", global_server_port);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    unsigned int Server_port = atoi(port);
    server_addr.sin_port = htons(Server_port);
    inet_pton(AF_INET, IP, &server_addr.sin_addr);
    
    if(connect(global_client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	perror("Connect failed");
    
    FD_SET(global_client_socket, &master_list);
    //return client_socket;
    
    stringstream strs;
    strs << list_port;
    string temp_str = strs.str();
    string temp_str2 = "ClientPort ";
    strcat((char *)temp_str2.c_str(), (char *)temp_str.c_str());
    char* list_port_char = (char*) temp_str2.c_str();
    //char* msg = "";

    if(send(global_client_socket, list_port_char, strlen(list_port_char), 0) != strlen(list_port_char))
        perror("Error while sending message to server.");
   
    
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd_str);
    cse4589_print_and_log("[%s:END]\n", cmd_str);
    
}

void getStatsServer(Client *clients){
    Client *temp = clients;
    int i=1;
    char cmd_str[] = "STATISTICS";
    cse4589_print_and_log("[%s:SUCCESS]\n", cmd_str);
    if(clients!=NULL){
	char status[20];    
   	while(temp!=NULL){
	    if(temp->status == 1){
		strcpy(status, "logged-in");
	    }
	    else if(temp->status == 0){
	    	strcpy(status, "logged-out");
	    }
	    printf("%-5d%-35s%-8d%-8d%-8s\n", i, temp->hostname, temp->msg_sent, temp->msg_receive, status);
	    temp=temp->next;
	    i=i+1;
        }

    }
    cse4589_print_and_log("[%s:END]\n", cmd_str);
}

void block_client(char *cmd, Client **clients, int my_sock){
    char *subs;
    char*IP;

    subs = strtok (cmd," ");
    int i=0;
    
   while (subs != NULL)
    {
        //printf ("%s\n",subs);
        subs = strtok (NULL, " ");
        ++i;
	if(i==1){
            IP=subs;
	    //printf("IP = %s\n",IP);
	    break;
        }
    }

    Client *temp = *clients;
    while(temp->socket_desc!=my_sock && temp!=NULL){

	temp=temp->next;
    }

    if(temp->socket_desc == my_sock){
	if(strcmp(IP, "128.205.36.46\n")==0){
	    //printf("Blocking IP = %s\n", IP);
	    temp->block[0] = 1;
	}
	else if(strcmp(IP, "128.205.36.35\n")==0){
	    //printf("Blocking IP = %s\n", IP);
	    temp->block[1] = 1;
	}
	else if(strcmp(IP, "128.205.36.33\n")==0){
	    //printf("Blocking IP = %s\n", IP);
	    temp->block[2] = 1;
	}
	else if(strcmp(IP, "128.205.36.34\n")==0){
	    //printf("Blocking IP = %s\n", IP);
	    temp->block[3] = 1;
	}

    }
    
}

void unblock_client(char *cmd, Client **clients, int my_sock){
    char *subs;
    char*IP;

    subs = strtok (cmd," ");
    int i=0;
    
   while (subs != NULL)
    {
        //printf ("%s\n",subs);
        subs = strtok (NULL, " ");
        ++i;
	if(i==1){
            IP=subs;
	    //printf("IP = %s\n",IP);
	    break;
        }
    }

    Client *temp = *clients;
    while(temp->socket_desc!=my_sock && temp!=NULL){

	temp=temp->next;
    }

    if(temp->socket_desc == my_sock){
	if(strcmp(IP, "128.205.36.46\n")==0){
	    temp->block[0] = 0;
	}
	else if(strcmp(IP, "128.205.36.35\n")==0){
	    temp->block[1] = 0;
	}
	else if(strcmp(IP, "128.205.36.33\n")==0){
	    temp->block[2] = 0;
	}
	else if(strcmp(IP, "128.205.36.34\n")==0){
	    temp->block[3] = 0;
	}

    }

    
}

void server_cmds(char *cmd, Client *clients){
   if(strstr(cmd, "STATISTICS")!=NULL){
	getStatsServer(clients);
   } 
}

void client_cmds(char *cmd, int port){

   int server_socket;
   if(strstr(cmd, "LOGIN")!=NULL){
	login_client(cmd, port);
   } 
   else if(strstr(cmd, "SEND")!=NULL){
        send_message_client(cmd);
    }
   else if(strstr(cmd, "BROADCAST")!=NULL){
	send_message_client(cmd);
    }
    else if(strcmp(cmd, "REFRESH\n")==0){
	char cmd_str[] = "REFRESH";
	cse4589_print_and_log("[%s:SUCCESS]\n", cmd_str);
	strcpy(cmd, "REFRESH");
	send_message_client(cmd); 
	cse4589_print_and_log("[%s:END]\n", cmd_str);

    }
    else if(strstr(cmd, "BLOCK")!=NULL){
	send_message_client(cmd);
        char cmd_str[] = "BLOCK";
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd_str);
        cse4589_print_and_log("[%s:END]\n", cmd_str);
    }
    else if(strstr(cmd, "UNBLOCK")!=NULL){
	send_message_client(cmd);
        char cmd_str[] = "UNBLOCK";
        cse4589_print_and_log("[%s:SUCCESS]\n", cmd_str);
        cse4589_print_and_log("[%s:END]\n", cmd_str);
    } 
    
}

void client_and_server_cmds(char *cmd, char* type, int server_socket, Client *clients, int port){
    if(strcmp(cmd, "AUTHOR\n")==0){
        strcpy(cmd, "AUTHOR");
        callAuthor(cmd);
    }
    else if(strcmp(cmd, "IP\n")==0){
        strcpy(cmd, "IP");
        getIP(cmd);
    }
    else if(strcmp(cmd, "LIST\n")==0){
        if(strcmp(type, "c")==0){
            displayClientList();
        }
        else if(strcmp(type, "s")==0){
            server_list(clients);
        }
    }
    else if(strcmp(cmd, "PORT\n")==0){
	strcpy(cmd, "PORT");
	getPORT(port, cmd);
    }
    
}

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
    /*Init. Logger*/
    cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));

    if(argc != 3) {
        printf("Usage:%s [port]\n", argv[0]);
        exit(-1);
    }
    
    Client *clients = NULL;
    
    int port, server_socket, head_socket, selret, sock_index, fdaccept=0;
    char client_ip[INET_ADDRSTRLEN];
    unsigned int caddr_len;
    struct sockaddr_in server_addr, client_addr;
    
    
    /* Socket */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0)
        perror("Cannot create socket");
    
    /* Fill up sockaddr_in struct */
    port = atoi(argv[2]);
    
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
        
    /* Listen */
    if(listen(server_socket, BACKLOG) < 0)
        perror("Unable to listen on port");
        
    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
    
    /* Register the listening socket */
    FD_SET(server_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);
    
    head_socket = server_socket;

    while(TRUE){
	memcpy(&watch_list, &master_list, sizeof(master_list));
        
        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if(selret < 0)
            perror("select failed.");

	/* Check if we have sockets/STDIN to process */
        if(selret > 0){
	     /* Loop through socket descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){
		if(FD_ISSET(sock_index, &watch_list)){
                    if(sock_index == STDIN){
                        char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);
                        
                        memset(cmd, '\0', CMD_SIZE);
                        if(fgets(cmd, CMD_SIZE-1, stdin) == NULL) //Mind the newline character that will be written to cmd
                            exit(-1);
                        
                        client_and_server_cmds(cmd, argv[1], server_socket, clients, port);
                        if((strcmp(argv[1], "c"))==0){
			    if(strcmp(cmd, "EXIT\n")==0){
        			return 0;
    			    }
                            client_cmds(cmd, port);
                        }
                        else if((strcmp(argv[1], "s"))==0){
                            server_cmds(cmd, clients);
                        }
                        
                        free(cmd);
                    }
		    /* Check if new client is requesting connection */
                    else if((sock_index == server_socket)){
                        caddr_len = sizeof(client_addr);
                        
                        fdaccept = accept(server_socket, (struct sockaddr *) &client_addr, &caddr_len);
                        if(fdaccept < 0){
                            perror("Accept failed.");
                        }
                        else{
			    
                            inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                            
			    //printf("PORT = %u", htons(client_addr.sin_port));
                            //unsigned int ip_address = (client_addr.sin_addr.s_addr);
                            AddClient(ntohs(client_addr.sin_port), &clients, client_ip, fdaccept, client_addr);
                        }
                        
                        //printf("\nRemote Host connected!\n");
                        
                        /* Add to watched socket list */
                        FD_SET(fdaccept, &master_list);
                        if(fdaccept > head_socket)
                            head_socket = fdaccept;
                    }
		    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);
                        
                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){
                            close(sock_index);
                            //printf("Remote Host terminated connection!\n");
                            
                            Client *temp = clients;
                            while((temp->next!=NULL)&&(temp->port!=client_addr.sin_port)){
                                temp = temp->next;
                            }
                            temp->status = 0;
                            
                            //printf("Client Port: %d\n", temp->port);
                            //printf("Status: logged-out\n");
                            /* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                            //Process incoming data from existing clients here ...
                            //printf("\nMessage Received: %s\n", buffer);
			    if(strstr(buffer, "ACK")!=NULL){
				if(strstr(buffer, "END")!=NULL){
					ackreceive(buffer);
				}
				else{	
					char *buffer_list = (char*) malloc(sizeof(char)*BUFFER_SIZE);
					int i=0;
				
					while(1){
						char *buffer_temp = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        			memset(buffer_temp, '\0', BUFFER_SIZE);
					
						if(recv(sock_index, buffer_temp, BUFFER_SIZE, 0) <= 0){
							printf("Remote Host unable to send data!\n");
						}
						else{
							//printf("Buffer : %s\n", buffer_temp);
							if(strstr(buffer_temp, "END")!=NULL){
								if(i==0){
									strcpy(buffer_list, buffer_temp); i=1;
								}
								else if(i==1){
									strcat(buffer_list, buffer_temp);
								}
								break;
							}
						}
						if(i==0){
							strcpy(buffer_list, buffer_temp); i=1;
						}
						else if(i==1){
							strcat(buffer_list, buffer_temp);
						}
					
					}
					ackreceive(buffer_list);
					//ackreceive(port);
					free(buffer_list);
				}
			    }
                            else if(strstr(buffer, "ClientPort")!=NULL){
				add_port_list(fdaccept, &clients, buffer);
			    }
			    else if(strstr(buffer, "SEND")!=NULL){
                                send_message_server(buffer, server_socket, &clients, sock_index);
                            }
			    else if(strstr(buffer, "BROADCAST")!=NULL){
			        broadcast_message_server(buffer, client_ip, clients);
			    }
			    else if(strcmp(buffer, "REFRESH")==0){
				Client *temp = clients;
				int my_port;
				while((temp!=NULL)&&(temp->socket_desc!=sock_index)){
			    	    temp = temp->next;
    				}
				my_port = temp->port;
				stringstream strs;
    				strs << my_port;
    				string temp_str = strs.str();
				char *temp_port = new char[temp_str.length() + 1];
				strcpy(temp_port, temp_str.c_str());
				
				sendListToClient(clients, temp_port, sock_index);

			    }
			    else if(strstr(buffer, "UNBLOCK")!=NULL){
				unblock_client(buffer, &clients, sock_index);
			    }
			    else if(strstr(buffer, "BLOCK")!=NULL){
				block_client(buffer, &clients, sock_index);
			    }
                            
                            fflush(stdout);
                        }
                        
                        free(buffer);
                    }
		}
	    }
	}
    }
  

	return 0;
}





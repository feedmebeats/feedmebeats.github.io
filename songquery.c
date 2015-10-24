#include <stdio.h>
#include <stdlib.h>
#include <strings.h> //only for bzero()
#include <string.h> //string functions
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> //to use struct hostent * s, s->h_name - to get official Hostname
#include <arpa/inet.h> //to use inet_pton(AF_INET,char * IP,&struct in_addr) - finds the IP and fills the structure to be able to get struct hostent * = gethostbyaddr(&struct in_addr, sizeof(struct in_addr),AF_INET) 

#define START_HTML "<!DOCTYPE html>\n<html>\n<head>\n<title>"
#define BODY_HTML "</title>\n</head>\n<body>\n"
#define END_HTML "\n</body>\n</html>"
//#define MY_HTML "<h1>Welcome to Arturo's Website</h1>\n<p>My name is Arturo Parrales Salinas</p>\n<p>I love snowboarding and soccer</p>"
#define MY_HTML "<!DOCTYPE html>\n<html>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>\n<title>MUSIC LOVERS</title>\n<body>\n\n<p><h2>Recommend music all over the world!</h2> </br><em>Please only input the name of the song in keyword input</em></p>\n<form action=\"http://127.0.0.1:2000/search.html\" method=\"get\">\n<p>Keyword: <input type=\"text\" name=\"keyword\" /></p>\n<input type=\"submit\" value=\"Let's play Music!\" />\n</p>\n</form>\n</body>\n</html>"


#define LINK_ABOUT "<a href=/about.html> My about page </a>" 
#define LINK_ROOT "<a href=/> List of Browsers </a>"

#define BUFFER_SIZE 356


void error(char *message)
{
	perror(message);
	exit(1);
}

void writeToSocket(int socketfd, const char * data)
{
	if(write(socketfd,data,strlen(data))<0)
		error("ERROR writing to socket");
}


void putFile(char * query_str)
{
	FILE * fd;
	fd = fopen("song.txt","w+");
	fprintf(fd,query_str);
	fclose(fd);
	
}

//gets the OBJECT requested by the client 
// "/" returns 1
// "/about.html" returns 0
int getreqobject(char * html_req)
{
	char * keyword = "/search.html?keyword=";
	char * search = "/search.html";
	char * slash = "/";
	char * object;
	int search_html = 0, str_store=1;
	object = strstr(html_req,slash);
	object = strtok(object," ");
	printf("\n\nOBJECT: %s\n\n",object);

	
	if(object == NULL){
		bzero(object,strlen(object)+1);
		return -1;
	}
	else if(strcmp(object,search)==0){//if strings equal returns 0 = FALSE
		bzero(object,strlen(object)+1);
		return search_html;
	}
	else{
		
		char * cut_object = (char*)malloc(strlen(keyword)+1);
		bzero(cut_object,strlen(keyword)+1);
		memcpy(cut_object, object, strlen(keyword));
		printf("HERE--------> %s\n",cut_object);
		if (strcmp(cut_object,keyword)==0){//!strcmp(object,slash))
			char * query_str = (char*)malloc(strlen(object)-strlen(keyword)+1);
			memcpy(query_str, object+strlen(cut_object), strlen(object)-strlen(keyword)+1);
			printf("KEYWORD %s\n",query_str);
			putFile(query_str);
			bzero(object,strlen(object)+1);
			return str_store;
		}
	}
	return -1;//no object found
}



//MAIN
int main (int argc, char ** argv)
{


	int sockfd, newsockfd; //socket descriptors
	int portno; //port number
	int n; //read and writte bytes
	socklen_t clilen;// address of the client
	char buffer[2*BUFFER_SIZE];
	char buffer1[BUFFER_SIZE];
	struct sockaddr_in serv_addr, cli_addr; // server and client structures

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\nUsage: %s PORT\n",argv[0]);
		exit (1);
	}
		
	//create-set up a socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));//set to zeros serv_addr

	//set all values and Binding
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);//to make sure bytes are in big endian config
	
	if (bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	
	//Listen
	listen(sockfd,5);//always must be 5
	clilen = sizeof(cli_addr);
	int count=0;
	//recursive server can start here
	while(1){//to make it finite -> count<=14){
	//accept - gives new socket descriptor
	
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if ( newsockfd < 0)
			error("ERROR on accept");
	
	//communication established
		bzero(buffer1,BUFFER_SIZE);
		bzero(buffer,2*BUFFER_SIZE);
		n = read(newsockfd,buffer1,BUFFER_SIZE-1);
		if (n<0) 
			error("ERROR reading from socket");
	
		//printf("Here is the message: %s\n", buffer1);
	
	
		//printf("Here is the message filtered: %s\n", buffer);

		char * buffer_copy1 = strdup(buffer1);

		//HTTP RESPONSE
		char * response_http = "HTTP/1.1 200 OK\nContent-type: text/html\n\n\n";
		//writeToSocket(newsockfd,response_http);
	
		//OBJECT REQUESTED
		
		
		int robject = getreqobject(buffer_copy1);
		free(buffer_copy1);
		//printf("%d\n", robject);	
	
		
		//object requested is robject
		//fprintf(stderr,"\nnumber: %d",robject);	
		if(!robject){
		
			//I WILL WRITE SEARCH PAGE
			writeToSocket(newsockfd,response_http);
			
			//LINK_ROOT = "<a href=\"/\"> My about page </a>";
			
			//writeToSocket(newsockfd,START_HTML);	
			//writeToSocket(newsockfd,BODY_HTML);
			writeToSocket(newsockfd,MY_HTML);
			//writeToSocket(newsockfd,LINK_ROOT);
			//writeToSocket(newsockfd,END_HTML);
			




		}
		else if(robject==1){
			//we got a query string
	
			writeToSocket(newsockfd,response_http);
			//char * hello_html ="<h1> List of Users and Browsers (most recent to oldest) </h1>";
			char * hello_html = "<a href=\"/search.html\"> Thank You Music Lover! Recommend Again! </a>";
			//LINK_ABOUT = "<a href=\"/about.html\"> List of Browsers </a>";			
			
			writeToSocket(newsockfd,START_HTML);
			writeToSocket(newsockfd,BODY_HTML);
			writeToSocket(newsockfd,hello_html);
			//writeToSocket(newsockfd,LINK_ABOUT);
			

			writeToSocket(newsockfd,END_HTML);
			
			//count++;//to keep adding elements to my structure
		

		}//end if

		
	close(newsockfd);

	}//end while
	
	close(sockfd);
	
	
	return (0);
}

/*
Name- Kushagra Gupta
Student ID-804729
login id- kushagrag
*/
#include<unistd.h>
#include<stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <header.h>


#define MYPORT "8001"
char * ROOT;

//p is the pointer to socket being created for client.
void *connection(void *p) {
  FILE *read_file; // stores the pointer to file being requested
  char recv_buf[1024]; // stores the http request
  char send_buf_temp[1024]; //stores the http response
  char file_name[99999]; //stores the final path
  char *file_data; //stores the data of the file
  char errormsg[69];
  int recv_len; //stores the length of the recv_buf
  int file_size;//stores the data of the file_size
  int temp;

  recv_len = 0;
  file_size = 0;

  struct tm *ptr;
  time_t local;
  char times[30];
  int *connfd_thread = (int *)p;	/*type cast the argument for use
  in this thread*/
  //read the request from the client
  temp = (recv (*connfd_thread, recv_buf, sizeof(recv_buf), 0));
  //check for error when receiving
  if (temp < 0){
    perror("ERROR reading from socket");
    exit(1);
  }
  recv_buf[temp-2] = '\0';
  //check if there is a valid get request, if not send an error message
  if ((strncmp(recv_buf, "GET ", 4)) != 0) {
    strcpy(errormsg,
      "Invalid Command Entered\nPlease Use The Format: GET <file_name.html>\n");
      send(*connfd_thread, errormsg, 69, 0);
    }
    //else get the name of the file
    else {
      recv_len = strlen(recv_buf);

      int i;
      int j = 0;
      for (i=4; i<recv_len ;i++, j++) {
        //If the end of the file path is reached, break.
        if ( (recv_buf[i] == '\0') || (recv_buf[i] == '\n')
        || (recv_buf[i] == ' ') ){
          break;
        }
        else {
          file_name[j] = recv_buf[i];	//copy the file name
        }
      }
      file_name[j] = '\0';
      char data[999];
      //copy the path and the file name into file_name
      strcat(data,ROOT);
      strcat(data,file_name);
      strcpy(file_name,data);
      read_file = fopen(file_name, "r");					//open the file

      time(&local);                           //Get the current time and date
      ptr = localtime(&local);
      strftime(times, 30, "%a, %d %b %Y %X %Z", ptr);


      //if the file doesn't exist then send a 404 response
      if (read_file == NULL) {


        send(*connfd_thread, "HTTP/1.1 404 Not Found\n\n", 24, 0);

      }

      else{
        printf("\nread %s\n",file_name);

        if(strstr(file_name, ".html"))
        {
          //format and create string response for output to client
          sprintf(send_buf_temp, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: text/html\n\n", times, file_size);
        }

        else if(strstr(file_name, ".css"))
        {
          //format and create string response for output to client
          sprintf(send_buf_temp, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: text/css\n\n", times, file_size);

        }

        else if(strstr(file_name, ".jpg")|| strstr(file_name, ".jpeg"))
        {
          //format and create string response for output to client
          sprintf(send_buf_temp, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: image/jpeg\r\n\r\n", times, file_size);

        }

        else if(strstr(file_name, ".js"))
        {
          //format and create string response for output to client
          sprintf(send_buf_temp, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: text/javascript\n\n", times, file_size);

        }
        else if(strstr(file_name, ".mp4"))
        {

          sprintf(send_buf_temp, "HTTP/1.1 200 OK\nDate: %s\nContent Length: %d\nConnection: close\nContent-Type: video/*\n\n", times, file_size);

        }
        fseek(read_file, 0L, SEEK_END);	//seek to the end of the file
        file_size = ftell(read_file);	//the size of the file
        fseek(read_file, 0L, SEEK_SET);//seek back to the begining
                                      // of the file
        file_data = (char *)malloc(file_size + 1);//allocate memory
                                                  //for the file data

        //send the info to the client
        send(*connfd_thread, send_buf_temp, strlen(send_buf_temp), 0);
        //read the file data into a string
        int read_value=fread(file_data,  (file_size),1, read_file);
        if (read_value < 0) {
          perror("ERROR on reading");
          exit(1);
        }
        file_data[file_size] = '\0';
        int write_value=write(*connfd_thread,file_data,file_size);
        if (write_value < 0) {
          perror("ERROR on writing");
          exit(1);
        }
        free(file_data);		//free the allocated memory for file_data
        fclose(read_file);
      }
    }
    close(*connfd_thread);												//close the current connection
    pthread_exit(NULL);													//exit the pthread with null
  }

  int main(int argc, char *argv[]) {
    int sockfd;
    int portno;
    int connfd[100]; //This will contain the socket file descriptor for the
    //different connections
    pthread_t threads[100]; //100 posible threads, just to be safe
    int thread_count = 0;
    struct sockaddr_in serv_addr;
    ROOT = getenv("PWD"); //The root is initialized to the present working
                          //directory

    if (argc < 2)
    {
      fprintf(stderr,"ERROR, no port provided\n");
      exit(1);
    }
    printf("%s",argv[1]);

    if(argc==3){
      strcpy(ROOT,argv[2]);
    }

    printf("Server started at port no. %s%s%s with root directory as %s%s%s\n",
    "\033[92m",argv[1],"\033[0m","\033[92m",ROOT,"\033[0m");

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("ERROR opening socket");
      exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); //getting the port no. from the arguments

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);  // store in machine-neutral format

    //bind address to socket
    if( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
    }
    //loop to count the number of connections/threads
    while(thread_count < 100){
      if( listen(sockfd, 10) != 0) {
        perror("ERROR on listing");
        exit(1);
      }

      connfd[thread_count] = accept(sockfd, NULL, NULL);/*accept gives the new
      socket file descriptor for new clinet connected.*/

      if (connfd[thread_count] < 0) {
        perror("ERROR on accept");
        exit(1);
      }
      //create a thread and receive data
      pthread_create(&threads[thread_count], NULL, connection, &connfd[thread_count]);
      pthread_detach(threads[thread_count]);
      thread_count++;
    }
    close(sockfd);	//close the socket
    return 0;
  }

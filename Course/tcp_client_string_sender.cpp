#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define DEST_PORT            40000
#define SERVER_IP_ADDRESS   "127.0.0.1"

#define SRC_PORT	     40010
#define LOCAL_IP_ADDRESS "127.0.0.1"

void
setup_tcp_communication(){

    /*Step 1 : Initialization*/
    /*Socket handle*/
    int sockfd = 0, 
        sent_recv_bytes = 0;

    socklen_t addr_len = 0;

    addr_len = sizeof(struct sockaddr);

    /*to store socket addesses : ip address and port*/
    struct sockaddr_in dest;

    /*Step 2: specify server information*/
    /*Ipv4 sockets, Other values are IPv6*/
    dest.sin_family = AF_INET;

    /*Client wants to send data to server process which is running on server machine, and listening on 
     * port on DEST_PORT, server IP address SERVER_IP_ADDRESS.
     * Inform client about which server to send data to : All we need is port number, and server ip address. Pls note that
     * there can be many processes running on the server listening on different no of ports, 
     * our client is interested in sending data to server process which is lisetning on PORT = DEST_PORT*/ 
    dest.sin_port = htons(DEST_PORT);
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    /*Step 3 : Create a TCP socket*/
    /*Create a socket finally. socket() is a system call, which asks for three parameters*/
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#if 0
    /*to specify the client IP Address and Port no*/
    struct sockaddr_in localaddr;
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDRESS);
    localaddr.sin_port = htons(SRC_PORT);  // Any local port will do
    bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
#endif
    printf("Connecting to Server\n");
    int rc = connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    if (rc == 0) {
    	printf("connected\n");
    }
    else {
    	printf("connection failed, error no %d\n", errno);
    	exit(0);
    }

    /*Step 4 : get the data to be sent to server*/
    /*Our client is now ready to send data to server. sendto() sends data to Server*/

    const char *msg1 = "Hello Abhishek, ";
    const char *msg2 = "how are you";
    int len1 = strlen (msg1);
    int len2 = strlen (msg2);

    while (1) {
    //usleep(1000);

    /*step 5 : send the data to server*/
    sent_recv_bytes = sendto(sockfd, 
           msg1,
           len1,
           0, 
           (struct sockaddr *)&dest, 
           sizeof(struct sockaddr));
 
    printf("No of bytes sent = %d\n", sent_recv_bytes);
    
    sent_recv_bytes = sendto(sockfd, 
           msg2,
           len2,
           0, 
           (struct sockaddr *)&dest, 
           sizeof(struct sockaddr));
    
    printf("No of bytes sent = %d\n", sent_recv_bytes);
    }
}
    

int
main(int argc, char **argv){

    setup_tcp_communication();
    printf("application quits\n");
    return 0;
}

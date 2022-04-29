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

#pragma pack (push,1)
typedef struct _test_struct{

    uint16_t msg_size;
    unsigned int a;
    unsigned int b;
} test_struct_t;
#pragma pack(pop)

typedef struct result_struct_{

    unsigned int c;

} result_struct_t;

test_struct_t client_data[2];
result_struct_t result;

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

PROMPT_USER:

    /*prompt the user to enter data*/
    printf("Enter a1 : ?\n");
    scanf("%u", &client_data[0].a);
    printf("Enter b1 : ?\n");
    scanf("%u", &client_data[0].b);
    client_data[0].msg_size = sizeof(client_data[0]);

    printf("Enter a2 : ?\n");
    scanf("%u", &client_data[1].a);
    printf("Enter b2 : ?\n");
    scanf("%u", &client_data[1].b);
    client_data[1].msg_size = sizeof(client_data[1]);

	if(client_data[0].a == 0 && client_data[0].b == 0) {
		close(sockfd);
		exit(0);
	}
    
    /*step 5 : send the data to server*/
    sent_recv_bytes = sendto(sockfd, 
           client_data,
           sizeof(client_data), 
           0, 
           (struct sockaddr *)&dest, 
           sizeof(struct sockaddr));
    
    printf("No of bytes sent = %d\n", sent_recv_bytes);
  
    /*Step 6 : Client also want to reply from server after sending data*/
    
    /*recvfrom is a blocking system call, meaning the client program will not run past this point
     * until the data arrives on the socket from server*/
    printf("Waiting for response:\n");
    sent_recv_bytes =  recvfrom(sockfd, (char *)&result, sizeof(result_struct_t), 0,
                    (struct sockaddr *)&dest, &addr_len);

    printf("No of bytes recvd = %d\n", sent_recv_bytes);
    
    printf("Result recvd = %u\n", result.c);
    /*Step 7: Client would want to send the data again to the server, go into infinite loop*/
    goto PROMPT_USER;
}
    

int
main(int argc, char **argv){

    setup_tcp_communication();
    printf("application quits\n");
    return 0;
}

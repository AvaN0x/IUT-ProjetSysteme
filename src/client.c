#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 6000
#define ADDRESS "127.0.0.1"

int main()
{
    // Get the socket
    int fdSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (fdSocket < 0)
    {
        printf("Incorrect socket\n");
        exit(EXIT_FAILURE);
    }

    char buffer[100];
    struct sockaddr_in serverCoords;
    memset(&serverCoords, 0x00, sizeof(struct sockaddr_in)); // allocate memory
    serverCoords.sin_family = PF_INET;                       // Set protocal family
    inet_aton(ADDRESS, &serverCoords.sin_addr);              // put server address to our struct
    serverCoords.sin_port = htons(PORT);                     // set address port

    if (connect(fdSocket, (struct sockaddr *)&serverCoords, sizeof(serverCoords)) == -1)
    {
        printf("Connection failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected to %s:%d\n", ADDRESS, PORT);

    //! temporary loop to chat 2 times with the server
    for (int i = 0; i < 2; i++)
    {
        printf("Enter some text : ");
        fgets(buffer, 100, stdin);
        send(fdSocket, buffer, strlen(buffer), 0); // send buffer to server
        int nbRecu = recv(fdSocket, buffer, 99, 0);
        if (nbRecu > 0)
        {
            buffer[nbRecu] = 0; // set last char of the buffer
            printf("Received : %s\n", buffer);
        }
    }

    close(fdSocket);
    return EXIT_SUCCESS;
}

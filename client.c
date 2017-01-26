#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SERVER "137.112.38.47"
#define MESSAGE "hello there"
#define PORT 1874
#define BUFSIZE 1024
#define SRCPORT 2742

char readRHPMessage(char* message, int size);
char readRHMPMessage(char* message, int size);
char checkRHPMessage(char* message, int size);

int main() {
  int clientSocket, nBytes;
  uint8_t buffer[BUFSIZE];
  struct sockaddr_in clientAddr, serverAddr;

  /*Create UDP socket*/
  if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket");
    return 0;
  }

  /* Bind to an arbitrary return address.
   * Because this is the client side, we don't care about the address 
   * since no application will initiate communication here - it will 
   * just send responses 
   * INADDR_ANY is the IP address and 0 is the port (allow OS to select port) 
   * htonl converts a long integer (e.g. address) to a network representation 
   * htons converts a short integer (e.g. port) to a network representation */
  memset((char *) &clientAddr, 0, sizeof (clientAddr));
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  clientAddr.sin_port = htons(SRCPORT);

  if (bind(clientSocket, (struct sockaddr *) &clientAddr, sizeof (clientAddr)) < 0) {
    perror("bind failed");
    return 0;
  }
  printf("%X\n", 'h');
  /* Configure settings in server address struct */
  memset((char*) &serverAddr, 0, sizeof (serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /* send a message to the server */
  uint8_t message[] = {0x01, 0x05, 0x00, 0xb6, 0x0a, 0x6f, 0x6c, 0x6c, 0x65, 0x68, 0x22, 0x01};

//    memset(&message,'\0', sizeof(message));
//    message[0] = 0x68;
  printf("%d\n", strlen(message));
  printf("%X\n", message[3]);

  if (sendto(clientSocket, message, sizeof message, 0,
    (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
    perror("sendto failed");
  return 0;
  }

  printf("%s\n", "here");
  memset(buffer, 0, BUFSIZE);
  /* Receive message from server */
  nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);

  printf("Received from server: %X\n", buffer);

  for(int i = 0; i < 46; i++){
    printf("%X, ", buffer[i]);
  }
  printf("\n");
  for(int i = 5; i < BUFSIZE; i++){
    printf("%c", buffer[i]);
  }
  printf("\n");

  readRHPMessage(message, sizeof message);

  readRHPMessage(buffer, sizeof buffer);

  close(clientSocket);
  return 0;
}

char readRHPMessage(char* message, int size) {
  printf("%d\n", size);
  if (!checkRHPMessage(message, size)) {
    printf("%s\n", "Checksum mismatch!");
    return 0;
  }
  printf("%s\n", "Checksum passed");
  /*
  uint16_t temp, length;
  printf("RHP type: %s\n", message[0]);
  temp = (message[1] << 8) + message[2];
  if (message[0]) {
    printf("RHP length: %d\n", temp);
    length = temp;
  } else {
    printf("dstPort: %s\n", htons(temp));
  }
  temp = ((uint16_t)message[3] << 8) + message[4];
  printf("srcPort: %d\n", temp);
  if (message[0]) {
    printf("(%.*s)\n", length, message[5]);
  } else {
    char rhmp[size - 7];
    memcpy(rhmp, message[5], size - 7);
    readRHMPMessage(rhmp, size);
  }*/
  return 1;
}

char readRHMPMessage(char* message, int size) {
  uint16_t temp;
  printf("RHMP type: %s\n", message[0] >> 2);
  temp = (((uint16_t)(message[0] & 0b11)) << 8) + message[1];
  printf("commID: %d\n", temp);
  printf("RHMP message: (%.*s)\n", message[2], message[3]);
  return 1;
}

char checkRHPMessage(char* message, int size) {
  uint16_t sum = 0;
  uint16_t temp = 0;
  printf("%d\n", size);
  for (uint16_t i = 0; i < size; i += 2) {
    temp = ((uint16_t)message[i+1] << 8) + message[i];

    sum += temp;
//    printf("%X, %X, i = %d\n", temp, sum, i);
    if (sum < temp) {
      sum++;
    }
  }
  return (sum == 0xFFFF);
}

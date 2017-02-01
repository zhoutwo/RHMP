#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SERVER "137.112.38.47"
#define MESSAGE "hello there"
#define PORT 1874
#define BUFSIZE 1024
#define SRCPORT 2742

char readRHPMessage(char* message, int size, char attempt);
char readRHMPMessage(char* message, char attempt);
char checkRHPMessage(char* message, int size);

int main() {
  int clientSocket, nBytes;
  
  uint8_t index;
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
  /* Configure settings in server address struct */
  memset((char*) &serverAddr, 0, sizeof (serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /* send a message to the server */
//  uint8_t message[] = {0x01, 0x05, 0x00, 0xb6, 0x0a, 0x6f, 0x6c, 0x6c, 0x65, 0x68, 0x22, 0x01};
  uint8_t message1[] = {0x01, 0x05, 0x00, 0xb6, 0x0a, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x22, 0x01};
  //message2
  uint8_t message2[] = {0x00, 0x69, 0x00, 0xb6, 0x0a, 0x08, 0x4e, 0x00, 0xa6, 0xd8};
  uint8_t message3[] = {0x00, 0x69, 0x00, 0xb6, 0x0a, 0x02, 0x4e, 0x00, 0xa6, 0xde};


//    memset(&message,'\0', sizeof(message));
//    message[0] = 0x68;
  for (index = 1; index < 4; index++) {
    uint8_t buffer[BUFSIZE];
    memset(buffer, 0, BUFSIZE);
    printf("Communication No. %u:\n", index);
    if (index == 1) {
      if (sendto(clientSocket, message1, sizeof message1, 0,
        (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
      }
    } else if (index == 2) {
      if (sendto(clientSocket, message2, sizeof message2, 0,
        (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
      }
    } else {
      if (sendto(clientSocket, message3, sizeof message3, 0,
        (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
      }
    }
    
    /* Receive message from server */
    nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);
    if (!readRHPMessage(buffer, BUFSIZE, index)) {
      index--;
    }
    printf("%s", "\n");
  }
  close(clientSocket);
  return 0;
}

char readRHPMessage(char* message, int size, char attempt) {
  if (!checkRHPMessage(message, size)) {
    printf("%s\n", "Checksum mismatch!");
    return 0; 
  }
  printf("RHP type: %u\n", message[0]);
  uint8_t type = message[0];
  uint16_t field2 = (uint8_t) message[1] + (((uint8_t) message[2]) << 8);
  if (type) {
    printf("length: %u\n", field2);
  } else {
    printf("dstPort: %u\n", field2);
  }
  uint16_t srcPort = (uint8_t) message[3] + (((uint8_t) message[4]) << 8);
  printf("srcPort: %u\n", srcPort);
  if (type) {
    for (int i = 0; i < field2; i++) {
      printf("%c", message[5 + i]);
    }
    printf("\n");
  } else {
    return readRHMPMessage(message+5, attempt);
  }
  return 1;
}

char readRHMPMessage(char* message, char attempt) {
  uint8_t type = ((uint8_t) message[0]) & 0b00111111;
  printf("RHMP type: %u\n", type);
  uint16_t commID = (((uint16_t) message[0]) & 0b11) + (((uint8_t) message[1]) << 2);
  printf("commID: %u\n", commID);
  uint8_t length = (uint8_t) message[2];
  printf("length: %u\n", length);
  if (attempt == 2) {
    printf("RHMP message: %.*s\n", length, message+3);
  } else {
    uint32_t payload = 0;
    for (uint8_t i = 0; i < length; i++) {
      payload += (((uint32_t) message[i+3]) << (i * 8));
    }
    printf("RHMP message: %u\n", payload);
  }
  return 1;
}

char checkRHPMessage(char* message, int size) {
  uint16_t sum = 0;
  uint16_t temp = 0;
  for (uint16_t i = 0; i < size; i += 2) {
    temp = ((uint16_t) message[i+1] << 8) + (uint8_t) message[i];
    sum += temp;
    if (sum < temp) {
      sum++;
    }
  }
  return (sum == 0xFFFF);
}

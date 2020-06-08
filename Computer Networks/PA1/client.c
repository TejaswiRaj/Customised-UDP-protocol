#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define PORT 32033
#define TIMEOUT 3

//Primitives
#define PACKET_ID 0XFFFF
#define CLIENT_ID 0XFF
#define ENDPACKET_ID 0XFFFF

//Packet Types
#define DATA 0XFFF1
#define ACKPACKET 0XFFF2
#define REJECTPACKET 0XFFF3

//Reject sub codes
#define LENGTHMISMATCHCODE 0XFFF5
#define ENDOFPACKETMISSINGCODE 0XFFF6
#define OUTOFSEQUENCECODE 0XFFF4
#define DUPLICATECODE 0XFFF7

//Data Packet Format
struct dataPacket{
	
	uint16_t packet_ID;
	uint8_t client_ID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	char payload[255];
	uint16_t endpacket_ID;
};


//create dataPacket with data
struct dataPacket createDataPacket() {
	
	struct dataPacket data;
	data.packet_ID = PACKET_ID;
	data.client_ID = CLIENT_ID;
	data.type = DATA;
	data.endpacket_ID = ENDPACKET_ID;
	
	return data;
}
//REJECT Packet Format
struct rejectPacket {
	
	uint16_t packet_ID;
	uint8_t client_ID;
	uint16_t type;
	uint16_t subcode;
	uint8_t segment_No;
	uint16_t endpacket_ID;
};


//print all the packet details
void printPacketDetails(struct dataPacket data) {
			
	printf("\n INFO: Sending packet:\n");
	printf("  PacketID: %x\n",data.packet_ID);
	printf("  Client id : %hhx\n",data.client_ID);
	printf("  Data: %x\n",data.type);
	printf("  Segment no : %d \n",data.segment_No);
	printf("  Length %d\n",data.length);
	printf("  Payload: %s\n",data.payload);
	printf("  End of data packet id : %x\n",data.endpacket_ID);
}


int main(){
	
	struct dataPacket data;
	struct rejectPacket receivedpacket;
	struct sockaddr_in cliaddr;
	socklen_t addr_size;
	FILE *fp;
	char line[255];
	int sockfd;
	int n = 0;
	int retryCounter = 0;
	int segmentNo = 1;

	sockfd = socket(AF_INET,SOCK_DGRAM,0); //socket(domain, type, protocol), UDP - DGRAM
	if(sockfd < 0) {
		printf("\n ERROR: Socket Failure \n");
	}
	
	//creation of UDP socket
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY); //host to network long address
	cliaddr.sin_port=htons(PORT); //host to network short address
	addr_size = sizeof cliaddr ;

	//socket timeout
	struct timeval timer;
	timer.tv_sec = TIMEOUT;
	timer.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timer,sizeof(struct timeval));
	data = createDataPacket();
	
	//get payload data from txt file
	fp = fopen("sample_input_pa1.txt", "rt");
	if(fp == NULL)
	{
		printf("\n ERROR: File not found \n");
		exit(0);
	}
	
	while(fgets(line, sizeof(line), fp) != NULL) {	
		
		n = 0;
		retryCounter = 0;
		printf("\n");
		data.segment_No = segmentNo;
		strcpy(data.payload,line);
		data.length = strlen(data.payload);
		//create length mismatch packet
		if(segmentNo == 8) {
			data.length++;
		}
		
		//create out of sequence error packet
		if(segmentNo == 10) {
			data.segment_No = data.segment_No + 4;
		}
		
		//create duplicate packet
		if(segmentNo == 7) {
			data.segment_No = 3;
		}
		
		//create enf of packet missing error packet
		if(segmentNo == 9) {
			data.endpacket_ID= 0;
		}
		
		
		if(segmentNo != 9) {
			data.endpacket_ID = ENDPACKET_ID;
		}

		printPacketDetails(data);
		while(n <= 0 && retryCounter < 3) {
			
			//send and receive packets
			sendto(sockfd,&data,sizeof(struct dataPacket),0,(struct sockaddr *)&cliaddr,addr_size);
			n = recvfrom(sockfd,&receivedpacket,sizeof(struct rejectPacket),0,NULL,NULL);

			if(n <= 0 ) {
				printf("\n ERROR: No response from server\n");
				printf("Sending packet again \n");
				retryCounter++;
			}
			
			else if(receivedpacket.type == ACKPACKET  ) {
				printf("\n INFO: ACK packet received \n");
			}
			
			else if(receivedpacket.type == REJECTPACKET ) {
				printf("\n ERROR: REJECT received \n");
				printf("type : %x \n" , receivedpacket.subcode);
				if(receivedpacket.subcode == LENGTHMISMATCHCODE ) {
					printf("Length Mismatch \n");
				}
				else if(receivedpacket.subcode == ENDOFPACKETMISSINGCODE ) {
					printf("End of Packet Missing \n");
				}
				else if(receivedpacket.subcode == OUTOFSEQUENCECODE ) {
					printf("Out of Sequence \n");
				}
				else if(receivedpacket.subcode == DUPLICATECODE) {
					printf("Duplicate Packet \n");
				}
			}
		}
		
		//no ACK received after sending packet 3 times
		if(retryCounter>= 3 ) {
			printf("\n ERROR: Server does not respond \n");
			exit(0);
		}
		segmentNo++;
		printf("\n ---------------------------------------------------------------------- \n");
	}
	fclose(fp);
}


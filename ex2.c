/*=========================================================
        NETWORK LAYERS SIMULATION
        PART 1
=========================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------- CONSTANTS ----------------*/

#define SIZE 10
#define MAXMESSAGE 1000
#define MAXBITS 10000
#define PACKETSIZE 16
#define FRAMESIZE 8
#define FLAG "01111110"
#define ESC  "01111101"

/*--------------- STRUCTURE ----------------*/

struct node
{
    char url[50];
    int ip[4];
    char mac[18];
    int flag;
};

struct node table[SIZE];

/*--------------- GLOBAL VARIABLES ----------------*/

char message[MAXMESSAGE];

char bitStream[MAXBITS];

char packets[500][PACKETSIZE + 1];

char frames[1000][FRAMESIZE + 1];

int totalBits = 0;
int totalPackets = 0;
int totalFrames = 0;

int sourcePort;
int destinationPort;
char originalData[12000];
char bitStuffedData[15000];
char bitDestuffedData[15000];

char byteStuffedData[15000];
char byteDestuffedData[15000];
char protocolName[20];
char protocolBinary[20];
/*================ Hamming =================*/

char hammingData[100];
char hammingCode[100];
char receivedHamming[100];
char originalHamming[100];

int hammingDataBits;
int hammingParityBits;
int hammingTotalBits;

/*================ 2D Parity =================*/

#define ROWS 4
#define COLS 4

int senderMatrix[ROWS+1][COLS+1];
int receiverMatrix[ROWS+1][COLS+1];

void networkLayer(int sourceIndex, int destinationIndex);
void dataLinkLayer(int sourceIndex, int destinationIndex);
void byteStuff(char input[], char output[]);
void bitStuff(char input[], char output[]);
void writeTransmission(char frame[]);
void bitDestuff(char input[], char output[]);
void byteDestuff(char input[], char output[]);
void displayStuffing(int sourceIndex, int destinationIndex);
void displayIP(int ip[]);  
unsigned char wrapAround(unsigned int sum);
int binaryToDecimal(char binary[]);
void decimalTo8BitBinary(int num, char binary[]);
void checksumCalculation(char data[], char checksum[]);
/*================ HAMMING =================*/

void hammingSender(char data[]);
void hammingReceiver();
void calculateHammingParity();
void detectHammingError();

/*================ 2D PARITY =================*/

void paritySender();
void parityReceiver();
void calculateRowParity();
void calculateColumnParity();
void checkParity();
/*=========================================================
                HASH FUNCTION
=========================================================*/

int hashFunction(char url[])
{
    int i;
    int sum = 0;

    for(i = 0; url[i] != '\0'; i++)
    {
        sum = sum + url[i];
    }

    return sum % SIZE;
}

/*=========================================================
                INSERT URL
=========================================================*/

void insertURL(char url[], int ip[], char mac[])
{
    int index;
    int i;

    index = hashFunction(url);

    for(i = 0; i < SIZE; i++)
    {
        if(table[index].flag == 0)
        {
            strcpy(table[index].url, url);

            table[index].ip[0] = ip[0];
            table[index].ip[1] = ip[1];
            table[index].ip[2] = ip[2];
            table[index].ip[3] = ip[3];

            strcpy(table[index].mac, mac);

            table[index].flag = 1;

            return;
        }

        index = (index + 1) % SIZE;
    }

    printf("\nHash Table Full.\n");
}

/*=========================================================
                SEARCH URL
=========================================================*/

int searchURL(char url[])
{
    int index;
    int i;

    index = hashFunction(url);

    for(i = 0; i < SIZE; i++)
    {
        if(table[index].flag == 1 &&
           strcmp(table[index].url, url) == 0)
        {
            return index;
        }

        index = (index + 1) % SIZE;
    }

    return -1;
}

/*=========================================================
                DELETE URL
=========================================================*/

void deleteURL(char url[])
{
    int pos;

    pos = searchURL(url);

    if(pos == -1)
    {
        printf("\nURL Not Found.\n");
        return;
    }

    table[pos].flag = 0;

    printf("\nDeleted Successfully.\n");
}

/*=========================================================
                DISPLAY TABLE
=========================================================*/

void displayTable()
{
    int i;

    printf("\n---------------------------------------------------------------\n");
    printf("Index\tURL\t\t\tIP Address\t\tMAC Address\n");
    printf("---------------------------------------------------------------\n");

    for(i = 0; i < SIZE; i++)
    {
        if(table[i].flag == 1)
        {
            printf("%d\t", i);

            printf("%-20s", table[i].url);

            printf("%d.%d.%d.%d\t",
                   table[i].ip[0],
                   table[i].ip[1],
                   table[i].ip[2],
                   table[i].ip[3]);

            printf("%s\n", table[i].mac);
        }
        else
        {
            printf("%d\tEMPTY\n", i);
        }
    }
}


/*=========================================================
            LOAD DEFAULT URLS
=========================================================*/

void loadDefaultURLs()
{
    int ip1[4] = {142,250,10,1};
    int ip2[4] = {142,250,10,2};
    int ip3[4] = {157,240,1,35};
    int ip4[4] = {205,251,242,103};
    int ip5[4] = {208,80,154,224};

    insertURL("www.google.com",ip1,"AA:11:22:33:44:55");

    insertURL("www.youtube.com",ip2,"AA:11:22:33:44:66");

    insertURL("www.facebook.com",ip3,"BB:22:33:44:55:66");

    insertURL("www.amazon.com",ip4,"CC:11:22:33:44:55");

    insertURL("www.wikipedia.org",ip5,"DD:11:22:33:44:55");
}

/*=========================================================
                DECIMAL TO BINARY
=========================================================*/

void decimalToBinary(int number, int bits, char binary[])
{
    int i;

    binary[bits] = '\0';

    for(i = bits - 1; i >= 0; i--)
    {
        binary[i] = (number % 2) + '0';
        number = number / 2;
    }
}

/*=========================================================
                CHARACTER TO BINARY
=========================================================*/

void characterToBinary(char ch, char binary[])
{
    decimalToBinary((unsigned char)ch, 8, binary);
}

/*=========================================================
                READ MESSAGE
=========================================================*/

void readMessage(char fileName[])
{
    FILE *fp;
    int i = 0;
    char ch;

    fp = fopen(fileName, "r");

    if(fp == NULL)
    {
        printf("\nCannot Open File.\n");
        return;
    }

    while((ch = fgetc(fp)) != EOF)
    {
        message[i] = ch;
        i++;
    }

    message[i] = '\0';

    fclose(fp);
}

/*=========================================================
                MESSAGE TO BINARY
=========================================================*/

void messageToBinary()
{
    int i;
    char binary[9];

    bitStream[0] = '\0';

    printf("\n=========== APPLICATION LAYER ===========\n");

    printf("\nOriginal Message : %s\n", message);

    printf("\nCharacter to Binary\n\n");



    for(i = 0; message[i] != '\0'; i++)
    {
        characterToBinary(message[i], binary);

        printf("%c  -->  %s\n", message[i], binary);

        strcat(bitStream, binary);
    }

    totalBits = strlen(bitStream);

    printf("\nComplete Bit Stream\n");

    printf("%s\n", bitStream);

    printf("\nTotal Bits = %d\n", totalBits);
}

/*=========================================================
                DISPLAY MESSAGE
=========================================================*/

void displayMessage()
{
    printf("\nMessage : %s\n", message);
}


/*=========================================================
                HEX TO DECIMAL
=========================================================*/

int hexToDecimal(char ch)
{
    if(ch >= '0' && ch <= '9')
        return ch - '0';

    if(ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;

    if(ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;

    return 0;
}

/*=========================================================
                IP TO BINARY
=========================================================*/

void ipToBinary(int ip[], char binary[])
{
    int i;
    char temp[9];

    binary[0] = '\0';

    for(i = 0; i < 4; i++)
    {
        decimalToBinary(ip[i], 8, temp);
        strcat(binary, temp);
    }
}

/*=========================================================
                MAC TO BINARY
=========================================================*/

void macToBinary(char mac[], char binary[])
{
    int i;
    int value;
    char temp[9];

    binary[0]='\0';

    for(i = 0; mac[i] != '\0'; i++)
    {
        if(mac[i] == ':')
            continue;

        value = hexToDecimal(mac[i]) * 16;
        value = value + hexToDecimal(mac[i+1]);

        decimalToBinary(value,8,temp);

        strcat(binary,temp);

        i++;
    }
}

/*=========================================================
                PORT TO BINARY
=========================================================*/

void portToBinary(int port,char binary[])
{
    decimalToBinary(port,16,binary);
}

void byteStuff(char input[], char output[])
{
    int i, j = 0;
    int stuffed = 0;
    char byte[9];

    for(i = 0; input[i] != '\0'; i += 8)
    {
        strncpy(byte, input + i, 8);
        byte[8] = '\0';

        if(strcmp(byte, FLAG) == 0 || strcmp(byte, ESC) == 0)
        {
            strcpy(output + j, ESC);
            j += 8;
            stuffed = 1;
        }

        strcpy(output + j, byte);
        j += 8;
    }

    output[j] = '\0';

    printf("\nByte Stuffed Data  : %s", output);

    if(stuffed)
        printf("\nByte Stuffing      : ESC inserted before FLAG/ESC\n");
    else
        printf("\nByte Stuffing      : No Byte Stuffing Required\n");
}

void byteDestuff(char input[], char output[])
{
    int i,j=0;

    while(input[i]!='\0')
    {
        if(strncmp(input+i,ESC,8)==0)
        {
            i+=8;
        }

        strncpy(output+j,input+i,8);

        j+=8;
        i+=8;
    }

    output[j]='\0';
}

void displayIP(int ip[])
{
    printf("%d.%d.%d.%d",
           ip[0],
           ip[1],
           ip[2],
           ip[3]);
}

void displayStuffing(int sourceIndex,int destinationIndex)
{
    bitStuff(originalData,bitStuffedData);
    char bitChecksum[9];

    checksumCalculation(bitStuffedData, bitChecksum);

    printf("\n========================================================");
    printf("\n                 BIT STUFFING");
    printf("\n========================================================");

printf("\n\nSender Side");

printf("\n\nOriginal Data :\n%s",originalData);

printf("\n\nAfter Bit Stuffing :\n%s",bitStuffedData);

printf("\n\nOriginal Length : %d",strlen(originalData));

printf("\nStuffed Length  : %d",strlen(bitStuffedData));

bitDestuff(bitStuffedData,bitDestuffedData);

printf("\n\n========================================================");
printf("\n                 BIT DESTUFFING");
printf("\n========================================================");

printf("\n\nReceiver Side");

printf("\n\nReceived Stuffed Data :\n%s",bitStuffedData);

printf("\n\nAfter Bit Destuffing :\n%s",bitDestuffedData);

printf("\n\nTransmission Status : SUCCESS");

printf("\n\n========================================================");
printf("\n                    FRAME FORMAT");
printf("\n========================================================");

printf("\n\nFlag\n-----");
printf("\n%s",FLAG);

printf("\n\nAddress");
printf("\n-------");

printf("\nSource MAC      : %s",table[sourceIndex].mac);
printf("\nDestination MAC : %s",table[destinationIndex].mac);

printf("\n\nData (Stuffed Data)");
printf("\n--------------------");

printf("\n%s",bitStuffedData);

printf("\n\nTrailer");
printf("\n-------");

printf("\n0000000000000000");

printf("\n\nFlag");
printf("\n-----");

printf("\n%s",FLAG);

byteStuff(originalData,byteStuffedData);
char byteChecksum[9];

checksumCalculation(byteStuffedData, byteChecksum);

printf("\n\n========================================================");
printf("\n               PPP BYTE STUFFING");
printf("\n========================================================");

printf("\n\nOriginal Data :\n%s",originalData);

printf("\n\nAfter Byte Stuffing :\n%s",byteStuffedData);

printf("\n\nOriginal Length : %d",strlen(originalData));

printf("\nStuffed Length  : %d",strlen(byteStuffedData));

byteDestuff(byteStuffedData,byteDestuffedData);
char receiverByteChecksum[9];

checksumCalculation(byteDestuffedData, receiverByteChecksum);

printf("\n\nReceived Checksum   : %s", byteChecksum);
printf("\nCalculated Checksum : %s", receiverByteChecksum);

if(strcmp(byteChecksum, receiverByteChecksum)==0)
{
    printf("\nChecksum Verified");
}
else
{
    printf("\nError Detected");
}
char receiverBitChecksum[9];

checksumCalculation(bitDestuffedData, receiverBitChecksum);

printf("\n\nReceived Checksum   : %s", bitChecksum);
printf("\nCalculated Checksum : %s", receiverBitChecksum);

if(strcmp(bitChecksum, receiverBitChecksum)==0)
{
    printf("\nChecksum Verified");
}
else
{
    printf("\nError Detected");
}
printf("\n\n================================");
printf("\nHAMMING CODE");
printf("\n================================");

hammingSender(byteStuffedData);
hammingReceiver();

printf("\n\n================================");
printf("\n2D PARITY");
printf("\n================================");

paritySender();
parityReceiver();
printf("\n\n========================================================");
printf("\n              PPP BYTE DESTUFFING");
printf("\n========================================================");

printf("\n\nReceiver Side");

printf("\n\nReceived Stuffed Data :\n%s",byteStuffedData);

printf("\n\nAfter Byte Destuffing :\n%s",byteDestuffedData);

printf("\n\nOriginal File Content :");

printf("\n%s",message);

printf("\n\nTransmission Status : SUCCESS");

printf("\n\n========================================================");
printf("\n               DESTUFFED FRAME FORMAT");
printf("\n========================================================");

printf("\n\nFlag");
printf("\n-----");
printf("\n01111110");

printf("\n\nAddress");
printf("\n-------");
printf("\nSource MAC      : %s", table[sourceIndex].mac);
printf("\nDestination MAC : %s", table[destinationIndex].mac);

printf("\n\nData (Destuffed Data)");
printf("\n----------------------");
printf("\n%s", bitDestuffedData);

printf("\n\nTrailer");
printf("\n-------");
printf("\n0000000000000000");

printf("\n\nFlag");
printf("\n-----");
printf("\n01111110");

printf("\n\n========================================================");
printf("\n                 PPP FRAME FORMAT (SENDER)");
printf("\n========================================================");

printf("\n\nFlag");
printf("\n-----");
printf("\n01111110");

printf("\n\nAddress");
printf("\n-------");
printf("\n11111111");

printf("\n\nControl");
printf("\n-------");
printf("\n00000011");

printf("\n\nProtocol");
printf("\n--------");

printf("\nProtocol Name : %s",protocolName);

printf("\nProtocol Code : %s",protocolBinary);

printf("\n\nInformation (Data)");
printf("\n-------------------");
printf("\n%s", byteStuffedData);

printf("\n\nFCS");
printf("\n---");
printf("\n%s", byteChecksum);

printf("\n\nFlag");
printf("\n-----");
printf("\n01111110");

printf("\n\n========================================================");
printf("\n              PPP FRAME DE-FORMAT (RECEIVER)");
printf("\n========================================================");

printf("\n\nFlag");
printf("\n-----");
printf("\n01111110");

printf("\n\nAddress");
printf("\n-------");
printf("\n11111111");

printf("\n\nControl");
printf("\n-------");
printf("\n00000011");

printf("\n\nProtocol");
printf("\n--------");

printf("\nProtocol Name : %s",protocolName);

printf("\nProtocol Code : %s",protocolBinary);
printf("\n\nInformation (Data)");
printf("\n-------------------");
printf("\n%s", byteDestuffedData);

printf("\n\nFCS");
printf("\n---");
printf("\n%s", receiverByteChecksum);

printf("\n\nFlag");
printf("\n-----");
printf("\n01111110");

printf("\n\n========================================================");
printf("\n                    PHYSICAL LAYER");
printf("\n========================================================");

printf("\nSource MAC          : %s", table[sourceIndex].mac);
printf("\nDestination MAC     : %s", table[destinationIndex].mac);

printf("\nSource IP           : ");
displayIP(table[sourceIndex].ip);

printf("\nDestination IP      : ");
displayIP(table[destinationIndex].ip);

printf("\nTotal Frames        : %d", totalFrames);

printf("\n\nComplete Physical Binary Data");
printf("\n------------------------------------");

printf("\n%s", originalData);

printf("\n\nTotal Binary Length : %d", (int)strlen(originalData));


}

void bitStuff(char input[], char output[])
{
    int i;
    int j = 0;
    int count = 0;
    int stuffed = 0;

    for(i = 0; input[i] != '\0'; i++)
    {
        output[j++] = input[i];

        if(input[i] == '1')
        {
            count++;

            if(count == 5)
            {
                output[j++] = '0';
                stuffed = 1;
                count = 0;
            }
        }
        else
        {
            count = 0;
        }
    }

    output[j] = '\0';

    printf("\nBit Stuffed Data   : %s", output);

    if(stuffed)
        printf("\nBit Stuffing       : 0 inserted after five consecutive 1s\n");
    else
        printf("\nBit Stuffing       : No Bit Stuffing Required\n");
}

void bitDestuff(char input[], char output[])
{
    int i,j=0,count=0;

    for(i=0;input[i]!='\0';i++)
    {
        output[j++]=input[i];

        if(input[i]=='1')
        {
            count++;

            if(count==5)
            {
                i++;
                count=0;
            }
        }
        else
        {
            count=0;
        }
    }

    output[j]='\0';
}

void writeTransmission(char frame[])
{
    FILE *fp;

    fp = fopen("transmission.txt", "a");

    if(fp == NULL)
    {
        printf("\nCannot open transmission.txt\n");
        return;
    }

    fprintf(fp, "%s\n", frame);

    fclose(fp);
}

unsigned char wrapAround(unsigned int sum)
{
    while(sum > 255)
    {
        sum = (sum & 255) + (sum >> 8);
    }

    return sum;
}

int binaryToDecimal(char binary[])
{
    int i;
    int value = 0;

    for(i=0;i<8;i++)
    {
        value = value * 2 + (binary[i]-'0');
    }

    return value;
}

void decimalTo8BitBinary(int num,char binary[])
{
    int i;

    binary[8]='\0';

    for(i=7;i>=0;i--)
    {
        binary[i]=(num%2)+'0';
        num=num/2;
    }
}

void checksumCalculation(char data[],char checksum[])
{
    unsigned int sum=0;
    unsigned char value;
    char byte[9];
    int i;

    printf("\nCHECKSUM CALCULATION\n");

    for(i=0;data[i]!='\0';i+=8)
    {
        strncpy(byte,data+i,8);
        byte[8]='\0';

        value=binaryToDecimal(byte);

        printf("\nCurrent Sum : ");
        decimalTo8BitBinary(sum,byte);
        printf("%s",byte);

        strncpy(byte,data+i,8);
        byte[8]='\0';

        printf("\nNext Byte   : %s",byte);

        sum=sum+value;

        sum=wrapAround(sum);

        decimalTo8BitBinary(sum,byte);

        printf("\nAfter Add   : %s\n",byte);
    }

    sum=255-sum;

    decimalTo8BitBinary(sum,checksum);

    printf("\nOne's Complement");
    printf("\nChecksum : %s\n",checksum);
}

void hammingSender(char data[])
{
    int i,j,k;
    int parity;

    hammingDataBits = strlen(data);

    hammingParityBits = 0;

    while((1<<hammingParityBits) < (hammingDataBits+hammingParityBits+1))
        hammingParityBits++;

    hammingTotalBits = hammingDataBits+hammingParityBits;

    j=0;

    for(i=1;i<=hammingTotalBits;i++)
    {
        if((i&(i-1))==0)
            hammingCode[i]='0';
        else
            hammingCode[i]=data[j++];
    }

    for(i=0;i<hammingParityBits;i++)
    {
        int position=1<<i;
        parity=0;

        for(j=position;j<=hammingTotalBits;j+=2*position)
        {
            for(k=j;k<j+position && k<=hammingTotalBits;k++)
            {
                if(k!=position && hammingCode[k]=='1')
                    parity++;
            }
        }

        hammingCode[position]=(parity%2)?'1':'0';
    }

    printf("\n\nGenerated Hamming Code : ");

    for(i=1;i<=hammingTotalBits;i++)
        printf("%c",hammingCode[i]);

    printf("\n");
}

void hammingReceiver()
{
    int i,j,k;
    int parity;
    int error=0;

    for(i=1;i<=hammingTotalBits;i++)
        receivedHamming[i]=hammingCode[i];

    printf("\nReceiver Hamming Code : ");

    for(i=1;i<=hammingTotalBits;i++)
        printf("%c",receivedHamming[i]);

    printf("\n");

    for(i=0;i<hammingParityBits;i++)
    {
        int position=1<<i;
        parity=0;

        for(j=position;j<=hammingTotalBits;j+=2*position)
        {
            for(k=j;k<j+position && k<=hammingTotalBits;k++)
            {
                if(receivedHamming[k]=='1')
                    parity++;
            }
        }

        if(parity%2)
            error+=position;
    }

    if(error==0)
    {
        printf("\nNo Error Detected\n");
    }
    else
    {
        printf("\nError at Bit %d\n",error);

        if(receivedHamming[error]=='0')
            receivedHamming[error]='1';
        else
            receivedHamming[error]='0';

        printf("Error Corrected\n");
    }
}

void paritySender()
{
    int i,j,count,index=0;

    printf("\n\n2D Parity Sender\n");

    for(i=0;i<ROWS;i++)
    {
        for(j=0;j<COLS;j++)
        {
            senderMatrix[i][j]=byteStuffedData[index++]-'0';
        }
    }

    for(i=0;i<ROWS;i++)
    {
        count=0;

        for(j=0;j<COLS;j++)
            count+=senderMatrix[i][j];

        senderMatrix[i][COLS]=count%2;
    }

    for(j=0;j<COLS;j++)
    {
        count=0;

        for(i=0;i<ROWS;i++)
            count+=senderMatrix[i][j];

        senderMatrix[ROWS][j]=count%2;
    }

    count=0;

    for(j=0;j<COLS;j++)
        count+=senderMatrix[ROWS][j];

    senderMatrix[ROWS][COLS]=count%2;

    printf("\nSender Matrix\n");

    for(i=0;i<=ROWS;i++)
    {
        for(j=0;j<=COLS;j++)
            printf("%d ",senderMatrix[i][j]);

        printf("\n");
    }
}

void parityReceiver()
{
    int i,j,count;
    int row=-1,col=-1;

    for(i=0;i<=ROWS;i++)
    {
        for(j=0;j<=COLS;j++)
            receiverMatrix[i][j]=senderMatrix[i][j];
    }

    printf("\nReceiver Matrix\n");

    for(i=0;i<=ROWS;i++)
    {
        for(j=0;j<=COLS;j++)
            printf("%d ",receiverMatrix[i][j]);

        printf("\n");
    }

    for(i=0;i<ROWS;i++)
    {
        count=0;

        for(j=0;j<COLS;j++)
            count+=receiverMatrix[i][j];

        if(count%2!=receiverMatrix[i][COLS])
            row=i;
    }

    for(j=0;j<COLS;j++)
    {
        count=0;

        for(i=0;i<ROWS;i++)
            count+=receiverMatrix[i][j];

        if(count%2!=receiverMatrix[ROWS][j])
            col=j;
    }

    if(row==-1 && col==-1)
    {
        printf("\nNo Error Detected\n");
    }
    else
    {
        printf("\nError Position : Row %d Column %d\n",row+1,col+1);

        receiverMatrix[row][col]^=1;

        printf("\nCorrected Matrix\n");

        for(i=0;i<=ROWS;i++)
        {
            for(j=0;j<=COLS;j++)
                printf("%d ",receiverMatrix[i][j]);

            printf("\n");
        }
    }
}





/*=========================================================
                DISPLAY MAC
=========================================================*/

void displayMAC(char mac[])
{
    printf("%s", mac);
}

/*=========================================================
                APPLICATION LAYER
=========================================================*/

void applicationLayer()
{
    readMessage("message.txt");

    messageToBinary();
}
/*=========================================================
                TRANSPORT LAYER
=========================================================*/

void transportLayer()
{
    

    char sourcePortBinary[17];
    char destinationPortBinary[17];

    printf("\n========================================");
    printf("\n         TRANSPORT LAYER");
    printf("\n========================================\n");

    sourcePort = rand() % (65535 - 1024 + 1) + 1024;
    destinationPort = rand() % (65535 - 1024 + 1) + 1024;

    portToBinary(sourcePort, sourcePortBinary);
    portToBinary(destinationPort, destinationPortBinary);

    printf("\nSource Port      : %d", sourcePort);
    printf("\nDestination Port : %d", destinationPort);

    printf("\n\nSource Port Binary      : %s", sourcePortBinary);
    printf("\nDestination Port Binary : %s\n", destinationPortBinary);
}

void selectPPPProtocol()
{
    int choice;

    printf("\n========================================");
    printf("\n      SELECT PPP PROTOCOL");
    printf("\n========================================");

    printf("\n1. LCP");
    printf("\n2. PAP");
    printf("\n3. CHAP");
    printf("\n4. NCP");
    printf("\n5. IPCP");

    printf("\n\nEnter Choice : ");
    scanf("%d",&choice);

    switch(choice)
    {
        case 1:
            strcpy(protocolName,"LCP");
            strcpy(protocolBinary,"11000010 00000001");
            break;

        case 2:
            strcpy(protocolName,"PAP");
            strcpy(protocolBinary,"11000010 00000010");
            break;

        case 3:
            strcpy(protocolName,"CHAP");
            strcpy(protocolBinary,"11000010 00000011");
            break;

        case 4:
            strcpy(protocolName,"NCP");
            strcpy(protocolBinary,"11000010 00000100");
            break;

        case 5:
            strcpy(protocolName,"IPCP");
            strcpy(protocolBinary,"10000000 00100001");
            break;

        default:
            printf("\nInvalid Choice.");
            printf("\nDefault Protocol : LCP");

            strcpy(protocolName,"LCP");
            strcpy(protocolBinary,"11000010 00000001");
    }
}
/*=========================================================

START TRANSMISSION
=========================================================*/

void startTransmission()
{
    char sourceURL[50];
    char destinationURL[50];

    int sourceIndex;
    int destinationIndex;

    int ip[4];
    char mac[18];

    printf("\nEnter Source URL : ");
    scanf("%s", sourceURL);

    printf("Enter Destination URL : ");
    scanf("%s", destinationURL);

    /*---------------- SOURCE ----------------*/

    sourceIndex = searchURL(sourceURL);

    if(sourceIndex == -1)
{
    printf("\nSource URL Not Found.\n");

    printf("Enter Source IP Address: ");
    scanf("%d %d %d %d",
          &ip[0], &ip[1], &ip[2], &ip[3]);

    printf("Enter Source MAC Address: ");
    scanf("%s", mac);

    insertURL(sourceURL, ip, mac);

    sourceIndex = searchURL(sourceURL);
}

    /*---------------- DESTINATION ----------------*/

    destinationIndex = searchURL(destinationURL);
    if(destinationIndex == -1)
    {

        printf("\nDestination URL Not Found.\n");

        printf("Enter Destination IP Address: ");
        scanf("%d %d %d %d",
              &ip[0], &ip[1], &ip[2], &ip[3]);

        printf("Enter Destination MAC Address: ");
        scanf("%s", mac);

        insertURL(destinationURL, ip, mac);

        destinationIndex = searchURL(destinationURL);
    }
        printf("\n========================================");
        printf("\nSOURCE DETAILS");
        printf("\n========================================\n");

        printf("URL : %s\n", table[sourceIndex].url);

        printf("IP  : ");
        displayIP(table[sourceIndex].ip);

        printf("\nMAC : ");
        displayMAC(table[sourceIndex].mac);

        printf("\n");
        printf("\n========================================");
        printf("\nDESTINATION DETAILS");
        printf("\n========================================\n");

        printf("URL : %s\n", table[destinationIndex].url);

        printf("IP  : ");
        displayIP(table[destinationIndex].ip);

        printf("\nMAC : ");
        displayMAC(table[destinationIndex].mac);

        printf("\n");
        selectPPPProtocol();
        applicationLayer();

    transportLayer();

    networkLayer(sourceIndex, destinationIndex);

    dataLinkLayer(sourceIndex, destinationIndex);
}
/*=========================================================
                NETWORK LAYER
=========================================================*/

void networkLayer(int sourceIndex, int destinationIndex)
{
    int i;
    int j;

    char sourceIPBinary[33];
    char destinationIPBinary[33];

    ipToBinary(table[sourceIndex].ip, sourceIPBinary);
    ipToBinary(table[destinationIndex].ip, destinationIPBinary);

    totalPackets = 0;

    printf("\n========================================");
    printf("\n          NETWORK LAYER");
    printf("\n========================================\n");

    for(i = 0; i < totalBits; i += PACKETSIZE)
    {
        for(j = 0; j < PACKETSIZE; j++)
        {
            if(i + j < totalBits)
                packets[totalPackets][j] = bitStream[i + j];
            else
                packets[totalPackets][j] = '0';
        }

        packets[totalPackets][PACKETSIZE] = '\0';

        printf("\nPacket %d\n", totalPackets + 1);

        printf("Source IP Binary      : %s\n",
               sourceIPBinary);

        printf("Destination IP Binary : %s\n",
               destinationIPBinary);

        printf("Packet Data           : %s\n",
               packets[totalPackets]);

        totalPackets++;
    }

    printf("\nTotal Packets = %d\n",
           totalPackets);
}
/*=========================================================
                DATA LINK LAYER
=========================================================*/
void dataLinkLayer(int sourceIndex,
                   int destinationIndex)
{
    int i;
    int frameNo = 0;
    originalData[0] = '\0';

    char sourceMACBinary[49];
    char destinationMACBinary[49];

    char trailer[] = "00000000";

    char sourceIPBinary[33];
    char destinationIPBinary[33];

    char sourcePortBinary[17];
    char destinationPortBinary[17];

    char completeFrame[300];
    char byteStuffed[100];
    char bitStuffed[150];
    FILE *fp;
    macToBinary(table[sourceIndex].mac,
                sourceMACBinary);

    macToBinary(table[destinationIndex].mac,
                destinationMACBinary);

    ipToBinary(table[sourceIndex].ip,
           sourceIPBinary);

    ipToBinary(table[destinationIndex].ip,
           destinationIPBinary);

    portToBinary(sourcePort,
             sourcePortBinary);

    portToBinary(destinationPort,
             destinationPortBinary);

    totalFrames = 0;

    printf("\n========================================");
    printf("\n         DATA LINK LAYER");
    printf("\n========================================\n");

    for(i = 0; i < totalPackets; i++)
    {
        strncpy(frames[frameNo],
                packets[i],
                8);

        frames[frameNo][8] = '\0';
        strcat(originalData, frames[frameNo]);

        printf("\nFrame %d\n",
               frameNo + 1);

        printf("Source MAC Binary      : %s\n",
               sourceMACBinary);

        printf("Destination MAC Binary : %s\n",
               destinationMACBinary);

        printf("Data                  : %s\n",
               frames[frameNo]);

        printf("Trailer               : %s\n",
               trailer);

        frameNo++;
        totalFrames++;

        strncpy(frames[frameNo],
                packets[i] + 8,
                8);

        frames[frameNo][8] = '\0';
        strcat(originalData, frames[frameNo]);

        printf("\nFrame %d\n",
               frameNo + 1);

        printf("Source MAC Binary      : %s\n",
               sourceMACBinary);


               printf("Destination MAC Binary : %s\n",
               destinationMACBinary);

        printf("Data                  : %s\n",
               frames[frameNo]);


        printf("Trailer               : %s\n",
               trailer);

        frameNo++;
        totalFrames++;
    }
    printf("\n========================================");
    printf("\n COMPLETE FRAME TRANSMISSION");
    printf("\n========================================\n");

    for(i = 0; i < totalFrames; i++)
    {
        completeFrame[0] = '\0';
        strcat(completeFrame, FLAG);
        strcat(completeFrame, sourceMACBinary);
        strcat(completeFrame, destinationMACBinary);
        strcat(completeFrame, FLAG);

        strcat(completeFrame, sourceIPBinary);
        strcat(completeFrame, destinationIPBinary);

        strcat(completeFrame, sourcePortBinary);
        strcat(completeFrame, destinationPortBinary);

        byteStuff(frames[i], byteStuffed);

        bitStuff(byteStuffed, bitStuffed);

        strcat(completeFrame, bitStuffed);
        strcat(completeFrame, FLAG);
        strcat(completeFrame, trailer);
        strcat(completeFrame, FLAG);
        printf("\nOriginal Data      : %s",frames[i]);
        printf("\nOriginal Character : %c", message[i]);

        printf("\nByte Stuffed Data  : %s",byteStuffed);

        printf("\nBit Stuffed Data   : %s\n",bitStuffed);

        printf("\nComplete Frame %d\n", i + 1);
        printf("%s\n", completeFrame);
        writeTransmission(completeFrame);
    }

    printf("\nTotal Frames = %d\n",
           totalFrames);
        displayStuffing(sourceIndex,destinationIndex);
    }
/*=========================================================
                    SUMMARY
=========================================================*/

void summary()
{
    printf("\n========================================");
    printf("\n             SUMMARY");
    printf("\n========================================");

    printf("\nTotal Bits    : %d", totalBits);
    printf("\nTotal Packets : %d", totalPackets);
    printf("\nTotal Frames  : %d", totalFrames);

    printf("\n========================================\n");
}
/*=========================================================
                    MAIN FUNCTION
=========================================================*/

int main()
{
    int choice;

    char url[50];

    int ip[4];

    char mac[18];

    loadDefaultURLs();

    while(1)
    {
        printf("\n========================================");
        printf("\n      NETWORK LAYER SIMULATION");
        printf("\n========================================");

        printf("\n1. Insert URL");
        printf("\n2. Delete URL");
        printf("\n3. Search URL");
        printf("\n4. Display Hash Table");
        printf("\n5. Start Transmission");
        printf("\n6. Exit");

        printf("\n\nEnter Choice : ");
        scanf("%d",&choice);

        switch(choice)
        {
        case 1:

            printf("\nEnter URL : ");
            scanf("%s", url);

            printf("Enter IP Address (Example: 142 250 10 1): ");
            scanf("%d %d %d %d",&ip[0], &ip[1], &ip[2], &ip[3]);

            printf("Enter MAC Address (Example: AA:11:22:33:44:55): ");
            scanf("%s", mac);

            insertURL(url, ip, mac);

            printf("\nInserted Successfully.\n");
            break;
        case 2:

            printf("\nEnter URL : ");
            scanf("%s",url);

            deleteURL(url);

            break;

        case 3:
        {
            int pos;

            printf("\nEnter URL : ");
            scanf("%s",url);

            pos = searchURL(url);

            if(pos==-1)
            {
                printf("\nURL Not Found.\n");
            }
            else
            {
                printf("\nURL Found.\n");

                printf("\nURL : %s",table[pos].url);

                printf("\nIP : ");
                displayIP(table[pos].ip);

                printf("\nMAC : %s\n",table[pos].mac);
            }

            break;
        }

        case 4:

            displayTable();

            break;
        
        case 5:


            startTransmission();

            summary();

            break;

        case 6:

            printf("\nProgram Ended.\n");

            exit(0);

        default:

            printf("\nInvalid Choice.\n");

        }

    }

    return 0;
}
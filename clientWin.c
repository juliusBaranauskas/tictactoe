 
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PORT 32700
#define CONFURMED 12345
#define Xs 0
#define Os 1
#define WON 69
#define LOST 420
#define TIE 13

struct Move{
    short int x;
    short int y;
};

void emptyBoard();
void draw();
void askPos(struct Move* move);



short int board[3][3];

int main(int argc, char* argv[]){

    // create socket for connecting to servers
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1){
        printf("Error occurred when connecting: %s\n", strerror(errno));
        close(network_socket);
        return -1;
    }

    struct Move* move = malloc(sizeof(struct Move*));
    int* confirmation = malloc(sizeof(int));
    int thisPlayer;

    emptyBoard();

    // receiving confirmation and who goes first(to set Os or Xs)
    if(recv(network_socket, confirmation, sizeof(confirmation), MSG_CONFIRM) != -1){
        if(*confirmation == CONFURMED+1){
            thisPlayer = Os;
        }else if(*confirmation == CONFURMED){
            thisPlayer = Xs;
        }else
            return -1;
        printf("I am %cs\n", thisPlayer == Xs ? 'X' : 'O');
    }else{
        printf("I am %d ded\n", *confirmation);
    }

    while(recv(network_socket, move, sizeof(move), MSG_CONFIRM) != -1)
    {
        if(move->x == LOST){
            printf("CONGRATULATIONS, you've lost :) \n");
            break;
        }else if(move->x == WON){
            printf("You won, you fucking piece of pizza with pineapple topping\n");
            break;
        }else if(move->x == TIE){
            printf("True masters only end up with a TIE. This game is soooo boring...\n");
            break;
        }else if(move->x != -1)
            board[move->x][move->y] = thisPlayer == Xs ? Os : Xs;

        draw();
        askPos(move);
        send(network_socket, move, sizeof(move), MSG_CONFIRM);
        board[move->x][move->y] = thisPlayer;
        printf("Wait for opponents move...\n");
    }

    close(network_socket);
    sleep(1);
    return 0;
}

void emptyBoard(){
    for(short int i = 0; i < 3; i++){
        for(short int y = 0; y < 3; y++){
            board[i][y] = -111;
        }
    }
}

void draw(){
    char symbol;
    for(short int i = 0; i < 3; i++){
        for(short int y = 0; y < 3; y++){
            symbol = ' ';
            if(board[y][i] == Xs){
                symbol = 'X';
            }else if(board[y][i] == Os){
                symbol = 'O';
            }
            printf("| %c ", symbol);
        }
        printf("\n");
    }
    printf("\n");
}

void askPos(struct Move* move){

    do{
        printf("Give X coord (1 - 3): ");
        do{
            scanf("%hd", &move->x);
        }while(move->x < 0 || move->x > 3);
        
        printf("Give Y coord (1 - 3): ");
        do{
            scanf("%hd", &move->y);
        }while(move->y < 0 || move->y > 3);

    }while(board[move->x-1][move->y-1] != -111);
    
    move->x--;
    move->y--;
}
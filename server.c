#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PORTdefault 32700
#define TRUE 1
#define FALSE 0
#define Xs 0
#define Os 1
#define CONFURMED 12345
#define WON 69
#define LOST 420
#define TIE 13

struct Move{
    short int x;
    short int y;
};

void emptyBoard();
int checkWinner();
void sendMsg(struct Move* move, int recipient, int msg);

short int board[3][3];

int main(int argc, char* argv[]){

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORTdefault);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    int client_sockets[2];

    int* confurmed = malloc(sizeof(int));
    int *confurmed1 = malloc(sizeof(int));
    *confurmed = CONFURMED;
    *confurmed1 = CONFURMED+1;

    listen(server_socket, 5);
    client_sockets[0] = accept(server_socket, NULL, NULL);
    client_sockets[1] = accept(server_socket, NULL, NULL);

    send(client_sockets[0], confurmed, sizeof(confurmed), MSG_CONFIRM);
    send(client_sockets[1], confurmed1, sizeof(confurmed1), MSG_CONFIRM);

    int winnerExists = FALSE;
    int currentPlayer = Os;
    int movesDone = 0;
    struct Move* lastMove = malloc(sizeof(struct Move*));
    lastMove->x = -1;

    emptyBoard();
    
    while(winnerExists != TRUE && movesDone != 9)
    {
        // change the current player to the other one
        currentPlayer = currentPlayer == Xs ? Os : Xs;

        // send info about opponent's move and wait for answer
        send(client_sockets[currentPlayer], lastMove, sizeof(lastMove), MSG_CONFIRM);
        if(recv(client_sockets[currentPlayer], lastMove, sizeof(lastMove), MSG_CONFIRM) != -1)    
            printf("Player %c placed on (%d,%d)\n", currentPlayer == Xs ? 'O' : 'X', lastMove->x, lastMove->y);
        
        board[lastMove->x][lastMove->y] = currentPlayer;
        winnerExists = checkWinner(currentPlayer);
        movesDone++;
    }

    if(winnerExists){
        printf("Player %c won\n", currentPlayer == Xs ? 'X' : 'O');
        sendMsg(lastMove, client_sockets[currentPlayer], WON);
        sendMsg(lastMove, client_sockets[currentPlayer == Xs ? Os : Xs], LOST);
    }else
        sendMsg(lastMove, client_sockets[Xs], TIE);
        sendMsg(lastMove, client_sockets[Os], TIE);
        printf("No one won\n");

    close(server_socket);
    sleep(1);
    return 0;
}

void emptyBoard(){
    for(short int i = 0; i < 3; i++){
        for(short int y = 0; y < 3; y++)
            board[i][y] = -111;
    }
}

int checkWinner(int currentPlayer){

    short int sum = 0;
    // check columns
    for(short int j = 0; j < 3; j++){
        for(short int i = 0; i < 3; i++)
            sum+=board[j][i];
        if(sum == currentPlayer*3)
            return TRUE; // player won
        sum = 0;
    }

    sum = 0;
    // check rows
    for(short int j = 0; j < 3; j++){
        for(short int i = 0; i < 3; i++)
            sum+=board[i][j];
        if(sum == currentPlayer*3)
            return TRUE; // player won
        sum = 0;
    }

    // check diagonals
    if(board[0][0] + board[1][1] + board[2][2] == currentPlayer*3 || board[0][2] + board[1][1] + board[2][0] == currentPlayer*3)
        return TRUE;
    return FALSE;
}

// sends a specific message to client
void sendMsg(struct Move* move, int recipient, int msg){
    move->x = msg;
    send(recipient, move, sizeof(move), MSG_CONFIRM);
}
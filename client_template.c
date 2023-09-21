#include <ncurses.h>
#include <termios.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

#define XMAX 44
#define YMAX 155
#define NB_COLORS 5
#define TRAIL_INDEX_SHIFT 50

#define BLUE_ON_BLACK 0
#define RED_ON_BLACK 2
#define YELLOW_ON_BLACK 1
#define MAGENTA_ON_BLACK 3
#define CYAN_ON_BLACK 4

#define BLUE_ON_BLUE 50
#define RED_ON_RED 52
#define YELLOW_ON_YELLOW 51
#define MAGENTA_ON_MAGENTA 53
#define CYAN_ON_CYAN 54

void tune_terminal()
{
    struct termios term;
    tcgetattr(0, &term);
    term.c_iflag &= ~ICANON;
    term.c_lflag &= ~ICANON;
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &term);
}

void init_graphics()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);
    start_color();
    init_pair(BLUE_ON_BLACK, COLOR_BLUE, COLOR_BLACK);
    init_pair(RED_ON_BLACK, COLOR_RED, COLOR_BLACK);
    init_pair(YELLOW_ON_BLACK, COLOR_YELLOW, COLOR_BLACK);
    init_pair(MAGENTA_ON_BLACK, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(CYAN_ON_BLACK, COLOR_CYAN, COLOR_BLACK);

    init_pair(BLUE_ON_BLUE, COLOR_BLUE, COLOR_BLUE);
    init_pair(RED_ON_RED, COLOR_RED, COLOR_RED);
    init_pair(YELLOW_ON_YELLOW, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(MAGENTA_ON_MAGENTA, COLOR_MAGENTA, COLOR_MAGENTA);
    init_pair(CYAN_ON_CYAN, COLOR_CYAN, COLOR_CYAN);
    init_pair(WALL, COLOR_WHITE, COLOR_WHITE);
}

void display_character(int color, int y, int x, char character)
{
    attron(COLOR_PAIR(color));
    mvaddch(y, x, character);
    attroff(COLOR_PAIR(color));
}

void affichageBoard(display_info game)
{

    for (int i = 0; i < XMAX; i++)
    {
        for (int j = 0; j < YMAX; j++)
        {
            if (game.board[i][j] == (char)VOID)
            {
                mvaddch(i, j, ' ');
            }

            if (game.board[i][j] == (char)WALL)
            {
                display_character(WALL, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)BLUE_ON_BLACK)
            {
                display_character(BLUE_ON_BLACK, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)RED_ON_BLACK)
            {
                display_character(RED_ON_BLACK, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)YELLOW_ON_BLACK)
            {
                display_character(YELLOW_ON_BLACK, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)MAGENTA_ON_BLACK)
            {
                display_character(MAGENTA_ON_BLACK, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)CYAN_ON_BLACK)
            {
                display_character(CYAN_ON_BLACK, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)BLUE_ON_BLUE)
            {
                display_character(BLUE_ON_BLUE, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)RED_ON_RED)
            {
                display_character(RED_ON_RED, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)YELLOW_ON_YELLOW)
            {
                display_character(YELLOW_ON_YELLOW, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)MAGENTA_ON_MAGENTA)
            {
                display_character(MAGENTA_ON_MAGENTA, i, j, ACS_VLINE);
            }

            if (game.board[i][j] == (char)CYAN_ON_CYAN)
            {
                display_character(CYAN_ON_CYAN, i, j, ACS_VLINE);
            }
        }
    }
    refresh();
}

int returnIdClientFromKeyPressed(char pressedKey)
{
    int idClient;
    if (pressedKey == 'z' || pressedKey == 's' || pressedKey == 'q' || pressedKey == 'd' || pressedKey == ' ')
    {
        idClient = 1;
    }
    if (pressedKey == 'i' || pressedKey == 'k' || pressedKey == 'j' || pressedKey == 'l' || pressedKey == 'm')
    {
        idClient = 2;
    }

    return idClient;
}

int showWinner(int winner)
{
    switch (winner)
    {
    case -1:
        break;

    case TIE:
        fprintf(stdout, "\nEgalité");
        break;

    case WIN:
        fprintf(stdout, "Vous avez gagné");

        break;

    case LOSE:
        fprintf(stdout, "Vous avez perdu");

        break;

    default:
        fprintf(stdout, "Victoire du joueur %d", winner - 3);

        break;
    }
    printf("\n");
    sleep(10);

    return 1;
}

void startClientGame(const char *ipAdress, int portServer, int numberPlayer)
{

    int sockfd, endGame = 0;
    SAI servaddr;
    display_info fromServer;
    struct client_init_infos nbPlayer;
    struct client_input inputClient, player1Client, player2Client;
    struct timeval tv;
    tune_terminal();
    init_graphics();
    refresh();
    int row;
    int col;
    getmaxyx(stdscr, col, row);

    WINDOW *board_win = newwin(YMAX, XMAX, 0, 0);

    // Socket creation and verification
    CHECK(sockfd = socket(AF_INET, SOCK_STREAM, 0));

    // assign IP and PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipAdress);
    servaddr.sin_port = htons(portServer);

    fd_set ens, tmp;

    FD_ZERO(&ens);
    FD_SET(sockfd, &ens);
    FD_SET(STDIN, &ens);
    tmp = ens;

    // connecting the client to the server
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        endwin();
        exit(0);
    }

    nbPlayer.nb_players = numberPlayer;

    // on envoie le nombre de joueur au serveur
    CHECK(send(sockfd, &nbPlayer, sizeof(nbPlayer), 0));

    // on attend de recevoir de la part du serveur l'id du client et la touche par defaut
    if (nbPlayer.nb_players == 1)
    {
        CHECK(recv(sockfd, &inputClient, sizeof(inputClient), 0));
    }

    // si il y a deux joueurs sur un seul client on attend l'id des des joueurs
    if (nbPlayer.nb_players == 2)
    {
        CHECK(recv(sockfd, &player1Client, sizeof(player1Client), 0));
        CHECK(recv(sockfd, &player2Client, sizeof(player2Client), 0));
    }

    int rv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    while (!endGame)
    {

        CHECK(rv = select(sockfd + 1, &tmp, NULL, NULL, &tv));

        if (FD_ISSET(STDIN, &tmp))
        {
            int key = getch();
            if (nbPlayer.nb_players == 1)
            {
                inputClient.input = (char)key;
                CHECK(send(sockfd, &inputClient, sizeof(inputClient), 0));
            }
            if (nbPlayer.nb_players == 2)
            {
                // la fonction returnIdClientFromPressedKey retourne un Id défini par le client
                // pour savoir si la touche a été entrée par le joueur 1 ou le joueur 2 et donc
                // envoyer la bonne information au serveur.
                if (returnIdClientFromKeyPressed((char)key) == 1)
                {
                    player1Client.input = (char)key;
                    CHECK(send(sockfd, &player1Client, sizeof(player1Client), 0));
                }
                if (returnIdClientFromKeyPressed((char)key) == 2)
                {
                    player2Client.input = (char)key;
                    CHECK(send(sockfd, &player2Client, sizeof(player2Client), 0));
                }
            }
        }

        if (FD_ISSET(sockfd, &tmp) || rv == 0)
        {
            CHECK(recv(sockfd, &fromServer, sizeof(fromServer), 0));
            affichageBoard(fromServer);
            if (fromServer.winner != -1)
            {
                endGame = showWinner(fromServer.winner);
            }
        }

        tmp = ens;
    }

    endwin();
    exit(0);
    // return 0;
}

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        printf("Usage : ./client [IP_SERVEUR] [PORT_SERVEUR] [NB_JOUEURS]\n");
        exit(0);
    }
    startClientGame(argv[1], atoi(argv[2]), atoi(argv[3]));
    return 0;
}

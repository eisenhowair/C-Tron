#include "common.h"
#include <sys/select.h>
#include <time.h>

typedef struct
{
  int playerPosition[2];
  int isLightCycleOn;
  char previousKeyBuffer;
} playerInfo;

struct paramGame
{
  int port;
  int refreshRate;
};

int serverInitialisation(int port)
{
  int serverSocket;
  SAI serverAddress;
  // socket creation and verification
  CHECK(serverSocket = socket(AF_INET, SOCK_STREAM, 0));
  // assign IP, PORT
  serverAddress.sin_family = AF_INET;
  // anyone should have the right to join
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(port);

  // pour réutiliser l'adresse et éviter "Address already in use"
  CHECK(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)));

  CHECK(bind(serverSocket, (SA *)&serverAddress, sizeof(serverAddress)));
 

  CHECK(listen(serverSocket, 2));


  return serverSocket;
}

int acceptConnection(int serverSocket)
{
  int clientSocket;
  SAI clientAddress;
  int clientSize = sizeof(SAI);
  CHECK(clientSocket = accept(serverSocket, (SA *)&clientAddress, (socklen_t *)&clientSize));

  return clientSocket;
}

int maximum(int a, int b)
{
  return a > b ? a : b;
}

int updatePlayerPosition(int *playerPosition, char fromClient)
{
  if (fromClient == 'z')
  {
    playerPosition[0] = playerPosition[0] - 1;
  }

  if (fromClient == 's')
  {
    playerPosition[0] = playerPosition[0] + 1;
  }

  if (fromClient == 'q')
  {
    playerPosition[1] = playerPosition[1] - 1;
  }

  if (fromClient == 'd')
  {
    playerPosition[1] = playerPosition[1] + 1;
  }
  return *playerPosition;
}

int updatePlayer2PositionOnUniqueClient(int *playerPosition, char fromClient)
{
  if (fromClient == 'i')
  {
    playerPosition[0] = playerPosition[0] - 1;
  }

  if (fromClient == 'k')
  {
    playerPosition[0] = playerPosition[0] + 1;
  }

  if (fromClient == 'j')
  {
    playerPosition[1] = playerPosition[1] - 1;
  }

  if (fromClient == 'l')
  {
    playerPosition[1] = playerPosition[1] + 1;
  }
  return *playerPosition;
}

int lightCycle(int isLightCycleOn)
{
  return (isLightCycleOn ? FALSE : TRUE);
}

int **boardInit()
{
  int **board = malloc(XMAX * sizeof(int *));
  for (int i = 0; i < XMAX; i++)
  {
    board[i] = malloc(YMAX * sizeof(int));
    for (int j = 0; j < YMAX; j++)
    {
      if (i == 0 || i == XMAX - 1 || j == 0 || j == YMAX - 1)
      {
        board[i][j] = WALL;
      }
      else
      {
        board[i][j] = VOID;
      }
    }
  }
  return board;
}

int checkForCollisions(int *playerPosition, int **board, int idClient, int bufferIdPlayer1, int bufferIdPlayer2)
{
  int collision = FALSE;
  if (board[playerPosition[0]][playerPosition[1]] == WALL)
  {
    collision = TRUE;
  }

  if (board[playerPosition[0]][playerPosition[1]] == bufferIdPlayer1 + TRAIL_INDEX_SHIFT || board[playerPosition[0]][playerPosition[1]] == bufferIdPlayer2 + TRAIL_INDEX_SHIFT)
  {
    collision = TRUE;
  }

  if (idClient == bufferIdPlayer1)
  {
    if (board[playerPosition[0]][playerPosition[1]] == bufferIdPlayer2)
    {
      collision = TIE;
    }
  }

  if (idClient == bufferIdPlayer2)
  {
    if (board[playerPosition[0]][playerPosition[1]] == bufferIdPlayer1)
    {
      collision = TIE;
    }
  }
  return collision;
};

display_info putBoard(display_info paquet, int **board)
{

  for (int i = 0; i < XMAX; i++)
  {
    for (int j = 0; j < YMAX; j++)
    {
      paquet.board[i][j] = board[i][j];
    }
  }
  return paquet;
}

struct client_input initAndSendPaquet(char input, int concurrent_id, int currentClient)
{
  struct client_input paquetClient;
  srand(time(NULL));

  do // pour éviter que les 2 paquets aient le même id
  {
    paquetClient.id = rand() % 5;
  } while (paquetClient.id == concurrent_id);

  paquetClient.input = input;

  CHECK(send(currentClient, &paquetClient, sizeof(paquetClient), 0));

  return paquetClient;
}

int **updateBoard(int *playerPosition, playerInfo player, int **board, struct client_input paquetClient, int bufferIdPlayerOne, int bufferIdPlayerTwo, int isTwoPlayerOnOneClient)
{
  display_info ending_paquet;
  ending_paquet = putBoard(ending_paquet, board);
  if (player.isLightCycleOn == TRUE)
  {
    board[playerPosition[0]][playerPosition[1]] = TRAIL_INDEX_SHIFT + paquetClient.id;
  }
  else
  {
    board[playerPosition[0]][playerPosition[1]] = VOID;
  }
  if (isTwoPlayerOnOneClient == TRUE)
  {
    if (paquetClient.id == bufferIdPlayerOne)
    {
      updatePlayerPosition(playerPosition, paquetClient.input);
      if (paquetClient.input != ' ')
      {
        if (checkForCollisions(playerPosition, board, paquetClient.id, bufferIdPlayerOne, bufferIdPlayerTwo) == TRUE)
        {
          if (paquetClient.id == 4)
            ending_paquet.winner = 5;
          else
            ending_paquet.winner = 4;

          CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));
          
            exit(0);
          
        }
        else if (checkForCollisions(playerPosition, board, paquetClient.id, bufferIdPlayerOne, bufferIdPlayerTwo) == TIE)
        {
          ending_paquet.winner = TIE;
          CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));

            exit(0);
          
        }
        else
          board[playerPosition[0]][playerPosition[1]] = paquetClient.id;
      }
    }

    if (paquetClient.id == bufferIdPlayerTwo)
    {
      updatePlayer2PositionOnUniqueClient(playerPosition, paquetClient.input);
      if (paquetClient.input != 'm')
      {
        if (checkForCollisions(playerPosition, board, paquetClient.id, bufferIdPlayerOne, bufferIdPlayerTwo) == TRUE)
        {
          
          if (paquetClient.id == 4)
            ending_paquet.winner = 5;
          else
            ending_paquet.winner = 4;
          CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));

          
            exit(0);
          
        }
        else if (checkForCollisions(playerPosition, board, paquetClient.id, bufferIdPlayerOne, bufferIdPlayerTwo) == TIE)
        {
          ending_paquet.winner = TIE;
          CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));
         
            exit(0);
          
        }
        else
          board[playerPosition[0]][playerPosition[1]] = paquetClient.id;
      }
    }
  }
  else
    updatePlayerPosition(playerPosition, paquetClient.input);

  // si la tête du client n'a pas bougé entre temps, pas de check collision
  if (paquetClient.input != ' ')
  {
    if (checkForCollisions(playerPosition, board, paquetClient.id, bufferIdPlayerOne, bufferIdPlayerTwo) == TRUE)
    {
      if (paquetClient.id == 4)
      {
        ending_paquet.winner = WIN;
        CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));
        ending_paquet.winner = LOSE;
        CHECK(send(5, &ending_paquet, sizeof(ending_paquet), 0));
      }
      else
      {
        ending_paquet.winner = WIN;
        CHECK(send(5, &ending_paquet, sizeof(ending_paquet), 0));
        ending_paquet.winner = LOSE;
        CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));
      }
        exit(0);
      
    }
    else if (checkForCollisions(playerPosition, board, paquetClient.id, bufferIdPlayerOne, bufferIdPlayerTwo) == TIE)
    {

      ending_paquet.winner = TIE;
      CHECK(send(4, &ending_paquet, sizeof(ending_paquet), 0));
      CHECK(send(5, &ending_paquet, sizeof(ending_paquet), 0));
      
        exit(0);
      
    }
    else
      board[playerPosition[0]][playerPosition[1]] = paquetClient.id;
  }
  return board;
}

void affichageBoard(int **board)
{
  for (int i = 0; i < XMAX; i++)
  {
    for (int j = 0; j < YMAX; j++)
    {
      printf("%d", board[i][j]);
    }
    printf("\n");
  }
}

playerInfo initializePlayer(playerInfo player, int defaultY)
{
  player.playerPosition[0] = XMAX / 2;
  player.playerPosition[1] = defaultY;
  player.isLightCycleOn = TRUE;
  return player;
}

void startGame(struct paramGame arguments)
{

  int port = arguments.port;
  int refresh_rate = arguments.refreshRate;
  int serverSocket;

  // initialisation du serveur
  serverSocket = serverInitialisation(port);


  int selectReturnValue, maximumSocketsNb;
  int currentClient = -1;
  int nbPlayer = 0;

  int printfDone = FALSE;
  int isTwoPlayerOnOneClient = FALSE;

  // FD initialisation
  fd_set fdSockets, fdBuffer;
  FD_ZERO(&fdSockets);
  FD_SET(serverSocket, &fdSockets);

  // initialisation du board par défaut
  int **board = NULL;
  board = boardInit();

  // initialisation de deux joueurs
  playerInfo playerOne, playerTwo;

  // timeout pour le select
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  int bufferIdPlayerOne, bufferIdPlayerTwo;

  struct client_init_infos paquetClientInitInfos;
  struct client_input paquetClientInputPlayerOne, paquetClientInputPlayerTwo, paquetFromClient;

  while (-1)
  {
    // select is destructive, so we need a copy
    fdBuffer = fdSockets;

    maximumSocketsNb = maximum(currentClient, serverSocket) + 1;

    CHECK(selectReturnValue = select(maximumSocketsNb, &fdBuffer, NULL, NULL, &timeout));

    for (int idClient = 3; idClient < maximumSocketsNb; idClient++)
    {
      if (FD_ISSET(idClient, &fdBuffer))
      {
        // en cas de nouvelle connexion (socket server)
        if (idClient == 3)
        {
          // tant qu'il n'y a pas deux joueurs
          while (nbPlayer != 2)
          {
            currentClient = acceptConnection(serverSocket);
            CHECK(recv(currentClient, &paquetClientInitInfos, sizeof(paquetClientInitInfos), 0));
            if (paquetClientInitInfos.nb_players == 2)
            {

              isTwoPlayerOnOneClient = TRUE;
              nbPlayer = paquetClientInitInfos.nb_players = 2;
              // ajout du client unique
              FD_SET(currentClient, &fdSockets);
              if (isTwoPlayerOnOneClient == TRUE)
              {
                if (currentClient == 4)
                {
                  playerOne = initializePlayer(playerOne, YMAX / 4);
                  paquetClientInputPlayerOne = initAndSendPaquet('d', 0, currentClient);
                  bufferIdPlayerOne = paquetClientInputPlayerOne.id;
                  board[playerOne.playerPosition[0]][playerOne.playerPosition[1]] = paquetClientInputPlayerOne.id;

                  playerTwo = initializePlayer(playerTwo, (YMAX / 2 + YMAX / 4) - 1);
                  paquetClientInputPlayerTwo = initAndSendPaquet('j', paquetClientInputPlayerOne.id, currentClient);
                  bufferIdPlayerTwo = paquetClientInputPlayerTwo.id;
                  board[playerTwo.playerPosition[0]][playerTwo.playerPosition[1]] = paquetClientInputPlayerTwo.id;
                }
              }
            }

            if (isTwoPlayerOnOneClient == FALSE)
            {
              // ajout de la nouvelle socket
              FD_SET(currentClient, &fdSockets);
              nbPlayer++;

              // initialise la position des clients entrants
              if (currentClient == 4)
              {
                playerOne = initializePlayer(playerOne, YMAX / 4);

                paquetClientInputPlayerOne = initAndSendPaquet('d', 0, currentClient);
                bufferIdPlayerOne = paquetClientInputPlayerOne.id;
                board[playerOne.playerPosition[0]][playerOne.playerPosition[1]] = paquetClientInputPlayerOne.id;

              }

              if (currentClient == 5)
              {
                playerTwo = initializePlayer(playerTwo, (YMAX / 2 + YMAX / 4) - 1);

                paquetClientInputPlayerTwo = initAndSendPaquet('q', paquetClientInputPlayerOne.id, currentClient);
                bufferIdPlayerTwo = paquetClientInputPlayerTwo.id;
                board[playerTwo.playerPosition[0]][playerTwo.playerPosition[1]] = paquetClientInputPlayerTwo.id;
                
              }
            }
          }
        }
        else
        {
          CHECK(recv(idClient, &paquetFromClient, sizeof(paquetFromClient), 0));
          char keyControl = paquetFromClient.input;
          if (idClient == 4)
          {
            if (isTwoPlayerOnOneClient)
            {
              // assignation des commandes du joueur 1
              if (paquetFromClient.id == bufferIdPlayerOne)
              {
                if (keyControl == 'z' || keyControl == 's' || keyControl == 'q' || keyControl == 'd')
                {
                  paquetClientInputPlayerOne.input = keyControl;
                }
                else if (keyControl == ' ')
                {
                  playerOne.isLightCycleOn = lightCycle(playerOne.isLightCycleOn);
                }
              }

              // assignation d'une commande pour le joueur 2
              if (paquetFromClient.id == bufferIdPlayerTwo)
              {
                if (keyControl == 'i' || keyControl == 'k' || keyControl == 'j' || keyControl == 'l')
                {
                  paquetClientInputPlayerTwo.input = keyControl;
                }
                else if (keyControl == 'm')
                {
                  playerTwo.isLightCycleOn = lightCycle(playerTwo.isLightCycleOn);
                }
              }
            }

            // assignation des touches pour le client 1
            if (!isTwoPlayerOnOneClient)
            {
              if (keyControl == 'z' || keyControl == 's' || keyControl == 'q' || keyControl == 'd')
              {
                paquetClientInputPlayerOne.input = keyControl;
              }
              if (keyControl == ' ')
              {
                playerOne.isLightCycleOn = lightCycle(playerOne.isLightCycleOn);
              }
            }
          }

          // assignation des touches pour le client 2
          if (idClient == 5)
          {
            if (keyControl == 'z' || keyControl == 's' || keyControl == 'q' || keyControl == 'd')
            {
              paquetClientInputPlayerTwo.input = keyControl;
            }
            if (keyControl == ' ')
            {
              playerTwo.isLightCycleOn = lightCycle(playerTwo.isLightCycleOn);
            }
          }
        }
      }
    }
    // si on ne recoit rien d'aucun client
    if (selectReturnValue == 0)
    {
      // on s'assure de ne rien faire si il n'y pas assez de joueur
      if (nbPlayer != 2)
      {
        if (printfDone == FALSE)
        {
          printfDone = TRUE;
        }
      }
      // on update le board avec les bonnes valeurs avant de l'envoyer
      else
      {
        display_info paquet = putBoard(paquet, board);
        paquet.winner = -1;

        // reste à transformer playerPosition pour avoir un argument en moins
        updateBoard(playerOne.playerPosition, playerOne, board, paquetClientInputPlayerOne, bufferIdPlayerOne, bufferIdPlayerTwo, isTwoPlayerOnOneClient);
        updateBoard(playerTwo.playerPosition, playerTwo, board, paquetClientInputPlayerTwo, bufferIdPlayerOne, bufferIdPlayerTwo, isTwoPlayerOnOneClient);

        usleep(refresh_rate);

        if (isTwoPlayerOnOneClient == TRUE)
        {
          CHECK(send(4, &paquet, sizeof(paquet), 0));
        }
        else
        {
          CHECK(send(4, &paquet, sizeof(paquet), 0));
          CHECK(send(5, &paquet, sizeof(paquet), 0));
        }
        // affichageBoard(board);
      }
    }

    FD_ZERO(&fdBuffer);
  }
  // close the socket
  CHECK(close(serverSocket));
}

int main(int argc, char const *argv[])
{
  if (argc != 3)
  {
    printf("Usage : ./server [PORT_SERVEUR] [REFRESH_RATE]\n");
    exit(0);
  }
  struct paramGame arg = {atoi(argv[1]), atoi(argv[2])};
  startGame(arg);
  return 0;
}

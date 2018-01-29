/**
 * ------------------------------------------------------------------------------*
 * File: snake.ino                                                               *
 *       Implementação do jogo da cobrinha para Arduino/PQDB                     *
 * Author: Rafael de Moura Moreira <rafaelmmoreira@gmail.com>                    *
 * License: MIT License                                                          *
 * ------------------------------------------------------------------------------*
 * 																				 *
 * O lendário jogo da cobrinha, em versão para Arduino.		                     *
 * Desenvolvido e testado em um Arduino Uno, Rev. 3                              *
 *																				 *
 * Essa implementação supõe o uso do shield PQDB Mini. Porém, nem todos os compo-*
 * nentes da mesma são usados. Para que ele funcione, é necessário ter 4 botões e*
 * o display Nokia 5110. Para instruções das ligações do circuito, conferir o es-*
 * quemático da PQDB no repositório original.                                    *
 *                                                                               *
 * As bibliotecas utilizadas são os drivers da própria PQDB, também disponíveis  *
 * no repositório original do projeto.                                           *
 * https://github.com/projetopqdb/                                               *
 *                                                                               *
 * ------------------------------------------------------------------------------*
 */

#include "keypad.h"
#include "nokia5110.h"

#define EMPTY 0
#define SNAKE 1
#define FOOD 2

// Cada nó (gominho) da cobrinha contém uma lista de coordenadas x e y e o próximo movimento que o nó fará.
typedef struct{
  unsigned char x;
  unsigned char y;
  char nextMove;
} Snake_node;

Snake_node snake[252]; 		// A cobrinha é uma lista de nós, definidos acima.
unsigned char game[21][12]; // O "mapa" do jogo é uma matriz onde cada posição pode conter cobra (SNAKE), comida (FOOD) ou estar vazia (EMPTY).
char nextMove;				// Armazena o input do usuário, que será o próximo movimento da cabeça da cobra.
unsigned char snakeLength;	// O comprimento da cobra.
unsigned char foodX, foodY; // Coordenadas x e y da comida.
bool food;					// Indica se há comida.
bool gameOver;				// Indica se o jogador perdeu.
bool win;                   // Indica se o jogador ganhou.

// A função gameInit() inicializa todas as variáveis. Coloca a cobrinha em posição +- centralizada e limpa o mapa.
void gameInit()
{
  food = false;
  foodX = 0;
  foodY = 0;
  gameOver = false;
  win = false;
  snakeLength = 1;
  snake[0].x = 10;
  snake[0].y = 5;
  snake[0].nextMove = 'R';
  
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 12; j++)
    {
      game[i][j] = EMPTY;
    }
  }  
}

// A função loop_Input é uma das 3 constituintes do loop principal do jogo.
// Ela lê se/qual tecla o usuário está pressionando. Se for uma tecla direcional, ela será o próximo movimento da cabeça.
// Se for uma tecla inválida, será ignorada (a cobrinha continuará na direção anterior).
// O loop dessa função garante que o programa perderá algum tempo fazendo a leitura da tecla.
// Sem este loop, o programa passa tão pouco tempo nessa função que muitas vezes perde o acionamento de uma tecla.
void loop_Input()
{
  for (unsigned char j = 0; j < 20; j++)
  {
    nextMove = kpReadKey();
    if (nextMove != 'U' && nextMove != 'D' && nextMove != 'L' && nextMove != 'R') 
      nextMove = 0;
  }
}

// A função loop_Update é a 2a das 3 constituintes do loop principal do jogo.
// Ela é responsável por atualizar todas as informações do jogo, na seguinte ordem:
// -> Limpa o mapa do jogo.
// -> Posiciona a comida no mapa.
// -> Atualiza os movimentos:
// 	  -> A cabeça recebe o input do usuário, caso tenha sido válido. Senão, segue na mesma direção.
//	  -> Os outros gomos recebem o movimento realizado pelo nó anterior a ele na rodada anterior a ele.
//    	 (Ex: se a cabeça vira à esquerda em uma rodada, o 2o gomo irá virar à esquerda na rodada seguinte, o 3o depois de 2 rodadas...)
// -> Atualiza a posição da cobra inteira de acordo com o movimento agendado:
// 	  -> Se a cobra esbarrar no limite do mapa ou em si mesma, game over.
//    -> Senão, marca a cobra no mapa.
// -> Se a cabeça da cobra encontrou a comida, adiciona um novo gomo ao fim da cobra e sorteia uma nova posição para aparecer comida.
//    -> A posição do novo gomo é sempre atrás do último gomo.
//		 (Ex: se o último gomo está se movendo para a direita, o novo gomo aparecerá à sua esquerda.)
//    -> Se a cobra atingir o comprimento de 252 (todas as posições do mapa são cobra), o jogador vence.
//	  -> A nova comida não pode aparecer sobre a cobra. Caso isso aconteça, uma nova posição é sorteada até que seja encontrada uma vazia.
void loop_Update()
{
   // Limpa o mapa
  for (int i = 0; i < 21; i++)
  {
    for (int j = 0; j < 12; j++)
    {
      game[i][j] = EMPTY;
    }
  }

  // Posiciona comida no mapa
  game[foodX][foodY] = FOOD;
  
  // Atualiza os movimentos da cobra
  for (unsigned char i = snakeLength - 1; i > 0; i--)
  {
    snake[i].nextMove = snake[i-1].nextMove;
  }
  if (nextMove)
    snake[0].nextMove = nextMove;
    
  // Move a cobra
  for (unsigned char i = 0; i < snakeLength; i++)
  {
    switch(snake[i].nextMove)
    {
      case 'U':
        snake[i].y -= 1;
      break;
      case 'D':
        snake[i].y += 1;
      break;
      case 'L':
        snake[i].x -= 1;
      break;
      case 'R':
        snake[i].x += 1;
      break;
    }
    if (snake[i].x >= 21 || snake[i].x < 0 || snake[i].y >= 12 || snake[i].y < 0)
      gameOver = true;
    if (game[snake[i].x][snake[i].y] == SNAKE)
      gameOver = true;
    else
      game[snake[i].x][snake[i].y] = SNAKE;

  }

  // Caso a cobra tenha comido nessa rodada, ela cresce
  if (snake[0].x == foodX && snake[0].y == foodY)
  {
    food = false;
    switch(snake[snakeLength-1].nextMove)
    {
      case 'U':
        snake[snakeLength].x = snake[snakeLength-1].x;
        snake[snakeLength].y = snake[snakeLength-1].y + 1;
      break;
      case 'D':
        snake[snakeLength].x = snake[snakeLength-1].x;
        snake[snakeLength].y = snake[snakeLength-1].y - 1;
      break;
      case 'L':
        snake[snakeLength].x = snake[snakeLength-1].x + 1;
        snake[snakeLength].y = snake[snakeLength-1].y;
      break;
      case 'R':
        snake[snakeLength].x = snake[snakeLength-1].x - 1;
        snake[snakeLength].y = snake[snakeLength-1].y;
      break;
    }
    if (snake[snakeLength].x >= 0 && snake[snakeLength].y >= 0 &&
    snake[snakeLength].x < 21 && snake[snakeLength].y < 12)
      game[snake[snakeLength].x][snake[snakeLength].y] = SNAKE;
      
    snakeLength++;
    if (snakeLength == 252)
    {
      win = true;
    }
  }
  // Se não há comida (e o usuário não atingiu o tamanho máximo), sorteie nova comida
  if (!food && !win)
  {
    do{
      foodX = random(0, 21);
      foodY = random(0, 12);
   } while(game[foodX][foodY] != EMPTY);
   game[foodX][foodY] = FOOD;
   food = true;
  }
}

// A função loop_Draw é a última das funções do loop principal do jogo.
// Ela é responsável por atualizar a tela.
// -> Caso o usuário tenha perdido, mostra a mensagem de Game Over e reinicia o jogo.
// -> Caso o usuário tenha vencido, mostra a mensagem de vitória e reinicia o jogo.
// -> Caso o jogo esteja em andamento, a matriz do jogo é impressa na tela.
// 	  -> Cada ponto do mapa original se torna um quadrado 4x4 na tela.
//	  -> Os gomos da cobrinha são quadrados preenchidos, enquanto a comida é um quadrado só com as bordas.
// -> O atraso ao final da função ajuda a desacelerar um pouco o jogo. Pode ser ajustado para deixá-lo mais fácil ou mais difícil. 
void loop_Draw()
{
  if (gameOver)
  {
    clearScreen();
    LcdXY(15,2);
    LcdWriteString("Game Over");
    delay(2000);
    gameInit();
  }
  else if (win)
  {
    clearScreen();
    LcdXY(21,2);
    LcdWriteString("You Win");
    delay(5000);
    gameInit();
  }
  else
  {
    for (unsigned char i = 0; i < 21; i++)
    {
      for (unsigned char j = 0; j < 12; j++)
      {
        if (game[i][j] == FOOD)
        {
          setPixel(4*i, 4*j, 1);
          setPixel(4*i, 4*j+1, 1);
          setPixel(4*i, 4*j+2, 1);
          setPixel(4*i, 4*j+3, 1);        
          setPixel(4*i+1, 4*j, 1);
          setPixel(4*i+1, 4*j+1, 0);
          setPixel(4*i+1, 4*j+2, 0);
          setPixel(4*i+1, 4*j+3, 1);
          setPixel(4*i+2, 4*j, 1);
          setPixel(4*i+2, 4*j+1, 0);
          setPixel(4*i+2, 4*j+2, 0);
          setPixel(4*i+2, 4*j+3, 1);
          setPixel(4*i+3, 4*j, 1);
          setPixel(4*i+3, 4*j+1, 1);
          setPixel(4*i+3, 4*j+2, 1);
          setPixel(4*i+3, 4*j+3, 1);
        }
        else
        {
          setPixel(4*i, 4*j, game[i][j]);
          setPixel(4*i, 4*j+1, game[i][j]);
          setPixel(4*i, 4*j+2, game[i][j]);
          setPixel(4*i, 4*j+3, game[i][j]);        
          setPixel(4*i+1, 4*j, game[i][j]);
          setPixel(4*i+1, 4*j+1, game[i][j]);
          setPixel(4*i+1, 4*j+2, game[i][j]);
          setPixel(4*i+1, 4*j+3, game[i][j]);
          setPixel(4*i+2, 4*j, game[i][j]);
          setPixel(4*i+2, 4*j+1, game[i][j]);
          setPixel(4*i+2, 4*j+2, game[i][j]);
          setPixel(4*i+2, 4*j+3, game[i][j]);
          setPixel(4*i+3, 4*j, game[i][j]);
          setPixel(4*i+3, 4*j+1, game[i][j]);
          setPixel(4*i+3, 4*j+2, game[i][j]);
          setPixel(4*i+3, 4*j+3, game[i][j]);
        }
      }
    }
    writeBuffer();
  }
  delay(100);
  
}

// Nessa função é feita a inicialização do hardware, da seed dos sorteios e do jogo em si.
void setup() 
{
  initializeDisplay(0xC0,0x07,0x014);
  kpInit();
  randomSeed(analogRead(0));
  gameInit();
}

// O loop principal:
// -> Leia o novo input do usuário, se houver.
// -> Atualize as informações do jogo.
// -> Redesenhe a tela.
void loop() 
{
  // put your main code here, to run repeatedly:
  loop_Input();
  loop_Update();
  loop_Draw();
}

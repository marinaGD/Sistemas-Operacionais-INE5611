# Sistemas-Operacionais-INE5611

    O trabalho tem por objetivo criar um jogo: 
O jogo consiste de um tabuleiro 11 X 11 casas (posições). Nesse tabuleiro, inicialmente são distribuídos  cinco tokens (fichas) aleatoriamente em qualquer uma das 121 casas. Além dos tokens, há um cursor posicionado inicialmente no meio do tabuleiro.

Após disparado o jogo, os tokens movimentam-se simultaneamente para qualquer posição do tabuleiro. O objetivo do jogo é que o jogador elimine todos os tokens dentro de um tempo máximo t. Para eliminar um token, o jogador tem que posicionar  o cursor na casa na qual o token encontra-se. 

O jogo tem três níveis de dificuldade: fácil, intermediário e difícil. No fácil, os tokens movem-se mais lentamente e o tempo t é maior; no difícil, os tokens movem-se mais rapidamente; no nível intermediário, a velocidade de movimentação fica entre as velocidades dos níveis e difícil.

Para rodar:
$ gcc -Wall -o jogo trabalhoSO.c -lcurses -lpthread

$ ./jogo
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define EMPTY     ' '
#define CURSOR_PAIR   1
#define TOKEN_PAIR    2
#define EMPTY_PAIR    3
#define LINES 11
#define COLS 11
#define TOKENS 5

// VARIÁVEIS
bool ganhou = FALSE;
bool perdeu = FALSE;
bool jogo_ativo;
char board[LINES][COLS];
unsigned int tempo_maximo;
unsigned int velocidade;

// MÉTODOS
void termina_jogo();
void menu();
void desenha_tabuleiro();
void board_refresh();
void inicializa_jogo();
int get_tokens_restantes();
void adiciona_tokens_no_tabuleiro();
bool esta_ocupado(int y, int x);
void *move_token(void *token);
void *conta_tempo();
void *move_cursor();

// ptheads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t t_timer, t_cursor, t_tokens[TOKENS];


typedef struct CoordStruct {
  int x;
  int y;
  bool morto;
} coord_type;
 
coord_type cursor, tokens[TOKENS];

int main(void) {
  srand(time(NULL));  /* inicializa gerador de numeros aleatorios */ 

  /* inicializa curses */

  initscr();
  keypad(stdscr, TRUE);
  cbreak();
  noecho();

  /* inicializa colors */

  if (has_colors() == FALSE) {
    endwin();
    printf("Seu terminal nao suporta cor\n");
    exit(1);
  }

    start_color();
    
  /* inicializa pares caracter-fundo do caracter */

  init_pair(CURSOR_PAIR, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(TOKEN_PAIR, COLOR_RED, COLOR_RED);
  init_pair(EMPTY_PAIR, COLOR_BLUE, COLOR_BLUE);
  clear();

  menu();
  endwin();
  exit(0);
}

void menu(){
    pthread_mutex_unlock(&mutex);
    clear();
    refresh();

    if(ganhou) {
        mvprintw(0, 0, "Parabens, voce ganhou!");
    } else if (perdeu) {
        mvprintw(0, 0, "Nao foi dessa vez :(");
    }


    mvprintw(1, 0, "Escolha uma dificuldade:");
    mvprintw(2, 0, "(F) Facil");
    mvprintw(3, 0, "(M) Medio");
    mvprintw(4, 0, "(D) Dificil");
    mvprintw(5, 0, "(Q) Sair");

    switch (getch()) {
    case 'F':
    case 'f':
        tempo_maximo = 120;
        velocidade = 1.5;
        inicializa_jogo();
        break;
    case 'M':
    case 'm':
        tempo_maximo = 60;
        velocidade = 1.0;
        inicializa_jogo();
        break;
    case 'D':
    case 'd':
        tempo_maximo = 30;
        velocidade = 0.5;
        inicializa_jogo();
        break;

    case 'q':
    case 'Q':
        pthread_mutex_destroy(&mutex);
        endwin();
        exit(0);
    default:
        menu();
        break;
    }

}

void board_refresh(){
  desenha_tabuleiro();
  adiciona_tokens_no_tabuleiro();
  refresh();
}

void desenha_tabuleiro(void) {
  int x, y;

  for (x = 0; x < COLS; x++) 
    for (y = 0; y < LINES; y++){
      attron(COLOR_PAIR(EMPTY_PAIR));
      mvaddch(y, x, EMPTY);
      attroff(COLOR_PAIR(EMPTY_PAIR));
  }
}


void adiciona_tokens_no_tabuleiro(){

  for (int i = 0; i < TOKENS; i++) {
    if (tokens[i].morto == FALSE){
      attron(COLOR_PAIR(TOKEN_PAIR));
      mvaddch(tokens[i].y, tokens[i].x, EMPTY);
      attroff(COLOR_PAIR(TOKEN_PAIR));
    }
  }

  attron(COLOR_PAIR(CURSOR_PAIR));
  mvaddch(cursor.y, cursor.x, EMPTY);
  attroff(COLOR_PAIR(CURSOR_PAIR));
}


void limpa_tempo(){
  for (int x = 0; x < COLS; x++){
      attron(COLOR_PAIR(EMPTY_PAIR));
      mvaddch(15, 9, EMPTY);
      mvaddch(15, 8, EMPTY);
      attroff(COLOR_PAIR(EMPTY_PAIR));
  }
}


void init_outros(){
    cursor.morto = FALSE;
    cursor.x = 5;
    cursor.y = 5;
    pthread_create(&t_cursor, NULL, move_cursor, NULL);
    pthread_create(&t_timer, NULL, conta_tempo, NULL);
    pthread_join(t_cursor, NULL);
    pthread_join(t_timer, NULL);

}

void *conta_tempo() {
    for(int i = 0; i <= tempo_maximo; i++) {
        limpa_tempo(); 
        pthread_mutex_lock(&mutex);
        mvprintw(15, 0, "Faltam %i segundos", tempo_maximo - i);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    perdeu = true;
    jogo_ativo = false;
    pthread_cancel(t_cursor);
    refresh();
    menu();
    pthread_exit(0);
}


void init_tokens(){
  for(int i = 0; i < TOKENS; i++) {
    tokens[i].morto = FALSE;
    tokens[i].x = 0;
    tokens[i].y = 0;
    pthread_create(&t_tokens[i], NULL, move_token, (void *) (intptr_t)i);
  }

}


void inicializa_jogo(){
    jogo_ativo = true;
    ganhou = FALSE;
    perdeu = FALSE;

    clear();
    refresh();
    desenha_tabuleiro();

    init_tokens();
    init_outros();
    for(int i =0; i < TOKENS; i++){
      pthread_join(t_tokens[i], NULL);
    }
  }


bool esta_ocupado(int x, int y) {
    for(int i = 0; i <= TOKENS; i++) {
        if((tokens[i].x == x && tokens[i].y == y)
            || (cursor.x == x && cursor.y == y)) {
            return true;
        }
    }
    return false;
}


void *move_token(void *arg) {
    int num_token = (intptr_t)arg;
    while (jogo_ativo && !perdeu && !ganhou && tokens[num_token].morto == FALSE) {
        pthread_mutex_lock(&mutex);

        int new_x, new_y;
        do {
            new_x = rand() % LINES;
            new_y = rand() % COLS;
        } while(esta_ocupado(new_x, new_y));

        tokens[num_token].x = new_x;
        tokens[num_token].y = new_y;

        board_refresh();
        pthread_mutex_unlock(&mutex);
        sleep(velocidade);
    }
    pthread_exit(0);
}

void *move_cursor() {
    int tecla;
    do {
        tecla = getch();
        pthread_mutex_lock(&mutex);
        
        switch (tecla) {
            case KEY_UP:
            case 'w':
            case 'W':
                if ((cursor.y > 0)) {
                    cursor.y = cursor.y - 1;
                }
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                if ((cursor.y < LINES - 1)) {
                    cursor.y = cursor.y + 1;
                }
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                if ((cursor.x > 0)) {
                    cursor.x = cursor.x - 1;
                }
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if ((cursor.x < COLS - 1)) {
                    cursor.x = cursor.x + 1;
                }
                break;
        }
        if (get_tokens_restantes() == 0){
          termina_jogo();
        }
        board_refresh();
        pthread_mutex_unlock(&mutex);
  } while ((tecla != 'q') && (tecla != 'Q'));

  jogo_ativo = false;
  pthread_mutex_unlock(&mutex);
  pthread_cancel(t_timer);
  pthread_exit(0);
}


void termina_jogo(){
  pthread_cancel(t_timer);
  ganhou = TRUE;
  jogo_ativo = FALSE;
  menu();
  pthread_exit(0);
}


int get_tokens_restantes() {
    int restantes = TOKENS;
    for(int i = 0; i < TOKENS; i++) {
        if (tokens[i].morto)
        { restantes--; }
        else if(tokens[i].morto == FALSE && tokens[i].x == cursor.x && tokens[i].y == cursor.y) {
            restantes--;
            tokens[i].morto = TRUE;
        }
    }
    return restantes;
}
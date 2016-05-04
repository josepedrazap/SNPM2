#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <minix/mthread.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>


#define green "\033[9;32m"        /* 4 -> underline ;  32 -> green */
#define red "\033[9;1m" 
#define none   "\033[0m"        /* to flush the previous property */




int dst = 0;
int rondas = 0;
/* Estructura para el manejo del programa*/
typedef struct MyData {

    int M, m;
    int N, n;
    int t_ini;
    int z, h;
    int **tablero;
    int **tablero_anterior;
    int **ini;
    mthread_barrier_t barrera;
    mthread_mutex_t  ghMutex2;

} MYDATA, *PMYDATA;

/* Estructura para el manejo de los hilos*/
typedef struct DataHilos {
    int **Regiones; //Matriz con las cordenadas de cada secion por hilo
    int cch;        //Cantidad de secciones por hilo
    int e;       
    MYDATA *data;
} DataHilos, *PDataHilos;
/*Funcion que lee el archivo config,
guarda los primeros 5 datos en arr[] (M,N,m,n,h) y
llena el vector ini[][] con los vivos iniciales y lo retorna. */ 

void Imprimir_tablero(PMYDATA s, int o);
int **obtener_valores(int *arr, int *t_ini, char *argv);
void ayuda(void);
int **dividir_tablero(int M, int N, int m, int n, int *t_div);
void iniciar_tablero(PMYDATA Datos);
int Estavivo(PMYDATA s, int x, int y);
void func_salir(PMYDATA s);
void Imprimir_tablero(PMYDATA s, int o);
int work2(int x1, int y1, int x2, int y2, PMYDATA Datos);
void work(PDataHilos DH);
void work3(PDataHilos Data);


int **obtener_valores(int *arr, int *t_ini, char *argv){

    FILE *fp;
    int M = 5;
    int N = 15;
    int i =0;
    int j =0;
    int co = 0;
    char ch, cha;

    char **c = (char **)malloc (M*sizeof(char *));
    for (i = 0; i < M; i++) c[i] = (char *) calloc (N,sizeof(char));

    fp = fopen ( argv, "r" );
    if (fp==NULL){
        fputs ("ERROR: No se escuentra archivo ",stderr); 
        printf("%s. Intente conway -h para obtener ayuda. \n", argv);
        exit (1);
    }

    while(feof(fp)==0){
        cha = ch;
        ch = fgetc(fp);
        if(cha == '\n' && ch != EOF)co++;
    }
    co = co - 2;
    *t_ini = co;

    int **ini = (int **)malloc(co*sizeof(int *));
    for (i = 0; i < co; i++) ini[i] = (int *) calloc (2, sizeof(int));

    rewind(fp);

    for(i = 0; i < M; i++){
             fscanf(fp, "%s" ,c[i]);
             if((*c[i]!=' ') && (*c[i]!='1') && (*c[i]!='2') && (*c[i]!='3') && (*c[i]!='4') && (*c[i]!='5') && (*c[i]!='6') && (*c[i]!='7') && (*c[i]!='8') && (*c[i]!='9') && (*c[i]!='0')){
                printf("Input incorrecto, para obtener ayuda use el comando 'conway -h' \n");
                exit(1);
             }
    }
        for(j =0; j < M; j++){
            arr[j] = atoi(c[j]);
        }
    char buffer[10];
    for(i = 0; i < co; i++){

          fscanf(fp, "%s" ,buffer);
          ini[i][0] = atoi(buffer);
          fscanf(fp, "%s" ,buffer);
          ini[i][1] = atoi(buffer);
    }
    //free(c);
    //free(buffer);
    return ini;
}

void ayuda(){
    system("clear");
    printf("---------------\n");
    printf("AYUDA DE CONWAY\n");
    printf("---------------\n\n");
    printf("Para iniciar conway escriba 'conway config', donde 'config' es un archivo con una configuracion inicial valida. \n" );
    printf("El archivo 'config' debe estar dentro del directorio en el que se encuentra conway.\n\n");
    printf("Procure que el archivo 'config' tenga la siguiente estructura:\n");
    printf("---------------------------------------------------------------------------\n\n" );
    printf("M    N    // Donde M y N son las dimensiones del tablero. \n");
    printf("m    n    // Donde m y n son las dimensiones de las regiones.\n" );
    printf("h         // Donde h es la cantidad de hilos de trabajo.\n" );
    printf("x1   y1   // Donde (x, y) representa las cordenadas de un vivo inicial.\n" );
    printf(".    .    \n" );
    printf(".    .    \n" );
    printf(".    .    \n\n" );
    printf("xn   yn   \n" );
    printf("---------------------------------------------------------------------------\n\n" );
    printf("**Las posiciones iniciales deben caer dentro del tablero** (xi,yi) < (M,N).\n");
    printf("**Todos los valores deben ser positivos o iguales a cero** \n");
    printf("(M, N, m, n, h, (x1,y1),...,(xn,yn)) >= 0\n");
}

int **dividir_tablero(int M, int N, int m, int n, int *t_div){

    int cM = M/m;
    int cN = N/n;

    int i,c,c2;
    c=0;
    c2 = 0;

    if(M%m != 0)cM++;
    if(N%n != 0)cN++;

    int t = cM*cN;
    *t_div = t;

    int **vector = (int **)malloc (t*sizeof(int *));
    for (i = 0; i < t; i++) vector[i] = (int *) calloc(4, sizeof(int));

    for(i = 0; i < t; i++){

        if((c2 + 1)*m > M){
            vector[i][3]=c2*m + M%m;
        }else{
            vector[i][3] = (c2 + 1)*m;
        }
        if(c + n > N){
            vector[i][2] = c + N%n;
        }else{
            vector[i][2] = c + n;
        }
        vector[i][0] = c;
        vector[i][1] = c2*m;

        c2++;

        if(c2 == cM){
            c2 = 0;
            c += n;
        }
    }

return vector;
}

void iniciar_tablero(PMYDATA Datos){ //inicia el tablero con los vivos iniciales

    int l = Datos->t_ini;
    int **vivos = Datos->ini;
    int **tablero_anterior = Datos->tablero_anterior;
    int i;

    for (i = 0; i < l; i++)
    {
        if(vivos[i][0] >= Datos->M || vivos[i][1] >= Datos->N){
            printf("Input incorrecto, para obtener ayuda use el comando 'conway -h' \n");
            exit(1);
        }
        tablero_anterior[vivos[i][0]][vivos[i][1]] = 1;
    }

}


int Estavivo(PMYDATA s, int x, int y){

    
    int M = s->M;
    int N = s->N;
    int cont = 0;
    //printf("%i %i \n", x,y);
   
    if( x-1 >= 0 && y-1 >= 0){
        if(s->tablero_anterior[x-1][y-1] == 1)cont++;
    }
    if( x-1 >= 0 && y+1 < N){
        if(s->tablero_anterior[x-1][y+1] == 1)cont++;
    }
    if(x+1 < M && y+1 < N){
        if(s->tablero_anterior[x+1][y+1] == 1)cont++;
    }
    if( x+1 < M && y-1 >= 0){
        if(s->tablero_anterior[x+1][y-1] == 1)cont++;
    }
    if(x+1 < M){
        if(s->tablero_anterior[x+1][y] == 1)cont++;
    }
    if(x-1 >= 0){
        if(s->tablero_anterior[x-1][y] == 1 )cont++;
    }
    if(y-1 >= 0){
        if(s->tablero_anterior[x][y-1] == 1 )cont++;
    }
    if(y+1 < N){
        if(s->tablero_anterior[x][y+1] == 1)cont++;
    }


    if (s->tablero_anterior[x][y] == 1 && ( cont == 2 || cont == 3))
    {   
        s->tablero[x][y] = 1;                  
        return 0;
    }
    if (s->tablero_anterior[x][y] == 0 && cont == 3)
    {
        s->tablero[x][y] = 1;
       
        return 0;
    }
    if (cont < 2 || cont > 3)
    {
        s->tablero[x][y] = 0;
        
        return 0;
    }
   
    return 0;
}



void func_salir(PMYDATA s){

    static struct termios oldt, newt;
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);          
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    int tiempo_de_espera = 1; //en segundos
    char aux[1]; 
    int aux2;
    fd_set input;
    struct timeval timeout;
    int leer = 0;
    int read_aux = 0;

    FD_ZERO(&input );
    
    FD_SET(0, &input);

    timeout.tv_sec = tiempo_de_espera;   
    timeout.tv_usec = 0;   

    leer = select(1, &input, NULL, NULL, &timeout);

    if(leer == -1){

        tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    }
    if(leer) {
        read_aux = read(0, aux, 19);
        aux2 = aux[read_aux - 1];
        if(aux2 >= -1){
            
            tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
            free(s->tablero);
            free(s->tablero_anterior);
            free(s->ini);

            mthread_barrier_destroy(&(s->barrera));
            mthread_mutex_destroy(&(s->ghMutex2));
            printf("\n");

            exit(1);

    }

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    return;
    }
}


void Imprimir_tablero(PMYDATA s, int o){

    mthread_mutex_lock(&(s->ghMutex2)); 
    s->z++;
    mthread_mutex_unlock(&(s->ghMutex2)); 

    if(s->z == s->h || o == 1){
    int i,j,c; 

    c = 0;                        
    system("clear"); 

    printf("%s %i hilos en regiones de %ix%i.%s \n", green, s->h, s->m, s->n, green);

    for(i = 0; i < s->N; i++)printf("----");

    printf("\n");

    for(i = 0; i < s->M; i++){
        for(j = 0; j < s->N; j++){


            if(o == 0)s->tablero_anterior[i][j] = s->tablero[i][j];

            if(s->tablero_anterior[i][j] == 1){

                
                printf("   %s#%s", green, none);
                c++;
                
                continue;
            }

            printf("   %s.%s", red, red);
        }printf("\n \n");
    }
    for(i = 0; i < s->N; i++)printf("----");

    printf("\n");
    rondas = rondas + 1;
    printf("%sVivos: %i. Ronda: %i.%s\n", green, c, rondas, none); 
    s->z = 0;
 }
}

int work2(int x1, int y1, int x2, int y2, PMYDATA Datos){
    int i,j;
    for(i = 0; i + x1 < x2; i++){
        for(j = 0; j + y1 < y2; j++){
            Estavivo(Datos, i + x1, j + y1);
        }
    }
    return 0;
}

void work(PDataHilos DH){

    int i;
   
        for(i = 0; i < DH->cch; i++){
            work2(DH->Regiones[i][0],DH->Regiones[i][1],DH->Regiones[i][2],DH->Regiones[i][3], DH->data);
       
        }
}

int a = 0;

void work3(PDataHilos Data){
    
            work(Data);

            mthread_barrier_sync(&(Data->data->barrera));
            Imprimir_tablero(Data->data, 0);//aca se copia la matriz a matriz anterior
            mthread_barrier_sync(&(Data->data->barrera));
}

int main(argc, argv)
int argc;
char *argv[];
{   
    if(argc == 1){
        printf("Falta archivo config. Para obtener ayuda ejecute 'conway -h'\n");
        return 0;
    }

    if(argv[1][0] == '-' && argv[1][1] == 'h'){
        ayuda();
        return 0;
    }
    

    PMYDATA Datos = (PMYDATA) malloc(sizeof(MYDATA));

    int **matriz;
    int **ini;
    int **div;
    int *arr;
    int t_ini;
    int t_div;

    int **matriz_anterior; //inicializacion del tablero anterior

    int M,N,n,m,h; //Variables de inicio

    int i, j, k; //variables auxiliares

    arr = (int *)calloc(5, sizeof(int));


    ini = obtener_valores(arr, &t_ini, argv[1]); //obtiene los datos del archivo

    /* Escribe los valores obtenidos del archivo confi*/
    M = arr[0];
    N = arr[1];
    m = arr[2];
    n = arr[3];
    h = arr[4];

    if(n > N || m > M || h > M*N){
         printf("Input incorrecto, para obtener ayuda use el comando 'conway -h' \n");
         exit(1);
    }

    /*Inicia las matrices del tablero y las posiciones iniciales en 0*/
    matriz = (int **)malloc (M*sizeof(int *));
    for (i = 0; i < M; i++) matriz[i] = (int *) calloc(N, sizeof(int));
    /*Inicia la matriz anterior, futura variable de  tablero_anterior de MYData*/
    matriz_anterior = (int **)malloc (M*sizeof(int *));
    for (i = 0; i < M; i++) matriz_anterior[i] = (int *) calloc(N, sizeof(int));


    /*Pasa los datos a la estructura MYDATA */
    Datos->M = M;
    Datos->N = N;
    Datos->m = m;
    Datos->n = n;
    Datos->z = 0;
    Datos->h = h;
    Datos->tablero = matriz;
    Datos->tablero_anterior = matriz_anterior; //copia matriz 1
    Datos->ini = ini;
    Datos->t_ini = t_ini;

    mthread_mutex_init(&(Datos->ghMutex2), NULL);
    mthread_barrier_init(&(Datos->barrera), h);

    /*Inicia el tablero con los vivos iniciales*/

    iniciar_tablero(Datos);

    /*Divide el tablero en los tamaños maximos asignados por m y n */
    div = dividir_tablero(M, N, m, n, &t_div);
    
    /*Inicio codigo de los hilos*/

    /*PONER LOS HILOS DE MINIX */
    PDataHilos Data[h];
    mthread_thread_t hilos[h];

    int cch = t_div/h;
    int h_aux = h;

    for(i = 0; i < h; i++){
        Data[i] = (PDataHilos) malloc(sizeof(DataHilos));
    }

    if(t_div%h != 0){
        h--;
        h_aux--;
        Data[h]->Regiones = (int **)malloc ((cch + t_div%(h + 1))*sizeof(int *));
        for (j = 0; j < (cch + t_div%(h + 1)); j++) Data[h]->Regiones[j] = (int *) calloc(4, sizeof(int));

        Data[h]->cch = cch + t_div%(h + 1);
        Data[h]->data = Datos;
        Data[h]->e = 1;

        for(k = 0; k < (cch + t_div%(h + 1)); k++){
            Data[h]->Regiones[k][0] = div[k + h_aux*cch][0];
            Data[h]->Regiones[k][1] = div[k + h_aux*cch][1];
            Data[h]->Regiones[k][2] = div[k + h_aux*cch][2];
            Data[h]->Regiones[k][3] = div[k + h_aux*cch][3];
        }
        h++;
    }

    for(i = 0; i < h_aux; i++){
        Data[i]->Regiones = (int **)malloc (cch*sizeof(int *));
        for (j = 0; j < cch; j++) Data[i]->Regiones[j] = (int *) calloc(4, sizeof(int));
        Data[i]->cch = cch;
        Data[i]->data = Datos;
        Data[i]->e = 1;

        for(k = 0; k < cch; k++){
            Data[i]->Regiones[k][0] = div[k + i*cch][0];
            Data[i]->Regiones[k][1] = div[k + i*cch][1];
            Data[i]->Regiones[k][2] = div[k + i*cch][2];
            Data[i]->Regiones[k][3] = div[k + i*cch][3];
        }
    }

    Imprimir_tablero(Datos, 1);
    
    while(1 == 1){

        for(i = 0; i < h; i++){
            mthread_create(&hilos[i], NULL, (void*)work3, Data[i]);
        }  

        for(i = 0; i < h; i++){
            mthread_join(hilos[i], NULL);
        }
       func_salir(Datos);
    }
    free(arr);
    free(div);
    /*Fin codigo de los hilos*/

}

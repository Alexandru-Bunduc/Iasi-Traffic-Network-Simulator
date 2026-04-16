#include "dictionar_protocoale.h"  
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <time.h>
#define PORT 8080
#define NR_CLIENTI_MAX 100
#define NR_NODURI 10

 typedef struct{
    char nume[50];
    int viteza_max;
    int distanta;
    bool exista;
 }Strada;

 //locatiile din Iasi
 const char* nume_noduri[NR_NODURI]={
 "Piata Unirii",
"Copou",
"Targu Cucu",
"Podu Ros",
"Tudor Vladimirescu",
"Gara Iasi",
"Pacurari",
"Alexandru cel Bun",
"Nicolina",
"Bucium"
 };

 Strada harta[NR_NODURI][NR_NODURI];

void adauga_strada(int u,int v , char* nume,int dist,int v_max)
{harta[u][v].exista=true;
 harta[u][v].distanta=dist;
 harta[u][v].viteza_max=v_max;
 strcpy(harta[u][v].nume,nume);
 harta[v][u] = harta[u][v];
}

void initializare_harta() {
    memset(harta, 0, sizeof(harta));

    // DEFINIREA CELOR 10 STRAZI DIN IASI
    adauga_strada(0, 1, "Bd. Carol I", 3, 50);          // Unirii si Copou
    adauga_strada(0, 2, "Bd. Independentei", 2, 50);    // Unirii si Tg Cucu
    adauga_strada(0, 5, "Strada Arcu", 2, 50);          // Unirii si Gara
    adauga_strada(2, 4, "Str. Elena Doamna", 3, 50);    // Tg Cucu si Tudor
    adauga_strada(3, 4, "Bd. Primaverii", 2, 60);       // Podu Ros si Tudor
    adauga_strada(3, 0, "Strada Palat", 2, 40);         // Podu Ros si Unirii
    adauga_strada(3, 8, "Sos. Nicolina", 5, 60);        // Podu Ros si Nicolina
    adauga_strada(3, 9, "Sos. Bucium", 6, 70);          // Podu Ros si Bucium
    adauga_strada(5, 7, "Bd. Alexandru", 3, 50);        // Gara si Alexandru
    adauga_strada(5, 6, "Sos. Pacurari", 4, 70);        // Gara si Pacurari
    
    printf("SERVER: Harta Iasi incarcata cu succes.\n");
}

typedef struct{
    int client_sockfd;
    int client_id;
    float viteza_curenta;
    int nod_curent;
    bool cerere_vreme;
    bool cerere_sport;
    bool cerere_preturi;
    
}ClientConectat;
ClientConectat clienti[NR_CLIENTI_MAX];

void initializare_clienti(){
    for(int i=0;i<NR_CLIENTI_MAX;i++){
        clienti[i].client_sockfd=-1;
        clienti[i].client_id=-1;
        clienti[i].viteza_curenta=0.0;
        clienti[i].cerere_vreme=false;
        clienti[i].cerere_sport=false;
        clienti[i].cerere_preturi=false;
    }
}




int main()
{
    srand(time(NULL)); 
    initializare_harta();
 int server_sockfd;
 
 initializare_clienti();

 //Socket Server
 server_sockfd=socket(AF_INET,SOCK_STREAM,0);
if(server_sockfd<0){
    perror("Eroare:Socket-ul serverului nu a fost creat");
    exit(EXIT_FAILURE);
}



//Refolosirea portului
int refolosire=1;
if(setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&refolosire,sizeof(refolosire)))
{
    perror("Eroare:Setoskopt");
    exit(EXIT_FAILURE);
}


//Crearea server_addr pentru bind
struct sockaddr_in server_addr;
memset(&server_addr,0,sizeof(server_addr));
server_addr.sin_family=AF_INET;
server_addr.sin_port = htons(PORT);
server_addr.sin_addr.s_addr = INADDR_ANY;

//Bind()
if(bind(server_sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))){
    perror("Eroare: Bind");
    exit(EXIT_FAILURE);
}


//Listen()
if(listen(server_sockfd,NR_CLIENTI_MAX)){
    perror("Eroare:Listen");
    exit(EXIT_FAILURE);
}
printf("SERVER: Serverul a pornit si asculta la portul %d\n",PORT);


//Select()
fd_set read_fds;
int max_fd;
int  activitate;
int socket_nou;
struct sockaddr_in client_addr;
socklen_t client_len=sizeof(client_addr);
 while(1)
 {
    FD_ZERO(&read_fds);
    FD_SET (server_sockfd,&read_fds);
    max_fd=server_sockfd;

    for(int i=0;i<NR_CLIENTI_MAX;i++){
        if(clienti[i].client_sockfd!=-1){
            FD_SET(clienti[i].client_sockfd,&read_fds);
            if(clienti[i].client_sockfd>max_fd){
                max_fd=clienti[i].client_sockfd;
            }
        }
    }

    activitate=select(max_fd+1,&read_fds,NULL,NULL,NULL);
    if(activitate<0 && errno!=EINTR){
        perror("Eroare: Select");
        exit(EXIT_FAILURE);
 }
 //CLIENT NOU
    if(FD_ISSET(server_sockfd,&read_fds)){
        socket_nou=accept(server_sockfd,(struct sockaddr*)&client_addr,&client_len);
        if(socket_nou<0){
            perror("Eroare: Accept");
            exit(EXIT_FAILURE);
        }
        printf("SERVER: S-a conectat un nou client, socket_nou=%d\n",socket_nou);
        for(int i=0;i<NR_CLIENTI_MAX;i++){
            if(clienti[i].client_sockfd==-1){
                clienti[i].client_sockfd=socket_nou;
                clienti[i].client_id=i;
                break;
            }
        }
    }

               // CLIENT EXISTENT
for (int i = 0; i < NR_CLIENTI_MAX; i++) {
    
    
    if (clienti[i].client_sockfd > 0) {
        
      
        if (FD_ISSET(clienti[i].client_sockfd, &read_fds)) {
            
            pachet_generic pachet_primit;
           
            int lungmsj = read(clienti[i].client_sockfd, &pachet_primit, sizeof(pachet_generic));
            
         
            pachet_generic pachet_raspuns;
            memset(&pachet_raspuns, 0, sizeof(pachet_generic));
            pachet_raspuns.id_sursa = 0; // Serverul are ID 0

            // Deconectare
            if (lungmsj == 0) {
            
                getpeername(clienti[i].client_sockfd, (struct sockaddr*)&client_addr, &client_len);
                printf("SERVER: Clientul %d (%s) s-a deconectat.\n", i, inet_ntoa(client_addr.sin_addr));
                close(clienti[i].client_sockfd);
                
              
                clienti[i].client_sockfd = -1;
                clienti[i].client_id = -1;
                clienti[i].viteza_curenta = 0.0;
                clienti[i].cerere_vreme = false;
                clienti[i].cerere_sport = false;
                clienti[i].cerere_preturi = false;
            } 
            
            //Mesaj Primit
            else if (lungmsj == sizeof(pachet_generic)) {
                printf("[SERVER] Primit comanda tip %d de la clientul %d\n", pachet_primit.tip, i);

                
                switch (pachet_primit.tip) {
                    
                    case COMANDA_LOGIN:
                        clienti[i].client_id = pachet_primit.id_sursa; 
                        clienti[i].nod_curent = rand() % NR_NODURI;
                        pachet_raspuns.tip = SUCCES;
                        strcpy(pachet_raspuns.text, "Login efectuat cu succes.");
                        break;
                    case COMANDA_CERE_VECINI: {
                            int nod = clienti[i].nod_curent;
                            char buffer[512] = "";
                            sprintf(buffer, "Din '%s' poti merge spre:\n", nume_noduri[nod]);
                            
                            bool gasit = false;
                            for(int j=0; j<NR_NODURI; j++) {
                                if(harta[nod][j].exista) {
                                    char linie[100];
                                    sprintf(linie, " -> Nod %d (%s) : %s [%d km, Max %d km/h]\n", 
                                            j, nume_noduri[j], harta[nod][j].nume, 
                                            harta[nod][j].distanta, harta[nod][j].viteza_max);
                                    strcat(buffer, linie);
                                    gasit = true;
                                }
                            }
                            if(!gasit) strcat(buffer, " -> Niciun drum disponibil (Fundatura).\n");
                            
                            strcpy(pachet_raspuns.text, buffer);
                            pachet_raspuns.tip = RASPUNS_LISTA_VECINI;
                            break;
                        }
                    case COMANDA_DEPLASARE: {
                            int destinatie = pachet_primit.id_sursa; 
                            float viteza_dorita = pachet_primit.viteza;
                            int curent = clienti[i].nod_curent;

                            // Verificari
                            if(destinatie < 0 || destinatie >= NR_NODURI || !harta[curent][destinatie].exista) {
                                pachet_raspuns.tip = EROARE;
                                strcpy(pachet_raspuns.text, "Nu exista drum direct spre acel nod!");
                            } else {
                                // Deplasare reusita
                                clienti[i].nod_curent = destinatie;
                                clienti[i].viteza_curenta = viteza_dorita;
                                
                                int limita = harta[curent][destinatie].viteza_max;
                                char mesaj_extra[100] = "";
                                
                                if(viteza_dorita > limita) {
                                    sprintf(mesaj_extra, " Ai depasit limita! (Viteza %.0f > %d)", viteza_dorita, limita);
                                } else {
                                    strcpy(mesaj_extra, " Ai condus regulamentar.");
                                }

                                sprintf(pachet_raspuns.text, "Ai ajuns in '%s'.%s", 
                                        nume_noduri[destinatie], mesaj_extra);
                                pachet_raspuns.tip = RASPUNS_DEPLASARE_OK;
                            }
                            break;
                        }
                    case COMANDA_LOGOUT:
                        pachet_raspuns.tip = SUCCES;
                        strcpy(pachet_raspuns.text, "Logout.");
                        break;

                    case COMANDA_VITEZA:
                        clienti[i].viteza_curenta = pachet_primit.viteza;
                        pachet_raspuns.tip = RASPUNS_VITEZA;
                        sprintf(pachet_raspuns.text, "Viteza %.2f km/h inregistrata.", pachet_primit.viteza);
                        break;

                    case COMANDA_RAPORTEAZA_ACCIDENT: {
    
                        int loc_accident = clienti[i].nod_curent;
    
    
                           char mesaj_alerta[512];
                            sprintf(mesaj_alerta, "ALERTA TRAFIC: Accident raportat in zona %s (Nod %d)!", 
                            nume_noduri[loc_accident], loc_accident);
    
                             printf("SERVER: Broadcast - %s\n", mesaj_alerta);

    
    for(int j = 0; j < NR_CLIENTI_MAX; j++) {
        if(clienti[j].client_sockfd != -1) {
            pachet_generic pachet_broadcast;
            memset(&pachet_broadcast, 0, sizeof(pachet_generic));
            pachet_broadcast.tip = RASPUNS_ACCIDENT;
            strcpy(pachet_broadcast.text, mesaj_alerta);
            write(clienti[j].client_sockfd, &pachet_broadcast, sizeof(pachet_generic));
        }
    }
    break;
}

                    case COMANDA_INFO_VREME:
                        clienti[i].cerere_vreme = true; 
                        pachet_raspuns.tip = RASPUNS_INFO_VREME;
                        strcpy(pachet_raspuns.text, "Cer noros, 10 grade.");
                        break;

                    case COMANDA_INFO_SPORT:
                        clienti[i].cerere_sport = true;
                        pachet_raspuns.tip = RASPUNS_INFO_SPORT;
                        strcpy(pachet_raspuns.text, "PSG a castigat cu 5-3.");
                        break;

                    case COMANDA_INFO_PRETURI:
                        clienti[i].cerere_preturi = true;
                        pachet_raspuns.tip = RASPUNS_INFO_PRETURI;
                        strcpy(pachet_raspuns.text, "Pret Diesel: 7.80 RON. Pret Benzina: 7.20 RON.");
                        break;

                    default:
                        pachet_raspuns.tip = EROARE;
                        strcpy(pachet_raspuns.text, "Comanda necunoscuta.");
                        break;
                }

               
                write(clienti[i].client_sockfd, &pachet_raspuns, sizeof(pachet_generic));
                printf("SERVER: Trimis raspuns tip %d catre clientul %d.\n", pachet_raspuns.tip, i);
            }
            
            else {
                printf("SERVER: Eroare de citire sau pachet incomplet de la clientul %d. Lungime primita: %d\n", i, lungmsj);
            }
        }
    }

 }
 }
 return 0;
}


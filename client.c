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
#define PORT 8080
void afiseaza_meniu_start() {
    printf("\n==================================================\n");
    printf("   BINE AI VENIT  \n");
    printf("==================================================\n");
    printf("COMENZI DISPONIBILE:\n");
    printf(" [1] login                 -> Autentificare si locatia de start\n");
    printf(" [2] vecini                -> Vezi unde poti merge din locatia curenta\n");
    printf(" [3] muta <id_nod> <km/h>  -> Te deplasezi \n");
    printf(" [4] accident              -> Raporteaza un accident in zona ta\n"); 
    printf(" [5] vreme                 -> Informatii meteo\n");
    printf(" [6] sport                 -> Stiri sportive\n");
    printf(" [7] preturi               -> Preturi carburant\n");
    printf(" [0] logout                -> Deconectare\n");
    printf("==================================================\n");
    printf(">> Scrie o comanda: ");
    fflush(stdout); 
}
int gestionare_comanda(int sockfd, char*comanda)
{
    float valoare=0.0;
    int destinatie=0;
    char comanda_copie[256];
    pachet_generic pachet_de_trimis;
    memset(&pachet_de_trimis,0,sizeof(pachet_generic));
    if (sscanf(comanda, "%s %f", comanda_copie, &valoare) < 1) {
        printf(" Comanda invalida. Acceptate: login, logout, vreme, sport, preturi, viteza <valoare>\n");
        return 1;
    }
    pachet_de_trimis.id_sursa = 1;
    if (strcmp(comanda_copie, "login") == 0) {
        pachet_de_trimis.tip = COMANDA_LOGIN;
        if (valoare > 0) pachet_de_trimis.id_sursa = (int)valoare; 
        strcpy(pachet_de_trimis.text, "Cerere de logare.");
    }
    else if (strcmp(comanda_copie, "vecini") == 0) {
        pachet_de_trimis.tip = COMANDA_CERE_VECINI;
        strcpy(pachet_de_trimis.text, "Cerere vecini");
    }
   else if (strcmp(comanda_copie, "muta") == 0) {
        
        if(sscanf(comanda, "%*s %d %f", &destinatie, &valoare) < 2) {
            printf("Folosire: muta <ID_NOD> <VITEZA>\n");
            return 1;
        }
        pachet_de_trimis.tip = COMANDA_DEPLASARE;
        pachet_de_trimis.id_sursa = destinatie; 
        pachet_de_trimis.viteza = valoare;
    }
    else if (strcmp(comanda_copie, "logout") == 0) {
        pachet_de_trimis.tip = COMANDA_LOGOUT;
        strcpy(pachet_de_trimis.text, "Cerere de deconectare.");
    }

    else if (strcmp(comanda_copie, "viteza") == 0) {
        pachet_de_trimis.tip = COMANDA_VITEZA;
        pachet_de_trimis.viteza = valoare;
        sprintf(pachet_de_trimis.text, "Raportare viteza: %.2f", valoare);
    }



    else if (strcmp(comanda_copie, "accident") == 0) {
        pachet_de_trimis.tip = COMANDA_RAPORTEAZA_ACCIDENT;
        strcpy(pachet_de_trimis.text, "Accident detectat/raportat manual.\n");
    }
    else if (strcmp(comanda_copie, "vreme") == 0) {
        pachet_de_trimis.tip = COMANDA_INFO_VREME;
        strcpy(pachet_de_trimis.text, "Cerere de informatii despre vreme.\n");

    }
    else if (strcmp(comanda_copie, "sport") == 0) {
        pachet_de_trimis.tip = COMANDA_INFO_SPORT;
        strcpy(pachet_de_trimis.text, "Cerere de informatii despre sport.\n");

    }
    else if (strcmp(comanda_copie, "preturi") == 0) {
        pachet_de_trimis.tip = COMANDA_INFO_PRETURI;
        strcpy(pachet_de_trimis.text, "Cerere de informatii despre preturi.\n");

    }
    else {
        printf("Comanda necunoscuta: %s\n", comanda_copie);
        return 1; 
    }
    write(sockfd, &pachet_de_trimis, sizeof(pachet_generic));
    printf("CLIENT: Trimis comanda tip %d catre server.\n", pachet_de_trimis.tip);

    if (pachet_de_trimis.tip == COMANDA_LOGOUT) {
        return 0; 
    }
    
    return 1; 
}



int main(){
    int client_sockfd;
    
    char comanda_tastatura[256];

     //Socket Client
     client_sockfd=socket(AF_INET,SOCK_STREAM,0);
     if(client_sockfd<0){
        perror("Eroare:Socket-ul clientului nu a fost creat");
        exit(EXIT_FAILURE);

     }
     struct sockaddr_in server_addr;
     memset(&server_addr,0,sizeof(server_addr));
     server_addr.sin_family=AF_INET;
     server_addr.sin_port=htons(PORT);

     if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr)<=0){
        perror("Eroare:Adresa IP este invalida");
        exit(EXIT_FAILURE);
     }
 //connect()
  if (connect(client_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Eroare: Conectarea la server a esuat");
    exit(EXIT_FAILURE);
}
  afiseaza_meniu_start();
 printf("CLIENT: Conectat la server\n");

 
 fd_set read_fds;
 int max_fd;
 int  activitate;
  max_fd=client_sockfd;
  while(1){
    FD_ZERO(&read_fds);
    FD_SET(0,&read_fds);
    FD_SET(client_sockfd,&read_fds);
    activitate=select(max_fd+1,&read_fds,NULL,NULL,NULL);
    if(activitate<0){
        perror("Eroare: Select (Client)");
        exit(EXIT_FAILURE);
    }
    //Clientul scrie
    if(FD_ISSET(0,&read_fds)){
       
        if(fgets(comanda_tastatura,sizeof(comanda_tastatura),stdin)==NULL){
            continue;
        }
        comanda_tastatura[strcspn(comanda_tastatura,"\n")]=0;
        if(gestionare_comanda(client_sockfd,comanda_tastatura)==0){
              break;
        }
    }
   //Clientul primeste rasp. de la sv
   if(FD_ISSET(client_sockfd,&read_fds)){
    pachet_generic pachet_raspuns;
    int bytes_primiti = read(client_sockfd, &pachet_raspuns, sizeof(pachet_generic));
    if(bytes_primiti==0){
        printf("CLIENT: Serverul a inchis conexiunea.\n");
        break;
    }
    else if(bytes_primiti<0){
        perror("Eroare: Citire de la server");
        exit(EXIT_FAILURE);
    }
    else if(bytes_primiti==sizeof(pachet_generic)){
        printf("CLIENT: Raspuns primit de tip %d: %s\n", pachet_raspuns.tip, pachet_raspuns.text);
        fflush(stdout);
    }
   }

  } 
  close(client_sockfd);
  return 0;
}
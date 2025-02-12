#include <stdio.h>       // printf(), perror()
#include <stdlib.h>      // getenv(), atoi()
#include <unistd.h>      // read(), write(), close()
#include <sys/socket.h>  // socket(), setsockopt(), bind(), listen(), accept()
#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY

#include "NetworkConfig.hpp"

// pour se connecter au serveur, utiliser l'adresse ip de la machine qui a lancé le serveur

int main() {

    // Initialisation du socket
    int sock_fd = create_socket();

    // Initialisation des paramètres de l'adresse
    struct sockaddr_in serv_addr;
    // memset(&serv_addr, 0, sizeof(serv_addr)); // Assurez-vous que tous les champs sont bien initialisés
    serv_addr.sin_family = AF_INET;

    // Récupération des variables d'environnement
    set_ip(serv_addr);
    set_port(serv_addr);

    // Connexion au serveur
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect()");
        close(sock_fd);
        return 1;
    }
    printf("Connexion réussie au serveur :%d\n",
           ntohs(serv_addr.sin_port));

    écrire(sock_fd);

    close(sock_fd);
    return 0;
}


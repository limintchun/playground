#include <stdio.h>       // printf(), perror()
#include <stdlib.h>      // getenv(), atoi()
#include <unistd.h>      // read(), write(), close()
#include <sys/socket.h>  // socket(), setsockopt(), bind(), listen(), accept()
#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY
#include <string.h>      // memset()

#define MAX_PLAYER 9

#include "NetworkConfig.hpp"

int main() {
    // Initialisation du socket
    int server_fd = create_socket();
    sockaddr_in address;
    init_address(address);


    // Liaison du socket à une adresse et un port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind()");
        close(server_fd);
        return 1;
    }

    // Écoute sur le socket pour les demandes de connexion
    if (listen(server_fd, MAX_PLAYER) < 0) { // Longueur de la file d'attente de connexion fixée à MAX_PLAYER
        perror("listen()");
        close(server_fd);
        return 1;
    }
    printf("Serveur en écoute sur le port %d...\n", ntohs(address.sin_port));

    // Attente d'une connexion client
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0) {
        perror("accept()");
        close(server_fd);
        return 1;
    }
    printf("Connexion acceptée.\n");

    lire(new_socket, server_fd);

    // Fermeture des sockets
    close(new_socket);
    close(server_fd);

    return 0;
}

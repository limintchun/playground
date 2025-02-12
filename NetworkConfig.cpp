#include <iostream>
#include <cerrno>
#include <cstring>
#include <unistd.h>      // read(), write(), close()
#include <arpa/inet.h>  // inet_pton()
#include <netinet/in.h> // struct sockaddr_in, INADDR_ANY
#include <sys/socket.h> // socket(), setsockopt()

#include "NetworkConfig.hpp"

#define MAX_SIZE 1024

int create_socket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket()" << std::strerror(errno) << std::endl;
        return 1;
    }

    // Ajout d'option au socket, comme la capacité de réutiliser une adresse ou un port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < -1) {
        perror("setsockopt()");
        close(server_fd);
        return 1;
    }

    return server_fd;
}

void set_port(sockaddr_in &address) {
    char *local_port = getenv("PORT_SERVEUR"); // En bash, utilisez export PORT_SERVEUR=1234
    if (local_port == NULL) {
        printf("PORT_SERVEUR non initialisé. PORT_SERVEUR défini à 1234\n");
        address.sin_port = htons(1234);
        local_port = "1234";
    }

    // Conversion de la variable d'environnement en entier et validation
    int port = atoi(local_port);
    if (port >= 1 && port <= 65535) {
        address.sin_port = htons(port); // Convertir en format réseau
    } else {
        printf("PORT_SERVEUR invalide. Utilisation du port par défaut : 1234.\n");
        address.sin_port = htons(1234); // Port par défaut
    }
}

void set_ip(sockaddr_in &address) {
    char *local_ip = getenv("IP_SERVEUR");
    if (local_ip == NULL) {
        printf("IP_SERVEUR non initialisée. L'adresse ip est initialisé par défaut à 127.0.0.1\n");
        inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    }
    else {
        if (inet_pton(AF_INET, local_ip, &address.sin_addr) == 1) {
            inet_pton(AF_INET, local_ip, &address.sin_addr);
        }
        else if (inet_pton(AF_INET, local_ip, &address.sin_addr) == 0){
            inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
        }
    }

}

struct sockaddr_in init_address(struct sockaddr_in &address) {
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    set_port(address);

    return address;
}

int lire(int new_socket, int server_fd) {
    // Lecture des données envoyées par le client
    char buffer[MAX_SIZE] = {0}; // Initialisation à 0 pour éviter des caractères parasites
    ssize_t bytes_read = read(new_socket, buffer, MAX_SIZE - 1); // MAX_SIZE - 1 pour garder la place pour le '\0'
    if (bytes_read < 0) {
        perror("read()");
        close(new_socket);
        close(server_fd);
        return 1;
    }
    printf("Message reçu : %s\n", buffer);
    return 0;

}

int écrire(int sock_fd) {
    char message[1024];
    scanf("%s", message);
    if (write(sock_fd, message, strlen(message)) < 0) {
        perror("write()");
        close(sock_fd);
        return 1;
    }
    return 0;
}

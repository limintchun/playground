#ifndef NETWORKCONFIG_H 
#define NETWORKCONFIG_H
#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY
#include <map>
#include <string>
#include <mutex>


extern std::map<std::string, int> clients;
extern std:: mutex clients_mutex;

int create_socket();
void set_port(sockaddr_in &address);
void set_ip(sockaddr_in & address);
struct sockaddr_in init_address(struct sockaddr_in &address);
int lire(int new_socket, int server_fd); // utilisé pour tester le server
int écrire(int sock_fd);


#endif // !NETWORK_CONFIG_H
//

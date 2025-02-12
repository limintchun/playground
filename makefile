# Nom des exécutables
SERVER = server
CLIENT = client

# Compilateur
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11

# Fichiers sources
SRV_SRC = server.cpp NetworkConfig.cpp
CLI_SRC = client.cpp NetworkConfig.cpp

# Fichiers objets
SRV_OBJ = $(SRV_SRC:.cpp=.o)
CLI_OBJ = $(CLI_SRC:.cpp=.o)

# Compilation des exécutables
all: $(SERVER) $(CLIENT)

$(SERVER): $(SRV_OBJ)
	$(CXX) $(CXXFLAGS) -o $(SERVER) $(SRV_OBJ)

$(CLIENT): $(CLI_OBJ)
	$(CXX) $(CXXFLAGS) -o $(CLIENT) $(CLI_OBJ)

# Compilation des fichiers objets
%.o: %.cpp sockets.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage des fichiers objets et exécutables
clean:
	rm -f $(SRV_OBJ) $(CLI_OBJ) $(SERVER) $(CLIENT)

# Nettoyage complet
distclean: clean
	rm -f *~


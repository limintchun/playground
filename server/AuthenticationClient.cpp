#include "AuthenticationClient.hpp"



AuthenticationClient::AuthenticationClient(): isUserAuthenticated_{false} {
    isUserAuthenticated_ = checkAuth();
}


bool AuthenticationClient::isPasswordStrong(const std::string& password) {
    const std::regex passwordPattern("^(?=.*[A-Z])(?=.*[a-z])(?=.*\\d)(?=.*[@#$%^&+=!]).{8,}$");
    return std::regex_match(password, passwordPattern);
}

void AuthenticationClient::singUserIn() {
    std::string username = getUsername();
    std::cout << "Mot de passe :";
    std::string password = getPassword();

    // Requête au serveur de connecter le compte et va renvoyer une struct contenant entre-autre l'ID de session si c'est bon.

    std::string sessionID = "testTEST55"; //TODO TEST À SUPP
    storeSessionID(sessionID);
}

void AuthenticationClient::singUserUp() {
    std::string username = getUsername();
    std::cout << "Entrer un mot de passe (min. 8 caractères, une majuscule, un chiffre et un symbole) :";
    std::string password = getPassword();
    while (!isPasswordStrong(password)) {
        std::cout <<
            "Le mot de passe doit contenir min. 8 caractères, une majuscule, un chiffre et un symbole. Mot de passe :";
        password = getPassword();
    }

    // Requête au serveur de créer un compte avec username,password. Le serveur réponds si c'est ok ou pas. Renvoie une struct contenant entre-autre l'ID de session si c'est ok.
    storeSessionID(password); // J'ai mis ça en attendant pr que ça compile.
}

bool AuthenticationClient::checkAuth() {
    // Check si il y'a un fichier de session, si oui on check
    std::ifstream file;
    file.open("session", std::ios::in | std::ios::trunc);
    if (file.fail()) {
        std::cerr << "[ERREUR] Problème d'I/O Fichier" << std::endl;
        isUserAuthenticated_ = false;
        return false;
    }
    std::string sessionID;
    std::getline(file, sessionID);
    file.close();

    if (sessionID.size() != 36) {
        std::cerr << "[WARNING] Token de session invalide" << std::endl;
    };

    //
    //
    //  ENVOI DE L'ID AU SERVEUR POUR UNE VERIF
    //
    //

    bool sessionIDStatus = true; // Sera la réponse du serveur
    if (sessionIDStatus) {
        std::cerr << "[LOG] L'utilisateur est authentifié" << std::endl;
        isUserAuthenticated_ = true;
        return true;
    }
    else {
        std::cout << "Vous n'êtes pas authentifié, veuillez créer ou vous connecter à un compte" << std::endl;
        isUserAuthenticated_ = false;
        return false;
    }
}



bool AuthenticationClient::getAuthStatus() const {
    return isUserAuthenticated_;
}

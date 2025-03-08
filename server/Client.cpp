#include "Client.h"

Client::Client(uint16_t port) : is_user_authenticated_(false), socket_(io_context_),
                                endpoint_(tcp::endpoint(ip::make_address("127.0.0.1"), port)),
                                messages_in_(std::make_shared<ThreadSafeQueue<message>>()) {}


void Client::test_client() {
    // ECHO TEST
    // // construction du message test
    // message msg;
    // msg.msg_header.instruction = instruct_t::ECHO_MSG;
    // std::string text = "Hello les guys";
    // msg.msg_header.category = category_t::SOCIAL;
    // msg.body = std::vector<uint8_t>(text.begin(), text.end());
    // msg.msg_header.size = msg.body.size();
    //
    // this->message_server(msg);

    checkAuth();
    checkAuth();
}

void Client::handle_message(message& msg) {
    // handling the message depending on the instruct...
    const header header = msg.msg_header;
    std::cout << "[Client] Handling a msg" << std::endl;
    switch (header.instruction) {
        case instruct_t::ECHO_MSG: {
            std::cout << std::string(msg.body.begin(), msg.body.end()) << std::endl;
            break;
        }
        case instruct_t::SIGN_IN: {
            std::string json_data_in = std::string(msg.body.begin(), msg.body.end());
            nlohmann::json json_in = nlohmann::json::parse(json_data_in);
            if (json_data_in != "0") {
                is_user_authenticated_ = true;
                const std::string username = json_in.at("username");
                const std::string token = json_in.at("token");
                username_ = username;
                storeSessionInfo(token, username);
                std::cout << "[CLIENT_LOG] Sucessfully logged in as " << username << std::endl;
            }
            else {
                is_user_authenticated_ = false;
                storeSessionInfo(json_data_in, json_data_in);
                std::cerr << "[CLIENT_LOG] Username/Password combo invalid." << std::endl;
            }
            break;
        }
        case instruct_t::CHECK_TOKEN: {
            std::cout << std::string(msg.body.begin(), msg.body.end()) << std::endl;
            break;
        }
        default:
            break;
    }
}

void Client::storeSessionInfo(const std::string& client_token, const std::string& username) {
    std::ofstream file;
    file.open("session", std::ios::out | std::ios::trunc);
    file << client_token << "\n" << username;
    file.close();
}

bool Client::getAuthStatus() const {
    return is_user_authenticated_;
}

std::string Client::getUsername() {
    std::string username;
    getline(std::cin, username);
    while (username.size() > 16 || username.size() < 3) {
        std::cout << "Enter Username (min 3 chararcters, max 16) : ";
        getline(std::cin, username);
    }
    return username;
}

std::string Client::getPassword() {
    boost::asio::io_context io;
    boost::asio::posix::stream_descriptor input(io, ::dup(STDIN_FILENO));
    termios old_t{}, new_t{};
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;
    new_t.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
    std::string password;
    getline(std::cin, password);
    std::cout << std::endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
    return password;
}

void Client::singUserIn() {
    std::cout << "Nom d'utilisateur : ";
    std::string username;
    getline(std::cin, username);
    std::cout << "Mot de passe : ";
    std::string password = getPassword();

    message msg;
    msg.msg_header.instruction = instruct_t::SIGN_IN;
    msg.msg_header.category = category_t::SOCIAL;
    nlohmann::json json_out;
    json_out["username"] = username;
    json_out["password"] = password;
    std::string json_data_out = json_out.dump();
    msg.body = std::vector<uint8_t>(json_data_out.begin(), json_data_out.end());
    msg.msg_header.size = msg.body.size();
    this->message_server(msg);


}

void Client::start() {
    running_ = true;
    // On tente de se connecter au serveur
    connect(endpoint_);
    // running the asio context in a secondary thread to allow
    // update to work !
    context_thread_ = std::thread([this]() { io_context_.run(); });
}

void Client::stop() {
    running_ = false;
    io_context_.stop();
    if (context_thread_.joinable()) context_thread_.join();
}


void Client::update() {
    // refreshly handle the messaging in
    while (!messages_in_->is_empty()) {
        auto msg = messages_in_->pop();
        // dispatching the message
        if (msg.has_value()) {
            handle_message(*msg);
        }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10000)); // 10 ms
}

void Client::checkAuth() {
    // Check si il y'a un fichier de session, si oui on check
    std::ifstream file("session");
    if (!file) {
        std::cerr << "[ERREUR] Problème d'I/O Fichier" << std::endl;
        is_user_authenticated_ = false;
        return;
    }

    std::string token;
    std::string username;
    std::getline(file, token);
    std::getline(file, username);
    file.close();

    if (token.size() != TOKEN_SIZE || username.size() < USERNAME_MIN_CHAR) {
        std::cerr << "[CLIENT_WARNING] Session token or username invalid" << std::endl;
        is_user_authenticated_ = false;
        return;
    };

    message msg;
    msg.msg_header.instruction = instruct_t::CHECK_TOKEN;
    msg.msg_header.category = category_t::SOCIAL;
    nlohmann::json json_out;
    json_out["username"] = username;
    json_out["token"] = token;
    std::string json_data_out = json_out.dump();
    msg.body = std::vector<uint8_t>(json_data_out.begin(), json_data_out.end());
    msg.msg_header.size = msg.body.size();
    this->message_server(msg);
}

void Client::message_server(const message& msg) {
    session->send_message(msg);
}

bool Client::is_running() const {
    return running_;
}

void Client::connect(tcp::endpoint& endpoint) {
    socket_.async_connect(endpoint, [this](boost::system::error_code ec) {
        std::cout << "[CLIENT_LOG] Accept" << std::endl;
        if (!ec) {
            session = std::make_unique<Session>(std::move(socket_), messages_in_, 0);
            session->start();
            test_client(); // j'ai échangé l'ordre pcq ça n'avait pas de sens aussi non (et des prob en +)
        }
        else {
            std::cerr << ec.message() << std::endl;
        }
    });
}

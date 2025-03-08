#include "SocialManager.h"

#include <boost/uuid.hpp>

SocialManager::SocialManager(std::shared_ptr<ThreadSafeQueue<message_out>> msg_out): message_out_(msg_out),
    database_user_manager_{database_.getDatabase()} {}

void SocialManager::start() {
    running_ = true;
    // launching the thread to handle the message in the queue
    worker_thread_ = std::thread([this]() {
        while (running_) {
            while (!message_queue_.is_empty()) {
                // handling the message depending on the instruction and if
                // needed to return smth to the client just build a msg_out
                // and call send_message
                auto msg = message_queue_.pop();
                if (msg.has_value()) {
                    // handling here
                    msg_handler(*msg);
                }

            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "[SOCIAL_MANAGER] Thread Stopped" << std::endl;
    });
}

void SocialManager::stop() {
    running_ = false;
    // https://stackoverflow.com/questions/38894188/thread-join-c-behaviour
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

std::string SocialManager::hashPassword(const std::string& plaintext) {
    return sw::sha512::calculate(plaintext);
}

std::string SocialManager::generateClientToken() {
    boost::uuids::random_generator generator;
    const boost::uuids::uuid session_id = generator();
    return boost::uuids::to_string(session_id);
}

std::string SocialManager::signUserIn(std::vector<uint8_t> user_combo) {
    std::string json_data_in(user_combo.begin(), user_combo.end());
    nlohmann::json json_in = nlohmann::json::parse(json_data_in);
    std::string username = json_in.at("username");
    std::transform(username.begin(), username.end(), username.begin(), [](unsigned char c) { return std::tolower(c); });
    const std::string password = json_in.at("password");
    const std::string password_hash = hashPassword(password);
    const std::string database_hash = database_user_manager_.getPasswordUsingName(username);
    if (database_hash == password_hash) {
        const std::string token = generateClientToken();
        database_user_manager_.setTokenUsingName(username, token);
        nlohmann::json json_out;
        json_out["username"] = username;
        json_out["token"] = token;
        std::string json_data_out = json_out.dump();
        return json_data_out;
    }
    return "0";
}

bool SocialManager::checkToken(std::vector<uint8_t> user_token) {
    std::string json_data_in(user_token.begin(), user_token.end());
    nlohmann::json json_in = nlohmann::json::parse(json_data_in);
    const std::string username = json_in.at("username");
    const std::string token = json_in.at("token");
    const std::string database_token = database_user_manager_.getTokenUsingName(username);
    return token == database_token;
}

void SocialManager::add_message(message& msg) {
    // handling the message properly
    message_queue_.push(msg);
}

void SocialManager::send_message(message_out& msg) {
    message_out_->push(msg);
}

void SocialManager::msg_handler(message& msg) {
    switch (msg.msg_header.instruction) {
        case instruct_t::ECHO_MSG: {
            std::cout << "[SOCIAL_MANAGER] ECHO_MSG received" << std::endl;
            // first init a new msg (maybe add a constructor...)
            message_out msg_out;
            // telling to who we gonna send here it is the same as echo does
            msg_out.session_id = msg.msg_header.sender_id;
            // changing the instruct if needed but here the Client send an echo
            // msg.msg_header.instruction = instruct_t::PING; as it is already a ping instruct...
            // moving the message to avoid any copy but at the end to avoid manipulating it after !!!
            msg_out.msg = std::move(msg);
            // adding to the queue for the server to handle
            send_message(msg_out);
            break;
        }
        case instruct_t::SIGN_IN: {
            std::cout << "[SOCIAL_MANAGER] SIGN_IN request received from " << msg.msg_header.sender_id << std::endl;
            std::string sign_in_output = signUserIn(msg.body);
            message_out msg_out;
            msg_out.session_id = msg.msg_header.sender_id;
            msg_out.msg = std::move(msg);
            msg_out.msg.body = std::vector<uint8_t>(sign_in_output.begin(), sign_in_output.end());
            msg_out.msg.msg_header.size = msg_out.msg.body.size();
            send_message(msg_out);
            break;
        }
        case instruct_t::CHECK_TOKEN: {
            std::cout << "[SOCIAL_MANAGER] CHECK_TOKEN request received from " << msg.msg_header.sender_id << std::endl;
            std::string check_token_output = std::to_string(checkToken(msg.body));
            message_out msg_out;
            msg_out.session_id = msg.msg_header.sender_id;
            msg_out.msg = std::move(msg);
            msg_out.msg.body = std::vector<uint8_t>(check_token_output.begin(),check_token_output.end());
            msg_out.msg.msg_header.size = msg_out.msg.body.size();
            send_message(msg_out);
            break;
        }
        default:
            break;
    }
}

Leaderboard::Leaderboard(const std::string& file) : filename(file) {
    loadFromFile();
} 

void Leaderboard::addScore(const std::string& name, int score) {
    std::sort(scores.begin(), scores.end(), [](const PlayerScore& a, const PlayerScore& b) {
        return a.score > b.score;
    });
    saveToFile();
}

void Leaderboard::display() {
    std::cout << "=== Leaderboard ===\n";
    for (size_t i = 0; i < scores.size(); ++i) {
        std::cout << i + 1 << ". " << scores[i].name << " - " << scores[i].score << "\n";
    }
    std::cout << "===================\n";
}

void Leaderboard::saveToFile() const {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << " pour l'écriture. \n";
        return;
    }

    for (const auto& entry: scores) {
        file << entry.name << " " << entry.score << "\n";
    }
}

void Leaderboard::loadFromFile() {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Aucun fichier existant. Création d'un nouveau leaderboard. \n";
        return ;
    }

    scores.clear();
    std::string name;
    int score;
    while (file >> name >> score) {
        scores.push_back({name, score});
    }

    std::sort(scores.begin(), scores.end(), [](const PlayerScore& a, const PlayerScore& b){
        return a.score > b.score;
    });
}


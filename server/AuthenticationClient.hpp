#pragma once
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <bits/fs_fwd.h>



class AuthenticationClient {
    bool isUserAuthenticated_;

public:
    AuthenticationClient();

    void singUserUp();
    bool isPasswordStrong(const std::string& password);
    bool checkAuth();
    [[nodiscard]] bool getAuthStatus() const;
};

#pragma once
#include <string>
#include "../storage/storage.h"

struct AuthResult {
    bool ok = false;
    std::string message;
};

class AuthService {
public:
    explicit AuthService(UserStorage storage);

    AuthResult register_user(const std::string& username, const std::string& password);
    AuthResult login_user(const std::string& username, const std::string& password);

    AuthResult update_nickname(const std::string& username, const std::string& new_nickname);
    AuthResult update_study_info(const std::string& username,
                                 const std::string& goal,
                                 const std::string& location,
                                 const std::string& time);

    // ✅ 新增：重置密码 + 删除用户
    AuthResult reset_password(const std::string& username, const std::string& new_password);
    AuthResult delete_user(const std::string& username);

private:
    UserStorage storage_;

    static bool is_valid_username(const std::string& s);
    static bool is_valid_password(const std::string& s);
    static std::string now_iso8601();
    static std::string weak_hash(const std::string& s);
};
//组员A：登录注册、密码管理、资料更新
#include "auth.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <functional>
#include <utility>

AuthService::AuthService(UserStorage storage)
    : storage_(std::move(storage)) {}

bool AuthService::is_valid_username(const std::string& s) {
    if (s.size() < 3 || s.size() > 32) return false;
    for (char c : s) {
        if (!(std::isalnum((unsigned char)c) || c == '_' || c == '-')) return false;
    }
    return true;
}

bool AuthService::is_valid_password(const std::string& s) {
    return s.size() >= 6 && s.size() <= 64;
}

std::string AuthService::now_iso8601() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

// 作业占位 hash
std::string AuthService::weak_hash(const std::string& s) {
    std::hash<std::string> h;
    auto v = h(s);
    std::ostringstream oss;
    oss << std::hex << v;
    return oss.str();
}

AuthResult AuthService::register_user(const std::string& username, const std::string& password) {
    if (!is_valid_username(username)) {
        return {false, "Invalid username (3-32 chars, [a-zA-Z0-9_-])."};
    }
    if (!is_valid_password(password)) {
        return {false, "Invalid password (6-64 chars)."};
    }

    std::string err;
    auto existing = storage_.find_by_username(username, err);
    if (!err.empty()) return {false, err};
    if (existing.has_value()) {
        return {false, "Username already exists."};
    }

    UserRecord u;
    u.schema_version = 2;
    u.uid = 0; // 简单版：不生成 uid
    u.username = username;
    u.password_hash = weak_hash(password);
    u.created_at = now_iso8601();
    u.nickname = username;

    u.goal = "";
    u.location = "";
    u.time = "";

    if (!storage_.append_user(u, err)) {
        return {false, err};
    }
    return {true, "Register success."};
}

AuthResult AuthService::login_user(const std::string& username, const std::string& password) {
    std::string err;
    auto uopt = storage_.find_by_username(username, err);
    if (!err.empty()) return {false, err};
    if (!uopt) return {false, "User not found."};

    auto input_hash = weak_hash(password);
    if (input_hash != uopt->password_hash) {
        return {false, "Wrong password."};
    }
    return {true, "Login success."};
}

AuthResult AuthService::update_nickname(const std::string& username, const std::string& new_nickname) {
    if (new_nickname.empty() || new_nickname.size() > 32) {
        return {false, "Invalid nickname (1-32 chars)."};
    }

    std::string err;
    auto uopt = storage_.find_by_username(username, err);
    if (!err.empty()) return {false, err};
    if (!uopt) return {false, "User not found."};

    UserRecord u = *uopt;
    u.nickname = new_nickname;

    if (!storage_.update_user(u, err)) {
        return {false, err};
    }
    return {true, "Nickname updated."};
}

AuthResult AuthService::update_study_info(const std::string& username,
                                         const std::string& goal,
                                         const std::string& location,
                                         const std::string& time) {
    std::string err;
    auto uopt = storage_.find_by_username(username, err);
    if (!err.empty()) return {false, err};
    if (!uopt) return {false, "User not found."};

    UserRecord u = *uopt;
    u.goal = goal;
    u.location = location;
    u.time = time;

    if (!storage_.update_user(u, err)) {
        return {false, err};
    }
    return {true, "Profile updated."};
}

//  重置密码（用户自己登录后改密码，或管理员重置用同一个接口）
AuthResult AuthService::reset_password(const std::string& username, const std::string& new_password) {
    if (!is_valid_password(new_password)) {
        return {false, "Invalid password (6-64 chars)."};
    }

    std::string err;
    auto uopt = storage_.find_by_username(username, err);
    if (!err.empty()) return {false, err};
    if (!uopt) return {false, "User not found."};

    UserRecord u = *uopt;
    u.password_hash = weak_hash(new_password);

    if (!storage_.update_user(u, err)) {
        return {false, err};
    }
    return {true, "Password reset."};
}

// 删除用户（管理员面板用）
AuthResult AuthService::delete_user(const std::string& username) {
    std::string err;
    if (!storage_.delete_user(username, err)) {
        return {false, err};
    }
    return {true, "User deleted."};
}
//组员A：登录注册、密码管理、资料更新
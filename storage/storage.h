#pragma once
#include <string>
#include <vector>
#include <optional>

struct UserRecord {
    std::string username;
    std::string password_hash;
    std::string created_at;
    int schema_version = 2;

    long long uid = 0;
    std::string nickname;

    // 学习信息
    std::string goal;       // 学习目标
    std::string location;   // 学习地点
    std::string time;       // 学习时间/学习时段
};

class UserStorage {
public:
    explicit UserStorage(std::string users_file);

    bool append_user(const UserRecord& user, std::string& err);
    bool load_all(std::vector<UserRecord>& out, std::string& err) const;
    std::optional<UserRecord> find_by_username(const std::string& username, std::string& err) const;

    bool update_user(const UserRecord& user, std::string& err);
    bool delete_user(const std::string& username, std::string& err);

private:
    std::string users_file_;

    static std::string escape_json(const std::string& s);
    static bool parse_user_line(const std::string& line, UserRecord& out);
};
//组员A：JSONL 存储、读取、更新、删除、迁移
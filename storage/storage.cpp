#include "storage.h"
#include <fstream>
#include <sstream>
#include <cstdio>

static std::string get_json_string_field(const std::string& line, const std::string& key) {
    std::string pat = "\"" + key + "\":\"";
    auto pos = line.find(pat);
    if (pos == std::string::npos) return "";
    pos += pat.size();
    auto end = line.find('"', pos);
    if (end == std::string::npos) return "";
    return line.substr(pos, end - pos);
}//查找并提取 JSON 字符串字段的值（如 "username":"xxx"）

static long long get_json_ll_field(const std::string& line, const std::string& key) {
    std::string pat = "\"" + key + "\":";
    auto pos = line.find(pat);
    if (pos == std::string::npos) return 0;
    pos += pat.size();
    auto end = line.find_first_of(",}", pos);
    if (end == std::string::npos) return 0;
    return std::stoll(line.substr(pos, end - pos));
}//查找并提取 JSON 数值字段的值（如 "uid":123）

UserStorage::UserStorage(std::string users_file)
    : users_file_(std::move(users_file)) {}//构造函数，保存用户数据文件名到成员变量 users_file_

std::string UserStorage::escape_json(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\\' || c == '"') out.push_back('\\');
        out.push_back(c);//写json字符串 遇到 \ 或 " 必须加 \ 转义，否则 JSON 解析会出错
    }
    return out;
}//安全写入 JSON

bool UserStorage::parse_user_line(const std::string& line, UserRecord& out) {
    try {
        out.schema_version = (int)get_json_ll_field(line, "schema_version");
        out.uid = get_json_ll_field(line, "uid");//查找 line 里 "uid":123 这种模式，把 123 提取出来，返回 long long 类型
        out.username = get_json_string_field(line, "username");//查找 line 里 "username":"xxx" 这种模式，把 xxx 提取出来，返回字符串
        out.password_hash = get_json_string_field(line, "password_hash");
        out.created_at = get_json_string_field(line, "created_at");
        out.nickname = get_json_string_field(line, "nickname");

        //  学习信息
        out.goal = get_json_string_field(line, "goal");
        out.location = get_json_string_field(line, "location");
        out.time = get_json_string_field(line, "time");

        // 兼容老数据：没 nickname 就默认 username
        if (out.nickname.empty()) out.nickname = out.username;
        return !out.username.empty();
    } catch (...) {
        return false;
    }
}//解析一行 JSON 字符串为 UserRecord 结构体，提取各字段

bool UserStorage::append_user(const UserRecord& u, std::string& err) {
    std::ofstream ofs(users_file_, std::ios::app);//以追加模式（std::ios::app）打开用户数据文件 users_file_，用于写入新用户
    if (!ofs) {
        err = "Failed to open users file for append: " + users_file_;
        return false;
    }

    ofs << "{"
        << "\"schema_version\":" << u.schema_version << ","
        << "\"uid\":" << u.uid << ","
        << "\"username\":\"" << escape_json(u.username) << "\","//每个字段都用 escape_json 进行转义
        << "\"password_hash\":\"" << escape_json(u.password_hash) << "\","
        << "\"created_at\":\"" << escape_json(u.created_at) << "\","
        << "\"nickname\":\"" << escape_json(u.nickname) << "\","
        << "\"goal\":\"" << escape_json(u.goal) << "\","
        << "\"location\":\"" << escape_json(u.location) << "\","
        << "\"time\":\"" << escape_json(u.time) << "\""
        << "}\n";

    if (!ofs) {
        err = "Failed to write user record.";
        return false;
    }
    return true;
}///追加写入一个用户到文件 打开文件 写入json格式的数据 失败则报错

bool UserStorage::load_all(std::vector<UserRecord>& out, std::string& err) const {
    std::ifstream ifs(users_file_);
    if (!ifs) {
        out.clear();
        return true; // 文件不存在=空库
    }

    out.clear();
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;//跳过空行
        UserRecord u;
        if (!parse_user_line(line, u)) {//提取各字段
            err = "Failed to parse user line.";
            return false;
        }
        out.push_back(std::move(u));//解析成功则把 u 加入 out
    }
    return true;
}//读取所有用户到 vector<UserRecord>。文件不存在则 out 清空并返回 true。逐行解析，遇到格式错误报错并返回 false

std::optional<UserRecord> UserStorage::find_by_username(const std::string& username, std::string& err) const {
    std::ifstream ifs(users_file_);
    if (!ifs) return std::nullopt;

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        UserRecord u;
        if (!parse_user_line(line, u)) {
            err = "Failed to parse user line.";
            return std::nullopt;
        }
        if (u.username == username) return u;
    }
    return std::nullopt;
}//查找指定用户名的用户。逐行读取并解析，找到则返回用户信息

bool UserStorage::update_user(const UserRecord& user, std::string& err) {
    std::vector<UserRecord> users;
    if (!load_all(users, err)) return false;//调用 load_all(users, err) 读取所有用户到 users 容器

    bool found = false;
    for (auto& u : users) {
        if (u.username == user.username) {
            u = user;
            found = true;
            break;
        }
    }//遍历
    if (!found) {
        err = "User not found for update.";
        return false;
    }
//更新的逻辑是写一个新文件覆盖旧文件 旧文件备份放丢失
    std::string tmp = users_file_ + ".tmp";//临时文件
    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs) {
        err = "Failed to open tmp file for rewrite: " + tmp;
        return false;
    }

    for (const auto& u : users) {
        ofs << "{"
            << "\"schema_version\":" << u.schema_version << ","
            << "\"uid\":" << u.uid << ","
            << "\"username\":\"" << escape_json(u.username) << "\","
            << "\"password_hash\":\"" << escape_json(u.password_hash) << "\","
            << "\"created_at\":\"" << escape_json(u.created_at) << "\","
            << "\"nickname\":\"" << escape_json(u.nickname) << "\","
            << "\"goal\":\"" << escape_json(u.goal) << "\","
            << "\"location\":\"" << escape_json(u.location) << "\","
            << "\"time\":\"" << escape_json(u.time) << "\""
            << "}\n";
    }

    ofs.close();
    if (!ofs) {
        err = "Failed while writing tmp file.";
        return false;
    }
    //备份
    std::string bak = users_file_ + ".bak";
    std::remove(bak.c_str());
    std::rename(users_file_.c_str(), bak.c_str());
    if (std::rename(tmp.c_str(), users_file_.c_str()) != 0) {
        err = "Failed to replace users file.";
        return false;
    }
    
    return true;
}//更新指定用户信息。先读出所有用户，找到目标用户并替换

bool UserStorage::delete_user(const std::string& username, std::string& err) {
    std::vector<UserRecord> users;
    if (!load_all(users, err)) return false;

    std::vector<UserRecord> kept;
    bool found = false;

    for (const auto& u : users) {
        if (u.username == username) {
            found = true;      // 发现要删的用户
            continue;          // 跳过 = 删除
        }//遍历 users，把要删除的用户跳过，其他用户放进 kept 容器
        kept.push_back(u);
    }

    if (!found) {
        err = "User not found.";
        return false;
    }

    // 重写文件：tmp -> 替换
    //把 kept 里的用户全部写入一个新的临时文件
    std::string tmp = users_file_ + ".tmp";
    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs) {
        err = "Failed to open tmp file.";
        return false;
    }

    for (const auto& u : kept) {
        ofs << "{"
            << "\"schema_version\":" << u.schema_version << ","
            << "\"uid\":" << u.uid << ","
            << "\"username\":\"" << escape_json(u.username) << "\","
            << "\"password_hash\":\"" << escape_json(u.password_hash) << "\","
            << "\"created_at\":\"" << escape_json(u.created_at) << "\","
            << "\"nickname\":\"" << escape_json(u.nickname) << "\","
            << "\"goal\":\"" << escape_json(u.goal) << "\","
            << "\"location\":\"" << escape_json(u.location) << "\","
            << "\"time\":\"" << escape_json(u.time) << "\""
            << "}\n";
    }

    ofs.close();
    if (!ofs) {
        err = "Failed while writing tmp file.";
        return false;
    }//检查写入是否成功

    std::string bak = users_file_ + ".bak";
    std::remove(bak.c_str());
    std::rename(users_file_.c_str(), bak.c_str()); // 旧文件可能不存在，忽略
    if (std::rename(tmp.c_str(), users_file_.c_str()) != 0) {
        err = "Failed to replace users file.";
        return false;
    }//备份原文件为 .bak，然后用新文件（.tmp）覆盖原文件

    return true;
}//删除指定用户名的用户。先读出所有用户，过滤掉要删除的用户，写入临时文件，最后用临时文件覆盖原文件
//组员A：JSONL 存储、读取、更新、删除、迁移
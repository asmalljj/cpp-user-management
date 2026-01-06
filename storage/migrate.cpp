#include "migrate.h"
#include "storage.h"
#include <fstream>
#include <vector>
#include <cstdio>   // std::remove, std::rename
//把旧版本的数据文件（比如只包含用户名、密码，没有uid、schema_version等字段）转换成新版本的数据格式（补齐新字段、调整内容）
bool migrate_users_v1_to_v2(const std::string& users_file, std::string& err) {
    UserStorage st(users_file);//创建 UserStorage 对象用于操作用户数据

    std::vector<UserRecord> users;
    if (!st.load_all(users, err)) return false;//调用 load_all 读取所有用户数据到 users

    // 生成新文件
    std::string tmp = users_file + ".tmp";
    std::ofstream ofs(tmp, std::ios::trunc);
    if (!ofs) {
        err = "Failed to create tmp file: " + tmp;
        return false;
    }//以覆盖方式打开

    long long next_uid = 1;
    for (auto& u : users) {//遍历所有用户，把旧数据补齐新字段
        u.schema_version = 2;
        if (u.uid == 0) u.uid = next_uid++;
        if (u.nickname.empty()) u.nickname = u.username;

        auto esc = [](const std::string& s) {//定义一个局部 lambda 函数 esc，用于对字符串做 JSON 转义（处理引号和反斜杠）
            std::string out;//如果遇到反斜杠（\）或双引号（"），就在前面加一个反斜杠，防止 JSON 解析出错
            for (char c : s) {//避免特殊字符导致格式错误
                if (c == '\\' || c == '"') out.push_back('\\');
                out.push_back(c);
            }
            return out;
        };

        ofs << "{"
            << "\"schema_version\":" << u.schema_version << ","
            << "\"uid\":" << u.uid << ","
            << "\"username\":\"" << esc(u.username) << "\","
            << "\"password_hash\":\"" << esc(u.password_hash) << "\","
            << "\"created_at\":\"" << esc(u.created_at) << "\","
            << "\"nickname\":\"" << esc(u.nickname) << "\""
            << "}\n";
    }//把每个用户按新格式写入 tmp 文件，每行一个 JSON 对象

    ofs.close();///关闭文件
    if (!ofs) {
        err = "Failed writing tmp file.";
        return false;
    }

    // 用 tmp 替换旧文件
    std::string bak = users_file + ".bak";///生成备份文件名
    std::remove(bak.c_str());//如果之前有备份文件，先删除它
    if (std::rename(users_file.c_str(), bak.c_str()) != 0) {
        //原来的用户数据文件重命名为 .bak（做备份）
    }
    if (std::rename(tmp.c_str(), users_file.c_str()) != 0) {
        err = "Failed to replace users file.";
        return false;
    }//刚才生成的 tmp 文件重命名为正式的用户数据文件
    return true;
}
//组员A：JSONL 存储、读取、更新、删除、迁移
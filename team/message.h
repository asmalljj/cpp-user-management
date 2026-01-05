#pragma once
#include <string>
#include <vector>

struct MessageRecord {
    long long team_id = 0;
    std::string from_user;
    std::string text;
    std::string time;
};

class MessageStorage {
public:
    explicit MessageStorage(std::string file);

    bool append(const MessageRecord& msg, std::string& err);
    bool load_by_team(long long team_id, std::vector<MessageRecord>& out, std::string& err) const;

private:
    std::string file_;

    static std::string escape_json(const std::string& s);
    static bool parse_line(const std::string& line, MessageRecord& out);
};
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
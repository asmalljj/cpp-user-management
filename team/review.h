#pragma once
#include <string>
#include <vector>

struct ReviewRecord {
    long long team_id = 0;
    std::string from_user;
    std::string to_user;
    int score = 0;           // 1~5
    std::string comment;
    std::string time;
};

class ReviewStorage {
public:
    explicit ReviewStorage(std::string file);

    bool append(const ReviewRecord& r, std::string& err);
    bool load_by_team(long long team_id, std::vector<ReviewRecord>& out, std::string& err) const;

private:
    std::string file_;

    static std::string escape_json(const std::string& s);
    static bool parse_line(const std::string& line, ReviewRecord& out);
};
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
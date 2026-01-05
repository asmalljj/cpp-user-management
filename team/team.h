#pragma once
#include <string>
#include <vector>
#include <optional>

// =====================
// Team storage (teams.jsonl)
// =====================
struct TeamRecord {
    long long team_id = 0;
    std::string member1;
    std::string member2;
    std::string created_at;
};

class TeamStorage {
public:
    explicit TeamStorage(std::string file);

    bool append(const TeamRecord& t, std::string& err);
    bool load_all(std::vector<TeamRecord>& out, std::string& err) const;

    std::optional<TeamRecord> find_by_team_id(long long team_id, std::string& err) const;
    std::optional<TeamRecord> find_by_username(const std::string& username, std::string& err) const;

    long long next_team_id(std::string& err) const;

private:
    std::string file_;

    static std::string escape_json(const std::string& s);
    static bool parse_line(const std::string& line, TeamRecord& out);
};

// =====================
// Business APIs
// =====================
void send_team_application(
    const std::string& current_username,
    const std::string& target_username,
    const std::string& app_file,
    std::string& err
);

void handle_incoming_applications(
    const std::string& current_username,
    const std::string& app_file,
    const std::string& team_file,
    std::string& err
);

void view_my_sent_applications(
    const std::string& current_username,
    const std::string& app_file,
    std::string& err
);

void team_chat_room(
    const std::string& current_username,
    const std::string& team_file,
    const std::string& msg_file,
    std::string& err
);

void team_review(
    const std::string& current_username,
    const std::string& team_file,
    const std::string& review_file,
    std::string& err
);

void analytics_dashboard(
    const std::string& user_file,
    const std::string& team_file,
    const std::string& review_file,
    std::string& err
);
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
#pragma once
#include <string>
#include <vector>
#include <optional>

struct ApplicationRecord {
    long long id = 0;
    std::string from_user;
    std::string to_user;
    std::string status;     // PENDING / ACCEPTED / REJECTED
    std::string created_at;
};

class ApplicationStorage {
public:
    explicit ApplicationStorage(std::string file);

    bool append(const ApplicationRecord& app, std::string& err);
    bool load_all(std::vector<ApplicationRecord>& out, std::string& err) const;
    std::optional<ApplicationRecord> find_by_id(long long id, std::string& err) const;

    bool update(const ApplicationRecord& app, std::string& err);
    bool delete_by_id(long long id, std::string& err);

    long long next_id(std::string& err) const;

private:
    std::string file_;

    static std::string escape_json(const std::string& s);
    static bool parse_line(const std::string& line, ApplicationRecord& out);
};
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
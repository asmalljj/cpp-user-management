#include "message.h"
#include <fstream>

MessageStorage::MessageStorage(std::string file) : file_(std::move(file)) {}

std::string MessageStorage::escape_json(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

static std::string extract_field(const std::string& line, const std::string& key) {
    auto pos = line.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = line.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < line.size() && (line[pos] == ' ')) pos++;

    if (pos < line.size() && line[pos] == '"') {
        size_t end = line.find("\"", pos + 1);
        return line.substr(pos + 1, end - pos - 1);
    } else {
        size_t end = line.find_first_of(",}", pos);
        return line.substr(pos, end - pos);
    }
}

bool MessageStorage::parse_line(const std::string& line, MessageRecord& out) {
    if (line.empty()) return false;
    out.team_id = std::stoll(extract_field(line, "team_id"));
    out.from_user = extract_field(line, "from");
    out.text = extract_field(line, "text");
    out.time = extract_field(line, "time");
    return true;
}

bool MessageStorage::append(const MessageRecord& msg, std::string& err) {
    std::ofstream fout(file_, std::ios::app);
    if (!fout.is_open()) { err = "open messages file failed"; return false; }

    fout << "{"
         << "\"team_id\":" << msg.team_id << ","
         << "\"from\":\"" << escape_json(msg.from_user) << "\","
         << "\"text\":\"" << escape_json(msg.text) << "\","
         << "\"time\":\"" << escape_json(msg.time) << "\""
         << "}\n";
    return true;
}

bool MessageStorage::load_by_team(long long team_id, std::vector<MessageRecord>& out, std::string& err) const {
    std::ifstream fin(file_);
    if (!fin.is_open()) { err = "open messages file failed"; return false; }

    std::string line;
    while (std::getline(fin, line)) {
        MessageRecord m;
        if (parse_line(line, m) && m.team_id == team_id) out.push_back(m);
    }
    return true;
}
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
#include "application.h"
#include <fstream>
#include <sstream>

ApplicationStorage::ApplicationStorage(std::string file) : file_(std::move(file)) {}

std::string ApplicationStorage::escape_json(const std::string& s) {
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

bool ApplicationStorage::parse_line(const std::string& line, ApplicationRecord& out) {
    if (line.empty()) return false;
    out.id = std::stoll(extract_field(line, "id"));
    out.from_user = extract_field(line, "from");
    out.to_user = extract_field(line, "to");
    out.status = extract_field(line, "status");
    out.created_at = extract_field(line, "created_at");
    return true;
}

bool ApplicationStorage::append(const ApplicationRecord& app, std::string& err) {
    std::ofstream fout(file_, std::ios::app);
    if (!fout.is_open()) { err = "open applications file failed"; return false; }

    fout << "{"
         << "\"id\":" << app.id << ","
         << "\"from\":\"" << escape_json(app.from_user) << "\","
         << "\"to\":\"" << escape_json(app.to_user) << "\","
         << "\"status\":\"" << escape_json(app.status) << "\","
         << "\"created_at\":\"" << escape_json(app.created_at) << "\""
         << "}\n";
    return true;
}

bool ApplicationStorage::load_all(std::vector<ApplicationRecord>& out, std::string& err) const {
    std::ifstream fin(file_);
    if (!fin.is_open()) { err = "open applications file failed"; return false; }

    std::string line;
    while (std::getline(fin, line)) {
        ApplicationRecord a;
        if (parse_line(line, a)) out.push_back(a);
    }
    return true;
}

std::optional<ApplicationRecord> ApplicationStorage::find_by_id(long long id, std::string& err) const {
    std::vector<ApplicationRecord> all;
    if (!load_all(all, err)) return std::nullopt;

    for (auto& a : all) {
        if (a.id == id) return a;
    }
    return std::nullopt;
}

bool ApplicationStorage::update(const ApplicationRecord& app, std::string& err) {
    std::vector<ApplicationRecord> all;
    if (!load_all(all, err)) return false;

    bool found = false;
    for (auto& a : all) {
        if (a.id == app.id) { a = app; found = true; break; }
    }
    if (!found) { err = "application id not found"; return false; }

    std::ofstream fout(file_, std::ios::trunc);
    if (!fout.is_open()) { err = "open applications file failed"; return false; }

    for (auto& a : all) {
        fout << "{"
             << "\"id\":" << a.id << ","
             << "\"from\":\"" << escape_json(a.from_user) << "\","
             << "\"to\":\"" << escape_json(a.to_user) << "\","
             << "\"status\":\"" << escape_json(a.status) << "\","
             << "\"created_at\":\"" << escape_json(a.created_at) << "\""
             << "}\n";
    }
    return true;
}

bool ApplicationStorage::delete_by_id(long long id, std::string& err) {
    std::vector<ApplicationRecord> all;
    if (!load_all(all, err)) return false;

    std::vector<ApplicationRecord> keep;
    bool found = false;
    for (auto& a : all) {
        if (a.id == id) { found = true; continue; }
        keep.push_back(a);
    }
    if (!found) { err = "application id not found"; return false; }

    std::ofstream fout(file_, std::ios::trunc);
    if (!fout.is_open()) { err = "open applications file failed"; return false; }

    for (auto& a : keep) {
        fout << "{"
             << "\"id\":" << a.id << ","
             << "\"from\":\"" << escape_json(a.from_user) << "\","
             << "\"to\":\"" << escape_json(a.to_user) << "\","
             << "\"status\":\"" << escape_json(a.status) << "\","
             << "\"created_at\":\"" << escape_json(a.created_at) << "\""
             << "}\n";
    }
    return true;
}

long long ApplicationStorage::next_id(std::string& err) const {
    std::vector<ApplicationRecord> all;
    if (!load_all(all, err)) return 1;

    long long mx = 0;
    for (auto& a : all) mx = std::max(mx, a.id);
    return mx + 1;
}
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
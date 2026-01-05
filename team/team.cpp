#include "team.h"
#include "application.h"
#include "message.h"
#include "review.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <limits>
#include <cctype>
#include <cstdio>

// ---------------------
// utils
// ---------------------
static std::string now_date() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return buf;
}

static std::string now_datetime() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min);
    return buf;
}

static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace((unsigned char)s[i])) i++;
    while (j > i && std::isspace((unsigned char)s[j - 1])) j--;
    return s.substr(i, j - i);
}

// 避免冲突
static std::string team_extract_field(const std::string& line, const std::string& key) {
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

// ---------------------
// TeamStorage impl
// ---------------------
TeamStorage::TeamStorage(std::string file) : file_(std::move(file)) {}

std::string TeamStorage::escape_json(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

bool TeamStorage::parse_line(const std::string& line, TeamRecord& out) {
    if (line.empty()) return false;
    out.team_id = std::stoll(team_extract_field(line, "team_id"));
    out.member1 = team_extract_field(line, "member1");
    out.member2 = team_extract_field(line, "member2");
    out.created_at = team_extract_field(line, "created_at");
    return true;
}

bool TeamStorage::append(const TeamRecord& t, std::string& err) {
    std::ofstream fout(file_, std::ios::app);
    if (!fout.is_open()) { err = "open teams file failed"; return false; }

    fout << "{"
         << "\"team_id\":" << t.team_id << ","
         << "\"member1\":\"" << escape_json(t.member1) << "\","
         << "\"member2\":\"" << escape_json(t.member2) << "\","
         << "\"created_at\":\"" << escape_json(t.created_at) << "\""
         << "}\n";
    return true;
}

bool TeamStorage::load_all(std::vector<TeamRecord>& out, std::string& err) const {
    std::ifstream fin(file_);
    if (!fin.is_open()) { err = "open teams file failed"; return false; }

    std::string line;
    while (std::getline(fin, line)) {
        TeamRecord t;
        if (parse_line(line, t)) out.push_back(t);
    }
    return true;
}

std::optional<TeamRecord> TeamStorage::find_by_team_id(long long team_id, std::string& err) const {
    std::vector<TeamRecord> all;
    if (!load_all(all, err)) return std::nullopt;
    for (auto& t : all) if (t.team_id == team_id) return t;
    return std::nullopt;
}

std::optional<TeamRecord> TeamStorage::find_by_username(const std::string& username, std::string& err) const {
    std::vector<TeamRecord> all;
    if (!load_all(all, err)) return std::nullopt;
    for (auto& t : all) {
        if (t.member1 == username || t.member2 == username) return t;
    }
    return std::nullopt;
}

long long TeamStorage::next_team_id(std::string& err) const {
    std::vector<TeamRecord> all;
    if (!load_all(all, err)) return 1;
    long long mx = 0;
    for (auto& t : all) mx = std::max(mx, t.team_id);
    return mx + 1;
}

// ---------------------
// Business APIs impl
// ---------------------
void send_team_application(
    const std::string& current_username,
    const std::string& target_username,
    const std::string& app_file,
    std::string& err
) {
    err.clear();
    if (target_username.empty()) { err = "target username empty"; return; }
    if (target_username == current_username) { err = "cannot apply to yourself"; return; }

    ApplicationStorage store(app_file);

    std::vector<ApplicationRecord> all;
    std::string tmp;
    store.load_all(all, tmp);

    for (auto& a : all) {
        if (a.from_user == current_username && a.to_user == target_username && a.status == "PENDING") {
            err = "already have a PENDING application";
            return;
        }
    }

    ApplicationRecord app;
    app.id = store.next_id(tmp);
    app.from_user = current_username;
    app.to_user = target_username;
    app.status = "PENDING";
    app.created_at = now_date();

    if (!store.append(app, err)) return;
    std::cout << "[OK] Application sent. id=" << app.id << "\n";
}

void view_my_sent_applications(
    const std::string& current_username,
    const std::string& app_file,
    std::string& err
) {
    err.clear();
    ApplicationStorage store(app_file);
    std::vector<ApplicationRecord> all;
    if (!store.load_all(all, err)) return;

    std::cout << "=== My Sent Applications ===\n";
    bool any = false;
    for (auto& a : all) {
        if (a.from_user == current_username) {
            any = true;
            std::cout << "id=" << a.id
                      << " -> " << a.to_user
                      << " status=" << a.status
                      << " at=" << a.created_at << "\n";
        }
    }
    if (!any) std::cout << "(none)\n";
}

void handle_incoming_applications(
    const std::string& current_username,
    const std::string& app_file,
    const std::string& team_file,
    std::string& err
) {
    err.clear();
    ApplicationStorage appStore(app_file);
    TeamStorage teamStore(team_file);

    std::vector<ApplicationRecord> all;
    if (!appStore.load_all(all, err)) return;

    std::vector<ApplicationRecord> incoming;
    for (auto& a : all) {
        if (a.to_user == current_username && a.status == "PENDING")
            incoming.push_back(a);
    }

    std::cout << "=== Incoming Applications (PENDING) ===\n";
    if (incoming.empty()) {
        std::cout << "(none)\n";
        return;
    }

    for (auto& a : incoming) {
        std::cout << "id=" << a.id << " from=" << a.from_user
                  << " at=" << a.created_at << "\n";
    }

    std::cout << "Enter application id to handle (0 to exit): ";
    long long id = 0;
    std::cin >> id;
    if (id == 0) return;

    auto opt = appStore.find_by_id(id, err);
    if (!opt.has_value()) { if (err.empty()) err = "application id not found"; return; }
    auto app = *opt;

    if (!(app.to_user == current_username && app.status == "PENDING")) {
        err = "can only handle your own PENDING applications";
        return;
    }

    std::cout << "Accept (A) / Reject (R)? ";
    std::string act;
    std::cin >> act;
    act = trim(act);

    if (act == "A" || act == "a") {
        std::string terr;
        if (teamStore.find_by_username(current_username, terr).has_value() ||
            teamStore.find_by_username(app.from_user, terr).has_value()) {
            err = "one user already in a team";
            return;
        }

        app.status = "ACCEPTED";
        if (!appStore.update(app, err)) return;

        TeamRecord t;
        t.team_id = teamStore.next_team_id(err);
        if (!err.empty()) return;
        t.member1 = app.from_user;
        t.member2 = current_username;
        t.created_at = now_date();

        if (!teamStore.append(t, err)) return;
        std::cout << "[OK] Accepted. Team created: team_id=" << t.team_id << "\n";
    }
    else if (act == "R" || act == "r") {
        app.status = "REJECTED";
        if (!appStore.update(app, err)) return;
        std::cout << "[OK] Rejected.\n";
    }
    else {
        err = "unknown action";
        return;
    }
}

void team_chat_room(
    const std::string& current_username,
    const std::string& team_file,
    const std::string& msg_file,
    std::string& err
) {
    err.clear();
    TeamStorage teamStore(team_file);
    auto teamOpt = teamStore.find_by_username(current_username, err);
    if (!teamOpt.has_value()) {
        if (err.empty()) err = "you are not in any team";
        return;
    }
    auto team = *teamOpt;

    MessageStorage msgStore(msg_file);

    std::cout << "=== Team Chat Room (team_id=" << team.team_id << ") ===\n";
    std::cout << "Members: " << team.member1 << " & " << team.member2 << "\n";

    std::vector<MessageRecord> history;
    std::string tmp;
    msgStore.load_by_team(team.team_id, history, tmp);

    if (history.empty()) {
        std::cout << "(no messages yet)\n";
    } else {
        std::cout << "--- history ---\n";
        for (auto& m : history) {
            std::cout << "[" << m.time << "] " << m.from_user << ": " << m.text << "\n";
        }
        std::cout << "---------------\n";
    }

    std::cout << "Type message. /exit to leave.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (true) {
        std::cout << "> ";
        std::string text;
        std::getline(std::cin, text);
        text = trim(text);

        if (text == "/exit") break;
        if (text.empty()) continue;

        MessageRecord msg;
        msg.team_id = team.team_id;
        msg.from_user = current_username;
        msg.text = text;
        msg.time = now_datetime();

        if (!msgStore.append(msg, err)) return;
        std::cout << "[sent]\n";
    }
}

void team_review(
    const std::string& current_username,
    const std::string& team_file,
    const std::string& review_file,
    std::string& err
) {
    err.clear();
    TeamStorage teamStore(team_file);
    auto teamOpt = teamStore.find_by_username(current_username, err);
    if (!teamOpt.has_value()) {
        if (err.empty()) err = "you are not in any team";
        return;
    }
    auto team = *teamOpt;

    std::string peer = (team.member1 == current_username) ? team.member2 : team.member1;

    std::cout << "=== Review Teammate ===\n";
    std::cout << "You will review: " << peer << "\n";
    std::cout << "Score (1-5): ";
    int score = 0;
    std::cin >> score;
    if (score < 1 || score > 5) { err = "score must be 1~5"; return; }

    std::cout << "Comment: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string comment;
    std::getline(std::cin, comment);
    comment = trim(comment);

    ReviewStorage rStore(review_file);
    ReviewRecord r;
    r.team_id = team.team_id;
    r.from_user = current_username;
    r.to_user = peer;
    r.score = score;
    r.comment = comment;
    r.time = now_datetime();

    if (!rStore.append(r, err)) return;
    std::cout << "[OK] Review submitted.\n";
}
//组员C：组队申请/同意/拒绝、聊天空间、互评反馈
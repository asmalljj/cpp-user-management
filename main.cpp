#include <iostream>
#include <string>
#include <limits>

#include "storage/storage.h"
#include "auth/auth.h"
#include "team/team.h"
#include "matching/matcher.h"
#include "matching/matching_config.h"
#include "analytics/analytics.h"


static constexpr const char* ADMIN_PASSWORD = "admin123";
const std::string USERS_FILE = "data/users.jsonl";
const std::string APP_FILE   = "data/applications.jsonl";
const std::string TEAM_FILE  = "data/teams.jsonl";
const std::string MSG_FILE   = "data/messages.jsonl";
const std::string REV_FILE   = "data/reviews.jsonl";

// ===== 用户登录后的菜单 =====
static void logged_in_menu(AuthService& auth,
                           UserStorage& storage,
                           const std::string& username) {
    while (true) {
        std::string err;
        auto uopt = storage.find_by_username(username, err);
        if (!err.empty()) {
            std::cout << "[ERR] " << err << "\n";
            return;
        }
        if (!uopt) {//如果没有找到该用户
            std::cout << "[ERR] User not found.\n";
            return;
        }

        const auto& u = *uopt;//后续就可以用 u 访问用户的所有属性

        std::cout << "\n== User Panel ==\n";
        std::cout << "uid: " << u.uid << "\n";
        std::cout << "username: " << u.username << "\n";
        std::cout << "nickname: " << u.nickname << "\n";
        std::cout << "goal: " << u.goal << "\n";
        std::cout << "location: " << u.location << "\n";
        std::cout << "time: " << u.time << "\n\n";

        std::cout << "=== Basic ===\n";
        std::cout << "1) Edit nickname\n";
        std::cout << "2) Edit study info\n";
        std::cout << "3) Change password\n";

        std::cout << "\n=== Matching & Recommendation  ===\n";
        std::cout << "4) Recommend teammates \n";
        std::cout << "5) Matching settings \n";

        std::cout << "\n=== Team Collaboration & Analytics  ===\n";
        std::cout << "6) Send team application\n";
        std::cout << "7) Handle incoming applications \n";
        std::cout << "8) View my sent applications\n";
        std::cout << "9) Enter team chat room\n";
        std::cout << "10) Review teammate\n";
        std::cout << "11) Analytics dashboard\n";

        std::cout << "\n0) Logout\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) return;

        if (op == 0) return;

        // =========================
        // Basic profile ops
        // =========================
        if (op == 1) {
            std::cout << "new nickname: ";
            std::string nn;
            std::cin >> nn;
            auto r = auth.update_nickname(username, nn);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        }
        else if (op == 2) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::string goal, location, time;
            //从标准输入读取一整行内容，直到用户按下回车为止，并把这行内容存到对应的变量
            std::cout << "goal: ";
            std::getline(std::cin, goal);
            std::cout << "location: ";
            std::getline(std::cin, location);
            std::cout << "time: ";
            std::getline(std::cin, time);

            auto r = auth.update_study_info(username, goal, location, time);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        }
        else if (op == 3) {
            std::cout << "new password: ";
            std::string pw;
            std::cin >> pw;
            auto r = auth.reset_password(username, pw);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        }

        // =========================
        // Group B: Matching
        // =========================
        else if (op == 4) {
            // recommend teammates
            MatchingConfig cfg;
            load_matching_config("data/config.json", cfg); // 你已有 data/config.json

            std::vector<UserRecord> all;
            std::string e;
            if (!storage.load_all(all, e)) {
                std::cout << "[ERR] load users failed: " << e << "\n";
                continue;
            }

            auto results = recommend_users(u, all, cfg);

            std::cout << "\n=== Recommended Teammates (strategy=" << cfg.strategy
                      << ", topN=" << cfg.topN << ") ===\n";
            if (results.empty()) {
                std::cout << "(no candidates)\n";
            } else {
                for (size_t i = 0; i < results.size(); ++i) {
                    const auto& r = results[i];
                    std::cout << (i + 1) << ") " << r.user.username
                              << "  nickname=" << r.user.nickname
                              << "  total=" << r.total
                              << "  [goal=" << r.s_goal
                              << ", loc=" << r.s_location
                              << ", time=" << r.s_time << "]\n";
                }
            }
        }
        else if (op == 5) {
            // matching settings
            MatchingConfig cfg;
            load_matching_config("data/config.json", cfg);

            while (true) {
                std::cout << "\n=== Matching Settings ===\n";
                std::cout << "strategy: " << cfg.strategy << "\n";
                std::cout << "w_goal=" << cfg.w_goal
                          << "  w_location=" << cfg.w_location
                          << "  w_time=" << cfg.w_time << "\n";
                std::cout << "topN=" << cfg.topN << "\n";

                std::cout << "1) Switch strategy (weighted/rule)\n";
                std::cout << "2) Set weights\n";
                std::cout << "3) Set topN\n";
                std::cout << "4) Save & Back\n";
                std::cout << "> ";

                int x;
                if (!(std::cin >> x)) break;

                if (x == 4) {
                    save_matching_config("data/config.json", cfg);
                    std::cout << "[OK] saved.\n";
                    break;
                }
                if (x == 1) {
                    std::cout << "enter strategy (weighted/rule): ";
                    std::cin >> cfg.strategy;
                    if (cfg.strategy != "weighted" && cfg.strategy != "rule") {
                        std::cout << "[ERR] invalid strategy\n";
                        cfg.strategy = "weighted";
                    }
                }
                else if (x == 2) {
                    std::cout << "w_goal: ";
                    std::cin >> cfg.w_goal;
                    std::cout << "w_location: ";
                    std::cin >> cfg.w_location;
                    std::cout << "w_time: ";
                    std::cin >> cfg.w_time;
                }
                else if (x == 3) {
                    std::cout << "topN: ";
                    std::cin >> cfg.topN;
                    if (cfg.topN <= 0) cfg.topN = 5;
                }
            }
        }

        // =========================
        // Group C: Team module
        // =========================
        else if (op == 6) {
            std::cout << "target username: ";
            std::string target;
            std::cin >> target;

            std::string e;
            send_team_application(username, target, APP_FILE, e);
            if (!e.empty()) std::cout << "[ERR] " << e << "\n";
        }
        else if (op == 7) {
            std::string e;
            handle_incoming_applications(username, APP_FILE, TEAM_FILE, e);
            if (!e.empty()) std::cout << "[ERR] " << e << "\n";
        }
        else if (op == 8) {
            std::string e;
            view_my_sent_applications(username, APP_FILE, e);
            if (!e.empty()) std::cout << "[ERR] " << e << "\n";
        }
        else if (op == 9) {
            std::string e;
            team_chat_room(username, TEAM_FILE, MSG_FILE, e);
            if (!e.empty()) std::cout << "[ERR] " << e << "\n";
        }
        else if (op == 10) {
            std::string e;
            team_review(username, TEAM_FILE, REV_FILE, e);
            if (!e.empty()) std::cout << "[ERR] " << e << "\n";
        }
        else if (op == 11) {
            std::string e;
            analytics_dashboard(USERS_FILE, TEAM_FILE, REV_FILE, e);
            if (!e.empty()) std::cout << "[ERR] " << e << "\n";
        }
    }
}


// ===== 管理员面板 =====
static void admin_panel(AuthService& auth) {
    while (true) {
        std::cout << "\n== Admin Panel ==\n";
        std::cout << "1) Delete user\n";
        std::cout << "2) Reset user password\n";
        std::cout << "3) Back\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) return;
        if (op == 3) return;

        if (op == 1) {
            std::string u;
            std::cout << "username to delete: ";
            std::cin >> u;
            auto r = auth.delete_user(u);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        } else if (op == 2) {
            std::string u, pw;
            std::cout << "username: ";
            std::cin >> u;
            std::cout << "new password: ";
            std::cin >> pw;
            auto r = auth.reset_password(u, pw);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        }
    }
}

int main() {
    UserStorage storage("data/users.jsonl");
    AuthService auth(storage);

    bool is_admin = false;

    while (true) {
        std::cout << "\n== Main Menu ==\n";
        std::cout << "1) Register\n";
        std::cout << "2) Login\n";
        std::cout << "3) Admin login\n";
        std::cout << "4) Admin panel\n";
        std::cout << "5) Admin logout\n";
        std::cout << "6) Exit\n";
        std::cout << "> ";

        int op;
        if (!(std::cin >> op)) break;
        if (op == 6) break;

        if (op == 1) {
            std::string u, p;
            std::cout << "username: ";
            std::cin >> u;
            std::cout << "password: ";
            std::cin >> p;
            auto r = auth.register_user(u, p);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
        } else if (op == 2) {
            std::string u, p;
            std::cout << "username: ";
            std::cin >> u;
            std::cout << "password: ";
            std::cin >> p;
            auto r = auth.login_user(u, p);
            std::cout << (r.ok ? "[OK] " : "[ERR] ") << r.message << "\n";
            if (r.ok) logged_in_menu(auth, storage, u);
        } else if (op == 3) {
            std::cout << "admin password: ";
            std::string pw;
            std::cin >> pw;
            if (pw == ADMIN_PASSWORD) {
                is_admin = true;
                std::cout << "[OK] Admin logged in.\n";
            } else {
                std::cout << "[ERR] Wrong admin password.\n";
            }
        } else if (op == 4) {
            if (!is_admin) {
                std::cout << "[ERR] Admin not logged in.\n";
            } else {
                admin_panel(auth);
            }
        } else if (op == 5) {
            is_admin = false;
            std::cout << "[OK] Admin logged out.\n";
        }
    }
    return 0;
}
//程序入口：主菜单 + 登录后菜单整合
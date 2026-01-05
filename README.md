
# StudyLink - C++ 学习组队管理系统（CLI）

一个基于 **C++** 的命令行（CLI）学习组队系统，支持用户注册登录、学习档案维护、队友匹配推荐、组队申请流转、聊天空间、互评反馈以及运营统计分析。  
系统使用 **JSONL（JSON Lines）** 作为本地持久化存储，结构清晰、无需数据库即可运行。

---

## ✨ 主要功能（Features）

### ✅ 用户系统
- 注册 / 登录 / 修改密码
- 修改昵称、学习信息（goal / location / time）
- 管理员登录与后台管理（删除用户 / 重置密码）

### ✅ 匹配与推荐（Matching & Recommendation）
- 多条件匹配（goal/location/time）
- 加权评分 + Top-N 推荐
- 策略切换：Weighted / Rule-based
- 权重配置：可保存到 config.json
- 推荐解释：展示分项得分与总分

### ✅ 组队协作（Team Collaboration）
- 组队申请流转：申请 / 同意 / 拒绝
- 同意后自动创建队伍（team_id）
- 组队空间：聊天记录查看 + 发送消息
- 互评反馈：评分（1~5）+ 评论

### ✅ 统计分析（Analytics Dashboard）
- 热门学习目标排行（Popular Goals）
- 成功组队排行（Successful Teaming Rank）
- 平均评分排行（Average Rating Rank）

---

## 📂 项目目录结构（含职责说明）

```

.
├── .vscode/              # VS Code 编译与运行配置（tasks.json 等）
├── analytics/            # 统计分析模块：热门目标/组队排行/评分排行
├── auth/                 # 用户认证模块：注册登录/修改密码/管理员功能
├── data/                 # 本地数据目录（JSONL）：users/applications/teams/messages/reviews/config
├── matching/             # 匹配推荐模块：加权Top-N/策略切换/权重配置/解释输出
├── storage/              # 存储与迁移模块：JSONL读写/更新删除/schema迁移/meta管理
├── team/                 # 组队协作模块：申请流转/队伍创建/聊天/互评
├── main.cpp              # 程序入口：主菜单 + 登录后用户面板（整合所有功能入口）
├── run.bat               # Windows 一键编译运行脚本（自动构建并运行 main.exe）
└── README.md

````

---

## 📄 数据文件说明（JSONL）

本项目使用 JSONL（每行一个 JSON 对象）进行持久化存储，位于 `data/` 目录。

| 文件 | 说明 |
|------|------|
| users.jsonl | 用户信息（含学习档案） |
| applications.jsonl | 组队申请记录（PENDING/ACCEPTED/REJECTED） |
| teams.jsonl | 成功组队记录 |
| messages.jsonl | 聊天消息记录 |
| reviews.jsonl | 互评记录 |
| config.json | 推荐配置（策略、权重、TopN） |

---

## 🧾 JSONL 文件格式示例（强烈建议老师看这个）

### users.jsonl（一行一个用户）
```json
{"username":"alice","password_hash":"...","created_at":"2026-01-01","schema_version":2,"uid":1,"nickname":"Alice","goal":"C++","location":"Library","time":"Evening"}
````

### applications.jsonl（组队申请）

```json
{"id":1,"from_user":"alice","to_user":"bob","status":"PENDING","created_at":"2026-01-01"}
```

### teams.jsonl（成功组队）

```json
{"team_id":1,"member1":"alice","member2":"bob","created_at":"2026-01-01"}
```

### messages.jsonl（聊天消息）

```json
{"team_id":1,"from_user":"alice","text":"hi","time":"2026-01-01 20:00"}
```

### reviews.jsonl（互评）

```json
{"team_id":1,"from":"alice","to":"bob","score":5,"comment":"good partner","time":"2026-01-01 21:00"}
```

---

## 🚀 运行方式（How to Run）

### ✅ 方式 1：Windows 一键编译运行（推荐）

双击 `run.bat` 即可自动：

* 结束旧 main.exe（避免 Permission denied）
* 自动创建缺失的 data/*.jsonl 文件
* 编译全部模块并运行程序

> 如果你修改代码后忘记重新编译，run.bat 可以保证运行的一定是最新版本。

---

### ✅ 方式 2：手动编译（通用）

在项目根目录执行：

```bash
g++ -std=c++17 main.cpp ^
auth/auth.cpp ^
storage/storage.cpp storage/meta.cpp storage/migrate.cpp ^
matching/matcher.cpp matching/matching_config.cpp ^
team/application.cpp team/message.cpp team/review.cpp team/team.cpp ^
analytics/analytics.cpp ^
-o main.exe
```

运行：

```bash
.\main.exe
```

---

## 🧪 推荐演示流程（给老师展示用）

### Step 1：注册两个用户

* alice / 123456
* bob / 123456

### Step 2：分别登录并填写学习档案

alice：

* goal: C++
* location: Library
* time: Evening

bob：

* goal: C++
* location: Library
* time: Evening

### Step 3：alice 登录 → 推荐队友

选择：

* `4) Recommend teammates`
  系统将推荐 bob，并输出分项得分解释。

### Step 4：alice 发起组队申请

选择：

* `6) Send team application` → 输入 `bob`

### Step 5：bob 登录 → 处理申请（同意/拒绝）

选择：

* `7) Handle incoming applications`
  输入申请 id → 输入 A 同意
  系统自动创建 team_id。

### Step 6：聊天空间

选择：

* `9) Enter team chat room`
  输入消息，退出使用 `/exit`

### Step 7：互评与统计分析

* `10) Review teammate`
* `11) Analytics dashboard`

---

## ⚙️ 推荐系统配置说明（config.json）

支持修改：

* `strategy`: weighted / rule
* `w_goal`, `w_location`, `w_time`
* `topN`

用户面板中选择 `5) Matching settings` 可修改并保存。

---

## ✨ 设计亮点（Highlights）

* **模块化结构清晰**：auth / storage / matching / team / analytics 解耦，方便多人协作开发
* **JSONL 本地持久化**：无需数据库，易读易维护
* **Schema 迁移机制**：支持数据结构升级（新增字段后可兼容旧数据）
* **推荐可解释性**：输出分项得分 + 总分，提高透明度
* **一键运行脚本**：run.bat 自动编译运行，避免运行旧版本/权限问题

---

## ❓ 常见问题（FAQ）

### Q1：为什么 run.bat 提示 Permission denied？

A：通常是 main.exe 正在运行导致无法覆盖。run.bat 已自动 taskkill 旧进程，如仍出现请手动关闭 main.exe。

### Q2：推荐结果为空？

A：系统会排除当前用户自己；同时若只有一个用户或档案信息为空，推荐效果会不明显。建议注册多个用户并填写 goal/location/time。

---

## 📌 Future Work（可扩展方向）

* 支持 3 人及以上组队（目前为 2 人队伍）
* 推荐算法扩展：历史互评加权、关键词匹配、协同过滤
* 聊天功能增强：分页、搜索、撤回消息
* 更严格的密码安全策略：salt + 更强 hash 算法



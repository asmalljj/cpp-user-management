# 📌 C++ 学习组队管理系统（CPP User Management & Team Matching System）

## 1. 项目简介（Project Overview）

本项目是一个基于 C++ 编写的 **学习组队管理系统**，支持用户注册登录、学习信息维护、队友匹配推荐、组队申请流转、组队聊天空间、互评反馈以及运营统计分析等功能。

系统使用 **JSONL（JSON Lines）文件**进行持久化存储，能够在不依赖数据库的情况下保存用户数据与组队信息。项目采用模块化结构，将核心功能拆分为：认证模块、存储模块、匹配推荐模块、组队协作模块与统计分析模块，便于团队协作开发与功能扩展。

---

## 2. 项目目录结构（Project Structure）

```
.
├── auth/                # 登录认证、用户信息更新
│   ├── auth.h
│   └── auth.cpp
│
├── storage/             # 用户数据存储、迁移、元信息管理
│   ├── storage.h
│   ├── storage.cpp
│   ├── meta.h
│   ├── meta.cpp
│   ├── migrate.h
│   └── migrate.cpp
│
├── matching/            # 匹配与推荐引擎（组员B）
│   ├── matcher.h
│   ├── matcher.cpp
│   ├── matching_config.h
│   └── matching_config.cpp
│
├── team/                # 组队协作系统（组员C）
│   ├── application.h / application.cpp  # 申请流转
│   ├── team.h / team.cpp                # 组队创建、聊天、互评、业务接口
│   ├── message.h / message.cpp          # 聊天消息存储
│   └── review.h / review.cpp            # 评价存储
│
├── analytics/           # 统计分析模块（组员C）
│   ├── analytics.h
│   └── analytics.cpp
│
├── data/                # 数据存储目录
│   ├── users.jsonl
│   ├── applications.jsonl
│   ├── teams.jsonl
│   ├── messages.jsonl
│   ├── reviews.jsonl
│   └── config.json
│
├── main.cpp             # 程序入口（主菜单 + 用户面板）
└── README.md
```

---

## 3. 核心功能（Main Features）

### ✅ 3.1 用户系统（注册 / 登录 / 个人资料管理）

* 用户注册（Register）
* 用户登录（Login）
* 修改昵称（Edit nickname）
* 修改学习信息（goal / location / time）
* 修改密码（Change password）
* 用户信息持久化保存到 `data/users.jsonl`

📌 学习信息字段：

* `goal`：学习目标，如 C++ / Data Structure / AI
* `location`：学习地点，如 Library / Dorm / Online
* `time`：学习时间段，如 Evening / Weekend / Morning

---

### ✅ 3.2 管理员系统（Admin）

管理员拥有后台权限，功能包括：

* 删除指定用户（Delete user）
* 重置指定用户密码（Reset user password）

管理员登录密码在 `main.cpp` 中定义：

```cpp
static constexpr const char* ADMIN_PASSWORD = "admin123";
```

---

## 4. 匹配与推荐系统（组员B模块）

### ✅ 4.1 多条件匹配算法（Weighted Scoring）

根据用户学习信息（goal/location/time）计算相似度：

* 计算每个维度得分（0~1）
* 按权重加权求总分
* 输出 Top-N 推荐结果
* 输出推荐解释：每项得分 + 总分

推荐结果示例：

```
1) bob total=0.92 [goal=1, loc=1, time=0.8]
```

### ✅ 4.2 策略切换（Weighted / Rule-based）

支持两种策略：

* **weighted**：加权评分排序（默认）
* **rule**：规则匹配（例如必须同地点+同时段）

### ✅ 4.3 权重配置（配置保存）

可在用户面板中修改：

* `w_goal`
* `w_location`
* `w_time`
* `topN`
* `strategy`

保存到：`data/config.json`
下次启动自动读取配置。

---

## 5. 组队协作系统（组员C模块）

### ✅ 5.1 组队申请流转（application）

用户可以发起组队申请，目标用户可：

* 同意（Accept）→ 自动创建 Team
* 拒绝（Reject）
  申请记录保存到 `data/applications.jsonl`

### ✅ 5.2 组队创建（team）

同意申请后系统自动创建队伍记录，保存到 `data/teams.jsonl`
每个队伍包含：

* team_id
* member1 / member2
* created_at

### ✅ 5.3 组队空间聊天（message）

同一队伍成员可以进入聊天空间：

* 查看历史聊天记录
* 发送消息
  消息保存到 `data/messages.jsonl`

聊天退出指令：

```
/exit
```

### ✅ 5.4 互评反馈（review）

队伍成员可以互相评价：

* 评分（1~5）
* 评论内容
  评价保存到 `data/reviews.jsonl`

---

## 6. 运营统计分析（analytics 模块，组员C）

统计分析页输出 3 种排行榜：

### ✅ 6.1 热门学习目标排行（Popular Goals）

统计 users.jsonl 中 `goal` 出现次数。

### ✅ 6.2 成功组队排行榜（Successful Teaming Rank）

统计 teams.jsonl 中每个用户参与组队次数。

### ✅ 6.3 平均评分排行榜（Average Rating Rank）

统计 reviews.jsonl 中用户平均评分。

---

## 7. 数据存储说明（JSONL Files）

| 文件                 | 说明                                |
| ------------------ | --------------------------------- |
| users.jsonl        | 用户信息（含学习目标/地点/时间）                 |
| applications.jsonl | 组队申请记录（PENDING/ACCEPTED/REJECTED） |
| teams.jsonl        | 组队成功记录                            |
| messages.jsonl     | 聊天消息记录                            |
| reviews.jsonl      | 互评记录                              |
| config.json        | 推荐权重与策略配置                         |

---

## 8. 运行方式（How to Run）

### ✅ 方式 1：双击 run.bat（Windows 推荐）

项目根目录提供 `run.bat`，双击即可自动：

* 编译所有 cpp 文件
* 生成 main.exe
* 自动运行程序

### ✅ 方式 2：手动编译（命令行）

在根目录执行：

```bash
g++ -std=c++17 main.cpp auth/auth.cpp storage/storage.cpp storage/meta.cpp storage/migrate.cpp matching/matcher.cpp matching/matching_config.cpp team/application.cpp team/message.cpp team/review.cpp team/team.cpp analytics/analytics.cpp -o main.exe
```

运行：

```bash
.\main.exe
```

---

## 9. 推荐演示流程（给老师展示用）

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

菜单选择：

* `4) Recommend teammates`
  应推荐 bob，并显示得分解释。

### Step 4：alice 发起组队申请

* `6) Send team application` → bob

### Step 5：bob 登录 → 处理申请

* `7) Handle incoming applications`
  输入 id → `A` 同意
  自动创建 team

### Step 6：聊天空间

* `9) Enter team chat room`
  发送消息并查看历史

### Step 7：互评

* `10) Review teammate`

### Step 8：统计分析面板

* `11) Analytics dashboard`
  查看热门 goal、组队排行、评分排行

---

## 10. 团队分工（Team Roles）

### ✅ 组员A：基础框架 / 用户系统 / 存储模块

* 用户注册登录
* 用户数据持久化（JSONL）
* 管理员功能
* 存储版本迁移与 schema 管理

### ✅ 组员B：匹配与推荐引擎负责人

* 多条件匹配算法（加权评分 Top-N）
* 策略切换（weighted / rule-based）
* 权重配置保存（config.json）
* 推荐结果解释（分项得分展示）

### ✅ 组员C：组队协同与运营分析负责人

* 组队申请流转（申请/同意/拒绝）
* 组队空间（留言/聊天记录）
* 互评反馈（评分/评论）
* 统计分析页（热门目标、成功组队排行、平均评分排行）

---

## 11. 可扩展方向（Future Work）

* 支持 3 人及以上小组组队（当前为 2 人队伍）
* 匹配算法扩展：引入历史评分加权、关键词提取、协同过滤
* 聊天功能支持分页、时间排序、撤回消息
* 数据加密与更安全的密码存储（Hash + Salt）

---



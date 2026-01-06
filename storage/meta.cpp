#include "meta.h"
#include <fstream>
#include <sstream>

MetaStorage::MetaStorage(std::string meta_file)
    : meta_file_(std::move(meta_file)) {}//保存到成员变量 meta_file_

bool MetaStorage::load_or_init(Meta& out, std::string& err) {
    std::ifstream ifs(meta_file_);
    if (!ifs) {
        // 不存在就初始化
        out.version = 1;
        return save(out, err);
    }

    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string s = ss.str();//如果文件存在，把文件内容读到字符串 s 里，准备解析

    auto pos = s.find("\"version\":");
    if (pos == std::string::npos) {
        err = "meta.json missing version field";
        return false;
    }
    pos += std::string("\"version\":").size();
    out.version = std::stoi(s.substr(pos));
    return true;
}//查找 "version" 字段，如果找不到就报错；找到后，提取 version 的值，赋给 out.version，返回 true。

bool MetaStorage::save(const Meta& meta, std::string& err) {
    std::ofstream ofs(meta_file_, std::ios::trunc);
    if (!ofs) {
        err = "Failed to write meta file: " + meta_file_;
        return false;
    }
    ofs << "{ \"version\": " << meta.version << " }\n";
    return true;
}//以覆盖方式写入 meta_file_ 文件，内容为 { "version": x }。写入失败就报错，成功返回 true
//组员A：JSONL 存储、读取、更新、删除、迁移
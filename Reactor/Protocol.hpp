#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <jsoncpp/json/json.h>

#define SEP " "
#define SEP_LEN strlen(SEP)
#define LINE_SEP "\r\n"
#define LINE_SEP_LEN strlen(LINE_SEP) // 不敢使用sizeof()

enum
{
    OK = 0,
    DIV_ZERO,
    MOD_ZERO,
    OP_ERROR
};

// "x op y" -> "content_len"\r\n"x op y"\r\n
// "exitcode result" -> "content_len"\r\n"exitcode result"\r\n
std::string enLength(const std::string &text)
{
    std::string send_str = std::to_string(text.size());
    send_str += LINE_SEP;
    send_str += text;
    send_str += LINE_SEP;

    return send_str;
}

// "content_len"\r\n"exitcode result"\r\n
bool deLength(const std::string &package, std::string *text)
{
    auto pos = package.find(LINE_SEP);
    if (pos == std::string::npos)
        return false;

    std::string text_len_str = package.substr(0, pos);
    int text_len = std::stoi(text_len_str);
    *text = package.substr(pos + LINE_SEP_LEN, text_len);

    return true;
}

class Request
{
public:
    Request() : _x(0), _y(0), _op(0) {}
    Request(int x, int y, char op) : _x(x), _y(y), _op(op) {}

    bool serialize(std::string *out)
    {
#ifdef MYSELF
        *out = "";
        // 结构化 -> "x op y";
        std::string x_str = std::to_string(_x);
        std::string y_str = std::to_string(_y);
        *out = x_str;
        *out += SEP;
        *out += _op;
        *out += SEP;
        *out += y_str;
#else
        Json::Value root;
        root["first"] = _x;
        root["second"] = _y;
        root["oper"] = _op;

        Json::FastWriter writer;
        *out = writer.write(root);
#endif
        return true;
    }

    bool deserialize(const std::string in)
    {
#ifdef MYSELF
        // "x op y" -> 结构化;
        auto left = in.find(SEP);
        auto right = in.rfind(SEP);
        if (left == std::string::npos || right == std::string::npos)
            return false;
        else if (left == right)
            return false;
        else if (right - (left + SEP_LEN) != 1)
            return false;

        std::string x_str = in.substr(0, left);
        std::string y_str = in.substr(right + SEP_LEN);
        if (x_str.empty() || y_str.empty())
            return false;

        _x = std::stoi(x_str);
        _y = std::stoi(y_str);
        _op = in[left + SEP_LEN];
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in, root);

        _x = root["first"].asInt();
        _y = root["second"].asInt();
        _op = root["oper"].asInt();
#endif
        return true;
    }

public:
    int _x;
    int _y;
    char _op;
};

class Response
{
public:
    Response() : _exitCode(0), _result(0) {}
    Response(int exitCode, int result) : _exitCode(exitCode), _result(result) {}
    bool serialize(std::string *out)
    {
#ifdef MYSELF
        *out = "";
        std::string ec_str = std::to_string(_exitCode);
        std::string ret_str = std::to_string(_result);
        *out = ec_str;
        *out += SEP;
        *out += ret_str;
#else
        Json::Value root;
        root["exitCode"] = _exitCode;
        root["result"] = _result;

        Json::FastWriter writer;
        *out = writer.write(root);
#endif
    }

    bool deserialize(const std::string &in)
    {
#ifdef MYSELF
        auto mid = in.find(SEP);
        if (mid == std::string::npos)
            return false;

        std::string ec_str = in.substr(0, mid);
        std::string ret_str = in.substr(mid + SEP_LEN);
        if (ec_str.empty() || ret_str.empty())
            return false;

        _exitCode = std::stoi(ec_str);
        _result = std::stoi(ret_str);
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in, root);

        _exitCode = root["exitCode"].asInt();
        _result = root["result"].asInt();
#endif
    }

public:
    int _exitCode;
    int _result;
};

// "content_len"\r\n"x op y"\r\n"content_len"\r\n"x op y"\r\n"content_len"\r\n"x op
bool ParseOnePackage(std::string &inbuffer, std::string *package)
{
    *package = "";
    // 分析处理
    auto pos = inbuffer.find(LINE_SEP);
    if (pos == std::string::npos)
        return false;

    std::string text_len_str = inbuffer.substr(0, pos);
    int text_len = std::stoi(text_len_str);
    int total_len = text_len_str.size() + 2 * LINE_SEP_LEN + text_len;
    if (inbuffer.size() < total_len)
        return false;

    *package = inbuffer.substr(0, total_len);
    inbuffer.erase(0, total_len);
    return true;
}
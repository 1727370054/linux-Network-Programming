#pragma once

#include <iostream>
#include <string>
#include <fstream>

class Util
{
public:
    static std::string getOneLine(std::string buffer, const std::string sep)
    {
        auto pos = buffer.find(sep);
        if (pos == std::string::npos)
            return "";
        std::string sub = buffer.substr(0, pos);
        buffer.erase(0, sub.size() + sep.size());
        return sub;
    }
    static bool readFile(const std::string &resource, char *buffer, int size)
    {
        std::ifstream ifs(resource, std::ios::binary);
        if (!ifs.is_open())
            return false;

        ifs.read(buffer, size);
        // std::string line;
        // while (std::getline(ifs, line))
        // {
        //     *out += line;
        // }
        ifs.close();
        return true;
    }
};
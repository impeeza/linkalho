#pragma once

#include <string>

class Generator{
private:

    unsigned long _nasId;
    std::string _nasIdStr;
    unsigned long _baasUserId;

public:
    Generator(); // Disallow instantiation outside of the class.
    // Generator(const Generator&) = delete;
    // Generator& operator=(const Generator &) = delete;
    // Generator(Generator &&) = delete;
    // Generator & operator=(Generator &&) = delete;

    // static auto& instance(){
    //     static Generator gen;
    //     return gen;
    // }

    const std::string& nasIdStr();
    void writeBaas(const std::string& fullpath);
    void writeProfileDat(const std::string& fullpath);
    void writeProfileJson(const std::string& fullpath);

};

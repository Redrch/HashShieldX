#pragma once

#include "crypto_utils.h"
#include <string>
#include <vector>
#include <algorithm>

class GetHash
{
public:
    // 支持的哈希算法类型
    enum class HashType {
        MD5,
        SHA1,
        SHA224,
        SHA256,
        SHA384,
        SHA512
    };

    GetHash();
    ~GetHash();

    // 计算文件的哈希值
    string calculateFileHash(const string& filename, HashType hashType);

    // 计算内存中数据的哈希值
    string calculateHash(const vector<unsigned char>& data, HashType hashType);

    // 获取最后计算的哈希值(十六进制字符串)
    string getLastHash() const;

    // 获取最后计算的原始哈希值(二进制)
    vector<unsigned char> getLastRawHash() const;

    // 验证文件的哈希值是否匹配
    bool verifyFileHash(const string& filename, const string& expectedHash, HashType hashType);

private:
    // 获取指定算法的 EVP_MD 指针
    const EVP_MD* getHashAlgorithm(HashType hashType);

    // 将哈希二进制值转换为十六进制字符串
    string hashToHexString(const vector<unsigned char>& hash);

    // 将十六进制字符串转换为小写格式
    string toLowerHex(const string& hexString);

    // 存储最后计算的哈希值
    vector<unsigned char> lastRawHash;
    string lastHashString;
};


#pragma once

#include "crypto_utils.h"
#include "split_file.h"
#include <zip.h>
#include <regex>

class Decrypt
{
public:
    Decrypt();
    ~Decrypt();

    // 使用RSA解密AES密钥
    vector<unsigned char> decryptRSA(EVP_PKEY* privkey, const vector<unsigned char>& encryptedData);

    // 使用AES-256-CBC解密文件内容
    vector<unsigned char> decryptAES(
        const vector<unsigned char>& ciphertext,
        const vector<unsigned char>& key,
        const vector<unsigned char>& iv
    );

    // 字节数组转换为整数
    uint32_t bytesToInt(const vector<unsigned char>& bytes);

    // Decrypt
    void decryptSmallFile(string inputFile, string outputFile, string keyFile, bool isDebug);
    void decrypt(string inputFile, string outputFile, string keyFile, bool isDebug);

    vector<string> split(const string& str, char delimiter);
    // 自定义的 contains 函数
    template <typename SubstrType>
    bool contains(const std::string& str, const SubstrType& substr) {
        return str.find(substr) != std::string::npos;
    }
};


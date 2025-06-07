#pragma once

#include "crypto_utils.h"

class Decrypt
{
public:
    Decrypt();
    ~Decrypt();

    // 使用RSA解密AES密钥
    std::vector<unsigned char> decryptRSA(EVP_PKEY* privkey, const std::vector<unsigned char>& encryptedData);

    // 使用AES-256-CBC解密文件内容
    std::vector<unsigned char> decryptAES(
        const std::vector<unsigned char>& ciphertext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    );

    // 字节数组转换为整数
    uint32_t bytesToInt(const std::vector<unsigned char>& bytes);

    // Decrypt
    void decrypt(string inputFile, string outputFile, string keyFile, bool isDebug);
};


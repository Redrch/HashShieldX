#pragma once

#include "crypto_utils.h"

class Encrypt
{
public:
	Encrypt();
	~Encrypt();

    // 使用RSA加密AES密钥
    std::vector<unsigned char> encryptRSA(EVP_PKEY* pubkey, const std::vector<unsigned char>& data);

    // 使用AES-256-CBC加密文件内容
    std::vector<unsigned char> encryptAES(
        const std::vector<unsigned char>& plaintext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    );

    // 将整数转换为字节向量
    std::vector<unsigned char> intToBytes(uint32_t value, int bytes);
};


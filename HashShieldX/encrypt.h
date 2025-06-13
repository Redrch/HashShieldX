#pragma once

#include "crypto_utils.h"
#include "split_file.h"
#include <zip.h>

class Encrypt
{
public:
	Encrypt();
	~Encrypt();

    // 使用RSA加密AES密钥
    vector<unsigned char> encryptRSA(EVP_PKEY* pubkey, const vector<unsigned char>& data);

    // 使用AES-256-CBC加密文件内容
    vector<unsigned char> encryptAES(
        const vector<unsigned char>& plaintext,
        const vector<unsigned char>& key,
        const vector<unsigned char>& iv
    );

    void encryptSmallFile(string inputFile, string outputFile, string keyFile);
    void encrypt(string inputFile, string outputFile, string keyFile, bool isDebug);

    // 将整数转换为字节向量
    vector<unsigned char> intToBytes(uint32_t value, int bytes);
};


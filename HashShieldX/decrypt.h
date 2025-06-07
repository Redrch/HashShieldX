#pragma once

#include "crypto_utils.h"

class Decrypt
{
public:
    Decrypt();
    ~Decrypt();

    // ʹ��RSA����AES��Կ
    std::vector<unsigned char> decryptRSA(EVP_PKEY* privkey, const std::vector<unsigned char>& encryptedData);

    // ʹ��AES-256-CBC�����ļ�����
    std::vector<unsigned char> decryptAES(
        const std::vector<unsigned char>& ciphertext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    );

    // �ֽ�����ת��Ϊ����
    uint32_t bytesToInt(const std::vector<unsigned char>& bytes);

    // Decrypt
    void decrypt(string inputFile, string outputFile, string keyFile, bool isDebug);
};


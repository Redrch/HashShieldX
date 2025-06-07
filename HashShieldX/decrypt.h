#pragma once

#include "crypto_utils.h"

class Decrypt
{
public:
    Decrypt();
    ~Decrypt();

    // ʹ��RSA����AES��Կ
    vector<unsigned char> decryptRSA(EVP_PKEY* privkey, const vector<unsigned char>& encryptedData);

    // ʹ��AES-256-CBC�����ļ�����
    vector<unsigned char> decryptAES(
        const vector<unsigned char>& ciphertext,
        const vector<unsigned char>& key,
        const vector<unsigned char>& iv
    );

    // �ֽ�����ת��Ϊ����
    uint32_t bytesToInt(const vector<unsigned char>& bytes);

    // Decrypt
    void decrypt(string inputFile, string outputFile, string keyFile, bool isDebug);
};


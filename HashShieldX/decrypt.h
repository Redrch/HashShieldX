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
    void decryptSmallFile(string inputFile, string outputFile, string keyFile, bool isDebug);
    void decrypt(string inputFile, string outputFile, string keyFile, bool isDebug);

    vector<string> split(const string& str, char delimiter);
    // �Զ���� contains ����
    template <typename SubstrType>
    bool contains(const std::string& str, const SubstrType& substr) {
        return str.find(substr) != std::string::npos;
    }
};


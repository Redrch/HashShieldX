#pragma once

#include "crypto_utils.h"
#include "split_file.h"
#include <zip.h>

class Encrypt
{
public:
	Encrypt();
	~Encrypt();

    // ʹ��RSA����AES��Կ
    vector<unsigned char> encryptRSA(EVP_PKEY* pubkey, const vector<unsigned char>& data);

    // ʹ��AES-256-CBC�����ļ�����
    vector<unsigned char> encryptAES(
        const vector<unsigned char>& plaintext,
        const vector<unsigned char>& key,
        const vector<unsigned char>& iv
    );

    void encryptSmallFile(string inputFile, string outputFile, string keyFile);
    void encrypt(string inputFile, string outputFile, string keyFile, bool isDebug);

    // ������ת��Ϊ�ֽ�����
    vector<unsigned char> intToBytes(uint32_t value, int bytes);
};


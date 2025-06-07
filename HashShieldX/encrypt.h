#pragma once

#include "crypto_utils.h"

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

    void encrypt(string inputFile, string outputFile, string keyFile);

    // ������ת��Ϊ�ֽ�����
    vector<unsigned char> intToBytes(uint32_t value, int bytes);
};


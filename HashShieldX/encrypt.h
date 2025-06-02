#pragma once

#include "crypto_utils.h"

class Encrypt
{
public:
	Encrypt();
	~Encrypt();

    // ʹ��RSA����AES��Կ
    std::vector<unsigned char> encryptRSA(EVP_PKEY* pubkey, const std::vector<unsigned char>& data);

    // ʹ��AES-256-CBC�����ļ�����
    std::vector<unsigned char> encryptAES(
        const std::vector<unsigned char>& plaintext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    );

    // ������ת��Ϊ�ֽ�����
    std::vector<unsigned char> intToBytes(uint32_t value, int bytes);
};


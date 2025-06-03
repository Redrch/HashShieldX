#pragma once

#include "crypto_utils.h"
#include <string>

class GenRSAKeys
{
public:
    GenRSAKeys();
    ~GenRSAKeys();

    // ����RSA��Կ�Բ����浽�ļ�
    bool generateRSAKeyPair(const std::string& publicKeyFile,
        const std::string& privateKeyFile,
        int keyBits = 2048);

    // ����˽Կ���뱣��
    void setPrivateKeyPassword(const std::string& password);

    // ��ȡ������Ϣ
    std::string getLastError() const;

private:
    std::string lastError;
    std::string privateKeyPassword;
    bool usePasswordProtection;

    // д��˽Կ���ļ�
    bool writePrivateKey(EVP_PKEY* pkey, const std::string& filename);

    // д�빫Կ���ļ�
    bool writePublicKey(EVP_PKEY* pkey, const std::string& filename);
};
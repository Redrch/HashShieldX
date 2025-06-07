#pragma once

#include "crypto_utils.h"
#include <string>

class GenRSAKeys
{
public:
    GenRSAKeys();
    ~GenRSAKeys();

    // ����RSA��Կ�Բ����浽�ļ�
    bool generateRSAKeyPair(const string& publicKeyFile,
        const string& privateKeyFile,
        int keyBits = 2048);

    // ����˽Կ���뱣��
    void setPrivateKeyPassword(const string& password);

    // ��ȡ������Ϣ
    string getLastError() const;

private:
    string lastError;
    string privateKeyPassword;
    bool usePasswordProtection;

    // д��˽Կ���ļ�
    bool writePrivateKey(EVP_PKEY* pkey, const string& filename);

    // д�빫Կ���ļ�
    bool writePublicKey(EVP_PKEY* pkey, const string& filename);
};
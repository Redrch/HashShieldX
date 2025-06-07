#pragma once

#include "crypto_utils.h"
#include <string>

class GenRSAKeys
{
public:
    GenRSAKeys();
    ~GenRSAKeys();

    // 生成RSA密钥对并保存到文件
    bool generateRSAKeyPair(const string& publicKeyFile,
        const string& privateKeyFile,
        int keyBits = 2048);

    // 设置私钥密码保护
    void setPrivateKeyPassword(const string& password);

    // 获取错误信息
    string getLastError() const;

private:
    string lastError;
    string privateKeyPassword;
    bool usePasswordProtection;

    // 写入私钥到文件
    bool writePrivateKey(EVP_PKEY* pkey, const string& filename);

    // 写入公钥到文件
    bool writePublicKey(EVP_PKEY* pkey, const string& filename);
};
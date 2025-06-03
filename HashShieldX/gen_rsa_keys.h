#pragma once

#include "crypto_utils.h"
#include <string>

class GenRSAKeys
{
public:
    GenRSAKeys();
    ~GenRSAKeys();

    // 生成RSA密钥对并保存到文件
    bool generateRSAKeyPair(const std::string& publicKeyFile,
        const std::string& privateKeyFile,
        int keyBits = 2048);

    // 设置私钥密码保护
    void setPrivateKeyPassword(const std::string& password);

    // 获取错误信息
    std::string getLastError() const;

private:
    std::string lastError;
    std::string privateKeyPassword;
    bool usePasswordProtection;

    // 写入私钥到文件
    bool writePrivateKey(EVP_PKEY* pkey, const std::string& filename);

    // 写入公钥到文件
    bool writePublicKey(EVP_PKEY* pkey, const std::string& filename);
};
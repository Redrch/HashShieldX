#include "gen_rsa_keys.h"
#include <openssl/pem.h>
#include <iostream>

GenRSAKeys::GenRSAKeys() : usePasswordProtection(false)
{
    // 初始化OpenSSL库
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

GenRSAKeys::~GenRSAKeys()
{
    // 清理OpenSSL库
    EVP_cleanup();
    ERR_free_strings();
}

bool GenRSAKeys::generateRSAKeyPair(const std::string& publicKeyFile,
    const std::string& privateKeyFile,
    int keyBits)
{
    // 验证输入参数
    if (keyBits < 1024) {
        lastError = "密钥长度太短，至少需要1024位";
        return false;
    }

    // 清除之前的错误信息
    lastError.clear();

    try {
        // 创建RSA密钥生成上下文
        std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
        if (!ctx) {
            handleOpenSSLErrors();
        }

        // 初始化密钥生成参数
        if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
            handleOpenSSLErrors();
        }

        // 设置RSA密钥长度
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), keyBits) <= 0) {
            handleOpenSSLErrors();
        }

        // 生成密钥对
        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) {
            handleOpenSSLErrors();
        }

        // 使用智能指针管理生成的密钥
        std::unique_ptr<EVP_PKEY, OpenSSLDeleter> keyPair(pkey);

        // 保存公钥到文件
        if (!writePublicKey(keyPair.get(), publicKeyFile)) {
            return false;
        }

        // 保存私钥到文件
        if (!writePrivateKey(keyPair.get(), privateKeyFile)) {
            return false;
        }

        std::cout << "RSA密钥对生成成功！" << std::endl;
        std::cout << "公钥已保存到: " << publicKeyFile << std::endl;
        std::cout << "私钥已保存到: " << privateKeyFile << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        lastError = e.what();
        return false;
    }
}

void GenRSAKeys::setPrivateKeyPassword(const std::string& password)
{
    privateKeyPassword = password;
    usePasswordProtection = !password.empty();
}

std::string GenRSAKeys::getLastError() const
{
    return lastError;
}

bool GenRSAKeys::writePrivateKey(EVP_PKEY* pkey, const std::string& filename)
{
    // 创建文件BIO
    std::unique_ptr<BIO, OpenSSLDeleter> bio(BIO_new_file(filename.c_str(), "w"));
    if (!bio) {
        lastError = "无法创建私钥文件";
        return false;
    }

    // 写入私钥
    const EVP_CIPHER* cipher = usePasswordProtection ? EVP_aes_256_cbc() : nullptr;
    int result;

    if (usePasswordProtection) {
        result = PEM_write_bio_PKCS8PrivateKey(
            bio.get(),
            pkey,
            cipher,
            nullptr,
            0,
            nullptr,
            const_cast<char*>(privateKeyPassword.c_str())
        );
    }
    else {
        result = PEM_write_bio_PrivateKey(
            bio.get(),
            pkey,
            nullptr,
            nullptr,
            0,
            nullptr,
            nullptr
        );
    }

    if (result != 1) {
        lastError = "写入私钥文件失败";
        return false;
    }

    return true;
}

bool GenRSAKeys::writePublicKey(EVP_PKEY* pkey, const std::string& filename)
{
    // 创建文件BIO
    std::unique_ptr<BIO, OpenSSLDeleter> bio(BIO_new_file(filename.c_str(), "w"));
    if (!bio) {
        lastError = "无法创建公钥文件";
        return false;
    }

    // 写入公钥
    if (PEM_write_bio_PUBKEY(bio.get(), pkey) != 1) {
        lastError = "写入公钥文件失败";
        return false;
    }

    return true;
}
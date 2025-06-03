#include "gen_rsa_keys.h"
#include <openssl/pem.h>
#include <iostream>

GenRSAKeys::GenRSAKeys() : usePasswordProtection(false)
{
    // ��ʼ��OpenSSL��
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

GenRSAKeys::~GenRSAKeys()
{
    // ����OpenSSL��
    EVP_cleanup();
    ERR_free_strings();
}

bool GenRSAKeys::generateRSAKeyPair(const std::string& publicKeyFile,
    const std::string& privateKeyFile,
    int keyBits)
{
    // ��֤�������
    if (keyBits < 1024) {
        lastError = "��Կ����̫�̣�������Ҫ1024λ";
        return false;
    }

    // ���֮ǰ�Ĵ�����Ϣ
    lastError.clear();

    try {
        // ����RSA��Կ����������
        std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
        if (!ctx) {
            handleOpenSSLErrors();
        }

        // ��ʼ����Կ���ɲ���
        if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
            handleOpenSSLErrors();
        }

        // ����RSA��Կ����
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(), keyBits) <= 0) {
            handleOpenSSLErrors();
        }

        // ������Կ��
        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx.get(), &pkey) <= 0) {
            handleOpenSSLErrors();
        }

        // ʹ������ָ��������ɵ���Կ
        std::unique_ptr<EVP_PKEY, OpenSSLDeleter> keyPair(pkey);

        // ���湫Կ���ļ�
        if (!writePublicKey(keyPair.get(), publicKeyFile)) {
            return false;
        }

        // ����˽Կ���ļ�
        if (!writePrivateKey(keyPair.get(), privateKeyFile)) {
            return false;
        }

        std::cout << "RSA��Կ�����ɳɹ���" << std::endl;
        std::cout << "��Կ�ѱ��浽: " << publicKeyFile << std::endl;
        std::cout << "˽Կ�ѱ��浽: " << privateKeyFile << std::endl;

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
    // �����ļ�BIO
    std::unique_ptr<BIO, OpenSSLDeleter> bio(BIO_new_file(filename.c_str(), "w"));
    if (!bio) {
        lastError = "�޷�����˽Կ�ļ�";
        return false;
    }

    // д��˽Կ
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
        lastError = "д��˽Կ�ļ�ʧ��";
        return false;
    }

    return true;
}

bool GenRSAKeys::writePublicKey(EVP_PKEY* pkey, const std::string& filename)
{
    // �����ļ�BIO
    std::unique_ptr<BIO, OpenSSLDeleter> bio(BIO_new_file(filename.c_str(), "w"));
    if (!bio) {
        lastError = "�޷�������Կ�ļ�";
        return false;
    }

    // д�빫Կ
    if (PEM_write_bio_PUBKEY(bio.get(), pkey) != 1) {
        lastError = "д�빫Կ�ļ�ʧ��";
        return false;
    }

    return true;
}
#include "decrypt.h"

Decrypt::Decrypt()
{
}

Decrypt::~Decrypt()
{
}

// ʹ��RSA����AES��Կ
std::vector<unsigned char> Decrypt::decryptRSA(EVP_PKEY* privkey, const std::vector<unsigned char>& encryptedData) {
    // ��������������
    std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new(privkey, nullptr));
    if (!ctx) {
        handleOpenSSLErrors();
    }

    // ��ʼ�����ܲ���
    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) {
        handleOpenSSLErrors();
    }

    // ���ý�����䷽ʽΪOAEP (�����ʱ��ͬ)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
        handleOpenSSLErrors();
    }

    // ȷ�����ܺ�����������С
    size_t outlen;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
        handleOpenSSLErrors();
    }

    // ִ�н���
    std::vector<unsigned char> decrypted(outlen);
    if (EVP_PKEY_decrypt(ctx.get(), decrypted.data(), &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
        handleOpenSSLErrors();
    }

    decrypted.resize(outlen);
    return decrypted;
}

// ʹ��AES-256-CBC�����ļ�����
std::vector<unsigned char> Decrypt::decryptAES(
    const std::vector<unsigned char>& ciphertext,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
) {
    // ��������ʼ������������
    std::unique_ptr<EVP_CIPHER_CTX, OpenSSLDeleter> ctx(EVP_CIPHER_CTX_new());
    if (!ctx) {
        handleOpenSSLErrors();
    }

    // ��ʼ�����ܲ�����ʹ��AES-256-CBCģʽ
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        handleOpenSSLErrors();
    }

    // ׼�����ܽ���Ļ������������ܴ�С��
    std::vector<unsigned char> plaintext(ciphertext.size() + AES_BLOCK_SIZE);
    int len1 = 0, len2 = 0;

    // ��������
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len1, ciphertext.data(), ciphertext.size()) != 1) {
        handleOpenSSLErrors();
    }

    // �������һ�鲢�Ƴ��κ����
    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len1, &len2) != 1) {
        handleOpenSSLErrors();
    }

    // �������������СΪʵ�ʽ��ܺ�����ݴ�С
    plaintext.resize(len1 + len2);
    return plaintext;
}

// �ֽ�����ת��Ϊ����
uint32_t Decrypt::bytesToInt(const std::vector<unsigned char>& bytes) {
    if (bytes.empty()) {
        return 0;
    }

    uint32_t result = 0;
    for (size_t i = 0; i < bytes.size(); i++) {
        result = (result << 8) | bytes[i];
    }
    return result;
}
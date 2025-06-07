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

void Decrypt::decrypt(string inputFile, string outputFile, string keyFile, bool isDebug)
{
    // ��ȡ���ܵ�����
    std::vector<unsigned char> fileContent = readFile(inputFile);
    // ���ļ���������ȡIV�����ܵ�AES��Կ�ͼ�������
    // 1. ��ȡIV����(1�ֽ�)
    if (fileContent.size() < 1) {
        throw std::runtime_error("Invalid encrypted file format: file too short");
    }
    size_t pos = 0;
    unsigned char ivLength = fileContent[pos++];

    // 2. ��ȡIV
    if (fileContent.size() < pos + ivLength) {
        throw std::runtime_error("Invalid encrypted file format: IV data incomplete");
    }
    std::vector<unsigned char> storedIV(fileContent.begin() + pos, fileContent.begin() + pos + ivLength);
    pos += ivLength;

    // 3. ��ȡ����AES��Կ����(2�ֽ�)
    if (fileContent.size() < pos + 2) {
        throw std::runtime_error("Invalid encrypted file format: key length data missing");
    }
    std::vector<unsigned char> keySizeBytes(fileContent.begin() + pos, fileContent.begin() + pos + 2);
    uint32_t keySize = this->bytesToInt(keySizeBytes);
    pos += 2;

    // 4. ��ȡ���ܵ�AES��Կ
    if (fileContent.size() < pos + keySize) {
        throw std::runtime_error("Invalid encrypted file format: encrypted key data incomplete");
    }
    std::vector<unsigned char> encryptedKey(fileContent.begin() + pos, fileContent.begin() + pos + keySize);
    pos += keySize;

    // 5. ��ȡ���ܵ��ļ�����
    std::vector<unsigned char> encryptedData(fileContent.begin() + pos, fileContent.end());

    if (isDebug) {
        std::cout << "IV Length: " << (int)ivLength << std::endl;
        std::cout << "IV Size: " << storedIV.size() << " bytes" << std::endl;
        std::cout << "Encrypted Key Size: " << keySize << " bytes" << std::endl;
        std::cout << "Encrypted Data Size: " << encryptedData.size() << " bytes" << std::endl;
    }

    // ʹ��RSA˽Կ����AES��Կ
    auto privateKey = loadPrivateKey(keyFile);
    std::vector<unsigned char> decryptedKey = this->decryptRSA(privateKey.get(), encryptedKey);

    // ʹ�ý��ܺ��AES��Կ�����ļ�����
    std::vector<unsigned char> decryptedContent = this->decryptAES(encryptedData, decryptedKey, storedIV);

    // ������ܺ�����ݵ��ļ�
    writeFile(outputFile, decryptedContent);
}
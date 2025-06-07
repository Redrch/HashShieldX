#include "decrypt.h"

Decrypt::Decrypt()
{
}

Decrypt::~Decrypt()
{
}

// 使用RSA解密AES密钥
vector<unsigned char> Decrypt::decryptRSA(EVP_PKEY* privkey, const vector<unsigned char>& encryptedData) {
    // 创建解密上下文
    unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new(privkey, nullptr));
    if (!ctx) {
        handleOpenSSLErrors();
    }

    // 初始化解密操作
    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) {
        handleOpenSSLErrors();
    }

    // 设置解密填充方式为OAEP (与加密时相同)
    if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
        handleOpenSSLErrors();
    }

    // 确定解密后的数据所需大小
    size_t outlen;
    if (EVP_PKEY_decrypt(ctx.get(), nullptr, &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
        handleOpenSSLErrors();
    }

    // 执行解密
    vector<unsigned char> decrypted(outlen);
    if (EVP_PKEY_decrypt(ctx.get(), decrypted.data(), &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
        handleOpenSSLErrors();
    }

    decrypted.resize(outlen);
    return decrypted;
}

// 使用AES-256-CBC解密文件内容
vector<unsigned char> Decrypt::decryptAES(
    const vector<unsigned char>& ciphertext,
    const vector<unsigned char>& key,
    const vector<unsigned char>& iv
) {
    // 创建并初始化解密上下文
    unique_ptr<EVP_CIPHER_CTX, OpenSSLDeleter> ctx(EVP_CIPHER_CTX_new());
    if (!ctx) {
        handleOpenSSLErrors();
    }

    // 初始化解密操作，使用AES-256-CBC模式
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        handleOpenSSLErrors();
    }

    // 准备解密结果的缓冲区（最大可能大小）
    vector<unsigned char> plaintext(ciphertext.size() + AES_BLOCK_SIZE);
    int len1 = 0, len2 = 0;

    // 解密数据
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len1, ciphertext.data(), ciphertext.size()) != 1) {
        handleOpenSSLErrors();
    }

    // 处理最后一块并移除任何填充
    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len1, &len2) != 1) {
        handleOpenSSLErrors();
    }

    // 调整输出向量大小为实际解密后的数据大小
    plaintext.resize(len1 + len2);
    return plaintext;
}

// 字节数组转换为整数
uint32_t Decrypt::bytesToInt(const vector<unsigned char>& bytes) {
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
    // 读取加密的数据
    vector<unsigned char> fileContent = readFile(inputFile);
    // 从文件内容中提取IV、加密的AES密钥和加密数据
    // 1. 读取IV长度(1字节)
    if (fileContent.size() < 1) {
        throw runtime_error("Invalid encrypted file format: file too short");
    }
    size_t pos = 0;
    unsigned char ivLength = fileContent[pos++];

    // 2. 读取IV
    if (fileContent.size() < pos + ivLength) {
        throw runtime_error("Invalid encrypted file format: IV data incomplete");
    }
    vector<unsigned char> storedIV(fileContent.begin() + pos, fileContent.begin() + pos + ivLength);
    pos += ivLength;

    // 3. 读取加密AES密钥长度(2字节)
    if (fileContent.size() < pos + 2) {
        throw runtime_error("Invalid encrypted file format: key length data missing");
    }
    vector<unsigned char> keySizeBytes(fileContent.begin() + pos, fileContent.begin() + pos + 2);
    uint32_t keySize = this->bytesToInt(keySizeBytes);
    pos += 2;

    // 4. 读取加密的AES密钥
    if (fileContent.size() < pos + keySize) {
        throw runtime_error("Invalid encrypted file format: encrypted key data incomplete");
    }
    vector<unsigned char> encryptedKey(fileContent.begin() + pos, fileContent.begin() + pos + keySize);
    pos += keySize;

    // 5. 读取加密的文件内容
    vector<unsigned char> encryptedData(fileContent.begin() + pos, fileContent.end());

    if (isDebug) {
        cout << "IV Length: " << (int)ivLength << endl;
        cout << "IV Size: " << storedIV.size() << " bytes" << endl;
        cout << "Encrypted Key Size: " << keySize << " bytes" << endl;
        cout << "Encrypted Data Size: " << encryptedData.size() << " bytes" << endl;
    }

    // 使用RSA私钥解密AES密钥
    auto privateKey = loadPrivateKey(keyFile);
    vector<unsigned char> decryptedKey = this->decryptRSA(privateKey.get(), encryptedKey);

    // 使用解密后的AES密钥解密文件内容
    vector<unsigned char> decryptedContent = this->decryptAES(encryptedData, decryptedKey, storedIV);

    // 保存解密后的内容到文件
    writeFile(outputFile, decryptedContent);
}
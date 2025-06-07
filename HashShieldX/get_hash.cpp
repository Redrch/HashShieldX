#include "get_hash.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>

GetHash::GetHash()
{
    // 初始化OpenSSL
    OpenSSL_add_all_digests();
}

GetHash::~GetHash()
{
    // 清理OpenSSL资源
    EVP_cleanup();
}

const EVP_MD* GetHash::getHashAlgorithm(HashType hashType)
{
    switch (hashType) {
    case HashType::MD5:    return EVP_md5();
    case HashType::SHA1:   return EVP_sha1();
    case HashType::SHA224: return EVP_sha224();
    case HashType::SHA256: return EVP_sha256();
    case HashType::SHA384: return EVP_sha384();
    case HashType::SHA512: return EVP_sha512();
    default:               return EVP_sha256(); // 默认使用SHA-256
    }
}

string GetHash::calculateFileHash(const string& filename, HashType hashType)
{
    // 打开文件
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("无法打开文件: " + filename);
    }

    // 获取哈希算法
    const EVP_MD* hashAlgorithm = getHashAlgorithm(hashType);
    if (!hashAlgorithm) {
        throw runtime_error("不支持的哈希算法");
    }

    // 创建并初始化哈希上下文
    unique_ptr<EVP_MD_CTX, OpenSSLDeleter> context(EVP_MD_CTX_new());
    if (!context) {
        throw runtime_error("无法创建哈希上下文");
    }

    if (EVP_DigestInit_ex(context.get(), hashAlgorithm, nullptr) != 1) {
        throw runtime_error("无法初始化哈希算法");
    }

    // 读取文件内容并更新哈希
    const size_t bufferSize = 8192; // 8KB buffer
    vector<unsigned char> buffer(bufferSize);

    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), bufferSize);
        size_t bytesRead = file.gcount();
        if (bytesRead > 0) {
            if (EVP_DigestUpdate(context.get(), buffer.data(), bytesRead) != 1) {
                throw runtime_error("更新哈希计算失败");
            }
        }
    }

    // 完成哈希计算
    unsigned int hashLength = 0;
    lastRawHash.resize(EVP_MAX_MD_SIZE);

    if (EVP_DigestFinal_ex(context.get(), lastRawHash.data(), &hashLength) != 1) {
        throw runtime_error("完成哈希计算失败");
    }

    // 调整哈希大小为实际长度
    lastRawHash.resize(hashLength);

    // 转换为十六进制字符串
    lastHashString = hashToHexString(lastRawHash);
    return lastHashString;
}

string GetHash::calculateHash(const vector<unsigned char>& data, HashType hashType)
{
    // 获取哈希算法
    const EVP_MD* hashAlgorithm = getHashAlgorithm(hashType);
    if (!hashAlgorithm) {
        throw runtime_error("不支持的哈希算法");
    }

    // 创建并初始化哈希上下文
    unique_ptr<EVP_MD_CTX, OpenSSLDeleter> context(EVP_MD_CTX_new());
    if (!context) {
        throw runtime_error("无法创建哈希上下文");
    }

    if (EVP_DigestInit_ex(context.get(), hashAlgorithm, nullptr) != 1) {
        throw runtime_error("无法初始化哈希算法");
    }

    // 更新哈希计算
    if (EVP_DigestUpdate(context.get(), data.data(), data.size()) != 1) {
        throw runtime_error("更新哈希计算失败");
    }

    // 完成哈希计算
    unsigned int hashLength = 0;
    lastRawHash.resize(EVP_MAX_MD_SIZE);

    if (EVP_DigestFinal_ex(context.get(), lastRawHash.data(), &hashLength) != 1) {
        throw runtime_error("完成哈希计算失败");
    }

    // 调整哈希大小为实际长度
    lastRawHash.resize(hashLength);

    // 转换为十六进制字符串
    lastHashString = hashToHexString(lastRawHash);
    return lastHashString;
}

string GetHash::getLastHash() const
{
    return lastHashString;
}

vector<unsigned char> GetHash::getLastRawHash() const
{
    return lastRawHash;
}

bool GetHash::verifyFileHash(const string& filename, const string& expectedHash, HashType hashType)
{
    try {
        string calculatedHash = calculateFileHash(filename, hashType);
        return toLowerHex(calculatedHash) == toLowerHex(expectedHash);
    }
    catch (const exception& e) {
        cerr << "验证哈希时出错: " << e.what() << endl;
        return false;
    }
}

string GetHash::hashToHexString(const vector<unsigned char>& hash)
{
    stringstream ss;
    for (unsigned char byte : hash) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

string GetHash::toLowerHex(const string& hexString)
{
    string result = hexString;
    transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return tolower(c); });
    return result;
}
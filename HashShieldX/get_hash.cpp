#include "get_hash.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>

GetHash::GetHash()
{
    // ��ʼ��OpenSSL
    OpenSSL_add_all_digests();
}

GetHash::~GetHash()
{
    // ����OpenSSL��Դ
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
    default:               return EVP_sha256(); // Ĭ��ʹ��SHA-256
    }
}

string GetHash::calculateFileHash(const string& filename, HashType hashType)
{
    // ���ļ�
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("�޷����ļ�: " + filename);
    }

    // ��ȡ��ϣ�㷨
    const EVP_MD* hashAlgorithm = getHashAlgorithm(hashType);
    if (!hashAlgorithm) {
        throw runtime_error("��֧�ֵĹ�ϣ�㷨");
    }

    // ��������ʼ����ϣ������
    unique_ptr<EVP_MD_CTX, OpenSSLDeleter> context(EVP_MD_CTX_new());
    if (!context) {
        throw runtime_error("�޷�������ϣ������");
    }

    if (EVP_DigestInit_ex(context.get(), hashAlgorithm, nullptr) != 1) {
        throw runtime_error("�޷���ʼ����ϣ�㷨");
    }

    // ��ȡ�ļ����ݲ����¹�ϣ
    const size_t bufferSize = 8192; // 8KB buffer
    vector<unsigned char> buffer(bufferSize);

    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), bufferSize);
        size_t bytesRead = file.gcount();
        if (bytesRead > 0) {
            if (EVP_DigestUpdate(context.get(), buffer.data(), bytesRead) != 1) {
                throw runtime_error("���¹�ϣ����ʧ��");
            }
        }
    }

    // ��ɹ�ϣ����
    unsigned int hashLength = 0;
    lastRawHash.resize(EVP_MAX_MD_SIZE);

    if (EVP_DigestFinal_ex(context.get(), lastRawHash.data(), &hashLength) != 1) {
        throw runtime_error("��ɹ�ϣ����ʧ��");
    }

    // ������ϣ��СΪʵ�ʳ���
    lastRawHash.resize(hashLength);

    // ת��Ϊʮ�������ַ���
    lastHashString = hashToHexString(lastRawHash);
    return lastHashString;
}

string GetHash::calculateHash(const vector<unsigned char>& data, HashType hashType)
{
    // ��ȡ��ϣ�㷨
    const EVP_MD* hashAlgorithm = getHashAlgorithm(hashType);
    if (!hashAlgorithm) {
        throw runtime_error("��֧�ֵĹ�ϣ�㷨");
    }

    // ��������ʼ����ϣ������
    unique_ptr<EVP_MD_CTX, OpenSSLDeleter> context(EVP_MD_CTX_new());
    if (!context) {
        throw runtime_error("�޷�������ϣ������");
    }

    if (EVP_DigestInit_ex(context.get(), hashAlgorithm, nullptr) != 1) {
        throw runtime_error("�޷���ʼ����ϣ�㷨");
    }

    // ���¹�ϣ����
    if (EVP_DigestUpdate(context.get(), data.data(), data.size()) != 1) {
        throw runtime_error("���¹�ϣ����ʧ��");
    }

    // ��ɹ�ϣ����
    unsigned int hashLength = 0;
    lastRawHash.resize(EVP_MAX_MD_SIZE);

    if (EVP_DigestFinal_ex(context.get(), lastRawHash.data(), &hashLength) != 1) {
        throw runtime_error("��ɹ�ϣ����ʧ��");
    }

    // ������ϣ��СΪʵ�ʳ���
    lastRawHash.resize(hashLength);

    // ת��Ϊʮ�������ַ���
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
        cerr << "��֤��ϣʱ����: " << e.what() << endl;
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
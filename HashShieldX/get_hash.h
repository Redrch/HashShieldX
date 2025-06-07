#pragma once

#include "crypto_utils.h"
#include <string>
#include <vector>
#include <algorithm>

class GetHash
{
public:
    // ֧�ֵĹ�ϣ�㷨����
    enum class HashType {
        MD5,
        SHA1,
        SHA224,
        SHA256,
        SHA384,
        SHA512
    };

    GetHash();
    ~GetHash();

    // �����ļ��Ĺ�ϣֵ
    string calculateFileHash(const string& filename, HashType hashType);

    // �����ڴ������ݵĹ�ϣֵ
    string calculateHash(const vector<unsigned char>& data, HashType hashType);

    // ��ȡ������Ĺ�ϣֵ(ʮ�������ַ���)
    string getLastHash() const;

    // ��ȡ�������ԭʼ��ϣֵ(������)
    vector<unsigned char> getLastRawHash() const;

    // ��֤�ļ��Ĺ�ϣֵ�Ƿ�ƥ��
    bool verifyFileHash(const string& filename, const string& expectedHash, HashType hashType);

private:
    // ��ȡָ���㷨�� EVP_MD ָ��
    const EVP_MD* getHashAlgorithm(HashType hashType);

    // ����ϣ������ֵת��Ϊʮ�������ַ���
    string hashToHexString(const vector<unsigned char>& hash);

    // ��ʮ�������ַ���ת��ΪСд��ʽ
    string toLowerHex(const string& hexString);

    // �洢������Ĺ�ϣֵ
    vector<unsigned char> lastRawHash;
    string lastHashString;
};


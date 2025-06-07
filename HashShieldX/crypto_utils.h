#pragma once

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace std;
// 自定义智能指针删除器，用于OpenSSL对象的安全释放
struct OpenSSLDeleter {
    void operator()(EVP_CIPHER_CTX* ctx) { EVP_CIPHER_CTX_free(ctx); }
    void operator()(EVP_PKEY* pkey) { EVP_PKEY_free(pkey); }
    void operator()(BIO* bio) { BIO_free_all(bio); }
    void operator()(EVP_PKEY_CTX* ctx) { EVP_PKEY_CTX_free(ctx); }
};

// 错误处理辅助函数
inline void handleOpenSSLErrors() {
    ERR_print_errors_fp(stderr);
    throw runtime_error("OpenSSL error occurred");
}

// 从文件加载RSA公钥
inline unique_ptr<EVP_PKEY, OpenSSLDeleter> loadPublicKey(const string& filename) {
    auto bio = unique_ptr<BIO, OpenSSLDeleter>(BIO_new_file(filename.c_str(), "r"));
    if (!bio) {
        handleOpenSSLErrors();
    }

    EVP_PKEY* pkey_raw = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    if (!pkey_raw) {
        handleOpenSSLErrors();
    }

    return unique_ptr<EVP_PKEY, OpenSSLDeleter>(pkey_raw);
}

// 从文件加载RSA私钥
inline unique_ptr<EVP_PKEY, OpenSSLDeleter> loadPrivateKey(const string& filename) {
    auto bio = unique_ptr<BIO, OpenSSLDeleter>(BIO_new_file(filename.c_str(), "r"));
    if (!bio) {
        handleOpenSSLErrors();
    }

    EVP_PKEY* pkey_raw = PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr);
    if (!pkey_raw) {
        handleOpenSSLErrors();
    }

    return unique_ptr<EVP_PKEY, OpenSSLDeleter>(pkey_raw);
}

// 读取整个文件内容
inline vector<unsigned char> readFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Cannot open file for reading: " + filename);
    }

    file.seekg(0, ios::end);
    size_t size = file.tellg();
    file.seekg(0, ios::beg);

    vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}

// 写入文件内容
inline void writeFile(const string& filename, const vector<unsigned char>& data) {
    ofstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Cannot open file for writing: " + filename);
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}
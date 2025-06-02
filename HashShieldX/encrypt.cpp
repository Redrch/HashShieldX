#include "encrypt.h"

Encrypt::Encrypt()
{

}

Encrypt::~Encrypt()
{

}

// 使用RSA加密AES密钥
std::vector<unsigned char> Encrypt::encryptRSA(EVP_PKEY* pubkey, const std::vector<unsigned char>& data) {
	// 创建加密上下文
	std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new(pubkey, nullptr));
	if (!ctx) {
		handleOpenSSLErrors();
	}

	// 初始化加密操作
	if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) {
		handleOpenSSLErrors();
	}

	// 设置加密填充方式为OAEP (用于RSA加密)
	if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
		handleOpenSSLErrors();
	}

	// 确定加密后的输出缓冲区大小
	size_t outlen;
	if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen, data.data(), data.size()) <= 0) {
		handleOpenSSLErrors();
	}

	// 执行加密
	std::vector<unsigned char> encrypted(outlen);
	if (EVP_PKEY_encrypt(ctx.get(), encrypted.data(), &outlen, data.data(), data.size()) <= 0) {
		handleOpenSSLErrors();
	}

	encrypted.resize(outlen);
	return encrypted;
}

// 使用AES-256-CBC加密文件内容
std::vector<unsigned char> Encrypt::encryptAES(
	const std::vector<unsigned char>& plaintext,
	const std::vector<unsigned char>& key,
	const std::vector<unsigned char>& iv
) {
	// 创建并初始化加密上下文
	std::unique_ptr<EVP_CIPHER_CTX, OpenSSLDeleter> ctx(EVP_CIPHER_CTX_new());
	if (!ctx) {
		handleOpenSSLErrors();
	}

	// 初始化加密操作，使用AES-256-CBC模式
	if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
		handleOpenSSLErrors();
	}

	// 分配加密结果的缓冲区（最大可能大小）
	std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
	int len1 = 0, len2 = 0;

	// 加密数据
	if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len1, plaintext.data(), plaintext.size()) != 1) {
		handleOpenSSLErrors();
	}

	// 处理最后的块并添加任何必要的填充
	if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len1, &len2) != 1) {
		handleOpenSSLErrors();
	}

	// 调整结果向量大小为实际加密后的数据大小
	ciphertext.resize(len1 + len2);
	return ciphertext;
}

// 将整数转换为字节向量
std::vector<unsigned char> Encrypt::intToBytes(uint32_t value, int bytes) {
	std::vector<unsigned char> result(bytes);
	for (int i = 0; i < bytes; i++) {
		result[bytes - i - 1] = (value >> (i * 8)) & 0xFF;
	}
	return result;
}

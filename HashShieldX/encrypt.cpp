#include "encrypt.h"

Encrypt::Encrypt()
{

}

Encrypt::~Encrypt()
{

}

// ʹ��RSA����AES��Կ
std::vector<unsigned char> Encrypt::encryptRSA(EVP_PKEY* pubkey, const std::vector<unsigned char>& data) {
	// ��������������
	std::unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new(pubkey, nullptr));
	if (!ctx) {
		handleOpenSSLErrors();
	}

	// ��ʼ�����ܲ���
	if (EVP_PKEY_encrypt_init(ctx.get()) <= 0) {
		handleOpenSSLErrors();
	}

	// ���ü�����䷽ʽΪOAEP (����RSA����)
	if (EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_OAEP_PADDING) <= 0) {
		handleOpenSSLErrors();
	}

	// ȷ�����ܺ�������������С
	size_t outlen;
	if (EVP_PKEY_encrypt(ctx.get(), nullptr, &outlen, data.data(), data.size()) <= 0) {
		handleOpenSSLErrors();
	}

	// ִ�м���
	std::vector<unsigned char> encrypted(outlen);
	if (EVP_PKEY_encrypt(ctx.get(), encrypted.data(), &outlen, data.data(), data.size()) <= 0) {
		handleOpenSSLErrors();
	}

	encrypted.resize(outlen);
	return encrypted;
}

// ʹ��AES-256-CBC�����ļ�����
std::vector<unsigned char> Encrypt::encryptAES(
	const std::vector<unsigned char>& plaintext,
	const std::vector<unsigned char>& key,
	const std::vector<unsigned char>& iv
) {
	// ��������ʼ������������
	std::unique_ptr<EVP_CIPHER_CTX, OpenSSLDeleter> ctx(EVP_CIPHER_CTX_new());
	if (!ctx) {
		handleOpenSSLErrors();
	}

	// ��ʼ�����ܲ�����ʹ��AES-256-CBCģʽ
	if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
		handleOpenSSLErrors();
	}

	// ������ܽ���Ļ������������ܴ�С��
	std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
	int len1 = 0, len2 = 0;

	// ��������
	if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len1, plaintext.data(), plaintext.size()) != 1) {
		handleOpenSSLErrors();
	}

	// �������Ŀ鲢����κα�Ҫ�����
	if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len1, &len2) != 1) {
		handleOpenSSLErrors();
	}

	// �������������СΪʵ�ʼ��ܺ�����ݴ�С
	ciphertext.resize(len1 + len2);
	return ciphertext;
}

// ������ת��Ϊ�ֽ�����
std::vector<unsigned char> Encrypt::intToBytes(uint32_t value, int bytes) {
	std::vector<unsigned char> result(bytes);
	for (int i = 0; i < bytes; i++) {
		result[bytes - i - 1] = (value >> (i * 8)) & 0xFF;
	}
	return result;
}

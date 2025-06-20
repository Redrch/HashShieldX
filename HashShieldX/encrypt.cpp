#include "encrypt.h"

Encrypt::Encrypt()
{

}

Encrypt::~Encrypt()
{

}

// 使用RSA加密AES密钥
vector<unsigned char> Encrypt::encryptRSA(EVP_PKEY* pubkey, const vector<unsigned char>& data) {
	// 创建加密上下文
	unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new(pubkey, nullptr));
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
	vector<unsigned char> encrypted(outlen);
	if (EVP_PKEY_encrypt(ctx.get(), encrypted.data(), &outlen, data.data(), data.size()) <= 0) {
		handleOpenSSLErrors();
	}

	encrypted.resize(outlen);
	return encrypted;
}

// 使用AES-256-CBC加密文件内容
vector<unsigned char> Encrypt::encryptAES(
	const vector<unsigned char>& plaintext,
	const vector<unsigned char>& key,
	const vector<unsigned char>& iv
) {
	// 创建并初始化加密上下文
	unique_ptr<EVP_CIPHER_CTX, OpenSSLDeleter> ctx(EVP_CIPHER_CTX_new());
	if (!ctx) {
		handleOpenSSLErrors();
	}

	// 初始化加密操作，使用AES-256-CBC模式
	if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
		handleOpenSSLErrors();
	}

	// 分配加密结果的缓冲区（最大可能大小）
	vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_SIZE);
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
vector<unsigned char> Encrypt::intToBytes(uint32_t value, int bytes) {
	vector<unsigned char> result(bytes);
	for (int i = 0; i < bytes; i++) {
		result[bytes - i - 1] = (value >> (i * 8)) & 0xFF;
	}
	return result;
}

void Encrypt::encryptSmallFile(string inputFile, string outputFile, string keyFile)
{
	// 读取文件
	vector<unsigned char> content = readFile(inputFile);
	// 加载RSA公钥
	auto public_key = loadPublicKey(keyFile);

	// 生成随机AES密钥(256bits=32Bytes)
	vector<unsigned char> aesKey(32);
	if (RAND_bytes(aesKey.data(), aesKey.size()) != 1) {
		handleOpenSSLErrors();
	}

	// 生成随机IV (AES块大小 = 16Bytes)
	vector<unsigned char> iv(AES_BLOCK_SIZE);
	if (RAND_bytes(iv.data(), iv.size()) != 1) {
		handleOpenSSLErrors();
	}

	// 使用RSA公钥加密AES密钥
	vector<unsigned char> encrypted_key = this->encryptRSA(public_key.get(), aesKey);

	// 使用AES-256-CBC加密文件内容
	vector<unsigned char> ciphertext = this->encryptAES(content, aesKey, iv);

	// 构建输出文件
	// 文件格式: [IV长度(1字节)][IV][加密AES密钥长度(2字节)][加密AES密钥][加密内容]
	vector<unsigned char> output;

	// 添加IV长度 (1字节)
	output.push_back(static_cast<unsigned char>(iv.size()));

	// 添加IV
	output.insert(output.end(), iv.begin(), iv.end());

	// 添加加密后AES密钥长度 (2字节)
	auto key_size_bytes = this->intToBytes(encrypted_key.size(), 2);
	output.insert(output.end(), key_size_bytes.begin(), key_size_bytes.end());

	// 添加加密后的AES密钥
	output.insert(output.end(), encrypted_key.begin(), encrypted_key.end());

	// 添加加密后的文件内容
	output.insert(output.end(), ciphertext.begin(), ciphertext.end());

	// 写入输出文件
	writeFile(outputFile, output);
}

void Encrypt::encrypt(string inputFile, string outputFile, string keyFile, bool isDebug)
{
	vector<unsigned char> content = readFile(inputFile);
	size_t size = content.size();
	if (size <= 256 * 1024 * 1024)  // size <= 256MiB
	{
		this->encryptSmallFile(inputFile, outputFile, keyFile);
	}
	else
	{
		filesystem::create_directory("C:/tmp");
		filesystem::create_directory("C:/tmp/e");
		SplitFile splitFile;
		splitFile.splitFile(inputFile, "C:/tmp", 256 * 1024 * 1024, isDebug);
		// 遍历目录
		for (auto& entry : filesystem::directory_iterator("C:/tmp"))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}
			string path = entry.path().string();
			this->encryptSmallFile(path, "C:/tmp/e/" + entry.path().filename().string(), keyFile);
		}
		// 压缩文件
		struct zip* archive = zip_open(outputFile.c_str(), ZIP_CREATE | ZIP_TRUNCATE, NULL);
		if (isDebug) cout << "zip output file: " << outputFile.c_str() << endl;
		if (!archive)
		{
			cerr << "无法创建输出文件！" << endl;
			return;
		}
		// 遍历目录
		for (auto& entry : filesystem::directory_iterator("C:/tmp/e"))
		{
			string path = entry.path().string();
			// Create source
			zip_source_t* source = zip_source_file(archive, path.c_str(), 0, -1);
			if (!source)
			{
				cerr << "无法创建源" << endl;
				zip_close(archive);
				return;
			}
			// Add to file
			zip_int64_t index = zip_file_add(archive, path.c_str(), source, ZIP_FL_OVERWRITE);
			if (index < 0)
			{
				cerr << "无法将文件进行组合" << endl;
				zip_source_free(source);
				zip_close(archive);
				return;
			}
			// Set file compression
			if (zip_set_file_compression(archive, index, ZIP_CM_DEFLATE, 9))
			{
				cerr << "无法设置压缩级别" << endl;
			}
			if (isDebug) cout << "Added file: " << entry.path().filename().string() << endl;
		}
		// Close file
		zip_close(archive);
		// Clear temp file
		filesystem::remove_all("C:/tmp");
	}
}
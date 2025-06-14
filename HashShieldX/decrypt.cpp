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

void Decrypt::decryptSmallFile(string inputFile, string outputFile, string keyFile, bool isDebug)
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

void Decrypt::decrypt(string inputFile, string outputFile, string keyFile, bool isDebug)
{
	int error = 0;
	zip_t* archive = zip_open(inputFile.c_str(), ZIP_CHECKCONS, &error);
	// 为单文件
	if (archive == nullptr && error == ZIP_ER_NOZIP)
	{
		this->decryptSmallFile(inputFile, outputFile, keyFile, isDebug);
	}
	// 为多文件
	else
	{
		const string tempDir = "C:/temp_decrypt";
		const string tempOutputDir = "C:/temp_decrypt/d";
		// 创建临时文件夹
		filesystem::create_directory(tempDir.c_str());
		filesystem::create_directory(tempOutputDir.c_str());
		// 获取条目数量
		zip_int64_t numEntries = zip_get_num_entries(archive, 0);
		if (isDebug) cout << "ZIP文件中包含 " << numEntries << "个条目" << endl;
		// 创建缓冲区
		const int BURRER_SIZE = 8192;
		vector<char> buffer(BURRER_SIZE);

		// 遍历所有条目
		for (zip_int64_t i = 0; i < numEntries; i++) {
			// 获取条目名称
			const char* name = zip_get_name(archive, i, 0);

			if (!name) {
				cerr << "无法获取条目 #" << i << " 的名称" << endl;
				continue;
			}

			string entryName = name;
			vector<string> parts = split(entryName, '/');
			entryName = parts[parts.size() - 1];

			entryName.erase(0, 2);

			string fullPath = tempDir + "/" + entryName;

			// 检查是否为目录
			if (entryName.back() == '/') {
				if (isDebug) cout << "创建目录: " << fullPath << endl;
				filesystem::create_directories(fullPath);
				continue;
			}

			// 创建父目录
			filesystem::path parentPath = filesystem::path(fullPath).parent_path();
			if (!parentPath.empty()) {
				filesystem::create_directories(parentPath);
			}

			// 打开压缩文件
			zip_file_t* file = zip_fopen_index(archive, i, 0);
			if (!file) {
				cerr << "无法打开条目: " << entryName << endl;
				continue;
			}

			// 创建输出文件
			ofstream outFile(fullPath, ios::binary);
			if (!outFile) {
				cerr << "无法创建文件: " << fullPath << endl;
				zip_fclose(file);
				continue;
			}

			// 读取并写入文件内容
			zip_int64_t bytesRead;
			while ((bytesRead = zip_fread(file, buffer.data(), buffer.size())) > 0) {
				outFile.write(buffer.data(), bytesRead);
				if (!outFile) {
					cerr << "写入文件失败: " << fullPath << endl;
					break;
				}
			}

			// 关闭文件
			outFile.close();
			zip_fclose(file);

			if (isDebug) cout << "已解压: " << entryName << endl;
		}

		zip_close(archive);

		string baseName;
		// 解密文件
		for (auto entry : filesystem::directory_iterator(tempDir))
		{
			string path = entry.path().string();
			// 标准化路径分隔符
			replace(path.begin(), path.end(), '\\', '/');
			if (!filesystem::is_directory(path))
			{
				this->decryptSmallFile(path, tempOutputDir + "/" + entry.path().filename().string(), keyFile, isDebug);
				if (isDebug) cout << "已成功解密文件：" << entry.path().filename().string() << endl;
				baseName = entry.path().filename().string();
			}
		}

		// 拼接文件
		SplitFile sf;
		// 寻找文件头
		regex pattern("part\\d*_of_\\d*");
		smatch postfixList;
		if (!regex_search(baseName, postfixList, pattern))
		{
			cerr << "加密文件被损坏，无法解密！" << endl;
			return;
		}
		string postfix = postfixList[0];
		
		// 从baseName中删除postfix部分
		string cleanBaseName = baseName;
		size_t postfixPos = cleanBaseName.find(postfix);
		if (postfixPos != string::npos) {
			// 找到了postfix，删除它
			cleanBaseName.erase(postfixPos, postfix.length());

			// 如果postfix前面有分隔符(如"_")，也一并删除
			if (postfixPos > 0 && cleanBaseName[postfixPos - 1] == '_') {
				cleanBaseName.erase(postfixPos - 1, 1);
			}
		}

		// 可能需要进一步清理字符串，删除多余的空格等
		cleanBaseName.erase(0, cleanBaseName.find_first_not_of(" \t\n\r\f\v"));
		cleanBaseName.erase(cleanBaseName.find_last_not_of(" \t\n\r\f\v") + 1);

		// 替换原来的baseName或使用cleanBaseName继续处理
		baseName = cleanBaseName;
		// 清除文件名末尾的逗号
		baseName.pop_back();
		// 输出调试信息
		if (isDebug) cout << "Base name: " << baseName << endl;

		// 合并文件
		sf.mergeChunks(tempOutputDir + "/" + baseName, outputFile, isDebug);

		// 删除临时文件
		filesystem::remove_all(tempDir);
	}
}

vector<string> Decrypt::split(const string& str, char delimiter) {
	vector<string> result;
	size_t start = 0;
	size_t end = str.find(delimiter);

	while (end != string::npos) {
		result.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimiter, start);
	}

	// 添加最后一个部分
	result.push_back(str.substr(start));

	return result;
}


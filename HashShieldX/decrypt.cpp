#include "decrypt.h"

Decrypt::Decrypt()
{
}

Decrypt::~Decrypt()
{
}

// ʹ��RSA����AES��Կ
vector<unsigned char> Decrypt::decryptRSA(EVP_PKEY* privkey, const vector<unsigned char>& encryptedData) {
	// ��������������
	unique_ptr<EVP_PKEY_CTX, OpenSSLDeleter> ctx(EVP_PKEY_CTX_new(privkey, nullptr));
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
	vector<unsigned char> decrypted(outlen);
	if (EVP_PKEY_decrypt(ctx.get(), decrypted.data(), &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
		handleOpenSSLErrors();
	}

	decrypted.resize(outlen);
	return decrypted;
}

// ʹ��AES-256-CBC�����ļ�����
vector<unsigned char> Decrypt::decryptAES(
	const vector<unsigned char>& ciphertext,
	const vector<unsigned char>& key,
	const vector<unsigned char>& iv
) {
	// ��������ʼ������������
	unique_ptr<EVP_CIPHER_CTX, OpenSSLDeleter> ctx(EVP_CIPHER_CTX_new());
	if (!ctx) {
		handleOpenSSLErrors();
	}

	// ��ʼ�����ܲ�����ʹ��AES-256-CBCģʽ
	if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
		handleOpenSSLErrors();
	}

	// ׼�����ܽ���Ļ������������ܴ�С��
	vector<unsigned char> plaintext(ciphertext.size() + AES_BLOCK_SIZE);
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
	// ��ȡ���ܵ�����
	vector<unsigned char> fileContent = readFile(inputFile);
	// ���ļ���������ȡIV�����ܵ�AES��Կ�ͼ�������
	// 1. ��ȡIV����(1�ֽ�)
	if (fileContent.size() < 1) {
		throw runtime_error("Invalid encrypted file format: file too short");
	}
	size_t pos = 0;
	unsigned char ivLength = fileContent[pos++];

	// 2. ��ȡIV
	if (fileContent.size() < pos + ivLength) {
		throw runtime_error("Invalid encrypted file format: IV data incomplete");
	}
	vector<unsigned char> storedIV(fileContent.begin() + pos, fileContent.begin() + pos + ivLength);
	pos += ivLength;

	// 3. ��ȡ����AES��Կ����(2�ֽ�)
	if (fileContent.size() < pos + 2) {
		throw runtime_error("Invalid encrypted file format: key length data missing");
	}
	vector<unsigned char> keySizeBytes(fileContent.begin() + pos, fileContent.begin() + pos + 2);
	uint32_t keySize = this->bytesToInt(keySizeBytes);
	pos += 2;

	// 4. ��ȡ���ܵ�AES��Կ
	if (fileContent.size() < pos + keySize) {
		throw runtime_error("Invalid encrypted file format: encrypted key data incomplete");
	}
	vector<unsigned char> encryptedKey(fileContent.begin() + pos, fileContent.begin() + pos + keySize);
	pos += keySize;

	// 5. ��ȡ���ܵ��ļ�����
	vector<unsigned char> encryptedData(fileContent.begin() + pos, fileContent.end());

	if (isDebug) {
		cout << "IV Length: " << (int)ivLength << endl;
		cout << "IV Size: " << storedIV.size() << " bytes" << endl;
		cout << "Encrypted Key Size: " << keySize << " bytes" << endl;
		cout << "Encrypted Data Size: " << encryptedData.size() << " bytes" << endl;
	}

	// ʹ��RSA˽Կ����AES��Կ
	auto privateKey = loadPrivateKey(keyFile);
	vector<unsigned char> decryptedKey = this->decryptRSA(privateKey.get(), encryptedKey);

	// ʹ�ý��ܺ��AES��Կ�����ļ�����
	vector<unsigned char> decryptedContent = this->decryptAES(encryptedData, decryptedKey, storedIV);

	// ������ܺ�����ݵ��ļ�
	writeFile(outputFile, decryptedContent);
}

void Decrypt::decrypt(string inputFile, string outputFile, string keyFile, bool isDebug)
{
	int error = 0;
	zip_t* archive = zip_open(inputFile.c_str(), ZIP_CHECKCONS, &error);
	// Ϊ���ļ�
	if (archive == nullptr && error == ZIP_ER_NOZIP)
	{
		this->decryptSmallFile(inputFile, outputFile, keyFile, isDebug);
	}
	// Ϊ���ļ�
	else
	{
		const string tempDir = "C:/temp_decrypt";
		const string tempOutputDir = "C:/temp_decrypt/d";
		// ������ʱ�ļ���
		filesystem::create_directory(tempDir.c_str());
		filesystem::create_directory(tempOutputDir.c_str());
		// ��ȡ��Ŀ����
		zip_int64_t numEntries = zip_get_num_entries(archive, 0);
		if (isDebug) cout << "ZIP�ļ��а��� " << numEntries << "����Ŀ" << endl;
		// ����������
		const int BURRER_SIZE = 8192;
		vector<char> buffer(BURRER_SIZE);

		// ����������Ŀ
		for (zip_int64_t i = 0; i < numEntries; i++) {
			// ��ȡ��Ŀ����
			const char* name = zip_get_name(archive, i, 0);

			if (!name) {
				cerr << "�޷���ȡ��Ŀ #" << i << " ������" << endl;
				continue;
			}

			string entryName = name;
			vector<string> parts = split(entryName, '/');
			entryName = parts[parts.size() - 1];

			entryName.erase(0, 2);

			string fullPath = tempDir + "/" + entryName;

			// ����Ƿ�ΪĿ¼
			if (entryName.back() == '/') {
				if (isDebug) cout << "����Ŀ¼: " << fullPath << endl;
				filesystem::create_directories(fullPath);
				continue;
			}

			// ������Ŀ¼
			filesystem::path parentPath = filesystem::path(fullPath).parent_path();
			if (!parentPath.empty()) {
				filesystem::create_directories(parentPath);
			}

			// ��ѹ���ļ�
			zip_file_t* file = zip_fopen_index(archive, i, 0);
			if (!file) {
				cerr << "�޷�����Ŀ: " << entryName << endl;
				continue;
			}

			// ��������ļ�
			ofstream outFile(fullPath, ios::binary);
			if (!outFile) {
				cerr << "�޷������ļ�: " << fullPath << endl;
				zip_fclose(file);
				continue;
			}

			// ��ȡ��д���ļ�����
			zip_int64_t bytesRead;
			while ((bytesRead = zip_fread(file, buffer.data(), buffer.size())) > 0) {
				outFile.write(buffer.data(), bytesRead);
				if (!outFile) {
					cerr << "д���ļ�ʧ��: " << fullPath << endl;
					break;
				}
			}

			// �ر��ļ�
			outFile.close();
			zip_fclose(file);

			if (isDebug) cout << "�ѽ�ѹ: " << entryName << endl;
		}

		zip_close(archive);

		string baseName;
		// �����ļ�
		for (auto entry : filesystem::directory_iterator(tempDir))
		{
			string path = entry.path().string();
			// ��׼��·���ָ���
			replace(path.begin(), path.end(), '\\', '/');
			if (!filesystem::is_directory(path))
			{
				this->decryptSmallFile(path, tempOutputDir + "/" + entry.path().filename().string(), keyFile, isDebug);
				if (isDebug) cout << "�ѳɹ������ļ���" << entry.path().filename().string() << endl;
				baseName = entry.path().filename().string();
			}
		}

		// ƴ���ļ�
		SplitFile sf;
		// Ѱ���ļ�ͷ
		regex pattern("part\\d*_of_\\d*");
		smatch postfixList;
		if (!regex_search(baseName, postfixList, pattern))
		{
			cerr << "�����ļ����𻵣��޷����ܣ�" << endl;
			return;
		}
		string postfix = postfixList[0];
		
		// ��baseName��ɾ��postfix����
		string cleanBaseName = baseName;
		size_t postfixPos = cleanBaseName.find(postfix);
		if (postfixPos != string::npos) {
			// �ҵ���postfix��ɾ����
			cleanBaseName.erase(postfixPos, postfix.length());

			// ���postfixǰ���зָ���(��"_")��Ҳһ��ɾ��
			if (postfixPos > 0 && cleanBaseName[postfixPos - 1] == '_') {
				cleanBaseName.erase(postfixPos - 1, 1);
			}
		}

		// ������Ҫ��һ�������ַ�����ɾ������Ŀո��
		cleanBaseName.erase(0, cleanBaseName.find_first_not_of(" \t\n\r\f\v"));
		cleanBaseName.erase(cleanBaseName.find_last_not_of(" \t\n\r\f\v") + 1);

		// �滻ԭ����baseName��ʹ��cleanBaseName��������
		baseName = cleanBaseName;
		// ����ļ���ĩβ�Ķ���
		baseName.pop_back();
		// ���������Ϣ
		if (isDebug) cout << "Base name: " << baseName << endl;

		// �ϲ��ļ�
		sf.mergeChunks(tempOutputDir + "/" + baseName, outputFile, isDebug);

		// ɾ����ʱ�ļ�
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

	// ������һ������
	result.push_back(str.substr(start));

	return result;
}


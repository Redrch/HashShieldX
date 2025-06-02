#include <iostream>
#include <string>
#include "CLI11.hpp"

#include "encrypt.h"
#include "decrypt.h"

using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"����һ��С���ߣ����԰���ӽ����ļ����У��ͼ������ļ����У���Hashֵ"};

	app.set_version_flag("--version", VERSION, "��ʾ�汾��");

	// Is open debug mode
	bool isDebug = false;
	app.add_flag("-d, --debug", isDebug, "���õ���ģʽ");

	// Encrypt Command
	auto* encryptCommand = app.add_subcommand("encrypt", "�����ļ�");
	// Required
	// Encrypt input file
	string encryptInputFile;
	encryptCommand->add_option("-i, --input", encryptInputFile, "Ҫ���ܵ��ļ�")->required();
	// Encrypt output file
	string encryptOutputFile;
	encryptCommand->add_option("-o, --output", encryptOutputFile, "����ļ�")->required();
	// Encrypt key file
	string encryptKeyFile;
	encryptCommand->add_option("-k, --key", encryptKeyFile, "��Կ�ļ�")->required();
	
	// Optional
	// Is force overwrite of the output file
	bool isForce = false;
	encryptCommand->add_flag("-f, --force", isForce, "�Ƿ�ǿ�Ƹ�������ļ�");

	// Decrypt Command
	auto* decryptCommand = app.add_subcommand("decrypt", "�����ļ�");
	// Required
	// Decrypt input file
	string decryptInputFile;
	decryptCommand->add_option("-i, --input", decryptInputFile, "Ҫ���ܵ��ļ�")->required();
	// Decrypt output file
	string decryptOutputFile;
	decryptCommand->add_option("-o, --output", decryptOutputFile, "����ļ�")->required();
	// Decrypt Key file
	string decryptKeyFile;
	decryptCommand->add_option("-k, --key", decryptKeyFile, "��Կ�ļ�")->required();

	// ֻ��Ҫһ��������
	app.require_subcommand(1);

	// ���������в���
	try {
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& e) {
		return app.exit(e);
	}

	// ��������
	// Encrypt
	if (encryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "������Debugģʽ" << endl;
		}
		cout << "�������ļ�(Input File): " << encryptInputFile << endl;
		cout << "����ļ�(Output File): " << encryptOutputFile << endl;
		cout << "��Կ�ļ�(Key File): " << encryptKeyFile << endl;
		if (isForce)
		{
			cout << "ǿ�Ƹ�������ļ�" << endl;
		}
		cout << "׼����ʼ����" << endl;

		// ��ʼ��OpenSSL
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();

		// ʵ����Encrypt
		Encrypt* encrypt = new Encrypt;
		try
		{
			// ��ȡ�ļ�
			vector<unsigned char> content = readFile(encryptInputFile);
			// ����RSA��Կ
			auto public_key = loadPublicKey(encryptKeyFile);

			// �������AES��Կ(256bits=32Bytes)
			std::vector<unsigned char> aesKey(32);
			if (RAND_bytes(aesKey.data(), aesKey.size()) != 1) {
				handleOpenSSLErrors();
			}

			// �������IV (AES���С = 16Bytes)
			std::vector<unsigned char> iv(AES_BLOCK_SIZE);
			if (RAND_bytes(iv.data(), iv.size()) != 1) {
				handleOpenSSLErrors();
			}

			// ʹ��RSA��Կ����AES��Կ
			std::vector<unsigned char> encrypted_key = encrypt->encryptRSA(public_key.get(), aesKey);

			// ʹ��AES-256-CBC�����ļ�����
			std::vector<unsigned char> ciphertext = encrypt->encryptAES(content, aesKey, iv);

			// ��������ļ�
			// �ļ���ʽ: [IV����(1�ֽ�)][IV][����AES��Կ����(2�ֽ�)][����AES��Կ][��������]
			std::vector<unsigned char> output;

			// ���IV���� (1�ֽ�)
			output.push_back(static_cast<unsigned char>(iv.size()));

			// ���IV
			output.insert(output.end(), iv.begin(), iv.end());

			// ��Ӽ��ܺ�AES��Կ���� (2�ֽ�)
			auto key_size_bytes = encrypt->intToBytes(encrypted_key.size(), 2);
			output.insert(output.end(), key_size_bytes.begin(), key_size_bytes.end());

			// ��Ӽ��ܺ��AES��Կ
			output.insert(output.end(), encrypted_key.begin(), encrypted_key.end());

			// ��Ӽ��ܺ���ļ�����
			output.insert(output.end(), ciphertext.begin(), ciphertext.end());

			// д������ļ�
			writeFile(encryptOutputFile, output);

			std::cout << "�ļ����ܳɹ��� " << encryptOutputFile << std::endl;
		}
		catch (const std::exception& e)
		{
			cerr << "Error: " << e.what() << endl;
			return 1;
		}
	}
	// Decrypt
	else if (decryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "������Debugģʽ" << endl;
		}
		cout << "�������ļ�(Input File): " << decryptInputFile << endl;
		cout << "����ļ�(Output File): " << decryptOutputFile << endl;
		cout << "��Կ�ļ�(Key File): " << decryptKeyFile << endl;
		if (isForce)
		{
			cout << "ǿ�Ƹ�������ļ�" << endl;
		}
		cout << "׼����ʼ����" << endl;

		// ��ʼ��OpenSSL
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();

		// ʵ��������
		Decrypt* decrypt = new Decrypt;
		try
		{
			// ��ȡ���ܵ�����
			std::vector<unsigned char> fileContent = readFile(decryptInputFile);
			// ���ļ���������ȡIV�����ܵ�AES��Կ�ͼ�������
			// 1. ��ȡIV����(1�ֽ�)
			if (fileContent.size() < 1) {
				throw std::runtime_error("Invalid encrypted file format: file too short");
			}
			size_t pos = 0;
			unsigned char ivLength = fileContent[pos++];

			// 2. ��ȡIV
			if (fileContent.size() < pos + ivLength) {
				throw std::runtime_error("Invalid encrypted file format: IV data incomplete");
			}
			std::vector<unsigned char> storedIV(fileContent.begin() + pos, fileContent.begin() + pos + ivLength);
			pos += ivLength;

			// 3. ��ȡ����AES��Կ����(2�ֽ�)
			if (fileContent.size() < pos + 2) {
				throw std::runtime_error("Invalid encrypted file format: key length data missing");
			}
			std::vector<unsigned char> keySizeBytes(fileContent.begin() + pos, fileContent.begin() + pos + 2);
			uint32_t keySize = decrypt->bytesToInt(keySizeBytes);
			pos += 2;

			// 4. ��ȡ���ܵ�AES��Կ
			if (fileContent.size() < pos + keySize) {
				throw std::runtime_error("Invalid encrypted file format: encrypted key data incomplete");
			}
			std::vector<unsigned char> encryptedKey(fileContent.begin() + pos, fileContent.begin() + pos + keySize);
			pos += keySize;

			// 5. ��ȡ���ܵ��ļ�����
			std::vector<unsigned char> encryptedData(fileContent.begin() + pos, fileContent.end());

			if (isDebug) {
				std::cout << "IV Length: " << (int)ivLength << std::endl;
				std::cout << "IV Size: " << storedIV.size() << " bytes" << std::endl;
				std::cout << "Encrypted Key Size: " << keySize << " bytes" << std::endl;
				std::cout << "Encrypted Data Size: " << encryptedData.size() << " bytes" << std::endl;
			}

			// ʹ��RSA˽Կ����AES��Կ
			auto privateKey = loadPrivateKey(decryptKeyFile);
			std::vector<unsigned char> decryptedKey = decrypt->decryptRSA(privateKey.get(), encryptedKey);

			// ʹ�ý��ܺ��AES��Կ�����ļ�����
			std::vector<unsigned char> decryptedContent = decrypt->decryptAES(encryptedData, decryptedKey, storedIV);

			// ������ܺ�����ݵ��ļ�
			writeFile(decryptOutputFile, decryptedContent);

			std::cout << "�ļ����ܳɹ��� " << encryptOutputFile << std::endl;
		}
		catch (const std::exception& e)
		{
			cerr << "Error: " << e.what() << endl;
		}
	}

	return 0;
}
#include <iostream>
#include <string>
#include "CLI11.hpp"

#include "encrypt.h"
#include "decrypt.h"

using namespace std;

int main(int argc, char* argv[])
{
	CLI::App app{"这是一个小工具，可以帮你加解密文件（夹）和计算多个文件（夹）的Hash值"};

	app.set_version_flag("--version", VERSION, "显示版本号");

	// Is open debug mode
	bool isDebug = false;
	app.add_flag("-d, --debug", isDebug, "启用调试模式");

	// Encrypt Command
	auto* encryptCommand = app.add_subcommand("encrypt", "加密文件");
	// Required
	// Encrypt input file
	string encryptInputFile;
	encryptCommand->add_option("-i, --input", encryptInputFile, "要加密的文件")->required();
	// Encrypt output file
	string encryptOutputFile;
	encryptCommand->add_option("-o, --output", encryptOutputFile, "输出文件")->required();
	// Encrypt key file
	string encryptKeyFile;
	encryptCommand->add_option("-k, --key", encryptKeyFile, "密钥文件")->required();
	
	// Optional
	// Is force overwrite of the output file
	bool isForce = false;
	encryptCommand->add_flag("-f, --force", isForce, "是否强制覆盖输出文件");

	// Decrypt Command
	auto* decryptCommand = app.add_subcommand("decrypt", "解密文件");
	// Required
	// Decrypt input file
	string decryptInputFile;
	decryptCommand->add_option("-i, --input", decryptInputFile, "要加密的文件")->required();
	// Decrypt output file
	string decryptOutputFile;
	decryptCommand->add_option("-o, --output", decryptOutputFile, "输出文件")->required();
	// Decrypt Key file
	string decryptKeyFile;
	decryptCommand->add_option("-k, --key", decryptKeyFile, "密钥文件")->required();

	// 只需要一个子命令
	app.require_subcommand(1);

	// 解析命令行参数
	try {
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError& e) {
		return app.exit(e);
	}

	// 处理命令
	// Encrypt
	if (encryptCommand->parsed())
	{
		if (isDebug)
		{
			cout << "已启用Debug模式" << endl;
		}
		cout << "被加密文件(Input File): " << encryptInputFile << endl;
		cout << "输出文件(Output File): " << encryptOutputFile << endl;
		cout << "密钥文件(Key File): " << encryptKeyFile << endl;
		if (isForce)
		{
			cout << "强制覆盖输出文件" << endl;
		}
		cout << "准备开始加密" << endl;

		// 初始化OpenSSL
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();

		// 实例化Encrypt
		Encrypt* encrypt = new Encrypt;
		try
		{
			// 读取文件
			vector<unsigned char> content = readFile(encryptInputFile);
			// 加载RSA公钥
			auto public_key = loadPublicKey(encryptKeyFile);

			// 生成随机AES密钥(256bits=32Bytes)
			std::vector<unsigned char> aesKey(32);
			if (RAND_bytes(aesKey.data(), aesKey.size()) != 1) {
				handleOpenSSLErrors();
			}

			// 生成随机IV (AES块大小 = 16Bytes)
			std::vector<unsigned char> iv(AES_BLOCK_SIZE);
			if (RAND_bytes(iv.data(), iv.size()) != 1) {
				handleOpenSSLErrors();
			}

			// 使用RSA公钥加密AES密钥
			std::vector<unsigned char> encrypted_key = encrypt->encryptRSA(public_key.get(), aesKey);

			// 使用AES-256-CBC加密文件内容
			std::vector<unsigned char> ciphertext = encrypt->encryptAES(content, aesKey, iv);

			// 构建输出文件
			// 文件格式: [IV长度(1字节)][IV][加密AES密钥长度(2字节)][加密AES密钥][加密内容]
			std::vector<unsigned char> output;

			// 添加IV长度 (1字节)
			output.push_back(static_cast<unsigned char>(iv.size()));

			// 添加IV
			output.insert(output.end(), iv.begin(), iv.end());

			// 添加加密后AES密钥长度 (2字节)
			auto key_size_bytes = encrypt->intToBytes(encrypted_key.size(), 2);
			output.insert(output.end(), key_size_bytes.begin(), key_size_bytes.end());

			// 添加加密后的AES密钥
			output.insert(output.end(), encrypted_key.begin(), encrypted_key.end());

			// 添加加密后的文件内容
			output.insert(output.end(), ciphertext.begin(), ciphertext.end());

			// 写入输出文件
			writeFile(encryptOutputFile, output);

			std::cout << "File encrypted successfully: " << encryptOutputFile << std::endl;
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
			cout << "已启用Debug模式" << endl;
		}
		cout << "被解密文件(Input File): " << decryptInputFile << endl;
		cout << "输出文件(Output File): " << decryptOutputFile << endl;
		cout << "密钥文件(Key File): " << decryptKeyFile << endl;
		if (isForce)
		{
			cout << "强制覆盖输出文件" << endl;
		}
		cout << "准备开始解密" << endl;

		// TODO: Decrypt
	}

	return 0;
}
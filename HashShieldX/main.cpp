#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <openssl/applink.c>

#include "CLI11.hpp"

#include "encrypt.h"
#include "decrypt.h"
#include "gen_rsa_keys.h"
#include "get_hash.h"

#include "split_file.h"

using namespace std;

vector<string> split(const string& str, char delimiter) {
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

int main(int argc, char* argv[])
{
	CLI::App app{ "这是一个小工具，可以帮你加解密文件和计算2个文件的Hash值" };

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

	// Gen Keys Command
	auto* genKeyCommand = app.add_subcommand("genkey", "生成密钥");
	// Required
	// Public Key File
	string publicKeyFile;
	genKeyCommand->add_option("-p, --public-key", publicKeyFile, "公钥文件")->required();
	// Private Key File
	string privateKeyFile;
	genKeyCommand->add_option("-P, --private-key", privateKeyFile, "私钥文件")->required();
	// Optional
	int keyLength = 2048;
	genKeyCommand->add_option("-l, --key-length", keyLength, "密钥长度");

	// Hash Command
	auto* hashCommand = app.add_subcommand("hash", "对多个文件进行Hash");
	// Required
	string hashInput;
	hashCommand->add_option("-i, --input", hashInput, "输入文件");

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
	// 初始化OpenSSL
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
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

		// 实例化Encrypt
		Encrypt* encrypt = new Encrypt;
		try
		{
			encrypt->encrypt(encryptInputFile, encryptOutputFile, encryptKeyFile, isDebug);

			cout << "文件加密成功： " << encryptOutputFile << endl;
		}
		catch (const exception& e)
		{
			cerr << "Error: " << e.what() << endl;
			return 1;
		}
		delete encrypt;
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

		// 实例化对象
		Decrypt* decrypt = new Decrypt;
		try
		{
			decrypt->decrypt(decryptInputFile, decryptOutputFile, decryptKeyFile, isDebug);

			cout << "文件解密成功： " << decryptInputFile << endl;
		}
		catch (const exception& e)
		{
			cerr << "Error: " << e.what() << endl;
		}
		delete decrypt;
	}
	// Gen Key
	else if (genKeyCommand->parsed())
	{
		GenRSAKeys* genRsaKeys = new GenRSAKeys;
		genRsaKeys->generateRSAKeyPair(publicKeyFile, privateKeyFile, keyLength);
		delete genRsaKeys;
	}
	// Hash
	else if (hashCommand->parsed())
	{
		// Calc Hash
		vector<string> inputList = split(hashInput, ',');
		map<string, string> hashMap;  // map < filename, hash > 
		vector<string> hashList = vector<string>();
		int len = inputList.size();
		GetHash* getHash = new GetHash;
		for (int i = 0; i < len; i++)
		{
			string fileName = inputList[i];
			string hash = getHash->calculateFileHash(fileName, GetHash::HashType::SHA1);
			hashList.push_back(hash);
			hashMap.insert(pair<string, string>(fileName, hash));
			if (isDebug)
			{
				cout << "File: " << fileName << "  hash: " << hash << endl;
			}
		}
		//// Find Same Hash
		//vector<vector<string>> sameHash;
		//for (int i = 0; i < len - 1; i++)
		//{
		//	if (hashList[i] == hashList[i + 1])
		//	{
		//		vector<string> subList = vector<string>();
		//		subList.push_back(hashMap[hashList[i]]);
		//		subList.push_back(hashMap[hashList[i + 1]]);
		//		cout << subList[0] << subList[1] << endl;
		//		sameHash.push_back(subList);
		//	}
		//}

		//// Output Same Hash
		//for (vector<string> sameHashFiles : sameHash)
		//{
		//	cout << "文件内容相同的文件：" << sameHashFiles[0] << "\t" << sameHashFiles[1] << endl;
		//}
		if (hashList[0] == hashList[1])
		{
			cout << "两个文件的hash一样" << endl;
		}
		delete getHash;
	}

	return 0;
}
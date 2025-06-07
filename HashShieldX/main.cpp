#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "CLI11.hpp"

#include "encrypt.h"
#include "decrypt.h"
#include "gen_rsa_keys.h"

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

	// Gen Keys Command
	auto* genKeyCommand = app.add_subcommand("genkey", "������Կ");
	// Required
	// Public Key File
	string publicKeyFile;
	genKeyCommand->add_option("-p, --public-key", publicKeyFile, "��Կ�ļ�")->required();
	// Private Key File
	string privateKeyFile;
	genKeyCommand->add_option("-P, --private-key", privateKeyFile, "˽Կ�ļ�")->required();
	// Optional
	int keyLength = 2048;
	genKeyCommand->add_option("-l, --key-length", keyLength, "��Կ����");

	// Hash Command
	auto* hashCommand = app.add_subcommand("hash", "�Զ���ļ�����Hash");
	// Required
	string hashInput;
	hashCommand->add_option("-i, --input", hashInput, "�����ļ�");

	vector<string> inputList;

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
	// ��ʼ��OpenSSL
	OpenSSL_add_all_algorithms();
	ERR_load_crypto_strings();
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

		// ʵ����Encrypt
		Encrypt* encrypt = new Encrypt;
		try
		{
			encrypt->encrypt(encryptInputFile, encryptOutputFile, encryptKeyFile);

			cout << "�ļ����ܳɹ��� " << encryptOutputFile << endl;
		}
		catch (const exception& e)
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

		// ʵ��������
		Decrypt* decrypt = new Decrypt;
		try
		{
			decrypt->decrypt(decryptInputFile, decryptOutputFile, decryptKeyFile, isDebug);

			cout << "�ļ����ܳɹ��� " << decryptInputFile << endl;
		}
		catch (const exception& e)
		{
			cerr << "Error: " << e.what() << endl;
		}
	}
	// Gen Key
	else if (genKeyCommand->parsed())
	{
		GenRSAKeys* genRsaKeys = new GenRSAKeys;
		genRsaKeys->generateRSAKeyPair(publicKeyFile, privateKeyFile, keyLength);
	}

	return 0;
}

vector<string> split(const string& str, char delimiter) {
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
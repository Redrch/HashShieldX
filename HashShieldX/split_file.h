#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <memory>
#include <functional>
using namespace std;

class SplitFile
{
public:
    SplitFile();
    ~SplitFile();

    // ���ļ��ָ��ָ����С�Ŀ�
    bool splitFile(
        const string& sourceFilePath,
        const string& outputDir,
        size_t chunkSizeBytes,
        bool showProgress = false);

    // �ϲ��ѷָ���ļ�
    bool mergeFiles(
        const vector<string>& chunkFiles,
        const string& outputFilePath,
        bool showProgress = false);

    // �Զ��ϲ��ӻ����������������зֿ��ļ�
    bool mergeChunks(
        const string& baseChunkPath,
        const string& outputFilePath,
        bool showProgress = false);

    // ���ý��Ȼص����� (0-100 �İٷֱ�ֵ)
    void setProgressCallback(function<void(int)> callback);

    // ��ȡ�ϴβ����Ĵ�����Ϣ
    string getLastError() const;

    // ��ȡ�ļ��ֿ������Ĺ���ֵ
    size_t estimateChunkCount(const string& filePath, size_t chunkSizeBytes);

    // ���ɷֿ��ļ���
    static string generateChunkFileName(
        const string& baseFileName,
        size_t chunkIndex,
        size_t totalChunks);

private:
    string lastError;
    function<void(int)> progressCallback;

    // ���½��Ȳ����ûص�����
    void updateProgress(size_t current, size_t total, bool showProgress);

    // ��ȡ�ļ���С
    size_t getFileSize(const string& filePath);

    // ��֤Ŀ¼�Ƿ���ڣ��������򴴽�
    bool validateOrCreateDirectory(const string& dirPath);
};
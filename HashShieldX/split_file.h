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

    // 将文件分割成指定大小的块
    bool splitFile(
        const string& sourceFilePath,
        const string& outputDir,
        size_t chunkSizeBytes,
        bool showProgress = false);

    // 合并已分割的文件
    bool mergeFiles(
        const vector<string>& chunkFiles,
        const string& outputFilePath,
        bool showProgress = false);

    // 自动合并从基本名称派生的所有分块文件
    bool mergeChunks(
        const string& baseChunkPath,
        const string& outputFilePath,
        bool showProgress = false);

    // 设置进度回调函数 (0-100 的百分比值)
    void setProgressCallback(function<void(int)> callback);

    // 获取上次操作的错误信息
    string getLastError() const;

    // 获取文件分块数量的估计值
    size_t estimateChunkCount(const string& filePath, size_t chunkSizeBytes);

    // 生成分块文件名
    static string generateChunkFileName(
        const string& baseFileName,
        size_t chunkIndex,
        size_t totalChunks);

private:
    string lastError;
    function<void(int)> progressCallback;

    // 更新进度并调用回调函数
    void updateProgress(size_t current, size_t total, bool showProgress);

    // 获取文件大小
    size_t getFileSize(const string& filePath);

    // 验证目录是否存在，不存在则创建
    bool validateOrCreateDirectory(const string& dirPath);
};
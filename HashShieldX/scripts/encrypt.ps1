# 接收从右键菜单传入的文件路径参数
param (
    [Parameter(Mandatory=$true)]
    [string]$InputFile
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic

# 获取输入文件的详细信息
$inputFileInfo = New-Object System.IO.FileInfo($InputFile)
$inputFileName = $inputFileInfo.Name
$inputDirectory = $inputFileInfo.DirectoryName
$inputFileNameWithoutExt = [System.IO.Path]::GetFileNameWithoutExtension($inputFileName)

# 默认输出文件名（原文件名基础上改后缀为.hsxe）
$defaultOutputFileName = "$inputFileNameWithoutExt.hsxe"

# 1. 创建文件选择对话框
$openFileDialog = New-Object System.Windows.Forms.OpenFileDialog
$openFileDialog.Title = "请选择公钥文件"
$openFileDialog.Filter = "公钥文件 (*.pem)|*.pem"
$openFileDialog.InitialDirectory = $inputDirectory
$openFileDialog.Multiselect = $false

# 显示文件选择对话框并获取结果
$result1 = $openFileDialog.ShowDialog()

if ($result1 -eq [System.Windows.Forms.DialogResult]::OK) {
    $selectedFile = $openFileDialog.FileName
    
    # 2. 创建文本输入对话框，默认值为原始文件名
    $inputBoxResult = [Microsoft.VisualBasic.Interaction]::InputBox(
        "请输入输出文件名:", 
        "输出文件名", 
        $defaultOutputFileName
    )
    
    # 检查用户是否输入了文本
    if ($inputBoxResult -ne "") {
        # 加密文件
        ../HashShieldX encrypt -i $InputFile -o $inputBoxResult -k $selectedFile
    }
}
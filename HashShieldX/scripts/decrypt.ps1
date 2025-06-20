# ���մ��Ҽ��˵�������ļ�·������
param (
    [Parameter(Mandatory=$true)]
    [string]$InputFile
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName Microsoft.VisualBasic

# ��ȡ�����ļ�����ϸ��Ϣ
$inputFileInfo = New-Object System.IO.FileInfo($InputFile)
$inputFileName = $inputFileInfo.Name
$inputDirectory = $inputFileInfo.DirectoryName
$inputFileNameWithoutExt = [System.IO.Path]::GetFileNameWithoutExtension($inputFileName)

# Ĭ������ļ���
$defaultOutputFileName = $inputFileNameWithoutExt

# 1. �����ļ�ѡ��Ի���
$openFileDialog = New-Object System.Windows.Forms.OpenFileDialog
$openFileDialog.Title = "��ѡ��˽Կ�ļ�"
$openFileDialog.Filter = "˽Կ�ļ� (*.key)|*.key"
$openFileDialog.InitialDirectory = $inputDirectory
$openFileDialog.Multiselect = $false

# ��ʾ�ļ�ѡ��Ի��򲢻�ȡ���
$result1 = $openFileDialog.ShowDialog()

if ($result1 -eq [System.Windows.Forms.DialogResult]::OK) {
    $selectedFile = $openFileDialog.FileName
    
    # 2. �����ı�����Ի���Ĭ��ֵΪԭʼ�ļ���
    $inputBoxResult = [Microsoft.VisualBasic.Interaction]::InputBox(
        "����������ļ���:", 
        "����ļ���", 
        $defaultOutputFileName
    )
    
    # ����û��Ƿ��������ı�
    if ($inputBoxResult -ne "") {
        # �����ļ�
        ../HashShieldX decrypt -i $InputFile -o $inputBoxResult -k $selectedFile
    }
}
# HashShieldX

HashShieldX是一个可以帮你加解密文件（夹)和计算文件（夹）Hash的一个小工具

## 构建项目

请先执行`git clone https://github.com/Redrch/HashShieldX.git`命令将HashShieldX项目克隆到本地，

然后使用Visual Studio 2022（推荐使用2022，其他版本应该也可以）打开HashShieldX.sln文件，然后就可以直接构建

**注意事项**

- Visual Studio 2022 一定要安装使用"C++的桌面开发"组件，否则无法构建项目
- 构建完成HashShieldX后Visual Studio默认会运行HashShieldX，但是并不会有任何效果，关掉即可，如何使用详见使用说明

## 使用说明

### 命令行模式

打开cmd后cd到HashShieldX所在的目录，或者将HashShieldX添加环境变量中

**命令格式：** `HashShieldX subcommand args`

**参数说明**

**子命令**

`encrypt` 加密文件

**参数**

**必填**

`-i, --input` 想要加密的文件

`-o, --output` 输出文件

`-k, --key` 密钥文件

**可选**

`-f. --force` 是否强制覆盖输出文件（默认：false）



`decrypt` 解密文件

**参数**

**必填**

`-i, --input` 想要解密的文件

`-o, --output` 输出文件

`-k, --key` 密钥文件



`genkey` 生成密钥

**参数**

**必填**

`-p, --public-key` 公钥文件

`-P, --private-key` 私钥文件

**可选**

`-l, --key-length` 密钥长度（默认：2048）

**还有Hash的部分，还没做**

### 右键菜单

没做完，先不写


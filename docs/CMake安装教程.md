# CMake安装教程

本教程针对完全不知道如何安装CMake的用户，无论你使用的是Linux、MacOS还是Windows，你安装CMake的过程都可参考本教程。

CMake 是一个**跨平台的自动化构建工具**，它不直接编译代码，而是根据项目中的 `CMakeLists.txt` 配置文件，**生成对应平台的原生构建文件**（比如 Linux 下的 Makefile、Windows 下的 VS 工程、macOS 下的 Xcode 工程等），然后再调用编译器完成编译、链接、安装、打包等全过程。



### Windows安装CMake

Windows安装CMake最简单的方式就是从[CMake官方下载](https://cmake.org/download/)应用程序的安装包并根据安装引导进行安装。

根据系统的要求选择对应的安装包，例如你的系统是Windows x64，那么你就选择`cmake-4.2.3-windows-x86_64.msi`这个安装包，版本请参考[CMake安装包下载页面](https://cmake.org/download/)的最新版本。

或者，你也可以打开Windows上的Power Shell窗口并运行以下命令，通过Windows系统自带的包管理器安装CMake应用程序并自动配置`PATH`环境变量：

~~~powershell
winget install Kitware.CMake
~~~

**Windows从源代码安装CMake**：

如果你在Windows平台上想通过编译源代码的方式安装CMake，你需要确保你的计算机上有编译C/C++源代码的环境。

~~~powershell
gcc --version
~~~

如果没有任何输出或找不到命令，说明你需要安装相关的编译环境。你可以从[MSYS2官网下载MSYS2的](https://www.msys2.org)安装程序并完成安装。

下一步，开 **MSYS2 MinGW 64-bit** 终端，执行以下命令来安装GCC、Git和make等基础工具：

~~~bash
pacman -S --needed git base-devel mingw-w64-x86_64-gcc
~~~

GCC、Git和make等基础工具安装完成以后，你需要从[CMake程序下载页面](https://cmake.org/download/)获取WIndows平台CMake程序的源代码。

完成源代码的下载后，你需要打开 **MSYS2 MinGW 64-bit** 终端，进入你保存源代码压缩包文件的路径并解压文件：

~~~bash
cd /c/dev/cmake-source # 进入保存源代码压缩包文件的路径
~~~

使用ZIP工具解压文件。

完成上述步骤后，进行配置、编译、安装这三个步骤。

MinGW环境提供了一个类似Unix的`bootstrap`脚本，可以像在Linux上一样进行配置和编译：

~~~bash
./bootstrap --prefix=/c/opt/cmake  # 配置编译环境，可以指定安装路径，例如 --prefix=/opt/cmake
make -j4  # 编译，-j4 用4个核心加速编译
make install
~~~



### Linux用户安装CMake

Ubuntu/Debian系统：

~~~bash
sudo apt-get update && apt-get install -y cmake
~~~

CentOS/RHEL/Fedora：

~~~bash
# 依次运行一下命令
sudo yum update
sudo yum install cmake
sudo yum groupinstall "Development Tools"
~~~

如果你使用的是上述Linux发行版之外的的系统，安装CMake的区别在于包管理器和包的名称的不同。

**Linux/Unix从源代码安装CMake**：

首先你需要确认你的计算机上有C/C++编译源代码的工具，如果没有，你需要在编译CMake源代码之前安装它们：

~~~bash
sudo apt-get update && apt-get install build-essential wget libssl-dev -y # Ubuntu/Debain
sudo yum groupinstall "Development Tools" -y && yum install wget openssl-devel -y # CentOS/RHEL/Fedora
~~~

接下来，你需要从[CMake的下载页](https://cmake.org/download/)面选择对应的源代码包。选择 `Unix/Linux Source (has \n line feeds)`所列举的安装包， 鼠标右键点击安装包的名称并拷贝链接。假设这里选择的是`cmake-4.3.0-rc3.tar.gz`：

~~~bash
wget https://github.com/Kitware/CMake/releases/download/v4.3.0-rc3/cmake-4.3.0-rc3.tar.gz
~~~

下载完成后你会在当前目录下看到CMake源代码压缩文件`cmake-4.3.0-rc3.tar.gz`，我们需要解压`.tar.gz`文件：

~~~bash
mkdir CmakeSource  # 创建一个文件夹用于保存解压后的源代码文件
tar -xzvf cmake-4.3.0-rc3.tar.gz -C CmakeSource # 解压文件到CmakeSource路径下
~~~

进入文件夹找到刚才解压的源代码文件，找到源代码文件中的`configure`脚本，这是一个编译前配置脚本，会检查环境并准备编译，你需要运行它：

~~~bash
./configure
~~~

然后通过系统自带的`make`工具进行编译：

~~~bash
sudo make -j4 # 开始编译，-j4 表示用4个核心同时编译以加快速度，可根据你的CPU核心数调整
~~~

完成编译后进行安装并确认CMake程序是否安装成功：

~~~bash
sudo make install # 安装编译好的CMake程序
cmake --version   # 如果能显示版本就证明安装成功
~~~





### MacOS用户安装CMake

Mac系统上推荐使用Homebrew作为包管理工具，你可以通过Homebrew官网的安装脚本来安装Homebrew：

~~~bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
~~~

我们使用Homebrew来安装CMake应用程序也非常简单：

~~~bash
brew install cmake
~~~

**MacOS从源代码安装CMake**：

如果有简单的安装方法你不用，非要选择从源代码安装来折磨自己，那么有以下在MacOS上从源代码安装CMake的步骤可以参考。

同样的，你得确保你的计算机上有编译CMake源代码的工具，你可以运行以下命令来检查：

~~~bash
gcc --version
~~~

如果终端中没有输出GCC的版本，那么大致可以确认你的计算机上没有安装相关的编译环境，你需要运行以下命令：

~~~bash
xcode-select --install  # 这个工具包包含了编译CMake所需的clang编译器、make等工具
~~~

接下来依然是获取CMake源代码。由于MacOS是构建在Unix之上的操作系统，所以选择`Unix/Linux Source (has \n line feeds)`所列举的源代码文件。

源代码下载完成后，解压缩文件：

~~~bash
mkdir CmakeSource  # 创建一个文件夹用于保存解压后的源代码文件
tar -xzvf cmake-4.3.0-rc3.tar.gz -C CmakeSource # 解压文件到CmakeSource路径下
~~~

进入文件夹找到刚才解压的源代码文件，找到源代码文件中的`configure`脚本。这是一个编译前配置脚本，会检查环境并准备编译，你需要运行它：

~~~bash
./configure
~~~

然后通过系统自带的`make`工具进行编译：

~~~bash
sudo make -j4 # 开始编译，-j4 表示用4个核心同时编译以加快速度，可根据你的CPU核心数调整
~~~

完成编译后进行安装并确认CMake程序是否安装成功：

~~~bash
sudo make install # 安装编译好的CMake程序
cmake --version   # 如果能显示版本就证明安装成功
~~~


# 📚LibrarySystem项目介绍

LibrarySystem是一个基于SQLite作为数据库的，在本地终端运行的图书管理系统。LibrarySystem能够实现通过判断管理员账号和普通用户账号来登录不同的操作界面。图书管理员能对图书信息进行增删改查、能查所有的图书信息、能查看所有用户的借阅信息、能修改普通用户的密码。普通用户能查看数据库中的图书信息，能够借阅图书、归还图书、续借图书。如果用户忘记了登录账号的密码可以通过提前设置好的安全口令找回密码。

✅ 适合作为《C语言程序设计》课程设计借鉴。

本项目提供C语言程序设计课程设计：图书管理系统设计的参考报告作为你作业的借鉴。



# 项目构建说明

LibrarySystem包含了SQLite的`sqlite3.h`、`sqlite3.c`文件，用户无需为了编译项目额外下载SQLite。做到拿来即用，用户无需为完成课程作业耗费大量脑细胞。



## 使用CMake构建项目

首先，你可以使用CMake构建项目，这要求你的计算机上已经安装了CMake应用程序。构建过程：

~~~CMake
cmake .
~~~

~~~bash
make
~~~



## 使用IDE编译

我知道你们在课程上所使用的IDE年纪都比你们还大，如果你用的是VC 6.0或者其他我没有听过的，UI界面走复古风的软件，那么你应该自己导入项目，然后进行编译。如过你所使用的IDE能抽象到连你都玩不明白，那么别指望我了，因为我大概也不会😂。所以，为了方便，你选择使用CMkae比较简单。



## 交叉编译

交叉编译用于你手上只有Macbook或者其他运行Linux系统的电脑，你在提交作业的时候你到老师要求你带一个能够在WIndows上运行的EXE程序，那么你应该使用到交叉编译。

在MacOS系统上你应该使用Homebrew作为你的包管理器，并运行一下命令下载安装CMake和MinGW-64：

~~~bash
brew install cmake
brew install mingw-w64v
~~~

然后，在项目根目录上创建一个build的文件夹（这个文件夹的名字根据个人的兴趣爱好随便起）：

~~~bash
mkdir build
~~~

然后执行一下命令：

~~~bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../Windows-x86.cmake
~~~

之后你就会在这个文件夹里看见一个名为`LibrarySystem.exe`的可执行文件。

对于Linux系统用户，同样需要下载MinGW-64和CMake：

~~~bash
sudo apt-get update
sudo apt install -y g++-mingw-w64-x86-64 gcc-mingw-w64-x86-64
~~~

之后的步骤与MacOS中介绍的步骤一样。

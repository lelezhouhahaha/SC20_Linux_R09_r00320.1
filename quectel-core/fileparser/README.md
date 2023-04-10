fileparser
=========
# **ini文本解析库** #

*注: 本文假设你已经有linux开发环境*

**本项目采用 GPL 授权协议，欢迎大家在这个基础上进行改进，并与大家分享，为开源事业贡献一点点力量。**<br>
**源码下载地址**<br>
**https://gitee.com/fulinux/fileparser.git**<br>

下面将简单的解析下项目：

## **一、项目的目录结构** ##
> 根目录<br>
> |-- src<br>
> |-- util<br>
> |-- doc<br>
> |-- build-aux<br>
>  `- m4<br>

**1、src目录**<br>
src目录用于存放项目的包及C源码文件。

## **二、编译流程** ##
**1) 下载源代码**<br>
> $ git clone https://gitee.com/fulinux/fileparser.git

**2) 进入fileparser目录中**<br>
> $ cd fileparser

**3) 配置选项, 检测环境、依赖关系等，然后编译**<br>
> $ ./autogen.sh<br>
> $ ./configure<br>
> $ make<br>

## **三、测试库套件使用** ##
**1) 进入util目录**<br>
> cd util<br>
> ./iniexample<br>
> [pizza]=UNDEF<br>
> [pizza:ham]=[yes]<br>
> [pizza:mushrooms]=[TRUE]<br>
> [pizza:capres]=[0]<br>
> [pizza:cheese]=[Non]<br>
> [wine]=UNDEF<br>
> [wine:grape]=[Cabernet Sauvignon]<br>
> [wine:year]=[1989]<br>
> [wine:country]=[Spain]<br>
> [wine:alcohol]=[12.5]<br>
> Pizza:<br>
> Ham:       [1]<br>
> Mushrooms: [1]<br>
> Capres:    [0]<br>
> Cheese:    [0]<br>
> Wine:<br>
> Grape:     [Cabernet Sauvignon]<br>
> Year:      [1989]<br>
> Country:   [Spain]<br>
> Alcohol:   [12.5]<br>

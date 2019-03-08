# Open Type Parser  
## 项目简介  
>根据[微软文档](https://docs.microsoft.com/en-us/typography/opentype/spec/)，将OpenType文件解析成由直线和贝塞尔曲线构成的图形数据文件，并根据解析得到的图形数据文件用OpenGL描绘出文字的轮廓。

## 演示
* 解析字体文件  
![image](https://github.com/lghfoo/DarkEcho/blob/master/demo/parse.gif)
* 根据图形数据描绘轮廓
	* 普通unicode字符（符号、数字和字母）  
	![image](https://github.com/lghfoo/DarkEcho/blob/master/demo/deparse_unicode_symbol.gif)
	* 普通unicode字符（中文）  
	![image](https://github.com/lghfoo/DarkEcho/blob/master/demo/deparse_unicode_ch.gif)
	* [iconfont](https://www.iconfont.cn/)  
	![image](https://github.com/lghfoo/DarkEcho/blob/master/demo/deparse_iconfont.gif)

## 输入
基本支持Win10的Fonts文件夹下的单一字体文件以及iconfont字体文件
## 解析

## 输出

#!/bin/sh 

## 
## convert file from GB2312 to UTF-8
## 

path="$1"
unset opt
if [ "$2" = "force" ]; then
	opt="-c -s"
fi

if [ -z "$path" ]; then
	echo "nUsage: $0 <file or dir>n"
elif [ ! -e "$path" ] ; then
	echo "nERROR: destination: $path does not exist.n"
fi

if [ -f "$path" ] ; then
	echo "Converting $path (utf-8 --> gbk) ... "
	if file "$path"|grep -q UTF-8 >/dev/null ; then
		echo "Already converted"
	else
		iconv -f utf-8 $opt -t gbk "$path" > /tmp/$$.tmp
		if [ $? -eq 0 ] ; then
			echo "Success"
			mv -f /tmp/$$.tmp "$path"
		else
			echo "Failed"
		fi
	fi
elif [ -d "$path" ] ; then
#	echo $path
#	path=`echo "$path/"|sed 's//////'`
#	find "$path" -path "$path.*" -prune -o -type f -print|while read i
#	排除Debug目录和Release目录，只转换.h文件和.cpp文件
	find "$path" \( -path "$path/Debug" -o -path "$path/Release" \) -prune \
	-o \( -type f -a \( -name "*.h" -o -name "*.cpp" \) \) -print | \
	while read i
	do
		echo $i
		dir=`dirname $i`
		file=`basename $i`
		if [ -d $file ]; then
			continue
		fi
		echo "Converting $dir/$file (utf-8 --> gbk) ..."
		if file "$i"|grep -q ISO-8859 >/dev/null; then # 已经是GBK编码
			echo "Already converted"
			continue
		fi
		iconv -f utf-8 -t gbk $opt "$i" > /tmp/$$.tmp 2>/dev/null
		if [ $? -eq 0 ] ; then
			echo "Success"
			mv -f /tmp/$$.tmp "$i" #如果执行成功则将临时目录里已经转换好的文件覆盖掉原来的文件
		else
			echo "Failed"
		fi
	done
fi

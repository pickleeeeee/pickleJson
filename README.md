# pickleJson

Learning from leptjson

C实现简易的json解析器，手写递归下降解析器。

> TODO 第三章解析字符串的性能优化

![img.png](picture/img.png)

> /uxxxx解析

Unicode编码的范围0x0000-0x10FFFF

表示：

用/uxxxx(0-ffff)或者/uxxxx/uxxxx(10000-10ffff)来表示一个Unicode编码的码点

码点：

码点就是规定每个字符对应一个整数码点0-10ffff

存储码点的方式：

UTF-8、UTF-16 和 UTF-32，本实现仅支持UTF-8

表示 --》 码点 --》 UTF-8
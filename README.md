# ESP32S3 驱动 ILI9341

# readme缺失
* 中文支持
* WS2812驱动

# 功能
* wifi nat功能
* 当一块表看时间
* 显示电脑性能数据
* 作为web服务器提供简单的界面
* 作为http文件服务器存储、共享文件
* 查看文本文档
* 显示经处理后的视频/图片
* 游玩俄罗斯方块
* 驱动WS2812灯带

# 环境
* ESP32S3
* ILI9341 spi接口
* ESP-IDF Extension for VS Code
* VS code

# 目录结构
```
.
├── build
├── CMakeLists.txt
├── dependencies.lock
├── main
│   ├── app
│   │   ├── app.hpp
│   │   ├── clock
│   │   │   ├── clock.cpp
│   │   │   └── clock.hpp
│   │   ├── desktop
│   │   │   ├── desktop.cpp
│   │   │   └── desktop.hpp
│   │   ├── explorer
│   │   │   ├── explorer.cpp
│   │   │   └── explorer.hpp
│   │   ├── input
│   │   │   ├── input.cpp
│   │   │   └── input.hpp
│   │   ├── picture
│   │   │   ├── picture.cpp
│   │   │   ├── picture.hpp
│   │   │   └── yuv.hpp
│   │   ├── server
│   │   │   ├── buildinHtml
│   │   │   │   ├── file.hpp
│   │   │   │   └── file.html
│   │   │   ├── server.cpp
│   │   │   ├── server.hpp
│   │   │   ├── serverKernal.cpp
│   │   │   ├── serverKernal.hpp
│   │   │   ├── tempture.cpp
│   │   │   └── tempture.hpp
│   │   ├── setting
│   │   │   ├── setting.cpp
│   │   │   ├── setting.hpp
│   │   │   ├── systemInfo.cpp
│   │   │   ├── systemInfo.hpp
│   │   │   ├── timeSetting.cpp
│   │   │   ├── timeSetting.hpp
│   │   │   ├── touchTest.cpp
│   │   │   ├── touchTest.hpp
│   │   │   ├── wifiSetting.cpp
│   │   │   └── wifiSetting.hpp
│   │   ├── tetris
│   │   │   ├── block.cpp
│   │   │   ├── block.hpp
│   │   │   ├── mapBase.hpp
│   │   │   ├── map.cpp
│   │   │   ├── map.hpp
│   │   │   ├── tetris.cpp
│   │   │   └── tetris.hpp
│   │   ├── textEditor
│   │   │   ├── textEditor.cpp
│   │   │   └── textEditor.hpp
│   │   └── tracker
│   │       ├── tracker.cpp
│   │       └── tracker.hpp
│   ├── CMakeLists.txt
│   ├── gpio.hpp
│   ├── idf_component.yml
│   ├── LCD
│   │   ├── clickable.cpp
│   │   ├── clickable.hpp
│   │   ├── color.cpp
│   │   ├── color.hpp
│   │   ├── drawable.hpp
│   │   ├── element.hpp
│   │   ├── finger.hpp
│   │   ├── font.hpp
│   │   ├── frame.hpp
│   │   ├── FT6X36.cpp
│   │   ├── FT6X36.hpp
│   │   ├── iic.cpp
│   │   ├── iic.hpp
│   │   ├── ILI9341.cpp
│   │   ├── ILI9341.hpp
│   │   ├── ILI9341.inl
│   │   ├── layar.hpp
│   │   ├── line.hpp
│   │   ├── pixel.hpp
│   │   ├── rectangle.hpp
│   │   └── text.hpp
│   ├── main.cpp
│   ├── mutex.hpp
│   ├── nonCopyAble.hpp
│   ├── spi.cpp
│   ├── spi.hpp
│   ├── storge
│   │   ├── fat.cpp
│   │   ├── fat.hpp
│   │   ├── mem.cpp
│   │   ├── mem.hpp
│   │   ├── sd.cpp
│   │   ├── sd.hpp
│   │   └── vfs.hpp
│   ├── stringCompare.cpp
│   ├── stringCompare.hpp
│   ├── vector.hpp
│   └── wifi
│       ├── http.cpp
│       ├── http.hpp
│       ├── mdns.cpp
│       ├── mdns.hpp
│       ├── nvs.hpp
│       ├── socketStream.cpp
│       ├── socketStream.hpp
│       ├── socketStreamWindow.cpp
│       ├── socketStreamWindow.hpp
│       ├── wifi.cpp
│       └── wifi.hpp
├── managed_components
│   └── espressif__mdns
├── partitions.csv
├── README.md
├── reserces
│   ├── mem
│   ├── sd
│   ├── server
│   │   ├── index.css
│   │   ├── index.html
│   │   ├── server
│   │   │   ├── server.html
│   │   │   ├── server.js
│   │   │   └── temperature.js
│   │   └── wifi
│   │       ├── wifi.html
│   │       └── wifi.js
│   └── show
│       ├── A.pic
│       └── B.pic
├── sdkconfig
├── x86FontArranger
│   ├── build
│   │   ├── fontArrangeer
│   │   └── out.font
│   ├── gbk.cpp
│   ├── gbk.hpp
│   ├── main.cpp
│   └── utf8.hpp
├── x86pictureTransformer
│   ├── build
│   │   └── pictureTransformer
│   ├── color.hpp
│   ├── files -> /home/lo/Mem
│   └── main.cpp
└── x86Tracker
    ├── build
    │   └── tracker
    ├── main.cpp
    ├── nonCopyAble.hpp
    ├── socketStream.cpp
    └── socketStream.hpp
```

# x86FontArranger
转换字模数组到特定格式以供esp32使用。font文件定义如下：
* unicode字符区：使用两字节编码一个字符。要求**单调递增**。
* 字模区：32字节编码一个字符。顺序与unicode字符区相同。
字模从左到右，从上到下按字节存储。每字节编码八个位，每行末尾不足八位的补足至一字节。

eg.
```
0bxxxxxxxx 0bxxxxxx00
0bxxxxxxxx 0bxxxxxx00
0bxxxxxxxx 0bxxxxxx00
```
存储着14x3大小的字模

# x86pictureTransformer
将转换多媒体到特定格式以供esp32使用。该工具使用ffmpeg提取视频帧，再通过sfml提供的多媒体库读取每一帧的内容。

依赖于：
* ffmpeg
* sfml-dev

提取帧率为10fps，均转为png于files子文件夹中，最终组合到files/out.pic中。

scale控制缩放系数，可节约存储空间
yuv420Enabled控制yuv420压缩，可节约存储空间。输入‘+’以启用。

## 已知问题
* 上一次转换完的png若未手动删除将被打包至下一次的pic中

# x86Tracker
获取电脑的cpu、gpu信息并使用socket传输给esp32。

依赖于：
* sfml-dev

通过读取`/sys/class/drm/card1`获取gpu信息，`/proc/stat`获取cpu信息。不同电脑具体文件路径可能不同，需自行修改代码。

windows平台暂未开发。

数据的格式化在PC端，因此此修改此程序即可获得不同的输出结果。

# managed_components & dependencies.lock
从idf 5.0版本起mDNS已从迁出至独立的仓库，

运行`idf.py add-dependency espressif/mdns`，在项目中添加mDNS组件。

[mDNS 服务 idf v5.4.2](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.4.2/esp32s3/api-reference/protocols/mdns.html)

# .vscode
code配置文件。
请通过idf插件修改此文件夹，以避免潜在的问题

# reserces & 文件系统
按照目录结构由[esp32s3.local/file](esp32s3.local/file)上传即可

## /root
由[fat.hpp](main/storge/fat.hpp)与[fat.cpp](main/spi.cpp)挂在在内置flash上。对应[分区表](partitions.csv)中的fat分区。

## /root/mem
由[mem.hpp](main/storge/mem.hpp)与[mem.cpp](main/storge/mem.cpp)挂载在psram内。有MemFileMaxSize = 6 * 1024 * 1024 限制总容量。

## /root/sd
由[sd.hpp](main/storge/sd.hpp)与[sd.cpp](main/storge/sd.cpp)挂在在外置sd卡上。走spi协议，其spi总线与屏幕共用

## /root 与 /
由于vfs组件限制挂在`/`，因此上述挂载点均有前缀`/root`，该特性定义于[vfs.hpp](main/storge/vfs.hpp)中。

serverKernal和explorer均对此进行了转换，给用户暴露统一的`/`作为文件系统根。

serverKernal为了方便管理会将http传入的路径从`/`转到`/server/`下，该位置定义于[serverKernal.cpp](main/app/server/serverKernal.cpp)内的`ServerPath`。但对`/file/`下的文件将转到`/`下为用户提供全能的接口。

同时serverKernal会忽略`/file` `/File` `/FILE`并将其定位为内置的管理界面，会将`/path/`转换到`/path/index.html`以供方便的进行访问。

server内的转换均由[serverKernal.cpp](main/app/server/serverKernal.cpp)提供。在`httpGet`函数中实现。

# main
main下存放着核心代码，包含基础驱动与app程序

* CMakeLists.txt  
    编译main的cmake配置文件。包含所有main下的cpp文件，引入需要的idf库

* main.cpp  
    主程序，负责初始化硬件并管理着app的生命周期。

* spi、stringCompare、vector、mutex、nonCopyAble、gpio  
    常用的驱动/工具性文件，图省事扔到这里来的

* idf_component.yml  
    idf生成，注册mdns组件

* wifi/  
    网络相关驱动程序。驱动wifi、nfs、mdns，提供封装后的socketStream和socketStreamWindow，提供http库。

* storge/  
    存储相关驱动程序。挂载flash、mem、sd卡到[vfs.hpp](main/storge/vfs.hpp)中指定的位置。

* LCD/  
    屏幕相关驱动程序。驱动ILI9341屏幕和FT6X36触摸，并提供一系列图形相关类。

* app/  
    应用程序。每一个文件夹都是一个程序，均为App的子类，根据外界(main)传入的数据进行渲染与事件处理。

* * desktop 桌面，提供app的入口
* * clock 表，显示时间
* * explorer 文件浏览器，可观察文件系统结构，可启动textEditor和picture查看文本文件和经x86pictureTransformer处理后的视频文件。
* * server 控制http服务器的状态
* * tracker 接受网络上的文本数据并显示到屏幕中，与x86Tracker组合使用
* * setting 配置网络，同步时间，查看内部资源占用
* * input 工具app，提供用户输入界面

### text*Editor*
名字叫作editor但是只能查看？初期设想能进行编辑与保存，接口也已预留。但是被FILE\*、fwrite和fseek一起摆了一手。日后使用fstream重置一遍或许就好了。

# app解析
每个app均是`App`的子类，外界提供`lcd`作为渲染的目标位置以及`touch`作为输入事件的数据来源。保证在运行前调用`init`，在结束前调用`deinit`，在屏幕刷新前调用`draw`，在触模发生后调用`touchUpdate`。并提供一个`back`作为用户按下回退按钮后调用的函数。同时在app运行结束后可调用`changeAppCallback`函数结束自己的生命周期，也可在中途调用`newAppCallback`函数创建一个新的app且保留自己。app还可通过`running`和`deleteAble`告知外界自己的运行状态并精细控制自己的生命周期，提供`drawMutex`和`touchMutex`锁定`draw`函数和`touchUpdate`函数的调用。

该设计分离了底层的驱动程序和cpu调度机制，使app专注于自己的逻辑之中。

## app生命周期
每个app的析构均由外界(main)处理，当app调用`changeAppCallback`后main会调用`deinit`然后等待`deleteAble`为`true`，之后对app发生析构。

app由new构造，传递给`changeAppCallback`或`newAppCallback`，之后app所有权移交给main，由main完成调度。部分app需要额外配置可在移交给main之间进行设置。

main会在初始化硬件后构造一个desktop作为第一个app。

main中存储一个app的调用栈，记录各app的调用情况，并根据`changeAppCallback`和`newAppCallback`维护app栈。changeApp会替换掉栈顶的app，newApp会添加一个元素到栈。如果切换到`nullptr`视为退出一个app，new nullptr为非法行为。只有栈顶的app会收到`draw`和`touchUpdate`调用。

main会对app的`draw`和`touchUpdate`进行适时调用，可通过对`drawMutex`和`touchMuex`上锁阻止这一行为。在`touchUpdate`中对`drawMutex`上锁是良构的。但在`draw`中对`touchMutex`可能导致锁死，该锁死现象可能发生于changeApp时。

在app内部开发者仅需对关心的事件进行响应，维护好app内的成员变量即可享受app带来的抽象。

# lcd 解析
ILI9341是LCD的驱动程序，可选择使用`Color666`或`Color565`进行显示与输出。在`display`调用后通过dma在spi总线上刷新帧缓存`frame`。

渲染部分由`element`、`drawable`和各种预定义的形状构成。构造好形状后即可通过`lcd.draw(shape)`在`frame`上绘制对应图形。

`layar`提供对多个`element`的打包功能。对`layar`的`draw`和`finger`会传递为对子`element`的`draw`和`finger`。`layar`还提供对整体的偏移、隐藏功能。

`drawable`提供渲染的能力，`clickable`提供点击的能力，`element`两者都提供。它们作为各种形状的基类为lcd提供统一的接口。

# wifi解析
`wifi`提供wifi连接功能，给上层提供ip协议的支持。

`socketStream`封装socket提供传输通道，给上层提供tcp协议的支持。

`http`基于socketStream解析与生成http协议包，给上层提供http协议的支持。

## wifi与server
serverKernal使用`socketStreamWindow`管理socket，通过`http`库接受解析发送http数据包给客户端。内置多个api提供管理功能。

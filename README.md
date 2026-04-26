# ESP32-S3 智能触摸终端

基于 ESP32-S3 的嵌入式智能设备，配备 ILI9341 触摸屏、音频播放、Web 服务器等功能。

## ✨ 功能特性

### 🖥️ 图形界面
- **ILI9341 屏幕** - 320×240 分辨率，SPI 接口，DMA 加速渲染
- **FT6X36 触摸** - 电容式多点触控
- **图形框架** - Element/Drawable/Clickable 组件化设计，支持图层(Layar)系统

### 📱 内置应用
| 应用 | 描述 |
|------|------|
| **Desktop** | 桌面启动器，应用入口 |
| **Audio** | 音乐播放器，支持 MP3/AAC 解码、播放列表、随机播放 |
| **Clock** | 时钟显示 |
| **Tetris** | 俄罗斯方块游戏 |
| **TextEditor** | 文本查看器 |
| **Picture** | 图片查看器，从文件管理器进入 |
| **Setting** | 系统设置（WiFi、时间、触摸测试、系统信息） |
| **Strip** | LED 灯带控制 |
| **Tracker** | 追踪器应用 |

### 🌐 网络服务
- **WiFi** - Station/AP 模式，自动连接，NAT 支持
- **mDNS** - 设备发现 (`esp32s3.local`)
- **Web 服务器**
  - 文件上传/管理 (`/file`)
  - 音乐远程控制 (`/audio/audio.html`)
  - WiFi 配置 (`/wifi/wifi.html`)
  - 服务器管理 (`/server/server.html`)

### 💾 存储系统
| 挂载点 | 位置 | 说明 |
|--------|------|------|
| `/root` | 内置 Flash | FAT 分区，约 14MB |
| `/root/mem` | PSRAM | 内存文件系统，上限 6MB |
| `/root/sd` | SD 卡 | 外置存储，SPI 协议 |

### 🎵 音频系统
- MP3/AAC 硬件解码
- 播放列表管理
- 随机播放、上下曲切换
- 音量控制

## 🛠️ 硬件要求

- **MCU**: ESP32-S3 (推荐 ESP32-S3-WROOM-1)
- **屏幕**: ILI9341 (SPI 接口)
- **触摸**: FT6X36 (I2C 接口)
- **存储**: SD 卡 (可选)
- **音频**: I2S DAC (可选)

### 引脚配置
| 功能 | GPIO |
|------|------|
| SPI MOSI | 45 |
| SPI MISO | 13 |
| SPI CLK | 14 |
| LCD DC | 21 |
| LCD RST | 47 |
| LCD CS | 48 |
| I2C SDA | 11 |
| I2C SCL | 12 |
| Touch INT | 10 |
| Touch RST | 9 |
| SD CS | 3 |
| Boot/Button | 0 |

## 🏗️ 项目结构

```
├── main/                    # 核心代码
│   ├── main.cpp            # 主程序入口
│   ├── LCD/                # 屏幕驱动与图形框架
│   │   ├── ILI9341.hpp     # LCD 驱动
│   │   ├── FT6X36.hpp      # 触摸驱动
│   │   ├── element.hpp     # UI 组件基类
│   │   └── ...             # 形状、字体、图层等
│   ├── wifi/               # 网络模块
│   │   ├── wifi.hpp        # WiFi 驱动
│   │   ├── http.hpp        # HTTP 服务器
│   │   ├── mdns.hpp        # mDNS 发现
│   │   └── socketStream.hpp # Socket 封装
│   ├── storge/             # 存储模块
│   │   ├── fat.hpp         # Flash 挂载
│   │   ├── mem.hpp         # PSRAM 文件系统
│   │   └── sd.hpp          # SD 卡挂载
│   ├── audio/              # 音频模块
│   │   ├── decoder.hpp     # 解码器基类
│   │   ├── mp3.hpp         # MP3 解码
│   │   ├── aac.hpp         # AAC 解码
│   │   └── iis.hpp         # I2S 输出
│   ├── app/                # 应用程序
│   │   ├── app.hpp         # App 基类
│   │   ├── desktop/        # 桌面
│   │   ├── audio/          # 音乐播放器
│   │   ├── explorer/       # 文件管理器
│   │   ├── server/         # Web 服务器
│   │   └── ...             # 其他应用
│   └── strip/               # LED 灯带控制
├── reserces/               # 资源文件 (部署到 /root/system/)
│   ├── system/             # 系统资源 (字体等)
│   ├── server/             # Web 服务器资源
│   └── show/               # 示例资源
├── x86Tracker/             # x86 调试工具
├── x86pictureTransformer/  # 图片转换工具
├── x86FontArranger/        # 字体处理工具
├── partitions.csv          # 分区表
└── CMakeLists.txt          # 构建配置
```

## 🚀 快速开始

### 环境准备
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) v5.x
- [ESP-IDF Extension for VS Code](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) (推荐)

### 编译与烧录

```bash
# 配置项目
idf.py menuconfig

# 编译
idf.py build

# 烧录
idf.py -p PORT flash

# 查看日志
idf.py -p PORT monitor
```

### 资源部署
1. 设备连接 WiFi 后访问 `http://esp32s3.local/file`
2. 按目录结构上传 `reserces/` 中的文件
3. 中文字体需放置到 `/system/chinese.font`

## 📐 架构设计

### App 生命周期
```
┌─────────┐    init()    ┌─────────┐
│  创建   │ ──────────▶ │  运行   │
└─────────┘              └────┬────┘
                              │
    ┌─────────────────────────┼─────────────────────────┐
    │                         │                         │
    ▼                         ▼                         ▼
changeApp()              newApp()                   back()
(替换栈顶)               (压入新App)               (返回上一级)
    │                         │                         │
    ▼                         ▼                         ▼
deinit()                  继续运行                  自定义返回逻辑
    │                         │                         │
    ▼                         │                         ▼
等待 deleteAble               │                      可能调用changeApp()
    │                         │                         │
    ▼                         │                         ▼
 delete                       │                      或其他返回方式
                              │
                         focusIn()
                         (恢复显示)
```

### 图形框架
```
Drawable (可绘制)     Clickable (可点击)
       └──────────┬──────────┘
                  │
              Element (元素基类)
                  │
    ┌─────┬───────┼───────┬─────┐
    │     │       │       │     │
  Pixel  Line Rectangle Text Layar (图层容器)
```

### 文件系统虚拟化
```
Web 请求路径          实际路径
─────────────────────────────────
/              →    /server/
/file/xxx      →    /root/xxx
/path/         →    /path/index.html
```

## 🔧 配置

### 分区表
| 名称 | 类型 | 大小 | 说明 |
|------|------|------|------|
| nvs | data | 16KB | 非易失存储 |
| otadata | data | 8KB | OTA 状态 |
| phy_init | data | 4KB | RF 校准 |
| ota_0 | app | 2MB | 应用程序 |
| fat | data | ~14MB | 文件系统 |

### 关键配置 (sdkconfig)
- `CONFIG_ESP32S3_DEFAULT_CPU_FREQ` - CPU 频率
- `CONFIG_SPIRAM_ALLOW_BSS_SEG_EXTERNAL_MEMORY` - PSRAM 启用
- `CONFIG_FATFS_LFN_HEAP` - 长文件名支持

## 📝 开发指南

### 创建新应用
```cpp
#include "app/app.hpp"

class AppMyApp : public App {
public:
    AppMyApp(LCD& lcd, FT6X36& touch, Callback_t changeApp, Callback_t newApp)
        : App(lcd, touch, changeApp, newApp) {}
    
    void init() override {
        App::init();
        // 初始化逻辑
    }
    
    void draw() override {
        // 渲染逻辑
        lcd.draw(LCD::Text{{10, 10}, "Hello World"});
    }
    
    void touchUpdate() override {
        // 触摸事件处理
    }
    
    void back() override {
        // 返回上一级逻辑
        // 可以是页面切换、菜单返回等
        changeAppCallback(nullptr); // 示例：返回上一应用
    }
    
    void deinit() override {
        // 清理资源
        App::deinit();
    }
};
```

### 使用图形组件
```cpp
// 绘制矩形
lcd.draw(LCD::Rectangle{{10, 10}, {100, 50}, Color::Red});

// 绘制文本
lcd.draw(LCD::Text{{10, 70}, "Hello", fontsDefault});

// 使用图层（推荐使用LayarClassicSize确定容量）
LCD::Layar<LCD::LayarClassicSize> layar{4};
// 通过索引直接赋值元素（无add函数）
layar.contents[0] = &title;
layar.contents[1] = &nowDate;
layar.contents[2] = &nowTime;
layar.contents[3] = &ntpUpdate;
lcd.draw(layar);
```

## 🙏 致谢

- [ESP-IDF](https://github.com/espressif/esp-idf) - Espressif IoT Development Framework

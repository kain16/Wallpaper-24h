# Wallpaper 24h

一个基于时间的Windows桌面壁纸自动切换工具，支持24小时不同时段设置不同壁纸，并提供平滑过渡效果。

## ✨ 功能特点

- 🕒 **按小时自动切换**：根据当前时间自动切换对应壁纸
- 🌈 **平滑过渡效果**：壁纸切换时支持淡入淡出效果
- 📁 **多格式支持**：支持PNG、JPG、BMP格式图片
- 🖥️ **系统托盘**：最小化到系统托盘，随时可访问设置
- 🔄 **配置刷新**：支持手动刷新配置文件
- 💤 **休眠恢复**：系统从休眠恢复后自动刷新壁纸
- 📝 **日志记录**：记录壁纸切换历史

## 📁 项目结构

```
Wallpaper-24h/
├── main.c              # 主入口文件
├── wallpaper.c/h       # 核心壁纸引擎
├── config.c/h          # 配置文件管理
├── scheduler.c/h       # 调度器功能
├── transition.c/h      # 过渡效果
├── tray.c              # 系统托盘
├── config.txt          # 配置文件
├── Wallpaper 24h.sln   # Visual Studio解决方案
└── LICENSE.txt         # 许可证文件
```

## 🚀 安装方法

### 方法一：使用Visual Studio编译
1. 使用Visual Studio打开 `Wallpaper 24h.sln` 解决方案
2. 编译项目生成可执行文件
3. 运行生成的 `Wallpaper 24h.exe`

### 方法二：使用MSBuild命令行编译
1. 确保系统已安装Visual Studio Build Tools或Visual Studio
2. 打开命令提示符或PowerShell
3. 根据您的Visual Studio版本执行相应的命令：
   
   **Visual Studio 2022**：
   ```powershell
   # 使用完整路径执行MSBuild
   & "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "Wallpaper 24h.sln" /p:Configuration=Release
   ```
   
   **Visual Studio 2026**：
   ```powershell
   # 使用完整路径执行MSBuild
   & "C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "Wallpaper 24h.sln" /p:Configuration=Release
   ```
   
   **Community版本**：
   ```powershell
   # Visual Studio 2022 Community
   & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "Wallpaper 24h.sln" /p:Configuration=Release
   
   # Visual Studio 2026 Community
   & "C:\Program Files\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\MSBuild.exe" "Wallpaper 24h.sln" /p:Configuration=Release
   ```
4. 编译完成后，在 `x64\Release` 目录中找到 `Wallpaper 24h.exe`

**版本兼容性说明**：
- 本项目支持Visual Studio 2022及以上版本
- 确保安装了C++桌面开发工作负载
- 如果使用Visual Studio Build Tools，确保安装了MSBuild和C++构建工具
- 不同版本的Visual Studio可能会有不同的安装路径，请根据实际安装位置调整MSBuild路径

### 方法三：使用一键发行脚本
1. 运行项目根目录下的 `publish.bat` 脚本
2. 脚本会自动编译项目并准备发行文件
3. 在 `x64\Release` 目录中找到发行版文件

### 方法四：直接运行
1. 下载编译好的可执行文件
2. 解压到任意目录
3. 运行 `Wallpaper 24h.exe`

## ⚙️ 配置说明

首次运行时，程序会自动生成 `config.txt` 配置文件，您可以根据需要修改：

### 配置文件格式

```
[Hour]
0
[Image]
C:\\path\\to\\image0.jpg
[Description]
午夜壁纸
[Transition]
2000

[Hour]
8
[Image]
C:\\path\\to\\image8.jpg
[Description]
早晨壁纸
[Transition]
2000

[Hour]
12
[Image]
C:\\path\\to\\image12.jpg
[Description]
中午壁纸
[Transition]
2000

[Hour]
18
[Image]
C:\\path\\to\\image18.jpg
[Description]
傍晚壁纸
[Transition]
2000
```

- **Hour**：小时（0-23）
- **Image**：壁纸图片路径（注意使用双反斜杠）
- **Description**：壁纸描述
- **Transition**：过渡时间（毫秒）

### 配置示例

```
[Hour]
0
[Image]
C:\\Wallpapers\\night.jpg
[Description]
深夜静谧
[Transition]
3000

[Hour]
6
[Image]
C:\\Wallpapers\\dawn.jpg
[Description]
黎明曙光
[Transition]
3000

[Hour]
12
[Image]
C:\\Wallpapers\\noon.jpg
[Description]
正午阳光
[Transition]
3000

[Hour]
18
[Image]
C:\\Wallpapers\\sunset.jpg
[Description]
日落黄昏
[Transition]
3000
```

## 📖 使用说明

1. **启动程序**：运行 `Wallpaper 24h.exe`，程序会在系统托盘显示图标
2. **查看当前状态**：点击系统托盘图标
3. **刷新配置**：在托盘菜单中选择「刷新配置」
4. **修改配置**：直接编辑 `config.txt` 文件，然后刷新配置
5. **退出程序**：在托盘菜单中选择「退出」

## 🔧 系统要求

- Windows 7 或更高版本
- .NET Framework 4.0 或更高版本
- 支持的图片格式：PNG、JPG、BMP

## 📝 日志文件

程序会生成 `wallpaper_log.txt` 日志文件，记录壁纸切换历史和系统事件：

```
[2024-01-01 00:00:00] 系统启动，加载配置成功
[2024-01-01 08:00:00] 切换壁纸：C:\Wallpapers\morning.jpg
[2024-01-01 12:00:00] 切换壁纸：C:\Wallpapers\noon.jpg
[2024-01-01 18:00:00] 切换壁纸：C:\Wallpapers\evening.jpg
[2024-01-01 23:59:59] 系统休眠，暂停调度
[2024-01-02 07:00:00] 系统恢复，刷新壁纸
```

## 🎯 技术实现

- **核心技术**：Windows API、GDI+图像处理
- **调度机制**：多线程调度，定时检查时间变化
- **过渡效果**：GDI+实现的淡入淡出效果
- **系统集成**：系统托盘、电源事件监听

## 🔒 许可证

本项目采用 MIT 许可证，详见 [LICENSE.txt](LICENSE.txt) 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目！

## 📞 联系方式

如果您有任何问题或建议，欢迎联系项目维护者。

---

**享受您的24小时动态壁纸体验！** 🌟
<a name="0.15.1"></a>
## [0.15.1](https://github.com/rime/weasel/compare/0.15.0...0.15.1) (2023-06-021)

#### 安装须知

**⚠️安装小狼毫前请保存好文件资料，于安装后重启 Windows ，否则正在使用小狼毫的应用将会崩溃。**

#### 主要更新

* 支持手动调整抗锯齿选项
  * `style/antialias_mode: "default" | "cleartype" | "grayscale" | "aliased" | "force_dword"`
* Composing 状态下候选框不再随光标移动
* 病毒误报问题改进
* 不再默认安装旧版 IME 输入框架

#### Bug 修复

* 修复若干 UI 布局问题
* 修复部分程序中无法键入文本的问题 (#937)
* 修复 WeaselServer 自启动问题

#### 已知问题

* 部分应用仍存在输入法无法输入文字的问题

<a name="0.15.0"></a>
## [0.15.0](https://github.com/rime/weasel/compare/0.15.0...0.14.3) (2023-06-06)


#### 安装须知

**⚠️安装小狼毫前请保存好文件资料，于安装后重启 Windows ，否则正在使用小狼毫的应用将会崩溃。**
**⚠️此版本的小狼毫需要使用 Windows 8.1 或更高版本的操作系统。**

#### 主要更新

* 升级核心算法库至 [librime 1.8.5](https://github.com/rime/librime/blob/master/CHANGELOG.md#185-2023-02-05)
* DPI 根据显示器自动调整
* 支持候选窗口等圆角显示
  * `style/layout/corner_radius: int`
* 兼容鼠须管中高亮圆角参数`style/layout/hilited_corner_radius: int`
* 支持主题颜色中含有透明通道代码, 支持格式 0xaabbggrr，0xbbggrr, 0xabgr, 0xbgr
* 配色主题支持默认ABGR顺序，或ARGB、RGBA顺序
  * `preset_color_schemes/color_scheme/color_format: "argb" | "rgba" | ""`
* 支持编码/高亮候选/普通候选/输入窗口/候选边框的阴影颜色绘制
  * `style/layout/shadow_radius: int`
  * `style/layout/shadow_offset_x: int`
  * `style/layout/shadow_offset_y: int`
  * `preset_color_schemes/color_scheme/shadow_color: color`
  * `preset_color_schemes/color_scheme/nextpage_color: color`
  * `preset_color_schemes/color_scheme/prevpage_color: color`
  * `preset_color_schemes/color_scheme/candidate_back_color: color`
  * `preset_color_schemes/color_scheme/candidate_shadow_color: color`
  * `preset_color_schemes/color_scheme/candidate_border_color: color`
  * `preset_color_schemes/color_scheme/hilited_shadow_color: color`
  * `preset_color_schemes/color_scheme/hilited_candidate_shadow_color: color`
  * `preset_color_schemes/color_scheme/hilited_candidate_border_color: color`
  * `preset_color_schemes/color_scheme/hilited_mark_color: color`
* 支持自定义标签、注解字体及字号
  * `style/label_font_face: string`
  * `style/comment_font_face: string`
  * `style/label_font_point: int`
  * `style/comment_font_point: int`
  * `style/layout/align_type: "top" | "center" | "bottom"`
* 支持指定字符 Unicode 区间字体设定
* 支持字重，字形风格设定
  * `style/font_face: font_name[:start_code_point:end_code_point][:weight_set][:style_set][,font2...]`
    * example: `"Segoe UI Emoji:20:39:bold:italic, Segoe UI Emoji:1f51f:1f51f, Noto Color Emoji SVG:80, Arial:600:6ff, Segoe UI Emoji:80, LXGW Wenkai Narrow"`
* 支持自定义字体回退范围、顺序定义
* 彩色字体支持
  * Windows 10 周年版前：需要使用 COLR 格式彩色字体
  * Windows 11 ：可以使用 SVG 字体
* 新增竖直文字布局
  * `style/vertical_text: bool`
  * `style/vertical_text_left_to_right: bool`
  * `style/vertical_text_with_wrap: bool`
* 新增竖直布局vertical窗口上移时自动倒序排列
  * `style/vertical_auto_reverse: bool`
* 新增「天圆地方」布局：由 margin 与 hilite_padding 确定, 当margin <= hilite_padding时生效
* margin_x 或 margin_y 设置为负值时，隐藏输入窗口，不影响方案选单显示
* 新增 preedit_type: preview_all ，在输入时将候选项显示于 composition 中
  * `style/preedit_type: "composition" | "preview" | "preview_all"`
* 新增输入法高亮提示标记
  * `style/mark_text: string`
* 新增输入方案图标显示，可在语言栏中显示，文件格式为ico
  * `schema/icon: string`
  * `schema/ascii_icon: string`
* 新增选项，允许在光标位置获取失败时于窗口左上角绘制候选框（而不是桌面左上角）
  * `style/layout/enhanced_position: bool`
* 新增鼠标点击截图到剪贴板功能
* 新增选项，支持越长自动折行/换列显示
  * `style/layout/max_width: int`
  * `style/layout/max_height: int`
* 支持方案内设定配色
  * `style/color_scheme: string`
* 支持多行内容显示，\r, \n, \r\n均支持
* 支持方案内设定配色
* 绘制性能提升
* composition 模式下新增下划线显示
* 随二进制文件提供调试符号

#### Bug 修复

* 转义日文键盘中特殊按键
* 候选文字过长时崩溃
* 修复用户目录下无 `default.custom.yaml` 或 `weasel.custom.yaml` 时，设定窗口无法弹出的问题
* 方案中设定inline_preedit为true时，部署后编码末端出现异常符号
* 部分应用无法输入文字的问题
* 修复部署时无显示提示的问题
* 修复中文路径相关问题
* 修复右键菜单打开程序目录/用户目录时，资源管理器无响应的问题
* 修复部分内存访问问题
* 修复操作系统 / WinGet 无法识别小狼毫版本号的问题
* 修复 composition 模式下光标位置不正常的问题
* 修复 Word 中小狼毫工作不正常的问题
* 若干开发环境配置问题修复

#### 已知问题

* 部分应用仍存在输入法无法输入文字的问题
* WeaselServer 仍可能发生崩溃
* 部分防病毒软件可能误报病毒

<a name="0.14.3"></a>
## 0.14.3 (2019-06-22)


#### 主要更新

* 升级核心算法库 [librime 1.5.3](https://github.com/rime/librime/blob/master/CHANGELOG.md#153-2019-06-22)
  * 修复 `single_char_filter` 组件
  * 完善上游项目 `librime` 的全自动发布流程，免去手工上传构建结果的步骤



<a name="0.14.2"></a>
## 0.14.2 (2019-06-17)


#### 主要更新

* 升级核心算法库 [librime 1.5.2](https://github.com/rime/librime/blob/master/CHANGELOG.md#152-2019-06-17)
  * 修复用户词的权重，稳定造句质量、平衡翻译器优先级 [librime#287](https://github.com/rime/librime/issues/287)
  * 建议 0.14.1 版本用家升级



<a name="0.14.1"></a>
## 0.14.1 (2019-06-16)


#### 主要更新

* 升级核心算法库 [librime 1.5.1](https://github.com/rime/librime/blob/master/CHANGELOG.md#151-2019-06-16)
  * 修复未装配语言模型时缺省的造句算法 ([weasel#383](https://github.com/rime/weasel/issues/383))



<a name="0.14.0"></a>
## 0.14.0 (2019-06-11)


#### 主要更新

* 升级核心算法库 [librime 1.5.0](https://github.com/rime/librime/blob/master/CHANGELOG.md#150-2019-06-06)
  * 迁移到VS2017构建工具；建设安全可靠的全自动构建、发布流程
  * 通过更新第三方库，修复userdb文件夹大量占用磁盘空间的问题
  * 将Rime插件纳入自动化构建流程。本次发行包含两款插件：
    - [librime-lua](https://github.com/hchunhui/librime-lua)
    - [librime-octagram](https://github.com/lotem/librime-octagram)
* 高清重制真彩输入法状态图标


#### Features

* **ui:**  high-res status icons; display larger icons in WeaselPanel ([093fa806](https://github.com/rime/weasel/commit/093fa80678422f972e7a7285060553eeedb0e591))



<a name="0.13.0"></a>
## 0.13.0 (2019-01-28)


#### 主要更新

* 升级核心算法库 [librime 1.4.0](https://github.com/rime/librime/blob/master/CHANGELOG.md#140-2019-01-16)
  * 新增 [拼写纠错](https://github.com/rime/librime/pull/228) 选项
    当前仅限 QWERTY 键盘布局及使用 `script_translator` 的方案
  * 修复升级、部署数据时发生的若干错误
* 更换输入法状态图标，适配高分辨率屏幕


#### Features

* **tsf:**  register as GUID_TFCAT_TIPCAP_UIELEMENTENABLED ([ae876916](https://github.com/rime/weasel/commit/ae8769166ea50b319aa89460b60890d598c618c5))
* **ui:**  high-res icons (#324) ([ad3e2027](https://github.com/rime/weasel/commit/ad3e2027644f80c6a384b7730da20dd239e780af))

#### Bug Fixes

* **WeaselSetup.vcxproj:**  Debug build linker options ([eb885fe0](https://github.com/rime/weasel/commit/eb885fe06ffd720d3de1101be2410a94bd3747c0))
* **output/install.nsi:**  bundle new yaml files from rime/rime-prelude ([cba35e9b](https://github.com/rime/weasel/commit/cba35e9b2c34d095b9ca1eb44e923e004cf23ddc))
* **test:**  Debug build ([c771126c](https://github.com/rime/weasel/commit/c771126c74fa1c4f91d4bfd8fb5ab8c16dcb7c4c))
* **tsf:**  set current page to 0 as page count is always 1 ([5447f63b](https://github.com/rime/weasel/commit/5447f63bc7c9d0e31d7ba8ead1e1229938be276d))



<a name="0.12.0"></a>
## 0.12.0  (2018-11-12)

#### 主要更新

* 合并小狼毫与小狼毫（TSF）两种输入法
* 合并32位与64位系统下的安装程序
* 使用系统的关闭输入法功能（默认快捷键 Ctrl + Space）后，输入法图标将显示禁用状态
* 修复一些情况下的崩溃问题
* 升级核心算法库 [librime 1.3.2](https://github.com/rime/librime/blob/master/CHANGELOG.md#132-2018-11-12)
  * 允许多个翻译器共用同一个词典时的组词，实现固定单字顺序的形码组词([librime#184](https://github.com/rime/librime/issues/184))。
  * 新增 translator/always_show_comments 选项，允许始终显示候选词注解。

#### Bug Fixes

* **candidate:** fix COM pointer reference ([63d6d9a](https://github.com/rime/weasel/commit/63d6d9a))
* **ipc:** eliminate some trivial warnings ([dae945c](https://github.com/rime/weasel/commit/dae945c))
* fix constructor ([b25f968](https://github.com/rime/weasel/commit/b25f968))


#### Features

* **compartment:** show IME disabled on language bar ([#263](https://github.com/rime/weasel/issues/263)) ([4015d18](https://github.com/rime/weasel/commit/4015d18))
* **install:** combine IME and TSF ([#257](https://github.com/rime/weasel/issues/257)) ([91cbd2c](https://github.com/rime/weasel/commit/91cbd2c))
* **tsf:** get IME keyboard identifier by searching registry ([#272](https://github.com/rime/weasel/issues/272)) ([b60b5b1](https://github.com/rime/weasel/commit/b60b5b1))
* **WeaselSetup:** detect 64-bit on single 32-bit build ([#266](https://github.com/rime/weasel/issues/266)) ([fb3ae0f](https://github.com/rime/weasel/commit/fb3ae0f))



<a name="0.11.1"></a>
## 0.11.1 (2018-04-26)

#### 主要更新

* 修复了在 Excel 中奇怪的输入丢失问题（[#185](https://github.com/rime/weasel/issues/185)）
* 功能键不再会触发输入焦点（[#194](https://github.com/rime/weasel/issues/194)、[#195](https://github.com/rime/weasel/issues/195)、[#204](https://github.com/rime/weasel/issues/204)）
* 「获取更多输入方案」功能优化（[#180](https://github.com/rime/weasel/issues/180)）
* 修复了后台可能同时出现多个算法服务的问题（[#199](https://github.com/rime/weasel/issues/199)）
* 恢复语言栏右键菜单中「用户资料同步」一项

#### Bug Fixes

* **server:**  use kernel mutex to ensure single instance (#207) ([bd0c4720](https://github.com/rime/weasel/commit/bd0c4720669c61087dd930b968640c60a526ecb2))
* **tsf:**
  *  do not reset composition on document focus set ([124fc947](https://github.com/rime/weasel/commit/124fc9475c30963a9bbbf9a097b452b52e8ab658))
  *  use `ITfContext::GetSelection` to get cursor position ([5664481c](https://github.com/rime/weasel/commit/5664481cc9ddd28db35c3155f7ddf83a55b65275))
  *  recover sync option in TSF language bar menu ([7a0a8cc2](https://github.com/rime/weasel/commit/7a0a8cc2a3dd913ce34204d6e966b263af766f3b))

#### Features

* **build.bat:**  build installer ([e18117b7](https://github.com/rime/weasel/commit/e18117b7b42d5af0fbfa807e4c858c40206b4967))
* **installer:**  bundle curl, update rime-install.bat, fixes #180 ([2f3b283d](https://github.com/rime/weasel/commit/2f3b283d6ef4aa0580d186e626dadb9e1030dfd5))
* **rime-install.bat:**  built-in ZIP package installer ([739be9bc](https://github.com/rime/weasel/commit/739be9bc9ba08e294f51e1d7232407148ded716c))



<a name="0.11.0"></a>
## 0.11.0 (2018-04-07)

#### 主要更新

* 新增 [Rime 配置管理器](https://github.com/rime/plum)，通过「输入法设定／获取更多输入方案」调用
* 在输入法语言栏显示状态切换按钮（TSF 模式）
* 修复多个前端兼容性问题
* 新增配色主题「现代蓝」`metroblue`、「幽能」`psionics`
* 安装程序支持繁体中文介面
* 修复 0.10 版升级安装后，因用户文件夹中保留旧文件、配置不生效的问题
* 升级 0.9 版 `.kct` 格式的用户词典
  **注意**：仅此一个版本支持格式升级，请务必由 0.9 升级到 0.11，再安装后续版本

#### Features

* **WeaselDeployer:**  add Get Schemata button to run plum script (#174) ([c786bb5b](https://github.com/rime/weasel/commit/c786bb5ba2f1cc7e79b66f36d0190e61cd7233ae))
* **build.bat:**  customize PLATFORM_TOOLSET settings ([c7a9a4fb](https://github.com/rime/weasel/commit/c7a9a4fb530e0274450e4296cb0db2906d2f1fb4))
* **config:**
  *  enable customization of label format ([76b08bae](https://github.com/rime/weasel/commit/76b08bae810735c5f1c8626ec39a7afd463f0269))
  *  alias `style/layout/border_width` to `style/layout/border` ([013eefeb](https://github.com/rime/weasel/commit/013eefebaa4474e7814b6cfb6c905bcc12543a7f))
* **install.nsi:**
  *  add Traditional Chinese for installer ([d1a9696a](https://github.com/rime/weasel/commit/d1a9696a57dfc9e04c51899572e156fb1676f786))
  *  upgrade to Modern UI 2 and prompt reboot (#128) ([f59006f8](https://github.com/rime/weasel/commit/f59006f8d195ca848e45cd934f44b3318fb135c1))
* **ipc:**  specify user name for named pipe ([2dfa5e1a](https://github.com/rime/weasel/commit/2dfa5e1a63ee1c26ef983d25471682f87cc60b62))
* **preset_color_schemes:**
  *  add homepage featured color scheme `psionics` ([89a0eb8b](https://github.com/rime/weasel/commit/89a0eb8b861b9b3f2abc42df65821254010b24ff))
  *  add metroblue color scheme ([f43e2af6](https://github.com/rime/weasel/commit/f43e2af608bde38a6d345ba540f4c37ec024853a))
* **submodules:**  switch from rime/brise to rime/plum ([f3ff5aa9](https://github.com/rime/weasel/commit/f3ff5aa962a7b8cce2b74a5cb583a69cb8938e55))
* **tsf:**
  *  enable language bar button (#170) ([2b660397](https://github.com/rime/weasel/commit/2b660397950f348205e6a93bf44a46e4a72bcc81))
  *  accomplish candidate UI interfaces (#156) ([1f0ae793](https://github.com/rime/weasel/commit/1f0ae7936fd495ecf4ff3ef162c0e38297d2d582))
  *  fix candidate selecting in preview preedit mode ([206efd69](https://github.com/rime/weasel/commit/206efd692124339d0e256198360c1860c72cd807))
  *  support user defined preedit display type ([f76379b0](https://github.com/rime/weasel/commit/f76379b01abe9d3971d68e2e272067e0bb855cc9))
* **weasel.yaml:**  enable ascii_mode in console applications by default ([28cdd096](https://github.com/rime/weasel/commit/28cdd09692f77e471784bf85ff7a19bc48e113f4))

#### Bug Fixes

*   fix defects according to Coverity Scan ([526a91d2](https://github.com/rime/weasel/commit/526a91d2954492cc8e23c2c4c8def2a053af7c20))
*   inline_preedit && fullscreen causing dead lock when there's no candidates. ([deb0bb24](https://github.com/rime/weasel/commit/deb0bb24b3f3aeaf73aef344968b7f15b471443f))
* **RimeWithWeasel:**  fix wild pointer ([ae2e3c4a](https://github.com/rime/weasel/commit/ae2e3c4a256fb9a2f7851c54114822d1bfbf0316))
* **ServerImpl:**  do finalization before exit process ([b1bae01e](https://github.com/rime/weasel/commit/b1bae01eb25c5e24e074807b7b3cb8a6d8401276))
* **WeaselUI:**
  *  specify default label format in constructor ([4374d244](https://github.com/rime/weasel/commit/4374d2440b99726894799861fb3bd5b93e73dec5), closes [#147](https://github.com/rime/weasel/issues/147))
  *  limit to subscript range when processing candidates ([6b686c71](https://github.com/rime/weasel/commit/6b686c717bfab141469c3d48ec1c6acbeb79921e), closes [#121](https://github.com/rime/weasel/issues/121))
* **composition:**
  *  improve compositions and edit sessions (#146) ([fbdb6679](https://github.com/rime/weasel/commit/fbdb66791da3291b740edf3c337032674e4377e8))
  *  fix crashes in notebook with inline preedit ([5e257088](https://github.com/rime/weasel/commit/5e257088be823a2569609f0b3591af3a51d47a46))
  *  fix crashes in notebook with inline preedit ([892930ce](https://github.com/rime/weasel/commit/892930cebc4235a0a1ef58803fe88c32ccc8b4e9))
* **install.bat:**  run in elevate cmd; detach WeaselServer process ([2194d9fb](https://github.com/rime/weasel/commit/2194d9fbd7d0341fef94efdbe9268af8a6237438))
* **ipc:**
  *  add version check for security descriptor initialization ([b97ccffe](https://github.com/rime/weasel/commit/b97ccffe76a6abf3e353724ce0607d5dd97de6f2), closes [#157](https://github.com/rime/weasel/issues/157))
  *  grant access to IE protected mode ([16c163a4](https://github.com/rime/weasel/commit/16c163a41d0afc9824723009ba8b9b9ba37b1c72))
  *  try to reconnect when failed ([3c286b6a](https://github.com/rime/weasel/commit/3c286b6a942769abf13188d88f9ab5e4c125807b))
* **librime:**  make rime_api.h available in librime\build\include\ ([3793e22c](https://github.com/rime/weasel/commit/3793e22c47b34c61d305ca80567dfdafe08b2302))
* **server:**  postpone tray icon updating when focusing on explorer ([45cf1120](https://github.com/rime/weasel/commit/45cf112099fa6db335cda06b1aaa0ae9c7975efe))
* **tsf:**
  *  fix candidate behavior ([9e2f9f17](https://github.com/rime/weasel/commit/9e2f9f17c059bf129c2c8b2561471670ea200dd7))
  *  fix `ITfCandidateListUIElement` implemention ([9ce1fa87](https://github.com/rime/weasel/commit/9ce1fa87e6ef788e791e68193700e2ebdd950d20))
  *  use commmit text preview to show inline preview ([b1d1ec43](https://github.com/rime/weasel/commit/b1d1ec43e132998ea8764d8dac2098a2b3d9a3e8))



<a name="0.10.0"></a>
## 小狼毫 0.10.0 (2018-03-14)


#### 主要更新

* 兼容 Windows 8 ~ Windows 10
* 支持高分辨率显示屏
* 介面风格选项
  * 在内嵌编码行预览结果文字
  * 可指定候选序号的样式
* 升级核心算法库 [librime 1.3.0](https://github.com/rime/librime/blob/master/CHANGELOG.md#130-2018-03-09)
  * 支持 YAML 节点引用，方便模块化配置
  * 改进部署流程，在 `build` 子目录集中存放生成的数据文件
* 精简安装包预装的输入方案，更多方案可由 [东风破](https://github.com/rime/plum) 取得

#### Features

* **build.bat:**  customize PLATFORM_TOOLSET settings ([c7a9a4fb](https://github.com/rime/weasel/commit/c7a9a4fb530e0274450e4296cb0db2906d2f1fb4))
* **config:**
  *  enable customization of label format ([76b08bae](https://github.com/rime/weasel/commit/76b08bae810735c5f1c8626ec39a7afd463f0269))
  *  alias `style/layout/border_width` to `style/layout/border` ([013eefeb](https://github.com/rime/weasel/commit/013eefebaa4474e7814b6cfb6c905bcc12543a7f))
* **tsf:**
  *  fix candidate selecting in preview preedit mode ([206efd69](https://github.com/rime/weasel/commit/206efd692124339d0e256198360c1860c72cd807))
  *  support user defined preedit display type ([f76379b0](https://github.com/rime/weasel/commit/f76379b01abe9d3971d68e2e272067e0bb855cc9))

#### Bug Fixes

*   Support High DPI Display [#28](https://github.com/rime/weasel/issues/28)
* **WeaselUI:**  limit to subscript range when processing candidates ([6b686c71](https://github.com/rime/weasel/commit/6b686c717bfab141469c3d48ec1c6acbeb79921e), closes [#121](https://github.com/rime/weasel/issues/121))
* **install.bat:**  run in elevate cmd; detach WeaselServer process ([2194d9fb](https://github.com/rime/weasel/commit/2194d9fbd7d0341fef94efdbe9268af8a6237438))
* **librime:**  make rime_api.h available in librime\build\include\ ([3793e22c](https://github.com/rime/weasel/commit/3793e22c47b34c61d305ca80567dfdafe08b2302))
* **tsf:**
  *  Results of auto-selection cleared by subsequent manual selection [#107](https://github.com/rime/weasel/issues/107)
  *  use commmit text preview to show inline preview ([b1d1ec43](https://github.com/rime/weasel/commit/b1d1ec43e132998ea8764d8dac2098a2b3d9a3e8))



<a name="0.9.30"></a>
## 小狼毫 0.9.30 (2014-04-01)


#### Rime 算法库变更集

* 新增：中西文切换方式 `clear`，切换时清除未完成的输入
* 改进：长按 Shift（或 Control）键不触发中西文切换
* 改进：并击输入，若按回车键则上屏按键对应的字符
* 改进：支持对用户设定中的列表元素打补靪，例如 `switcher/@0/reset: 1`
* 改进：缺少词典源文件 `*.dict.yaml` 时利用固态词典 `*.table.bin` 完成部署
* 修复：自动组词的词典部署时未检查【八股文】的变更，导致索引失效、候选字缺失
* 修复：`comment_format` 会对候选注释重复使用多次的BUG

#### 【东风破】变更集

* 新增：快捷键 `Control+.` 切换中西文标点
* 更新：【八股文】【朙月拼音】【地球拼音】【五笔画】
* 改进：【朙月拼音·语句流】`/0` ~ `/10` 输入数字符号



<a name="0.9.29.1"></a>
## 小狼毫 0.9.29.1 (2013-12-22)


#### 【小狼毫】变更集

* 变更：不再支持 Windows XP SP2，因升级编译器以支持 C++11
* 修复：输入语言选为中文（台湾）在 Windows 8 系统上出现多余的输入法选项
* 修复：升级安装后，外观设定介面未及时显示出新增的配色方案
* 修复：配色方案 Google+ 的预览图

#### Rime 算法库变更集

* 更新：librime 升级到 1.1
* 新增：固定方案选单排列顺序的选项 `default.yaml`: `switcher/fix_schema_list_order: true`
* 修复：正确匹配嵌套的“‘弯引号’”
* 改进：码表输入法自动上屏及顶字上屏（[示例](https://gist.github.com/lotem/f879a020d56ef9b3b792)）<br/>
    若有 `speller/auto_select: true`，则选项 `speller/max_code_length:` 限定第N码无重码自动上屏
* 优化：为词组自动编码时，限制因多音字而产生的组合数目，避免穷举消耗过量资源

#### 【东风破】变更集

* 更新：【粤拼】汇入众多粤语词汇
* 优化：调整部分异体字的字频



<a name="0.9.28"></a>
## 小狼毫 0.9.28 <2013-12-01>


#### 【小狼毫】变更集

* 新增：一组配色方案，作者：P1461、Patricivs、skoj、五磅兔
* 修复：[Issue 528](https://code.google.com/p/rimeime/issues/detail?id=528) Windows 7 IE11 文字无法上屏
* 修复：[Issue 531](https://code.google.com/p/rimeime/issues/detail?id=531) Windows 8 卸载输入法后在输入法列表中有残留项
* 变更：注册输入法时同时启用 IME、TSF 模式

#### Rime 算法库变更集

* 更新：librime 升级到 1.0
* 改进：`affix_segmentor` 支持向匹配到的代码段添加标签 `extra_tags`
* 修复：`table_translator` 按字符集过滤候选字，修正对 CJK-D 汉字的判断

#### 【东风破】变更集

* 优化：【粤拼】兼容[教育学院拼音方案](http://zh.wikipedia.org/wiki/%E6%95%99%E8%82%B2%E5%AD%B8%E9%99%A2%E6%8B%BC%E9%9F%B3%E6%96%B9%E6%A1%88)
* 更新：`symbols.yaml` 由 Patricivs 重新整理符号表
* 更新：Emoji 提供更加丰富的绘文字（需要字体支持）
* 更新：【八股文】【朙月拼音】【地球拼音】【中古全拼】修正错别字、注音错误



<a name="0.9.27"></a>
## 小狼毫 0.9.27 (2013-11-06)


#### 【小狼毫】变更集

* 变更：动态链接 `rime.dll`，减小程序文件的体积
* 修复：尝试解决 Issue 487 避免服务进程以 SYSTEM 帐号执行
* 新增：开始菜单项「安装选项」，Vista 以降提示以管理员权限启动
* 优化：更换图标，解决 Windows 8 TSF 图标不清楚的问题

#### Rime 算法库变更集

* 优化：同步用户资料时自动备份自定义短语等 .txt 文件
* 修复：【地球拼音】反查拼音失效的问题
* 变更：编码提示不再添加括弧（，）及逗号，可自行设定样式

#### 输入方案设计支持

* 新增：`affix_segmentor` 分隔编码的前缀、后缀
* 改进：`translator` 支持匹配段落标签
* 改进：`simplifier` 支持多个实例，匹配段落标签
* 新增：`switches:` 输入方案选项支持多选一
* 新增：`reverse_lookup_filter` 为候选字标注指定种类的输入码

#### 【东风破】变更集

* 更新：【粤拼】补充大量单字的注音
* 更新：【朙月拼音】【地球拼音】导入 Unihan 读音资料
* 改进：【地球拼音】【注音】启用自定义短语
* 新增：【注音·台湾正体】
* 修复：【朙月拼音·简化字】通过快捷键 `Control+Shift+4` 简繁切换
* 改进：【仓颉五代】开启繁简转换时，提示简化字对应的传统汉字
* 变更：间隔号采用「·」`U+00B7`



<a name="0.9.26.1"></a>
## 小狼毫 0.9.26.1 (2013-10-09)

* 修复：从上一个版本升级【仓颉】输入方案不会自动更新的问题



<a name="0.9.26"></a>
## 小狼毫 0.9.26 (2013-10-08)

* 新增：【仓颉】开启自动造词<br/>
  连续上屏的5字（依设定）以内的组合，或以连打方式上屏的短语，
  按构词规则记忆为新词组；再次输入该词组的编码时，显示「☯」标记
* 变更：【五笔】开启自动造词；从码表中删除与一级简码重码的键名字
* 变更：【地球拼音】当以简拼输入时，为5字以内候选标注完整带调拼音
* 新增：【五笔画】输入方案（`stroke`），取代 `stroke_simp`
* 新增：支持在输入方案中设置介面样式（`style:`）<br/>
  如字体、字号、横排／直排等；配色方案除外
* 修复：多次按「.」键翻页后继续输入，不应视为网址而在编码中插入「.」
* 修复：开启候选字的字符集过滤，导致有时不出现连打候选词的 BUG
* 修复：`table_translator` 连打组词时产生的内存泄漏（0.9.25.2）
* 修复：为所有用户创建开始菜单项
* 更新：修订【八股文】词典、【朙月拼音】【地球拼音】【粤拼】【吴语】
* 更新：2013款 Rime 输入法图标



<a name="0.9.25.2"></a>
## 小狼毫 0.9.25.2 (2013-07-26)

* 改进：码表输入法连打，Shift+BackSpace 以字、词为单位回退
* 修复：演示模式下开启内嵌编码行、查无候选字时程序卡死



<a name="0.9.25.1"></a>
## 小狼毫 0.9.25.1 (2013-07-25)

* 新增：开始菜单项「检查新版本」，手动升级到最新测试版
* 新增：【地球拼音】5 字内候选标注完整带调拼音



<a name="0.9.25"></a>
## 小狼毫 0.9.25 (2013-07-24)

* 新增：演示模式（全屏的输入窗口）`style/fullscreen: true`
* 新增：【仓颉】按快趣取码规则生成常用词组
* 修复：【地球拼音】「-」键输入第一声失效的BUG
* 更新：拼音、粤拼等输入方案
* 更新：`symbols.yaml` 增加一批特殊字符



<a name="0.9.24"></a>
## 小狼毫 0.9.24 (2013-07-04)

* 新增：支持全角模式
* 更新：中古汉语【全拼】【三拼】输入方案；三拼亦采用全拼词典
* 修复：大陆与台湾异读的字「微」「档」「蜗」「垃圾」等
* 修复：繁简转换错词「么么哒」
* 新增：（输入方案设计用）可设定对特定类型的候选词不做繁简转换<br/>
  如不转换反查字使用选项 `simplifier/excluded_types: [ reverse_lookup ]`
* 新增：（输入方案设计用）干预多个 translator 之间的结果排序<br/>
  选项 `translator/initial_quality: 0`
* 修复：用户词典未能完整支持 `derive` 拼写运算产生的歧义切分



<a name="0.9.23"></a>
## 小狼毫 0.9.23 (2013-06-09)

* 改进：方案选单按选用输入方案的时间排列
* 新增：快捷键 Control+Shift+1 切换至下一个输入方案
* 新增：快捷键 Control+Shift+2~5 切换输入模式
* 新增：初次安装时由用户指定输入语言：中文（中国／台湾）
* 新增：可屏蔽符合 fuzz 拼写规则的单字候选，仅以其输入词组<br/>
  选项 `translator/strict_spelling: true`
* 改进：综合候选词的词频和词条质量比较不同 translator 的结果
* 修复：自定义短语不应参与组词
* 修复：八股文错词及「链」字无法以简化字组词的 BUG



<a name="0.9.22.1"></a>
## 小狼毫 0.9.22.1 (2013-04-24)

* 修复：禁止自定义短语参与造句
* 修复：GVim 里进入命令模式或在插入模式换行错使输入法重置为初始状态



<a name="0.9.22"></a>
## 小狼毫 0.9.22 (2013-04-23)

* 新增：配色方案【晒经石】／Solarized Rock
* 新增：Control+BackSpace 或 Shift+BackSpace 回退一个音节
* 新增：固态词典可引用多份码表文件以实现分类词库
* 新增：在输入方案中加载翻译器的多个具名实例
* 新增：以选项 `translator/user_dict:` 指定用户词典的名称
* 新增：支持从用户文件夹加载文本码表作为自定义短语词典<br/>
  【朙月拼音】系列自动加载名为 `custom_phrase.txt` 的码表
* 修复：繁简转换使无重码自动上屏失效的 BUG
* 修复：若非以 Caps Lock 键进入西文模式，<br/>
  按 Caps Lock 只切换大小写，不返回中文模式
* 变更：`r10n_translator` 更名为 `script_translator`，旧名称仍可使用
* 变更：用户词典快照改为文本格式
* 改进：【八股文】导入《萌典》词汇，并修正了不少错词
* 改进：【仓颉五代】打单字时，以拉丁字母和仓颉字母并列显示输入码
* 改进：使自动生成的 YAML 文档更合理地缩排、方便阅读
* 改进：码表中 `# no comments` 行之后不再识别注释，以支持 `#` 作文字内容
* 改进：检测到因断电造成用户词典损坏时，自动在后台线程恢复数据文件



<a name="0.9.20"></a>
## 小狼毫 0.9.20 (2013-02-01)

* 变更：Caps Lock 灯亮时默认输出大写字母 [Gist](https://gist.github.com/2981316)
  升级安装后若 Caps Lock 的表现不正确，请注销并重新登录
* 新增：无重码自动上屏 `speller/auto_select:`<br/>
  输入方案【仓颉·快打模式】
* 改进：允许以空格做输入码，或作为符号顶字上屏<br/>
  `speller/use_space:`, `punctuator/use_space:`
* 改进：【注音】输入方案以空格输入第一声（阴平）
* 新增：特殊符号表 `symbols.yaml` 用法见↙
* 改进：【朙月拼音·简化字】以 `/ts` 等形式输入特殊符号
* 改进：标点符号注明〔全角〕〔半角〕
* 优化：同步用户资料时更聪明地备份用户自定义的 YAML 文件
* 修复：避免创建、使用不完整的词典文件
* 修复：纠正用户词典中无法调频的受损词条
* 修复：用户词典管理／输出词典快照后定位文件出错
* 修复：TSF 内嵌输入码没有反选效果、候选窗位置频繁变化



<a name="0.9.19.1"></a>
## 小狼毫 0.9.19.1 (2013-01-16)

* 新增：Caps Lock 点亮时，切换到西文模式，输出小写字母<br/>
  选项 `ascii_composer/switch_key/Caps_Lock:`
* 修复：Control + 字母编辑键在临时西文模式下无效
* 修复：用户词典有可能因读取时 I/O 错误导致部份词序无法调整
* 改进：用户词典同步／合入快照的字频合并算法



<a name="0.9.18.6"></a>
## 小狼毫 0.9.18.6 (2013-01-09)

* 修复：从 0.9.16 及以下版本升级用户词典出错



<a name="0.9.18.5"></a>
## 小狼毫 0.9.18.5 (2013-01-07)

* 修复：含简化字的候选词不能以音节为单位移动光标
* 改进：同步用户资料时也备份用户修改的YAML文件



<a name="0.9.18"></a>
## 小狼毫 0.9.18 (2013-01-05)

* 新增：同步用户词典，详见 [Wiki » UserGuide](https://code.google.com/p/rimeime/wiki/UserGuide)
* 新增：上屏错误的词组后立即按回退键（BackSpace）撤销组词
* 改进：拼音输入法中，按左方向键以音节为单位移动光标
* 修复：【地球拼音】不能以 - 键输入第一声



<a name="0.9.17.1"></a>
## 小狼毫 0.9.17.1 (2012-12-25)

* 修复：设置为默认输入语言后再安装，IME 注册失败
* 修复：启用托盘图标的选项无效
* 新增：从开始菜单访问用户文件夹的快捷方式
* 修复：【小鹤双拼】拼音 an 显示错误



<a name="0.9.17"></a>
## 小狼毫 0.9.17 (2012-12-23)

* 新增：切换模式、输入方案时，短暂显示状态图标
* 新增：隐藏托盘图标，设定、部署、词典管理请用开始菜单。<br/>
  配置项 `style/display_tray_icon:`
* 修复BUG：TSF 前端在 MS Office 里不能正常上屏中文
* 删除：默认不启用 TSF 前端，如有需要可在「文本服务与输入语言」设置对话框添加。
* 新增：分别以 `` ` ' `` 标志编码反查的开始结束，例如 `` `wbb'yuepinyin ``
* 改进：形码与拼音混打的设定下，降低简拼候选的优先级，以降低对逐键提示的干扰
* 优化：控制用户词典文件大小，提高大容量（词条数>100,000）时的查询速度
* 删除：因有用家向用户词典导入巨量词条，故取消自动备份的功能，后续代之以用户词典同步
* 修复：【小鹤双拼】diao, tiao 等拼音回显错误
* 更新：【朙月拼音】【地球拼音】【粤拼】修正用户反馈的注音错误



<a name="0.9.16"></a>
## 小狼毫 0.9.16 (2012-10-20)

* 新增：TSF 输入法框架（测试阶段）及嵌入式编码行
* 新增：支持 IE 8 ~ 10 的「保护模式」
* 新增：识别 gVim 模式切换
* 新增：开关码表输入法连打功能的设定项 `translator/enable_sentence: `
* 修复：「语句流」模式直接回车上屏不能记忆用户词组的BUG
* 改进：部署时自动编译输入方案的自订依赖项，如自选的反查码
* 改进：更精细的排版，修正注释文字宽度、调整间距
* 改进：未曾翻页时按减号键，不上屏候选字及符号「-」以免误操作
* 变更：《注音》以逗号或句号（<> 键）上屏句子，书名号改用 [] 键
* 更新：《朙月拼音》《地球拼音》《粤拼》，修正多音字
* 更新：《上海吴语》《上海新派》，修正注音
* 新增：寒寒豆作《苏州吴语》输入方案，方案标识为 `soutzoe`
* 新增：配色方案【谷歌／Google】，skoj 作品



<a name="0.9.15"></a>
## 小狼毫 0.9.15 (2012-09-12)

* 新增：横排候选栏——欢迎 wishstudio 同学加入开发！
* 新增：绿色安装工具 WeaselSetup，注册输入语言、自订用户目录
* 新增：码表输入法启用用户词典、字频调整
* 优化：自动编译输入方案依赖项，如五笔·拼音的反查词典
* 修改：日志系统改用glog，输出到 `%TEMP%\rime.weasel.*`
* 修复：托盘图标在重新登录后不可见的BUG
* 更新：【明月拼音】【粤拼】【吴语】修正注音错误、缺字



<a name="0.9.14.2"></a>
## 小狼毫 0.9.14.2 (2012-07-13)

* 重新编译了 `opencc.dll` 安全软件不吭气了



<a name="0.9.14.1"></a>
## 小狼毫 0.9.14.1 (2012-07-07)

* 解决【中古全拼】不可用的问题



<a name="0.9.14"></a>
## 小狼毫 0.9.14 (2012-07-05)

* 介面采用新的 Rime logo，状态图示用较柔和的颜色
* 新特性：码表方案支持与反查码混合输入，无需切换或引导键
* 新特性：码表方案可在选单中使用字符集过滤开关
* 新方案：【五笔86】衍生的【五笔·拼音】混合输入
* 新方案：《广韵》音系的中古汉语全拼、三拼输入法
* 新方案：X-SAMPA 国际音标输入法
* 更新：【吴语】码表，审定一些字词的读音，统一字形
* 更新：【朙月拼音】码表，修正多音字
* 改进：当前设定的字体缺字时，使用系统后备字体显示文字
* 解决与MacType同时使用，Ext-B/C/D区文字排版不正确的问题



<a name="0.9.13"></a>
## 小狼毫 0.9.13 (2012-06-10)

* 编码提示用淡墨来写，亦可在配色方案中设定颜色
* 新增多键并击组件及输入方案【宫保拼音】
* 未经转换的输入如网址等不再显示为候选项
* `default.custom.yaml`: `menu/page_size:` 设定全局页候选数
* 新增选项：导入【八股文】词库时限制词语的长度、词频
* 【仓颉】支持连续输入多个字的编码（阶段成果，不会记忆词组）
* 【注音】改为语句输入风格，更接近台湾用户的习惯
* 较少用的【笔顺五码】、【速记打字法】不再随鼠须管发行
* 修复「用户词典管理」导入文本码表不生效的BUG；<br/>
  部署时检查并修复已存在于用户词典中的无效条目
* 检测到用户词典文件损坏时，重建词典并从备份中恢复资料
* 修改BUG：简拼 zhzh 因切分歧义使部分用户词失效



<a name="0.9.12"></a>
## 小狼毫 0.9.12 (2012-05-05)

* 用 Shift+Del 删除已记入用户词典的词条，详见 Issue 117
* 可选用Shift或Control为中西文切换键，详见 Issue 133
* 数字后的句号键识别为小数点、冒号键识别为时分秒分隔符
* 解决在QQ等应用程序中的定位问题
* 支持设置为系统默认输入法
* 支持多个Windows用户（新用户执行一次布署后方可使用）



<a name="0.9.11"></a>
## 小狼毫 0.9.11 (2012-04-14)

* 使用 `express_editor` 的输入方案中，数字、符号键直接上屏
* 优化「方案选单」快捷键操作，连续按键选中下一个输入方案
* 输入简拼、模糊音时提示正音，【粤拼】【吴语】中默认开启
* 拼音反查支持预设的多音节词、形码反查可开启编码补全
* 修复整句模式运用定长编码顶字功能导致崩溃的问题
* 修复码表输入法候选排序问题
* 修复【朙月拼音】lo、yo 等音节的候选错误
* 修复【地球拼音】声调显示不正确、部分字的注音缺失问题
* 【五笔86】反查引导键改为 z、反查词典换用简化字拼音
* 更新【粤拼】词典，调整常用粤字的排序、增补粤语常用词
* 新增输入方案【小鹤双拼】、【笔顺五码】



<a name="0.9.10"></a>
## 小狼毫 0.9.10 (2012-03-26)

* 记忆繁简转换、全／半角符号开关状态
* 支持定长编码顶字上屏
* 新增「用户词典管理」介面
* 延迟加载繁简转换、编码反查词典，降低资源占用
* 纯单字构词时不调频
* 新增输入方案【速成】，速成、仓颉词句连打
* 新增【智能ABC双拼】、【速记打字法】



<a name="0.9.9"></a>
## 小狼毫 0.9.9

* 新增「介面风格设定」，快速选择预设的六款配色方案
* 优化长句中字词的动态调频
* 新增【注音】与【地球拼音】输入方案
* 支持自订选词按键
* 修复编码反查失效的BUG
* 修改标点符号「间隔号」及「浪纹」



<a name="0.9.8"></a>
## 小狼毫 0.9.8

* 新增「输入方案选单」设定介面
* 优化包含简拼的音节切分
* 修复部分用户组词无效的BUG
* 新增预设输入方案「MSPY双拼」



<a name="0.9.7"></a>
## 小狼毫 0.9.7

* 逐键提示、反查提示码支持拼写运算（如显示仓颉字母等）
* 重构部署工具；以 `*.custom.yaml` 文件持久保存自定义设置
* 制作【粤拼】、【吴语】输入方案「预发行版」



<a name="0.9.6"></a>
## 小狼毫 0.9.6

* 关机时妥善保存数据，降低用户词库损坏机率；执行定期备份
* 新增基于【朙月拼音】的衍生方案：
  * 【语句流】，整句输入，空格分词，回车上屏
  * 【双拼】，兼容自然码双拼方案，演示拼写运算常用技巧
* 修复BUG：简拼「z h, c h, s h」的词候选先於单字简拼
* 修复BUG：「拼写运算」无法替换为空串
* 完善拼写运算的错误日志；清理调试日志



<a name="0.9.5"></a>
## 小狼毫 0.9.5

* Rime 独门绝活之「拼写运算」
* 升级【朙月拼音】，支持简拼、纠错；增设【简化字】方案
* 升级【仓颉五代】，以仓颉字母显示编码
* 重修配色方案【碧水／Aqua】、【青天／Azure】



<a name="0.9.4"></a>
## 小狼毫 0.9.4

* 增设编码反查功能，预设方案以「`」为反查的引导键
* 修复Windows XP中西文状态变更时的通知气球



<a name="0.9.3"></a>
## 小狼毫 0.9.3

* 新增预设输入方案【五笔86】、【台湾正体】拼音
* 以托盘图标表现输入法状态变更
* 新增输入法维护模式，更安全地进行部署作业
* 优化中西文切换、自动识别小数、百分数、网址、邮箱



<a name="0.9.2"></a>
## 小狼毫 0.9.2

* 增设半角标点符号
* 增设Shift键切换中／西文模式
* 繁简转换、左Shift切换中西文对当前输入即时生效
* 可自定义OpenCC异体字转换字典
* 提升码表查询效率，更新仓颉七万字码表
* 增设托盘图标，快速访问配置管理工具
* 改进安装程序



<a name="0.9"></a>
## 小狼毫 0.9

* 用C++重写核心算法（阶段成果）
* 将输入法介面从前端迁移到后台服务进程
* 兼容64位系统



## 小狼毫 0.1 ~ 0.3

* 以Python开发的实验版本
* 独创「拼写运算」技术
* 预装标调拼音、注音、粤拼、吴语等多种输入方案

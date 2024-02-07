--启动外部程序
function translator(input, seg)
	local path0=debug.getinfo(1,"S").source:sub(2)--获取Lua脚本的完整路径
	 pathCY=path0:sub(0,-20)--根目录
	 pathC=pathCY.."weasel\\"--程序目录
	 pathY=pathCY.."rime\\"--用户目录
	 pathL=pathY.."lua\\"--lua目录

   if (input == "/jsq") then  --打开计算器
    strProgram = '"C:\\Windows\\system32\\calc.exe"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/eme") then  --打开Emeditor
    strProgram = '"C:\\emed64_19.6\\EmEditor.exe"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/cmd") then  --打开CMD窗口
    strProgram = '"C:\\Windows\\system32\\cmd.exe"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/kz") then  --打开控制面板
    strProgram = '"C:\\Windows\\system32\\control.exe"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/bg") then  --打开excel
    strProgram = '"D:\\Program Files\\Microsoft Office\\Office16\\EXCEL.EXE"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/wz") then  --打开word
    strProgram = '"D:\\Program Files\\Microsoft Office\\Office16\\WINWORD.EXE"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/cx") then  --打开小狼毫程式文件
    strProgram = pathCY..'"weasel"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/yh") then  --打开小狼毫用户文件
    strProgram = pathCY..'"rime"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/tebr") then  --同步
    strProgram = pathC..'"WeaselDeployer.exe" /sync' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/bfse") then  --部署
    strProgram = pathC..'"WeaselDeployer.exe" /deploy' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/sdds") then  --设定
    strProgram = pathC..'"WeaselDeployer.exe"'-- /weaseldir' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/clqd") then  --重启服务
    strProgram = pathCY..'"重启服务.bat"' --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/lua") then  --LUA文件
    strProgram = pathY.."rime.lua" --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/xlh") then  --小狼毫主题
    strProgram = pathY.."weasel.yaml" --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/xkc") then  --打开星空两笔词组码表
    strProgram = pathY.."_xklbXY53_ci.dict.yaml" --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/xkd") then  --打开星空两笔单字码表
    strProgram = pathY.."_xklbXY53.dict.yaml" --此项路径自行修改
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/hd") then  --打开汉典网
    strProgram = '"https://www.zdic.net/"' 
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/bd") then  --打开百度
    strProgram = '"https://www.baidu.com/"' 
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
    if (input == "/wp") then  --打开百度网盘
    strProgram = '"https://pan.baidu.com/disk/home?errno=0&errmsg=Auth%20Login%20Sucess&&bduss=&ssnerror=0&traceid=#/all?path=%2F&vmode=list"' 
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/tb") then  --打开淘宝
    strProgram = '"https://www.taobao.com/"' 
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   if (input == "/bz") then  --打开B站
    strProgram = '"https://www.bilibili.com/"' 
    strCmd = 'start "" '..strProgram 
    os.execute(strCmd)
   end
   end

return translator

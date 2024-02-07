-- datetime translator
local function datetime(input, seg)
    -- Candidate(type, start, end, text, comment)
    if (input == "date") then
        yield(Candidate("date", seg.start, seg._end, os.date("%Y年%m月%d日"), ""))
        yield(Candidate("date", seg.start, seg._end, os.date("%Y-%m-%d"), ""))
        yield(Candidate("date", seg.start, seg._end, os.date("%Y-%m-%d %H:%M"), ""))
    elseif (input == "time") then 
        yield(Candidate("time", seg.start, seg._end, os.date("%H:%M:%S"), ""))
        yield(Candidate("time", seg.start, seg._end, os.date("%Y-%m-%d %H:%M:%S"), ""))
    elseif (input == "mver") then 
        yield(Candidate("mver", seg.start, seg._end, os.date("%Y%m%d00"), "版本号"))
    elseif (input == "week") then 
        if (os.date("%w") == "0") then
            weekstr = "日"
        end
        if (os.date("%w") == "1") then
            weekstr = "一"
        end
        if (os.date("%w") == "2") then
            weekstr = "二"
        end
        if (os.date("%w") == "3") then
            weekstr = "三"
        end
        if (os.date("%w") == "4") then
            weekstr = "四"
        end
        if (os.date("%w") == "5") then
            weekstr = "五"
        end
        if (os.date("%w") == "6") then
            weekstr = "六"
        end
        yield(Candidate("week", seg.start, seg._end, " 星期"..weekstr, ""))
        yield(Candidate("week", seg.start, seg._end, os.date("%Y年%m月%d日").." 星期"..weekstr, ""))
        yield(Candidate("week", seg.start, seg._end, os.date("%A"),""))
        yield(Candidate("week", seg.start, seg._end, os.date("%a"),""))
        yield(Candidate("week", seg.start, seg._end, os.date("%W"), "第~周"))
    end
end

return datetime
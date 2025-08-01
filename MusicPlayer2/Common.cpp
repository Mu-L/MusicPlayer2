﻿#include "stdafx.h"
#include "Common.h"
#include "resource.h"
#include "FilePathHelper.h"
#include <random>
#include <strsafe.h>
// #include <pathcch.h>

CCommon::CCommon()
{
}


CCommon::~CCommon()
{
}


//void CCommon::GetAllFormatFiles(wstring path, vector<wstring>& files, const vector<wstring>& format, size_t max_file)
//{
//	//文件句柄
//	int hFile = 0;
//	//文件信息（用Unicode保存使用_wfinddata_t，多字节字符集使用_finddata_t）
//	_wfinddata_t fileinfo;
//	wstring file_path;
//	for (auto a_format : format)
//	{
//		if ((hFile = _wfindfirst(file_path.assign(path).append(L"\\*.").append(a_format).c_str(), &fileinfo)) != -1)
//		{
//			do
//			{
//				if (files.size() >= max_file) break;
//				files.push_back(file_path.assign(fileinfo.name));  //将文件名保存
//			} while (_wfindnext(hFile, &fileinfo) == 0);
//		}
//		_findclose(hFile);
//	}
//	std::sort(files.begin(), files.end());		//对容器里的文件按名称排序
//}

bool CCommon::FileExist(const wstring& file, bool is_case_sensitive)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileW(file.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)  // 没有找到说明不区分大小写也没有匹配文件
        return false;
    FindClose(hFind);
    if (is_case_sensitive)              // 如果需要区分大小写那么重新严格比较
        return CFilePathHelper(file).GetFileName() == findFileData.cFileName;
    return true;
}

bool CCommon::FolderExist(const wstring& file)
{
    DWORD dwAttrib = GetFileAttributes(file.c_str());
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

bool CCommon::IsFolder(const wstring& path)
{
    DWORD dwAttrib = GetFileAttributes(path.c_str());
    return (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool CCommon::CheckAndFixFile(wstring& file)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFileW(file.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    FindClose(hFind);
    CFilePathHelper file_path(file);
    file = file_path.GetDir() + findFileData.cFileName;  // 修正file的文件名大小写到与文件一致
    return true;
}

bool CCommon::GetFileLastModified(const wstring& file_path, unsigned __int64& modified_time)
{
    // 使用GetFileAttributesEx，耗时大约为FindFirstFile的2/3
    WIN32_FILE_ATTRIBUTE_DATA file_attributes;
    if (GetFileAttributesEx(file_path.c_str(), GetFileExInfoStandard, &file_attributes))
    {
        ULARGE_INTEGER last_modified_time{};
        last_modified_time.HighPart = file_attributes.ftLastWriteTime.dwHighDateTime;
        last_modified_time.LowPart = file_attributes.ftLastWriteTime.dwLowDateTime;
        modified_time = last_modified_time.QuadPart;
        return true;
    }
    return false;
}

bool CCommon::GetFileCreateTime(const wstring& file_path, unsigned __int64& create_time)
{
    // 使用GetFileAttributesEx，耗时大约为FindFirstFile的2/3
    WIN32_FILE_ATTRIBUTE_DATA file_attributes;
    if (GetFileAttributesEx(file_path.c_str(), GetFileExInfoStandard, &file_attributes))
    {
        ULARGE_INTEGER time{};
        time.HighPart = file_attributes.ftCreationTime.dwHighDateTime;
        time.LowPart = file_attributes.ftCreationTime.dwLowDateTime;
        create_time = time.QuadPart;
        return true;
    }
    return false;
}

time_t CCommon::FileTimeToTimeT(unsigned __int64 file_time)
{
    if (file_time == 0)
        return 0;
    // 1601年到1970年之间的时间间隔，以100纳秒为单位
    const ULONGLONG epochDelta = 116444736000000000ULL;

    // 将 FILETIME 转换为自1970年1月1日以来的100纳秒间隔数
    ULONGLONG ull = file_time - epochDelta;

    // 将100纳秒转换为秒
    time_t t = ull / 10000000ULL;

    return t;
}

bool CCommon::IsFileHidden(const wstring& file_path)
{
    DWORD attirbute = GetFileAttributes(file_path.c_str());
    return (attirbute & FILE_ATTRIBUTE_HIDDEN) != 0;
}

void CCommon::SetFileHidden(const wstring& file_path, bool hidden)
{
    DWORD attirbute = GetFileAttributes(file_path.c_str());
    if (hidden)
        attirbute |= FILE_ATTRIBUTE_HIDDEN;
    else
        attirbute &= ~FILE_ATTRIBUTE_HIDDEN;
    SetFileAttributes(file_path.c_str(), attirbute);
}

void CCommon::DeleteStringBom(string& str)
{
    //去掉utf8的BOM
    if (str.size() >= 3 && str.substr(0, 3) == string{ '\xef', '\xbb', '\xbf' })
        str = str.substr(3);
    //去掉utf16的BOM
    if (str.size() >= 2 && str.substr(0, 2) == string{ '\xff', '\xfe' })
        str = str.substr(2);
}

bool CCommon::StringCsvNormalize(CString& str)
{
    bool rtn{ false };
    //如果字符串中有双引号，则将其替换为两个双引号
    if (str.Replace(L"\"", L"\"\""))
        rtn = true;

    //如果字符串中包含换行符、双引号和/或逗号，则将其用双引号包裹
    const LPCTSTR spec_str = L"\r\",/";
    if (str.FindOneOf(spec_str) >= 0)
    {
        str = L'\"' + str;
        str.AppendChar(L'\"');
        rtn = true;
    }
    return rtn;
}

//bool CCommon::FileIsMidi(const wstring & file_name)
//{
//	wstring type;
//	type = file_name.substr(file_name.size() - 4, 4);			//获取扩展名
//	std::transform(type.begin(), type.end(), type.begin(), tolower);		//将扩展名转换成小写
//	return (type == L".mid");
//}

void CCommon::StringCopy(char* dest, int size, string source)
{
    int source_size = source.size();
    int i{};
    for (; i < size && i < source_size; i++)
    {
        dest[i] = source[i];
    }
    if (i < size)
    {
        dest[i] = '\0';
    }
}

//bool CCommon::StringCompareNoCase(const wstring & str1, const wstring & str2)
//{
//	wstring _str1{ str1 }, _str2{ str2 };
//	StringTransform(_str1, false);
//	StringTransform(_str2, false);
//	return (_str1 == _str2);
//}

//size_t CCommon::StringFindNoCase(const wstring & str, const wstring & find_str)
//{
//	wstring _str{ str }, _find_str{ find_str };
//	StringTransform(_str, false);
//	StringTransform(_find_str, false);
//	return _str.find(_find_str);
//}

bool CCommon::IsDivideChar(wchar_t ch)
{
    if ((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z') || ch > 255)
        return false;
    else
        return true;
}

bool CCommon::StrIsNumber(const wstring& str)
{
    if (str.empty())
        return false;
    for (const auto& ch : str)
    {
        if (ch < L'0' || ch > L'9')
            return false;
    }
    return true;
}

bool CCommon::CharIsNumber(wchar_t ch)
{
    return (ch >= L'0' && ch <= L'9');
}

int CCommon::StringToInt(const wstring& str)
{
    size_t start{ std::wstring::npos };
    size_t end{ std::wstring::npos };
    for (size_t i{}; i < str.size(); i++)
    {
        //标记第一个数字的位置
        if (CharIsNumber(str[i]) && start == std::wstring::npos)
            start = i;
        //标记找到第一个数字后第一个不是数字的位置
        if (start != std::wstring::npos && !CharIsNumber(str[i]))
        {
            end = i;
            break;
        }
    }
    if (start < str.size() && end > start)
    {
        wstring num_str = str.substr(start, end - start);
        return _wtoi(num_str.c_str());
    }
    return 0;
}

void CCommon::StringSplitLine(const wstring& source_str, vector<wstring>& results, bool skip_empty, bool trim)
{
    results.clear();
    if (source_str.empty())
        return;

    auto push_back_str = [&](const wchar_t* start, const wchar_t* end)
        {
            wstring tmp(start, end);
            if (trim)
                StringNormalize(tmp);
            if (!skip_empty || !tmp.empty())
                results.push_back(std::move(tmp));
        };

    const wchar_t* line_start_pos = source_str.data();
    const wchar_t* cur_pos = line_start_pos;
    const wchar_t* end_pos = line_start_pos + source_str.size();
    while (cur_pos < end_pos)
    {
        if (*cur_pos == L'\r' || *cur_pos == L'\n')
        {
            push_back_str(line_start_pos, cur_pos);
            ++cur_pos;                                          // 指针移动到下一个字符
            if (*cur_pos == L'\n' && *(cur_pos - 1) == L'\r')   // 如果下一个字符是LF且位于CR后面那么跳过
                ++cur_pos;
            line_start_pos = cur_pos;
        }
        else
            ++cur_pos;
    }
    push_back_str(line_start_pos, cur_pos);
}

template<class T>
static void _StringSplit(const T& str, wchar_t div_ch, vector<T>& results, bool skip_empty, bool trim)
{
    results.clear();
    size_t split_index = -1;
    size_t last_split_index = -1;
    while (true)
    {
        split_index = str.find(div_ch, split_index + 1);
        T split_str = str.substr(last_split_index + 1, split_index - last_split_index - 1);
        if (trim)
            CCommon::StringNormalize(split_str);
        if (!split_str.empty() || !skip_empty)
            results.push_back(split_str);
        if (split_index == wstring::npos)
            break;
        last_split_index = split_index;
    }
}

template<class T>
void _StringSplit(const T& str, const T& div_str, vector<T>& results, bool skip_empty /*= true*/, bool trim)
{
    results.clear();
    size_t split_index = 0 - div_str.size();
    size_t last_split_index = 0 - div_str.size();
    while (true)
    {
        split_index = str.find(div_str, split_index + div_str.size());
        T split_str = str.substr(last_split_index + div_str.size(), split_index - last_split_index - div_str.size());
        if (trim)
            CCommon::StringNormalize(split_str);
        if (!split_str.empty() || !skip_empty)
            results.push_back(split_str);
        if (split_index == wstring::npos)
            break;
        last_split_index = split_index;
    }
}

void CCommon::StringSplit(const wstring& str, wchar_t div_ch, vector<wstring>& results, bool skip_empty, bool trim)
{
    _StringSplit<std::wstring>(str, div_ch, results, skip_empty, trim);
}

void CCommon::StringSplit(const wstring& str, const wstring& div_str, vector<wstring>& results, bool skip_empty /*= true*/, bool trim)
{
    _StringSplit<std::wstring>(str, div_str, results, skip_empty, trim);
}

void CCommon::StringSplit(const string& str, char div_ch, vector<string>& result, bool skip_empty, bool trim)
{
    _StringSplit<std::string>(str, div_ch, result, skip_empty, trim);
}

void CCommon::StringSplit(const string& str, const string& div_str, vector<string>& results, bool skip_empty, bool trim)
{
    _StringSplit<std::string>(str, div_str, results, skip_empty, trim);
}

void CCommon::StringSplitWithMulitChars(const wstring& str, const wstring& div_ch, vector<wstring>& results, bool skip_empty /*= true*/)
{
    results.clear();
    size_t split_index = -1;
    size_t last_split_index = -1;
    while (true)
    {
        split_index = str.find_first_of(div_ch, split_index + 1);
        wstring split_str = str.substr(last_split_index + 1, split_index - last_split_index - 1);
        StringNormalize(split_str);
        if (!split_str.empty() || !skip_empty)
            results.push_back(split_str);
        if (split_index == wstring::npos)
            break;
        last_split_index = split_index;
    }
}

void CCommon::StringSplitWithSeparators(const wstring& str, const vector<wstring>& separators, vector<wstring>& results, bool skip_empty /*= true*/)
{
    results.clear();
    size_t split_index = 0;
    size_t last_split_index = 0;

    wstring split_str;
    size_t i{};
    for (; i < separators.size(); i++)
    {
        if (i == 0)
            split_index = str.find(separators[i]);
        else
            split_index = str.find(separators[i], split_index + separators[i - 1].size());

        if (split_index == wstring::npos)
            break;

        if (i == 0)
            split_str = str.substr(0, split_index);
        else
            split_str = str.substr(last_split_index + separators[i - 1].size(), split_index - last_split_index - separators[i - 1].size());
        if (!split_str.empty() || !skip_empty)
            results.push_back(split_str);

        last_split_index = split_index;
    }

    if (i - 1 >= 0 && i - 1 < separators.size())
    {
        size_t index = last_split_index + separators[i - 1].size();
        if (index < str.size())
            split_str = str.substr(last_split_index + separators[i - 1].size());
        if (!split_str.empty() || !skip_empty)
            results.push_back(split_str);
    }
}

wstring CCommon::StringMerge(const vector<wstring>& strings, wchar_t div_ch)
{
    wstring result;
    for (const auto& str : strings)
    {
        result.append(str).push_back(div_ch);
    }
    if (!strings.empty())
        result.pop_back();
    return result;
}

wstring CCommon::MergeStringList(const vector<wstring>& values)
{
    wstring str_merge;
    int index = 0;
    // 在每个字符串前后加上引号，再将它们用逗号连接起来
    for (wstring str : values)
    {
        StringNormalize(str);
        if (str.empty()) continue;
        if (index > 0)
            str_merge.push_back(L',');
        str_merge.push_back(L'\"');
        str_merge += str;
        str_merge.push_back(L'\"');
        index++;
    }
    return str_merge;
}

void CCommon::SplitStringList(vector<wstring>& values, const wstring& str_value)
{
    CCommon::StringSplit(str_value, L"\",\"", values);
    if (!values.empty())
    {
        // 结果中第一项前面和最后一项的后面各还有一个引号，将它们删除
        values.front() = values.front().substr(1);
        values.back().pop_back();
    }
}

wstring CCommon::TranslateToSimplifiedChinese(const wstring& str)
{
    wstring result;
    size_t size{ str.size() };
    if (size == 0) return wstring();
    wchar_t* out_buff = new wchar_t[size + 1];
    WORD wLanguageID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
    LCID Locale = MAKELCID(wLanguageID, SORT_CHINESE_PRCP);
    int rtn = LCMapString(Locale, LCMAP_SIMPLIFIED_CHINESE, str.c_str(), -1, out_buff, size * sizeof(wchar_t));
    result.assign(out_buff);
    delete[] out_buff;
    return result;
}

wstring CCommon::TranslateToTranditionalChinese(const wstring& str)
{
    wstring result;
    size_t size{ str.size() };
    if (size == 0) return wstring();
    wchar_t* out_buff = new wchar_t[size + 1];
    WORD wLanguageID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
    LCID Locale = MAKELCID(wLanguageID, SORT_CHINESE_PRCP);
    int rtn = LCMapString(Locale, LCMAP_TRADITIONAL_CHINESE, str.c_str(), -1, out_buff, size * sizeof(wchar_t));
    result.assign(out_buff);
    delete[] out_buff;
    return result;
}

void CCommon::FileNameNormalize(wstring& file_name)
{
    //查找字符串中的无效字符，并将其替换成‘_’
    const wstring invalid_chars{ L"\\\"/:*?<>|\a\b\f\n\r\t\v" };
    int index{ -1 };
    while (true)
    {
        index = file_name.find_first_of(invalid_chars, index + 1);
        if (index == wstring::npos)
        {
            return;
        }
        else
        {
            if (file_name[index] == L'<')
                file_name[index] = L'(';
            else if (file_name[index] == L'>')
                file_name[index] = L')';
            else
                file_name[index] = L'_';
        }
    }
}

bool CCommon::IsFileNameValid(const wstring& file_name)
{
    const wstring invalid_chars{ L"\\\"/:*?<>|\a\b\f\n\r\t\v" };
    return (file_name.find_first_of(invalid_chars) == wstring::npos);
}

size_t CCommon::GetFileSize(const wstring& file_name)
{
    std::streampos l, m;
    ifstream file(file_name, std::ios::in | std::ios::binary);
    l = file.tellg();
    file.seekg(0, std::ios::end);
    m = file.tellg();
    file.close();
    return static_cast<size_t>(m - l);
}

wstring CCommon::StrToUnicode(const string& str, CodeType code_type, bool auto_utf8)
{
    // 当使用ANSI，UTF8，UTF8_NO_BOM，UTF16LE，UTF16BE时不检测直接转换
    // 使用默认值CodeType::AUTO时先检测BOM，如果没有BOM则按ANSI转换
    // 将auto_utf8置true使AUTO检测没有BOM的字串是否为UTF8序列，如果符合则按UTF8_NO_BOM转换
    // auto_utf8有可能将过短的ANSI误认为是UTF8导致乱码
    if (str.empty()) return wstring();
    // code_type为AUTO时从这里开始
    if (code_type == CodeType::AUTO)	// 先根据BOM判断编码类型
    {
        code_type = JudgeCodeType(str, CodeType::ANSI, auto_utf8);
    }
    bool result_ready = false;
    int size;
    wstring result;
    // 如果编码类型此时已经可以确定并转换好不必回落到ANSI则置位result_ready
    if (code_type == CodeType::UTF8 || code_type == CodeType::UTF8_NO_BOM)
    {
        string temp;
        //如果前面有BOM，则去掉BOM
        if (str.size() >= 3 && str[0] == -17 && str[1] == -69 && str[2] == -65)
            temp = str.substr(3);
        else
            temp = str;
        size = MultiByteToWideChar(CP_UTF8, 0, temp.c_str(), -1, NULL, 0);
        if (size <= 0) return wstring();
        wchar_t* str_unicode = new wchar_t[size + 1];
        MultiByteToWideChar(CP_UTF8, 0, temp.c_str(), -1, str_unicode, size);
        result.assign(str_unicode);
        delete[] str_unicode;
        result_ready = true;
    }
    else if (code_type == CodeType::UTF16LE)
    {
        string temp;
        //如果前面有BOM，则去掉BOM
        if (str.size() >= 2 && str[0] == -1 && str[1] == -2)
            temp = str.substr(2);
        else
            temp = str;
        if (temp.size() % 2 == 1)
            temp.pop_back();
        temp.push_back('\0');
        result = (const wchar_t*)temp.c_str();
        result_ready = true;
    }
    else if (code_type == CodeType::UTF16BE)
    {
        string temp;
        //如果前面有BOM，则去掉BOM
        if (str.size() >= 2 && str[0] == -2 && str[1] == -1)
            temp = str.substr(2);
        else
            temp = str;
        if (temp.size() % 2 == 1)
            temp.pop_back();
        temp.push_back('\0');
        wchar_t* p = (wchar_t*)temp.c_str();
        convertBE_LE(p, temp.size() >> 1);
        result = p;
        result_ready = true;
    }

    // 如果以上均未执行那么按系统ANSI编码处理
    if (!result_ready)
    {
        size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
        if (size <= 0) return wstring();
        wchar_t* str_unicode = new wchar_t[size + 1];
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, str_unicode, size);
        result.assign(str_unicode);
        delete[] str_unicode;
    }
    return result;
}

string CCommon::UnicodeToStr(const wstring& wstr, CodeType code_type, bool* char_cannot_convert)
{
    if (wstr.empty()) return string();
    if (char_cannot_convert != nullptr)
        *char_cannot_convert = false;
    BOOL UsedDefaultChar{ FALSE };
    string result;
    int size{ 0 };
    if (code_type == CodeType::ANSI)
    {
        size = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        if (size <= 0) return string();
        char* str = new char[size + 1];
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, str, size, NULL, &UsedDefaultChar);
        result.assign(str);
        delete[] str;
    }
    else if (code_type == CodeType::UTF8 || code_type == CodeType::UTF8_NO_BOM)
    {
        result.clear();
        if (code_type == CodeType::UTF8)
        {
            result.push_back(-17);
            result.push_back(-69);
            result.push_back(-65);
        }
        size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        if (size <= 0) return string();
        char* str = new char[size + 1];
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, str, size, NULL, NULL);
        result.append(str);
        delete[] str;
    }
    else if (code_type == CodeType::UTF16LE)
    {
        result.clear();
        result.push_back(-1);	//在前面加上UTF16LE的BOM
        result.push_back(-2);
        result.append((const char*)wstr.c_str(), (const char*)wstr.c_str() + wstr.size() * 2);
        result.push_back('\0');
    }
    else if (code_type == CodeType::UTF16BE)
    {
        result.clear();
        result.push_back(-2);	//在前面加上UTF16BE的BOM
        result.push_back(-1);
        wchar_t* p = (wchar_t*)wstr.c_str();
        convertBE_LE(p, wstr.size());
        wstring temp{ p };
        result.append((const char*)temp.c_str(), (const char*)temp.c_str() + temp.size() * 2);
        result.push_back('\0');
    }
    if (char_cannot_convert != nullptr)
        *char_cannot_convert = (UsedDefaultChar != FALSE);
    return result;
}

wstring CCommon::ASCIIToUnicode(const string& ascii)
{
    wstring result;
    result.resize(ascii.size());
    for (size_t i = 0; i < ascii.size(); i++)
    {
        result[i] = ascii[i];
    }
    return result;
}

string CCommon::UnicodeToAscii(const wstring& wstr)
{
    string result;
    result.resize(wstr.size());
    for (size_t i = 0; i < wstr.size(); i++)
    {
        result[i] = static_cast<char>(wstr[i]);
    }
    return result;
}

bool CCommon::IsUTF8Bytes(const char* data)
{
    int charByteCounter = 1;  //计算当前正分析的字符应还有的字节数
    unsigned char curByte; //当前分析的字节.
    bool ascii = true;
    int length = strlen(data);
    for (int i = 0; i < length; i++)
    {
        curByte = static_cast<unsigned char>(data[i]);
        if (charByteCounter == 1)
        {
            if (curByte >= 0x80)
            {
                ascii = false;
                //判断当前
                while (((curByte <<= 1) & 0x80) != 0)
                {
                    charByteCounter++;
                }
                //标记位首位若为非0 则至少以2个1开始 如:110XXXXX...........1111110X
                if (charByteCounter == 1 || charByteCounter > 6)
                {
                    return false;
                }
            }
        }
        else
        {
            //若是UTF-8 此时第一位必须为1
            if ((curByte & 0xC0) != 0x80)
            {
                return false;
            }
            charByteCounter--;
        }
    }
    if (ascii) return false;		//如果全是ASCII字符，返回false
    else return true;
}

CodeType CCommon::JudgeCodeType(const string& str, CodeType default_code, bool auto_utf8)
{
    CodeType code_type{ default_code };
    if (!str.empty())
    {
        // 如果前面有UTF8的BOM，则编码类型为UTF8
        if (str.size() >= 3 && str[0] == -17 && str[1] == -69 && str[2] == -65)
            code_type = CodeType::UTF8;
        // 如果前面有UTF16LE的BOM，则编码类型为UTF16LE
        else if (str.size() >= 2 && str[0] == -1 && str[1] == -2)
            code_type = CodeType::UTF16LE;
        // 如果前面有UTF16BE的BOM，则编码类型为UTF16BE
        else if (str.size() >= 2 && str[0] == -2 && str[1] == -1)
            code_type = CodeType::UTF16BE;
        // AUTO时是否将符合UTF8格式的str作为UTF8_NO_BOM处理
        else if (auto_utf8 && IsUTF8Bytes(str.c_str()))
            code_type = CodeType::UTF8_NO_BOM;
    }
    return code_type;
}

bool CCommon::IsURL(const wstring& str)
{
    return (str.substr(0, 7) == L"http://" || str.substr(0, 8) == L"https://" || str.substr(0, 6) == L"ftp://" || str.substr(0, 6) == L"mms://");
}

bool CCommon::IsWindowsPath(const wstring& str)
{
    return (str.size() >= 3                                                             // windows 路径至少3个字符
        && ((str[0] >= L'A' && str[0] <= L'Z') || (str[0] >= L'a' && str[0] <= L'z'))   // 第1个字符必须为字母
        && str[1] == L':'                                                               // 第2个字符必须为冒号
        && (str[2] == L'/' || str[2] == L'\\')                                          // 第3个字符必须为斜杠
        );
}

bool CCommon::IsPath(const wstring& str)
{
    if (str.size() < 3)
        return false;

    bool is_windows_path{ IsWindowsPath(str) };
    bool is_UNC_path{ str[0] == L'\\' && str[1] == L'\\' };

    if (!is_windows_path && !is_UNC_path)
        return false;

    const wstring invalid_chars{ L":*?\"<>|" };
    if (str.find_first_of(invalid_chars, is_windows_path ? 2 : 0) != wstring::npos)
        return false;

    return true;
}

bool CCommon::StringCharacterReplace(wstring& str, wchar_t ch, wchar_t ch_replaced)
{
    bool replaced = false;
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ch)
        {
            str[i] = ch_replaced;
            replaced = true;
        }
    }
    return replaced;
}

bool CCommon::StringReplace(wstring& str, const wstring& str_old, const wstring& str_new)
{
    if (str.empty())
        return false;
    bool replaced{ false };
    size_t pos = 0;
    while ((pos = str.find(str_old, pos)) != std::wstring::npos)
    {
        str.replace(pos, str_old.length(), str_new);
        replaced = true;
        pos += str_new.length();    // 前进到替换后的字符串末尾
    }
    return replaced;
}

CString CCommon::DataSizeToString(size_t data_size)
{
    CString size_info;
    if (data_size < 1024)
        size_info.Format(_T("%u B"), data_size);
    else if (data_size < 1024 * 1024)
        size_info.Format(_T("%.2f KB"), data_size / 1024.0f);
    else if (data_size < 1024 * 1024 * 1024)
        size_info.Format(_T("%.2f MB"), data_size / 1024.0f / 1024.0f);
    else
        size_info.Format(_T("%.2f GB"), data_size / 1024.0f / 1024.0f / 1024.0f);
    return size_info;
}

wstring CCommon::GetExePath()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    size_t index;
    wstring current_path{ path };
    index = current_path.find_last_of(L'\\');
    current_path = current_path.substr(0, index + 1);
    return current_path;
}

wstring CCommon::GetDesktopPath()
{
    LPITEMIDLIST ppidl;
    TCHAR pszDesktopPath[MAX_PATH];
    if (SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &ppidl) == S_OK)
    {
        SHGetPathFromIDList(ppidl, pszDesktopPath);
        CoTaskMemFree(ppidl);
    }
    return wstring(pszDesktopPath);
}

wstring CCommon::GetTemplatePath()
{
    wstring result;
    wchar_t buff[MAX_PATH];
    GetTempPath(MAX_PATH, buff);		//获取临时文件夹的路径
    result = buff;
    if (!result.empty() && result.back() != L'\\' && result.back() != L'/')		//确保路径后面有斜杠
        result.push_back(L'\\');
    return result;
}


wstring CCommon::GetAppDataConfigDir()
{
    LPITEMIDLIST ppidl;
    TCHAR pszAppDataPath[MAX_PATH];
    if (SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &ppidl) == S_OK)
    {
        SHGetPathFromIDList(ppidl, pszAppDataPath);
        CoTaskMemFree(ppidl);
    }
    wstring app_data_path{ pszAppDataPath };        //获取到C:/User/用户名/AppData/Roaming路径
    CreateDirectory(app_data_path.c_str(), NULL);       //如果Roaming不存在，则创建它
    app_data_path += L'\\';
    app_data_path += APP_NAME;
#ifdef _DEBUG
    app_data_path += L" (Debug)";
#endif
    app_data_path += L'\\';
    CreateDirectory(app_data_path.c_str(), NULL);       //如果C:/User/用户名/AppData/Roaming/MusicPlayer2不存在，则创建它

    return app_data_path;
}

wstring CCommon::GetSpecialDir(int csidl)
{
    bool rtn{};
    LPITEMIDLIST ppidl;
    wchar_t folder_dir[MAX_PATH];
    if (SHGetSpecialFolderLocation(NULL, csidl, &ppidl) == S_OK)
    {
        rtn = SHGetPathFromIDListW(ppidl, folder_dir);
        CoTaskMemFree(ppidl);
    }
    ASSERT(rtn);
    if (rtn)
        return wstring(folder_dir);
    return wstring();
}

bool CCommon::CreateDir(const _tstring& path)
{
    if (!FolderExist(path))
    {
        if (CreateDirectory(path.c_str(), NULL))
        {
            return true;
        }
    }
    return false;
}

_tstring CCommon::FileRename(const _tstring& file_path, const _tstring& new_file_name)
{
    if (!FileExist(file_path))
        return _tstring();

    _tstring dir;
    size_t index = file_path.find_last_of(_T("/\\"));
    if (index == _tstring::npos)
        return _tstring();
    dir = file_path.substr(0, index + 1);

    _tstring extension;
    size_t index_ext = file_path.rfind(_T('.'));
    if (index_ext < file_path.size() - 1 && index_ext > index)
        extension = file_path.substr(index_ext);

    _tstring new_file_path = dir + new_file_name + extension;
    try
    {
        CFile::Rename(file_path.c_str(), new_file_path.c_str());
    }
    catch (CFileException* pEx)
    {
        new_file_path.clear();
        pEx->Delete();
    }
    return new_file_path;
}

_tstring CCommon::RelativePathToAbsolutePath(const _tstring& relative_path, const _tstring& cur_dir)
{
    // relative_path如果是盘符开头的绝对路径那么返回此绝对路径
    // 否则将relative_path视为相对路径或斜杠开头的绝对路径并与cur_dir拼接
    _tstring result = relative_path;
    _tstring dir = cur_dir;
    if (!IsPath(result) && !dir.empty())
    {
        // https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/
        // PathCombine拼接简化两个合法路径，当result为斜杠开头的绝对路径时将其只与dir的盘符拼接
        // PathCchCombine 是处理了缓冲区溢出的安全版本，Windows 8开始支持，位于pathcch.h
        TCHAR lpBuffer[MAX_PATH]{};
        PathCombine(lpBuffer, dir.c_str(), result.c_str());
        // PathCchCombine(lpBuffer, MAX_PATH, dir.c_str(), result.c_str());
        result = lpBuffer;
    }
    return result;
}

bool CCommon::CopyStringToClipboard(const wstring& str)
{
    if (OpenClipboard(NULL))
    {
        HGLOBAL clipbuffer;
        EmptyClipboard();
        size_t size = (str.size() + 1) * 2;
        clipbuffer = GlobalAlloc(GMEM_DDESHARE, size);
        memcpy_s(GlobalLock(clipbuffer), size, str.c_str(), size);
        GlobalUnlock(clipbuffer);
        if (SetClipboardData(CF_UNICODETEXT, clipbuffer) == NULL)
            return false;
        CloseClipboard();
        return true;
    }
    else return false;
}

wstring CCommon::GetStringFromClipboard()
{
    if (OpenClipboard(NULL))
    {
        if (IsClipboardFormatAvailable(CF_TEXT))
        {
            HANDLE hClip;
            wchar_t* pBuf;
            hClip = GetClipboardData(CF_UNICODETEXT); //获取剪贴板数据
            if (hClip == NULL)
                return wstring();
            pBuf = (wchar_t*)GlobalLock(hClip);
            if (pBuf == nullptr)
                return wstring();
            CloseClipboard();
            return wstring(pBuf);
        }
    }
    return wstring();
}

void CCommon::WriteLog(const wchar_t* path, const wstring& content)
{
    SYSTEMTIME cur_time;
    GetLocalTime(&cur_time);
    char buff[32];
    sprintf_s(buff, "%d/%.2d/%.2d %.2d:%.2d:%.2d.%.3d ", cur_time.wYear, cur_time.wMonth, cur_time.wDay,
        cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);
    ofstream out_put{ path, std::ios::app };
    out_put << buff << CCommon::UnicodeToStr(content, CodeType::UTF8_NO_BOM) << std::endl;
    out_put.close();    // 这里需要显式关闭以避免被不同线程连续调用时丢失内容（不过还是不能承受并发，多线程并发时请自行加锁
}

void CCommon::DisposeCmdLineFiles(const wstring& cmd_line, vector<wstring>& files)
{
    // 解析命令行参数中的文件/文件夹路径放入files
    // files 中能够接受音频文件路径/播放列表文件路径/文件夹路径随意乱序出现
    // files 中无法被识别为“播放列表文件路径”“文件夹路径”的项目会被直接加入默认播放列表
    // TODO: 这里可能需要添加以下功能，我没有其他windows版本的经验，不确定这里怎样改
    //       文件/文件夹存在判断；路径通配符展开；相对路径转换绝对路径；支持不在同一文件夹下的多个文件路径
    files.clear();
    if (cmd_line.empty()) return;
    wstring path;
    //先找出字符串中的文件夹路径，从命令行参数传递过来的文件肯定都是同一个文件夹下的
    if (cmd_line[0] == L'\"')		//如果第一个文件用双引号包含起来
    {
        int index1 = cmd_line.find(L'\"', 1);		//查找和第1个双引号匹配的双引号
        int index2 = cmd_line.rfind(L'\\', index1);		//往前查找反斜杠
        path = cmd_line.substr(1, index2);		//获取文件夹路径（包含最后一个反斜杠）
        files.push_back(cmd_line.substr(1, index1 - 1));
    }
    else		//如果第一个文件没有用双引号包含起来，则说明路径中不包含空格，
    {
        int index1 = cmd_line.find(L' ');		//查找和第1空格
        int index2 = cmd_line.rfind(L'\\', index1);		//往前查找反斜杠
        path = cmd_line.substr(0, index2 + 1);		//获取文件夹路径（包含最后一个反斜杠）
        files.push_back(cmd_line.substr(0, index1));
    }
    int path_size = path.size();
    if (path_size < 2) return;
    int index{};
    while (true)
    {
        index = cmd_line.find(path, index + path_size);		//从第2个开始查找路径出现的位置
        if (index == string::npos) break;
        if (index > 0 && cmd_line[index - 1] == L'\"')		//如果路径前面一个字符是双引号
        {
            int index1 = cmd_line.find(L'\"', index);
            files.push_back(cmd_line.substr(index, index1 - index));
        }
        else
        {
            int index1 = cmd_line.find(L' ', index);
            files.push_back(cmd_line.substr(index, index1 - index));
        }
    }
    return;
    //CString out_info;
    //out_info += _T("命令行参数：");
    //out_info += cmd_line.c_str();
    //out_info += _T("\r\n");
    //out_info += _T("路径：");
    //out_info += path.c_str();
    //out_info += _T("\r\n");
    //CCommon::WriteLog(L".\\command.log", wstring{ out_info });
}

bool CCommon::GetCmdLineCommand(const wstring& cmd_line, int& command)
{
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-play_pause")) != string::npos || CCommon::StringNatchWholeWord(cmd_line, wstring(L"-p")) != string::npos)
        command |= ControlCmd::PLAY_PAUSE;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-previous")) != string::npos || CCommon::StringNatchWholeWord(cmd_line, wstring(L"-pre")) != string::npos)
        command |= ControlCmd::_PREVIOUS;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-next")) != string::npos || CCommon::StringNatchWholeWord(cmd_line, wstring(L"-n")) != string::npos)
        command |= ControlCmd::_NEXT;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-stop")) != string::npos || CCommon::StringNatchWholeWord(cmd_line, wstring(L"-s")) != string::npos)
        command |= ControlCmd::STOP;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-ff")) != string::npos)
        command |= ControlCmd::FF;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-rew")) != string::npos)
        command |= ControlCmd::REW;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-vol_up")) != string::npos)
        command |= ControlCmd::VOLUME_UP;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-vol_down")) != string::npos)
        command |= ControlCmd::VOLUME_DOWM;
    if (CCommon::StringNatchWholeWord(cmd_line, wstring(L"-mini")) != string::npos)
        command |= ControlCmd::MINI_MODE;
    return command != ControlCmd::NONE;
}

bool CCommon::CreateFileShortcut(LPCTSTR lpszLnkFileDir, LPCTSTR lpszFileName, LPCTSTR lpszLnkFileName, LPCTSTR lpszWorkDir, WORD wHotkey, LPCTSTR lpszDescription, int iShowCmd, LPCTSTR lpszArguments, int nIconOffset)
{
    if (lpszLnkFileDir == NULL)
        return false;

    HRESULT hr;
    IShellLink* pLink;  //IShellLink对象指针
    IPersistFile* ppf; //IPersisFil对象指针

    //创建IShellLink对象
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
    if (FAILED(hr))
        return false;

    //从IShellLink对象中获取IPersistFile接口
    hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(hr))
    {
        pLink->Release();
        return false;
    }

    TCHAR file_path[MAX_PATH];		//当前进程exe文件路径
    GetModuleFileName(NULL, file_path, MAX_PATH);
    LPCTSTR pFilePath;				//快捷方式目标

    //目标
    if (lpszFileName == NULL)
        pFilePath = file_path;
    else
        pFilePath = lpszFileName;
    pLink->SetPath(pFilePath);

    //工作目录
    if (lpszWorkDir != NULL)
    {
        pLink->SetWorkingDirectory(lpszWorkDir);
    }
    else
    {
        //设置工作目录为快捷方式目标所在位置
        _tstring workDir = pFilePath;
        int index = workDir.find_last_of(_T("\\/"));
        if (index != workDir.npos)
            workDir = workDir.substr(0, index);
        pLink->SetWorkingDirectory(workDir.c_str());
    }

    //快捷键
    if (wHotkey != 0)
        pLink->SetHotkey(wHotkey);

    //备注
    if (lpszDescription != NULL)
        pLink->SetDescription(lpszDescription);

    //显示方式
    pLink->SetShowCmd(iShowCmd);

    //参数
    if (lpszArguments != NULL)
        pLink->SetArguments(lpszArguments);

    //图标
    if (nIconOffset > 0)
        pLink->SetIconLocation(pFilePath, nIconOffset);

    //快捷方式的路径 + 名称
    _tstring shortcutName;
    shortcutName = lpszLnkFileDir;
    if (!shortcutName.empty() && shortcutName.back() != _T('\\') && shortcutName.back() != _T('/'))
        shortcutName.push_back(_T('\\'));

    if (lpszLnkFileName != NULL) //指定了快捷方式的名称
    {
        shortcutName += lpszLnkFileName;
    }
    else
    {
        //没有指定名称，就从取指定文件的文件名作为快捷方式名称。
        _tstring fileName = pFilePath;

        int index1, index2;
        index1 = fileName.find_last_of(_T("\\/"));
        index2 = fileName.rfind(_T('.'));

        if (index1 == _tstring::npos || index2 == _tstring::npos || index1 >= index2)
        {
            ppf->Release();
            pLink->Release();
            return false;
        }
        fileName = fileName.substr(index1 + 1, index2 - index1 - 1);
        fileName += _T(".lnk");
        shortcutName += fileName;
    }

    //保存快捷方式到指定目录下
    hr = ppf->Save(shortcutName.c_str(), TRUE);

    ppf->Release();
    pLink->Release();
    return SUCCEEDED(hr);
}

void CCommon::GetFiles(wstring file_name, vector<wstring>& files, std::function<bool(const wstring&)> fun_is_valid)
{
    files.clear();
    //文件句柄
    intptr_t hFile = 0;
    //文件信息（用Unicode保存使用_wfinddata_t，多字节字符集使用_finddata_t）
    _wfinddata_t fileinfo;
    if ((hFile = _wfindfirst(file_name.c_str(), &fileinfo)) != -1)
    {
        do
        {
            if (fun_is_valid(fileinfo.name))
                files.push_back(fileinfo.name);
        } while (_wfindnext(hFile, &fileinfo) == 0);
    }
    _findclose(hFile);
}

void CCommon::GetImageFiles(wstring file_name, vector<wstring>& files)
{
    files.clear();
    //文件句柄
    intptr_t hFile = 0;
    //文件信息（用Unicode保存使用_wfinddata_t，多字节字符集使用_finddata_t）
    _wfinddata_t fileinfo;
    if ((hFile = _wfindfirst(file_name.c_str(), &fileinfo)) != -1)
    {
        do
        {
            if (FileIsImage(fileinfo.name))
                files.push_back(fileinfo.name);
        } while (_wfindnext(hFile, &fileinfo) == 0);
    }
    _findclose(hFile);
}

bool CCommon::IsFolderMatchKeyWord(wstring dir, const wstring& key_word)
{
    //文件句柄
    intptr_t hFile = 0;
    //文件信息
    _wfinddata_t fileinfo;
    if (dir.back() != '\\' && dir.back() != '/')
        dir.push_back('\\');

    wstring folder_name;
    size_t index = dir.rfind(L'\\', dir.size() - 2);
    folder_name = dir.substr(index + 1, dir.size() - index - 2);

    if ((hFile = _wfindfirst((dir + L"*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            wstring file_name = fileinfo.name;
            if (file_name == L"." || file_name == L"..")
                continue;

            if (CCommon::IsFolder(dir + file_name))        //当前是文件夹，则递归调用
            {
                if (IsFolderMatchKeyWord(dir + file_name, key_word))
                {
                    _findclose(hFile);
                    return true;
                }
            }
            //else
            //{
            //    if (StringFindNoCase(file_name, key_word) != wstring::npos)
            //        return true;
            //}
        } while (_wfindnext(hFile, &fileinfo) == 0);

        if (StringFindNoCase(folder_name, key_word) != wstring::npos)
        {
            _findclose(hFile);
            return true;
        }
    }
    _findclose(hFile);
    return false;
}

bool CCommon::FileIsImage(const wstring& file_name)
{
    size_t index;
    index = file_name.find_last_of(L'.');
    wstring type;
    if (index != string::npos)
        type = file_name.substr(index);			//获取扩展名
    std::transform(type.begin(), type.end(), type.begin(), tolower);		//将扩展名转换成小写
    return (type == L".jpg" || type == L".jpeg" || type == L".png" || type == L".gif" || type == L".bmp");
}

wstring CCommon::GetRandomString(int length)
{
    wstring result;
    SYSTEMTIME current_time;
    GetLocalTime(&current_time);			//获取当前时间
    srand(current_time.wMilliseconds);		//用当前时间的毫秒数设置产生随机数的种子
    int char_type;		//当前要生成的字符类型 0：数字；1：小写字母；2：大写字母
    for (int i{}; i < length; i++)
    {
        char_type = rand() % 3;		//随机确定要生成的字符类型
        wchar_t current_char;
        switch (char_type)
        {
        case 0:
            current_char = L'0' + (rand() % 10);
            break;
        case 1:
            current_char = L'a' + (rand() % 26);
            break;
        case 2:
            current_char = L'A' + (rand() % 26);
            break;
        }
        result.push_back(current_char);
    }
    return result;
}

bool CCommon::IsFileNameNumbered(const wstring& file_name, int& number, size_t& index)
{
    if (file_name.empty())
        return false;
    if (file_name.back() != L')')
        return false;
    size_t size{ file_name.size() };
    //size_t index;
    index = file_name.rfind(L'(');		//往前查找右括号
    if (index == wstring::npos || index == 0)		//左括号不能在字符串开头
        return false;
    wstring number_str{ file_name.substr(index + 1, size - index - 2) };		//获取两个括号之间的文本
    if (StrIsNumber(number_str))				//判断文本是否是数字
    {
        number = _wtoi(number_str.c_str());
        return true;
    }
    else
    {
        return false;
    }
}

void CCommon::SizeZoom(CSize& size, int side)
{
    float width_height_ratio{ static_cast<float>(size.cx) / size.cy };	//长宽比
    if (width_height_ratio > 1)		//长宽比大于1：1
    {
        size.cx = side;
        size.cy = static_cast<int>(side / width_height_ratio);
    }
    else if (width_height_ratio < 1)
    {
        size.cy = side;
        size.cx = static_cast<int>(side * width_height_ratio);
    }
    else
    {
        size.cx = size.cy = side;
    }
}

COLORREF CCommon::GetWindowsThemeColor()
{
#ifdef COMPILE_IN_WIN_XP
    return RGB(0, 120, 215);
#else
    DWORD crColorization;
    BOOL fOpaqueBlend;
    COLORREF theme_color{};
    HRESULT result = DwmGetColorizationColor(&crColorization, &fOpaqueBlend);
    if (result == S_OK)
    {
        BYTE r, g, b;
        r = (crColorization >> 16) % 256;
        g = (crColorization >> 8) % 256;
        b = crColorization % 256;
        theme_color = RGB(r, g, b);
    }
    return theme_color;
#endif
}


int CCommon::AppendMenuOp(HMENU hDst, HMENU hSrc)
{
    int iCnt = 0;
    ASSERT(hDst && hSrc);

    for (int iSrc = 0, iDst = GetMenuItemCount(hDst); iSrc < GetMenuItemCount(hSrc); iSrc++)
    {
        TCHAR szMenuStr[256] = { 0 };
        MENUITEMINFO mInfo = { 0 };
        mInfo.cbSize = sizeof(mInfo);
        mInfo.fMask = 0
            | MIIM_CHECKMARKS //Retrieves or sets the hbmpChecked and hbmpUnchecked members.
            | MIIM_DATA //Retrieves or sets the dwItemData member.
            | MIIM_ID //Retrieves or sets the wID member.
            | MIIM_STATE //Retrieves or sets the fState member.
            | MIIM_SUBMENU //Retrieves or sets the hSubMenu member.
            | MIIM_FTYPE //Retrieves or sets the fType and dwTypeData members.
            //这里如果不添加MIIM_STRING和MIIM_BITMAP，会导致有图标的菜单项文本不显示
            | MIIM_STRING
            | MIIM_BITMAP
            | 0;
        mInfo.dwTypeData = szMenuStr;
        mInfo.cch = _countof(szMenuStr);

        VERIFY(GetMenuItemInfo(hSrc, iSrc, TRUE, &mInfo));

        if (mInfo.hSubMenu)
        {
            HMENU hSub = CreatePopupMenu();
            AppendMenuOp(hSub, mInfo.hSubMenu);
            mInfo.hSubMenu = hSub;
        }

        InsertMenuItem(hDst, iDst++, TRUE, &mInfo);
        iCnt++;
    }

    return iCnt;
}

bool CCommon::IsMenuItemInMenu(CMenu* pMenu, UINT id)
{
    if (pMenu == nullptr)
        return false;

    int item_count = pMenu->GetMenuItemCount();
    for (int i = 0; i < item_count; i++)
    {
        if (pMenu->GetMenuItemID(i) == id)
            return true;
        CMenu* pSubMenu = pMenu->GetSubMenu(i);
        if (IsMenuItemInMenu(pSubMenu, id))
            return true;
    }
    return false;
}

int CCommon::GetMenuItemPosition(CMenu* pMenu, UINT id)
{
    int pos = -1;
    int item_count = pMenu->GetMenuItemCount();
    for (int i = 0; i < item_count; i++)
    {
        if (pMenu->GetMenuItemID(i) == id)
        {
            pos = i;
            break;
        }
    }
    return pos;
}

void CCommon::IterateMenuItem(CMenu* pMenu, std::function<void(CMenu*, UINT)> func)
{
    if (pMenu != nullptr)
    {
        int item_count = pMenu->GetMenuItemCount();
        for (int i = 0; i < item_count; i++)
        {
            CMenu* pSubMenu = pMenu->GetSubMenu(i);
            if (pSubMenu != nullptr)
                IterateMenuItem(pSubMenu, func);
            UINT id = pMenu->GetMenuItemID(i);
            if (id > 0)
                func(pMenu, id);
        }
    }
}

bool CCommon::StringLeftMatch(const std::wstring& str, const std::wstring& matched_str)
{
    if (str.size() < matched_str.size())
        return false;
    return str.substr(0, matched_str.size()) == matched_str;
}

bool CCommon::GetThreadLanguageList(vector<wstring>& language_tag)
{
    language_tag.clear();
    ULONG num{};
    vector<wchar_t> buffer(LOCALE_NAME_MAX_LENGTH, L'\0');
    ULONG buffer_size{ static_cast<ULONG>(buffer.size()) };
    bool rtn = GetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, &num, buffer.data(), &buffer_size); // >=Windows Vista
    if (!rtn) return false;
    ASSERT(buffer_size <= LOCALE_NAME_MAX_LENGTH);
    size_t pos{};
    for (size_t i{}; i < buffer_size; ++i)
    {
        if (buffer[i] != L'\0' || pos == i)
            continue;
        wstring tmp(buffer.begin() + pos, buffer.begin() + i);
        language_tag.push_back(tmp);
        pos = i + 1;
    }
    ASSERT(language_tag.size() == num);
    return true;
}

bool CCommon::SetThreadLanguageList(const vector<wstring>& language_tag)
{
    vector<wchar_t> buffer;
    size_t buffer_size{ 1 };
    for (const wstring& tag : language_tag)
    {
        if (tag.empty()) continue;
        buffer_size += tag.size() + 1;
        if (buffer_size > LOCALE_NAME_MAX_LENGTH)
            break;
        buffer.insert(buffer.end(), tag.begin(), tag.end());
        buffer.push_back(L'\0');
    }
    if (buffer.empty()) // buffer为空时多插入一个\0确保双\0结尾（空buffer会重置之前设置的线程首选UI语言列表）
        buffer.push_back(L'\0');
    buffer.push_back(L'\0');
    bool rtn = SetThreadPreferredUILanguages(MUI_LANGUAGE_NAME, buffer.data(), nullptr); // >=Windows Vista
    return rtn;
}

wstring CCommon::GetSystemDefaultUIFont()
{
    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
    wstring font_name = metrics.lfMessageFont.lfFaceName;
    return font_name;
}

void CCommon::WStringCopy(wchar_t* str_dest, int dest_size, const wchar_t* str_source, int source_size)
{
    if (dest_size <= 0)
        return;
    if (source_size <= 0 || str_source == nullptr)
    {
        str_dest[0] = L'\0';
        return;
    }
    int i;
    for (i = 0; i < dest_size && i < source_size && str_source[i] != L'\0'; i++)
        str_dest[i] = str_source[i];
    //确保目标字符串末尾有一个\0
    int copy_cnt = i;
    if (copy_cnt < dest_size)
        str_dest[copy_cnt] = L'\0';
    else
        str_dest[dest_size - 1] = L'\0';
}

void CCommon::NormalizeFont(LOGFONT& font)
{
    wstring name;
    wstring style;
    name = font.lfFaceName;
    if (name.empty())
        return;
    if (name.back() == L' ')
        name.pop_back();
    size_t index = name.rfind(L' ');
    if (index == wstring::npos)
        return;
    style = name.substr(index + 1);
    bool style_acquired = false;
    if (style == L"Light")
    {
        font.lfWeight = FW_LIGHT;
        style_acquired = true;
    }
    else if (style == L"Semilight")
    {
        font.lfWeight = 350;
        style_acquired = true;
    }
    else if (style == L"ExtraLight")
    {
        font.lfWeight = FW_EXTRALIGHT;
        style_acquired = true;
    }
    else if (style == L"Semibold")
    {
        font.lfWeight = FW_SEMIBOLD;
        style_acquired = true;
    }
    else if (style == L"Bold")
    {
        font.lfWeight = FW_BOLD;
        style_acquired = true;
    }
    else if (style == L"ExtraBold")
    {
        font.lfWeight = FW_EXTRABOLD;
        style_acquired = true;
    }
    else if (style == L"Black" || style == L"Heavy")
    {
        font.lfWeight = FW_BLACK;
        style_acquired = true;
    }
    else if (style == L"Normal" || style == L"Regular")
    {
        font.lfWeight = FW_NORMAL;
        style_acquired = true;
    }

    if (style_acquired)
    {
        name = name.substr(0, index);
    }
    //wcsncpy_s(font.lfFaceName, name.c_str(), 32);
    WStringCopy(font.lfFaceName, 32, name.c_str(), name.size());
}

int CCommon::GetMenuBarHeight(HWND hWnd)
{
    MENUBARINFO menuInfo{};
    menuInfo.cbSize = sizeof(MENUBARINFO);
    int rtn = GetMenuBarInfo(hWnd, OBJID_MENU, 0, &menuInfo);

    int menu_bar_height = 0;
    if (rtn != 0)
        menu_bar_height = menuInfo.rcBar.bottom - menuInfo.rcBar.top;

    return menu_bar_height;
}

double CCommon::DoubleRound(double dVal, int format)
{
    auto ipow = std::pow(10, format);
    return std::floor(dVal * ipow + 0.5) / ipow;
}

int CCommon::IconSizeNormalize(int size)
{
    //查找标准图标大小列表中和指定大小最接近的一个，然后将其返回

    const vector<int> standard_size{ 16, 20, 24, 28, 32, 48, 64, 128, 256, 512 };
    int min_diff = MAXINT;
    int index = 0;
    for (size_t i = 0; i < standard_size.size(); i++)
    {
        int diff = std::abs(size - standard_size[i]);
        if (diff == min_diff && diff > 2)
            return size;

        if (diff < min_diff)
        {
            min_diff = diff;
            index = i;
        }
    }
    return standard_size[index];
}

void CCommon::SetWindowOpacity(HWND hWnd, int opacity)
{
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    ::SetLayeredWindowAttributes(hWnd, 0, opacity * 255 / 100, LWA_ALPHA);  //透明度取值范围为0~255
}

bool CCommon::StringIsVersion(LPCTSTR str)
{
    CString version_str{ str };
    return (version_str.GetLength() == 4 || version_str.GetLength() == 5) && version_str[1] == _T('.') && CharIsNumber(version_str[0]) && CharIsNumber(version_str[2]) && CharIsNumber(version_str[3]);
}

bool CCommon::GetFileContent(const wchar_t* file_path, string& contents_buff, size_t max_size)
{
    std::ifstream file{ file_path, std::ios::binary | std::ios::in };
    if (file.fail())
        return false;
    //获取文件长度
    file.seekg(0, file.end);
    unsigned int length{ static_cast<unsigned int>(file.tellg()) };
    file.seekg(0, file.beg);

    char* buff = new char[length];
    file.read(buff, length);
    file.close();

    contents_buff.assign(buff, length);
    delete[] buff;

    return true;
}

const char* CCommon::GetFileContent(const wchar_t* file_path, size_t& length)
{
    std::ifstream file{ file_path, std::ios::binary };
    length = 0;
    if (file.fail())
        return nullptr;
    //获取文件长度
    file.seekg(0, file.end);
    length = static_cast<size_t>(file.tellg());
    file.seekg(0, file.beg);

    char* buff = new char[length];
    file.read(buff, length);
    file.close();

    return buff;
}

bool CCommon::SaveDataToFile(const string& data, const wstring& file_path)
{
    ofstream out_put{ file_path, std::ios::binary };
    if (!out_put.is_open())
        return false;
    if (!data.empty())
        out_put << data;
    out_put.close();
    return true;
}

void CCommon::DoOpenFileDlg(const wstring& filter, vector<wstring>& path_list, CWnd* pParent)
{
    path_list.clear();
    //构造打开文件对话框
    CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, filter.c_str(), pParent);
    //设置保存文件名的字符缓冲的大小为128kB（如果以平均一个文件名长度为32字节计算，最多可以打开大约4096个文件）
    fileDlg.m_ofn.nMaxFile = 128 * 1024;
    LPTSTR ch = new TCHAR[fileDlg.m_ofn.nMaxFile];
    fileDlg.m_ofn.lpstrFile = ch;
    //对内存块清零
    ZeroMemory(fileDlg.m_ofn.lpstrFile, sizeof(TCHAR) * fileDlg.m_ofn.nMaxFile);
    //显示打开文件对话框
    if (IDOK == fileDlg.DoModal())
    {
        POSITION posFile = fileDlg.GetStartPosition();
        while (posFile != NULL)
        {
            path_list.push_back(fileDlg.GetNextPathName(posFile).GetString());
        }
    }
    delete[] ch;

}

void CCommon::SetDialogFont(CWnd* pDlg, CFont* pFont)
{
    if (pDlg->GetSafeHwnd() != NULL)
    {
        CWnd* pWndChild;
        pWndChild = pDlg->GetWindow(GW_CHILD);
        while (pWndChild)
        {
            pWndChild->SetFont(pFont);
            pWndChild = pWndChild->GetWindow(GW_HWNDNEXT);
        }
    }
}

void CCommon::FileAutoRename(wstring& file_path)
{
    while (CCommon::FileExist(file_path))
    {
        //判断文件名的末尾是否符合“(数字)”的形式
        wstring file_name;		//文件名（不含扩展名）
        CFilePathHelper c_file_path(file_path);
        file_name = c_file_path.GetFileNameWithoutExtension();
        wstring ext{ c_file_path.GetFileExtension() };
        int num;
        size_t index;
        bool is_numbered{ CCommon::IsFileNameNumbered(file_name, num, index) };		//文件名的末尾是否符合“(数字)”的形式
        if (!is_numbered)		//如果文件名末尾没有“(数字)”，则在末尾添加“ (1)”
        {
            file_name += L" (1)";
        }
        else		//否则，将原来的数字加1
        {
            file_name = file_name.substr(0, index);
            CString num_str;
            num_str.Format(_T("(%d)"), num + 1);
            file_name += num_str;
        }
        file_path = c_file_path.GetDir() + file_name + L'.' + ext;
    }

}

int CCommon::StringCompareInLocalLanguage(const wstring& str1, const wstring& str2, bool no_case)
{
#ifndef COMPILE_IN_WIN_XP
    int rtn = CompareStringEx(LOCALE_NAME_USER_DEFAULT, (no_case ? NORM_IGNORECASE : 0) | SORT_DIGITSASNUMBERS, str1.c_str(), str1.size(), str2.c_str(), str2.size(), NULL, NULL, 0);
#else
    int rtn = CompareString(LOCALE_NAME_USER_DEFAULT, (no_case ? NORM_IGNORECASE : 0), str1.c_str(), str1.size(), str2.c_str(), str2.size());
#endif
    if (rtn == CSTR_EQUAL)
        return 0;
    else if (rtn == CSTR_GREATER_THAN)
        return 1;
    else
        return -1;
}

void CCommon::SetNumberBit(unsigned short& num, int bit, bool value)
{
    if (value)
    {
        num |= (1 << bit);
    }
    else
    {
        num &= ~(1 << bit);
    }
}

bool CCommon::GetNumberBit(unsigned short num, int bit)
{
    return (num & (1 << bit)) != 0;
}

std::string CCommon::GetTextResourceRawData(UINT id)
{
    std::string res_data;
    HINSTANCE hInstance = AfxGetResourceHandle();
    HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(id), _T("TEXT"));
    if (hRsrc != NULL)
    {
        HGLOBAL hGlobal = LoadResource(NULL, hRsrc);
        if (hGlobal != NULL)
        {
            const char* pResource = static_cast<const char*>(LockResource(hGlobal));
            if (pResource != NULL)
            {
                // 获取资源大小
                DWORD dwSize = SizeofResource(hInstance, hRsrc);
                // 将资源数据存储到std::string
                return std::string(pResource, dwSize);
            }
            UnlockResource(hGlobal);
        }
    }
    return res_data;
}

wstring CCommon::GetTextResource(UINT id, CodeType code_type)
{
    string res_data = GetTextResourceRawData(id);
    wstring res_str = CCommon::StrToUnicode(res_data, code_type);
    return res_str;
}

Gdiplus::Image* CCommon::GetPngImageResource(UINT id)
{
    Gdiplus::Image* pImage = nullptr;
    HINSTANCE hInst = AfxGetResourceHandle();
    if (HRSRC hRes = ::FindResource(hInst, MAKEINTRESOURCE(id), _T("PNG")))
    {
        DWORD imageSize = ::SizeofResource(hInst, hRes);
        if (HGLOBAL hResData = ::LoadResource(hInst, hRes))
        {
            LPVOID pResourceData = ::LockResource(hResData);
#ifdef COMPILE_IN_WIN_XP
            if (HGLOBAL hBuffer = ::GlobalAlloc(GMEM_FIXED, imageSize))
            {
                ::CopyMemory(hBuffer, pResourceData, imageSize);
                IStream* pStream = nullptr;
                if (SUCCEEDED(::CreateStreamOnHGlobal(hBuffer, TRUE, &pStream)))
                {
                    pImage = Gdiplus::Image::FromStream(pStream);
                    pStream->Release();
                }
                ::GlobalFree(hBuffer);  // 释放内存句柄
            }
#else       // 在缓冲区上创建内存流
            if (IStream* pStream = SHCreateMemStream(static_cast<const BYTE*>(pResourceData), imageSize))
            {
                pImage = Gdiplus::Image::FromStream(pStream);
                pStream->Release();
            }
#endif // COMPILE_IN_WIN_XP
            ::FreeResource(hResData);
        }
    }
    return pImage;
}


string CCommon::GetPngImageResourceData(UINT id)
{
    string data;
    HINSTANCE hInst = AfxGetResourceHandle();
    if (HRSRC hRes = ::FindResource(hInst, MAKEINTRESOURCE(id), _T("PNG")))
    {
        DWORD imageSize = ::SizeofResource(hInst, hRes);
        if (HGLOBAL hResData = ::LoadResource(hInst, hRes))
        {
            LPVOID pResourceData = ::LockResource(hResData);
            data = string(static_cast<const char*>(pResourceData), imageSize);
            ::FreeResource(hResData);
        }
    }
    return data;
}

int CCommon::Random(int min, int max)
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> dis(min, max - 1);
    int dis_{ dis(engine) };
    return dis_;
}

CString CCommon::GetDesktopBackgroundPath()
{
    CString path;
    CRegKey key;
    if (key.Open(HKEY_CURRENT_USER, _T("Control Panel\\Desktop\\")) == ERROR_SUCCESS)
    {
        TCHAR buff[MAX_PATH]{};
        ULONG size{ MAX_PATH };
        key.QueryStringValue(_T("WallPaper"), buff, &size);
        path = buff;
    }
    return path;
}

POINT CCommon::CalculateWindowMoveOffset(CRect& check_rect, vector<CRect>& screen_rects)
{
    POINT mov{};                                                        // 所需偏移量
    // 确保窗口在一个监视器内并且可见，判断移动距离并向所需移动距离较小的方向移动
    LONG mov_xy = 0;                                                    // 记录移动距离
    for (auto& a : screen_rects)
    {
        LONG x = 0, y = 0;
        if (check_rect.left < a.left || check_rect.Width() > a.Width()) // 需要向右移动
            x = a.left - check_rect.left;
        else if (check_rect.right > a.right)                            // 需要向左移动
            x = a.right - check_rect.right;
        if (check_rect.top < a.top || check_rect.Height() > a.Height()) // 需要向下移动
            y = a.top - check_rect.top;
        else if (check_rect.bottom > a.bottom)                          // 需要向上移动
            y = a.bottom - check_rect.bottom;
        if (x == 0 && y == 0)                                           // 窗口已在一个监视器内
        {
            mov.x = 0;
            mov.y = 0;
            break;
        }
        else if (abs(x) + abs(y) < mov_xy || mov_xy == 0)
        {
            mov.x = x;
            mov.y = y;
            mov_xy = abs(x) + abs(y);
        }
    }
    return mov;
}

void CCommon::GetLastCompileTime(wstring& time_str, wstring& hash_str)
{
    wstring compile_time = GetTextResource(IDR_COMPILE_TIME, CodeType::ANSI);
    size_t pos = compile_time.find(L"\r\n");
    time_str = compile_time.substr(0, pos);
    if (compile_time.size() > pos + 10)                 // 如果hash存在
        hash_str = compile_time.substr(pos + 2, 8);     // 截取hash前8位
}

unsigned __int64 CCommon::GetCurTimeElapse()
{
    SYSTEMTIME sys_time;
    GetLocalTime(&sys_time);
    CTime cur_time(sys_time);
    unsigned __int64 c_time = cur_time.GetTime();
    static unsigned __int64 last_time{};
    // 此处用作排序时间戳，需要避免重复
    last_time = (last_time < c_time) ? c_time : (last_time + 1);
    return last_time;
}

wstring CCommon::EncodeURIComponent(wstring uri) {
    StrReplace(uri, L"%", L"%25");
    StrReplace(uri, L":", L"%3A");
    StrReplace(uri, L"/", L"%2F");
    StrReplace(uri, L"?", L"%3F");
    StrReplace(uri, L"#", L"%23");
    StrReplace(uri, L"[", L"%5B");
    StrReplace(uri, L"]", L"%5D");
    StrReplace(uri, L"@", L"%40");
    StrReplace(uri, L"$", L"%24");
    StrReplace(uri, L"&", L"%26");
    StrReplace(uri, L"+", L"%2B");
    StrReplace(uri, L",", L"%2C");
    StrReplace(uri, L";", L"%3B");
    StrReplace(uri, L"=", L"%3D");
    return uri;
}

void CCommon::OutputDebugStringFormat(LPCTSTR str, ...)
{
    va_list args;
    va_start(args, str);
    TCHAR buf[1024] = { 0 };
    StringCchVPrintf(buf, 1023, str, args);
    va_end(args);
    buf[1023] = L'\0';
    OutputDebugString(buf);
}

wstring CCommon::StrReplace(wstring& input, wstring pattern, wstring new_content) {
    auto loc = input.find(pattern, 0);
    auto len = pattern.length();
    auto len2 = new_content.length();
    while (loc != -1) {
        input.replace(loc, len, new_content);
        if (loc + len2 < input.length()) loc = input.find(pattern, max(0, loc + len2));
        else break;
    }
    return input;
}

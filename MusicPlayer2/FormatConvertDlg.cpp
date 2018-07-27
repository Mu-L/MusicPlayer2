// FormatConvertDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "FormatConvertDlg.h"
#include "afxdialogex.h"

CBASSEncodeLibrary CFormatConvertDlg::m_bass_encode_lib;
CBASSWmaLibrary CFormatConvertDlg::m_bass_wma_lib;

// CFormatConvertDlg 对话框

IMPLEMENT_DYNAMIC(CFormatConvertDlg, CDialog)

CFormatConvertDlg::CFormatConvertDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_FORMAT_CONVERT_DIALOG, pParent)
{
}

CFormatConvertDlg::CFormatConvertDlg(const vector<int>& items_selected, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_FORMAT_CONVERT_DIALOG, pParent)
{
	//获取文件列表
	for (const auto& index : items_selected)
	{
		CTagEditDlg::ItemInfo item;
		const SongInfo& song{ theApp.m_player.GetPlayList()[index] };
		item.song_info = song;
		item.file_path = theApp.m_player.GetCurrentDir() + song.file_name;
		CAudioCommon::ClearDefaultTagStr(item.song_info);
		if(!CCommon::IsItemInVector(m_file_list, item))
			m_file_list.push_back(item);
	}
}

CFormatConvertDlg::~CFormatConvertDlg()
{
	m_bass_encode_lib.UnInit();
	m_bass_wma_lib.UnInit();
}

void CFormatConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SONG_LIST1, m_file_list_ctrl);
	DDX_Control(pDX, IDC_OUT_FORMAT_COMBO, m_encode_format_combo);
}

void CFormatConvertDlg::LoadConfig()
{
	CIniHelper ini;
	ini.SetPath(theApp.m_config_path);
	m_encode_format = static_cast<EncodeFormat>(ini.GetInt(L"format_convert", L"encode_format", 1));
	m_write_tag = ini.GetBool(L"format_convert", L"write_tag", true);
	m_write_album_cover = ini.GetBool(L"format_convert", L"write_album_cover", true);
	m_file_exist_action = ini.GetInt(L"format_convert", L"file_exist_action", 0);
	m_add_file_serial_num = ini.GetBool(L"format_convert", L"add_file_serial_num", false);
	m_out_dir = ini.GetString(L"format_convert", L"out_dir", L"");
}

void CFormatConvertDlg::SaveConfig() const
{
	CIniHelper ini;
	ini.SetPath(theApp.m_config_path);
	ini.WriteInt(L"format_convert", L"encode_format", static_cast<int>(m_encode_format));
	ini.WriteBool(L"format_convert", L"write_tag", m_write_tag);
	ini.WriteBool(L"format_convert", L"write_album_cover", m_write_album_cover);
	ini.WriteInt(L"format_convert", L"file_exist_action", m_file_exist_action);
	ini.WriteBool(L"format_convert", L"add_file_serial_num", m_add_file_serial_num);
	ini.WriteString(L"format_convert", L"out_dir", m_out_dir);
}

void CFormatConvertDlg::LoadEncoderConfig()
{
	CIniHelper ini;
	ini.SetPath(theApp.m_local_dir + L"Encoder\\encoder.ini");

	m_mp3_encode_para.encode_type = ini.GetInt(L"mp3_encoder", L"encode_type", 0);
	m_mp3_encode_para.cbr_bitrate = ini.GetString(L"mp3_encoder", L"cbr_bitrate", L"128");
	m_mp3_encode_para.abr_bitrate = ini.GetString(L"mp3_encoder", L"abr_bitrate", L"128");
	m_mp3_encode_para.vbr_quality = ini.GetInt(L"mp3_encoder", L"vbr_quality", 4);
	m_mp3_encode_para.cmd_para = ini.GetString(L"mp3_encoder", L"cmd_para", L"");
	m_mp3_encode_para.joint_stereo = ini.GetBool(L"mp3_encoder", L"joint_stereo", true);
	m_wma_encode_para.cbr = ini.GetBool(L"wma_encoder", L"cbr", true);
	m_wma_encode_para.cbr_bitrate = ini.GetInt(L"wma_encoder", L"cbr_bitrate", 64);
	m_wma_encode_para.vbr_quality = ini.GetInt(L"wma_encoder", L"vbr_quality", 75);
	m_ogg_encode_quality = ini.GetInt(L"ogg_encoder", L"quality", 3);
}

void CFormatConvertDlg::SaveEncoderConfig() const
{
	CIniHelper ini;
	ini.SetPath(theApp.m_local_dir + L"Encoder\\encoder.ini");

	ini.WriteInt(L"mp3_encoder", L"encode_type", m_mp3_encode_para.encode_type);
	ini.WriteString(L"mp3_encoder", L"cbr_bitrate", m_mp3_encode_para.cbr_bitrate);
	ini.WriteString(L"mp3_encoder", L"abr_bitrate", m_mp3_encode_para.abr_bitrate);
	ini.WriteInt(L"mp3_encoder", L"vbr_quality", m_mp3_encode_para.vbr_quality);
	ini.WriteString(L"mp3_encoder", L"cmd_para", m_mp3_encode_para.cmd_para);
	ini.WriteBool(L"mp3_encoder", L"joint_stereo", m_mp3_encode_para.joint_stereo);
	ini.WriteBool(L"wma_encoder", L"cbr", m_wma_encode_para.cbr);
	ini.WriteInt(L"wma_encoder", L"cbr_bitrate", m_wma_encode_para.cbr_bitrate);
	ini.WriteInt(L"wma_encoder", L"vbr_quality", m_wma_encode_para.vbr_quality);
	ini.WriteInt(L"ogg_encoder", L"quality", m_ogg_encode_quality);

}


BEGIN_MESSAGE_MAP(CFormatConvertDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_OUT_FORMAT_COMBO, &CFormatConvertDlg::OnCbnSelchangeOutFormatCombo)
	ON_BN_CLICKED(IDC_START_CONVERT_BUTTON, &CFormatConvertDlg::OnBnClickedStartConvertButton)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CFormatConvertDlg::OnBnClickedBrowseButton)
	ON_MESSAGE(WM_CONVERT_PROGRESS, &CFormatConvertDlg::OnConvertProgress)
	ON_MESSAGE(WM_CONVERT_COMPLETE, &CFormatConvertDlg::OnConvertComplete)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ENCODER_CONFIG_BUTTON, &CFormatConvertDlg::OnBnClickedEncoderConfigButton)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_COPY_TAG_CHECK, &CFormatConvertDlg::OnBnClickedCopyTagCheck)
	ON_BN_CLICKED(IDC_COPY_ALBUM_COVER_CHECK, &CFormatConvertDlg::OnBnClickedCopyAlbumCoverCheck)
	ON_CBN_SELCHANGE(IDC_TARGET_FILE_EXIST_COMBO, &CFormatConvertDlg::OnCbnSelchangeTargetFileExistCombo)
	ON_BN_CLICKED(IDC_ADD_NUMBER_CHECK, &CFormatConvertDlg::OnBnClickedAddNumberCheck)
	ON_NOTIFY(NM_RCLICK, IDC_SONG_LIST1, &CFormatConvertDlg::OnNMRClickSongList1)
	ON_COMMAND(ID_ADD_FILE, &CFormatConvertDlg::OnAddFile)
	ON_COMMAND(ID_DELETE_SELECT, &CFormatConvertDlg::OnDeleteSelect)
	ON_COMMAND(ID_CLEAR_LIST, &CFormatConvertDlg::OnClearList)
	ON_COMMAND(ID_MOVE_UP, &CFormatConvertDlg::OnMoveUp)
	ON_COMMAND(ID_MOVE_DOWN, &CFormatConvertDlg::OnMoveDown)
	ON_COMMAND(ID_EDIT_TAG_INFO, &CFormatConvertDlg::OnEditTagInfo)
	ON_WM_INITMENU()
END_MESSAGE_MAP()


// CFormatConvertDlg 消息处理程序


BOOL CFormatConvertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CenterWindow();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);		// 设置小图标

	LoadConfig();
	LoadEncoderConfig();

	//获取初始时窗口的大小
	CRect rect;
	GetWindowRect(rect);
	m_min_size.cx = rect.Width();
	m_min_size.cy = rect.Height();

	m_list_popup_menu.LoadMenu(IDR_FORMAT_CONVERT_POPUP_MENU);

	//初始化文件列表
	m_file_list_ctrl.GetClientRect(rect);
	int width0{ DPI(40) }, width1, width2{ DPI(65) };
	width1 = rect.Width() - width0 - width2 - DPI(21);
	//插入列
	m_file_list_ctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
	m_file_list_ctrl.InsertColumn(0, _T("序号"), LVCFMT_LEFT, width0);		//插入第1列
	m_file_list_ctrl.InsertColumn(1, _T("文件名"), LVCFMT_LEFT, width1);		//插入第2列
	m_file_list_ctrl.InsertColumn(2, _T("转换状态"), LVCFMT_LEFT, width2);		//插入第3列
	//插入项目
	ShowFileList();
	//设置主题颜色
	m_file_list_ctrl.SetColor(theApp.m_app_setting_data.theme_color);

	//初始化转换格式的下拉列表
	m_encode_format_combo.AddString(_T("WAV"));
	m_encode_format_combo.AddString(_T("MP3 (lame编码器)"));
	m_encode_format_combo.AddString(_T("WMA"));
	m_encode_format_combo.AddString(_T("OGG"));
	m_encode_format_combo.SetCurSel(static_cast<int>(m_encode_format));

	//初始化选项控件的状态
	((CButton*)GetDlgItem(IDC_COPY_TAG_CHECK))->SetCheck(m_write_tag);
	((CButton*)GetDlgItem(IDC_COPY_ALBUM_COVER_CHECK))->SetCheck(m_write_album_cover);
	((CButton*)GetDlgItem(IDC_ADD_NUMBER_CHECK))->SetCheck(m_add_file_serial_num);
	CComboBox* file_exist_combo = (CComboBox*)GetDlgItem(IDC_TARGET_FILE_EXIST_COMBO);
	file_exist_combo->AddString(_T("自动重命名"));
	file_exist_combo->AddString(_T("忽略"));
	file_exist_combo->AddString(_T("覆盖"));
	file_exist_combo->SetCurSel(m_file_exist_action);

	if (!m_out_dir.empty() && m_out_dir.back() != L'\\')
		m_out_dir.push_back(L'\\');
	SetDlgItemText(IDC_OUT_DIR_EDIT, m_out_dir.c_str());

	m_encoder_successed = InitEncoder();

#ifndef COMPILE_IN_WIN_XP
	CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTaskbar));	//创建ITaskbarList3的实例
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CFormatConvertDlg::EnableControls(bool enable)
{
	GetDlgItem(IDC_OUT_FORMAT_COMBO)->EnableWindow(enable);
	GetDlgItem(IDC_BROWSE_BUTTON)->EnableWindow(enable);
	GetDlgItem(IDC_START_CONVERT_BUTTON)->EnableWindow(enable);
	GetDlgItem(IDC_ENCODER_CONFIG_BUTTON)->EnableWindow(enable);
	//GetDlgItem(IDC_ADD_BUTTON)->EnableWindow(enable);
	//GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(enable);
	//GetDlgItem(IDC_CLEAR_BUTTON)->EnableWindow(enable);
	//GetDlgItem(IDC_MOVE_UP_BUTTON)->EnableWindow(enable);
	//GetDlgItem(IDC_MOVE_DOWN_BUTTON)->EnableWindow(enable);
	GetDlgItem(IDC_COPY_TAG_CHECK)->EnableWindow(enable);
	GetDlgItem(IDC_TARGET_FILE_EXIST_COMBO)->EnableWindow(enable);
	GetDlgItem(IDC_COPY_ALBUM_COVER_CHECK)->EnableWindow(enable);
	GetDlgItem(IDC_ADD_NUMBER_CHECK)->EnableWindow(enable);
}

void CFormatConvertDlg::ShowFileList()
{
	m_file_list_ctrl.DeleteAllItems();
	for (size_t i{}; i<m_file_list.size(); i++)
	{
		CString tmp;
		tmp.Format(_T("%d"), i + 1);
		m_file_list_ctrl.InsertItem(i, tmp);
		CFilePathHelper file_path(m_file_list[i].file_path);
		m_file_list_ctrl.SetItemText(i, 1, file_path.GetFileName().c_str());
	}
}

bool CFormatConvertDlg::InitEncoder()
{
	//初始化bass encode库
	wstring encode_dll_path;
	wstring wma_dll_path;
	wstring plugin_dir;
	m_encode_dir = theApp.m_local_dir + L"Encoder\\";
	plugin_dir = theApp.m_local_dir + L"Plugins\\";
	encode_dll_path = m_encode_dir + L"bassenc.dll";
	m_bass_encode_lib.Init(encode_dll_path);

	wma_dll_path = plugin_dir + L"basswma.dll";
	m_bass_wma_lib.Init(wma_dll_path);

	return m_bass_encode_lib.IsSuccessed();
}

bool CFormatConvertDlg::EncodeSingleFile(CFormatConvertDlg* pthis, int file_index)
{
	//设置输出文件路径
	SongInfo& song_info{ pthis->m_file_list[file_index].song_info };
	const wstring& file_path{ pthis->m_file_list[file_index].file_path };
	CFilePathHelper c_file_path(file_path);
	switch (pthis->m_encode_format)
	{
	case EncodeFormat::WAV:
		c_file_path.ReplaceFileExtension(L"wav");
		break;
	case EncodeFormat::MP3:
		c_file_path.ReplaceFileExtension(L"mp3");
		break;
	case EncodeFormat::WMA:
		c_file_path.ReplaceFileExtension(L"wma");
		break;
	case EncodeFormat::OGG:
		c_file_path.ReplaceFileExtension(L"ogg");
		break;
	}
	//为目标文件添加序号
	wstring out_file_path{ pthis->m_out_dir };
	if (pthis->m_add_file_serial_num)
	{
		CString index_str;
		if (pthis->m_file_list.size() < 10)
			index_str.Format(_T("%d."), file_index + 1);
		else if (pthis->m_file_list.size() < 10)
			index_str.Format(_T("%2d."), file_index + 1);
		else
			index_str.Format(_T("%3d."), file_index + 1);
		out_file_path += index_str;
	}
	out_file_path += c_file_path.GetFileName();
	//判断目标文件是否存在
	if (pthis->m_file_exist_action == 0)		//如果设置了“目标文件存在时自动重命名”，自动在文件名后面添加形如“ (数字)”的编号
	{
		while (CCommon::FileExist(out_file_path))
		{
			//判断文件名的末尾是否符合“(数字)”的形式
			wstring file_name;		//文件名（不含扩展名）
			CFilePathHelper c_file_path(out_file_path);
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
			out_file_path = pthis->m_out_dir + file_name + L'.' + ext;
		}
	}
	else if (pthis->m_file_exist_action == 1)		//如果设置了“目标文件存在时忽略”
	{
		if (CCommon::FileExist(out_file_path))
		{
			::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, 102);
			return false;
		}
	}

	//创建解码通道
	const int BUFF_SIZE{ 20000 };
	char buff[BUFF_SIZE];
	HSTREAM hStream = BASS_StreamCreateFile(FALSE, file_path.c_str(), 0, 0, BASS_STREAM_DECODE/* | BASS_SAMPLE_FLOAT*/);
	if (hStream == 0)
	{
		::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, -1);
		return false;
	}
	//获取通道信息
	BASS_CHANNELINFO channel_info;
	BASS_ChannelGetInfo(hStream, &channel_info);
	bool is_midi = (CAudioCommon::GetAudioTypeByBassChannel(channel_info.ctype) == AU_MIDI);
	if (theApp.m_player.m_bass_midi_lib.IsSuccessed() && is_midi && theApp.m_player.GetSoundFont().font != 0)
		theApp.m_player.m_bass_midi_lib.BASS_MIDI_StreamSetFonts(hStream, &theApp.m_player.GetSoundFont(), 1);
	if (is_midi)
	{
		//如果被转换的文件是 MIDI 音乐，则把SF2音色库名称保存到注释信息
		CString comment;
		comment.Format(_T("Converted from MIDI by MusicPlayer2. SF2: %s"), theApp.m_player.GetSoundFontName().c_str());
		song_info.comment = comment;
	}
	//获取流的长度
	QWORD length = BASS_ChannelGetLength(hStream, 0);
	if (length == 0) length = 1;	//防止length作为除数时为0

	HENCODE hEncode;
	if (pthis->m_encode_format != EncodeFormat::WMA)
	{
		//设置解码器命令行参数
		wstring cmdline;
		switch (pthis->m_encode_format)
		{
		case EncodeFormat::MP3:
		{
			//获取源文件的专辑封面
			int album_cover_size{};
			bool write_album_cover{};
			wstring album_cover_path;
			if (pthis->m_write_album_cover)
			{
				SongInfo song_info_tmp;
				CAudioTag audio_tag(hStream, file_path, song_info_tmp);
				int cover_type;
				album_cover_path = audio_tag.GetAlbumCover(cover_type, ALBUM_COVER_NAME_ENCODE);
				CImage image;
				image.Load(album_cover_path.c_str());
				if (image.IsNull())		//如果没有内嵌的专辑封面，则获取外部封面
					album_cover_path = CPlayer::GetRelatedAlbumCover(file_path, song_info);
				else
					image.Destroy();
				album_cover_size = CCommon::GetFileSize(album_cover_path);
				write_album_cover = (!album_cover_path.empty() && album_cover_size > 0 && album_cover_size < 128 * 1024);	//是否要写入专辑封面（专辑封面文件必须小于128KB）
			}

			//设置lame命令行参数
			cmdline = pthis->m_encode_dir;
			cmdline += L"lame.exe ";
			cmdline += pthis->m_mp3_encode_para.cmd_para;
			CString str;
			if (pthis->m_write_tag)
			{
				str.Format(_T(" --tt \"%s\" --ta \"%s\" --tl \"%s\" --ty \"%s\" --tc \"%s\" --tn \"%s\" --tg \"%s\""),
					song_info.title.c_str(), song_info.artist.c_str(), song_info.album.c_str(), song_info.year.c_str(), song_info.comment.c_str(),
					CAudioCommon::TrackToString(song_info.track).GetString(), song_info.genre.c_str());
				cmdline += str;
			}
			if (write_album_cover)
			{
				str.Format(_T(" --ti \"%s\""), album_cover_path.c_str());
				cmdline += str;
			}
			if(pthis->m_write_tag || write_album_cover)
				cmdline += L" --add-id3v2";
			cmdline += L" - \"";
			cmdline += out_file_path;
			cmdline.push_back(L'\"');
		}
			break;
		case EncodeFormat::OGG:
		{
			cmdline = pthis->m_encode_dir;
			CString str;
			str.Format(_T("oggenc -q %d"), pthis->m_ogg_encode_quality);
			cmdline += str;
			if (pthis->m_write_tag)
			{
				str.Format(_T(" -t \"%s\" -a \"%s\" -l \"%s\" -N \"%s\""),
					song_info.title.c_str(), song_info.artist.c_str(), song_info.album.c_str(), CAudioCommon::TrackToString(song_info.track).GetString());
				cmdline += str;
			}
			cmdline += L" -o \"";
			cmdline += out_file_path;
			cmdline += L"\" -";
		}
			break;
		default:
			cmdline = out_file_path;
		}
		//开始解码
		hEncode = m_bass_encode_lib.BASS_Encode_StartW(hStream, cmdline.c_str(), BASS_ENCODE_AUTOFREE | (pthis->m_encode_format == EncodeFormat::WAV ? BASS_ENCODE_PCM : 0), NULL, 0);
		if (hEncode == 0)
		{
			BASS_StreamFree(hStream);
			::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, -2);
			return false;
		}
		while (BASS_ChannelIsActive(hStream) != 0)
		{
			if (theApp.m_format_convert_dialog_exit)
			{
				m_bass_encode_lib.BASS_Encode_Stop(hEncode);
				BASS_StreamFree(hStream);
				return false;
			}

			BASS_ChannelGetData(hStream, buff, BUFF_SIZE);
			if (m_bass_encode_lib.BASS_Encode_IsActive(hStream) == 0)
			{
				m_bass_encode_lib.BASS_Encode_Stop(hEncode);
				BASS_StreamFree(hStream);
			}

			//获取转换百分比
			QWORD position = BASS_ChannelGetPosition(hStream, 0);
			static int last_percent{ -1 };
			int percent = static_cast<int>(position * 100 / length);
			if (percent < 0 || percent>100)
			{
				::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, -3);
				BASS_StreamFree(hStream);
				return false;
			}
			if (last_percent != percent)
				::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, percent);
			last_percent = percent;
		}
	}
	else		//输出为WMA时使用专用的编码器
	{
		if(!m_bass_wma_lib.IsSuccessed())
		{
			BASS_StreamFree(hStream);
			return false;
		}
		//开始解码
		int wma_bitrate;
		if (pthis->m_wma_encode_para.cbr)
			wma_bitrate = pthis->m_wma_encode_para.cbr_bitrate * 1000;
		else
			wma_bitrate = pthis->m_wma_encode_para.vbr_quality;
		hEncode = m_bass_wma_lib.BASS_WMA_EncodeOpenFileW(hStream, NULL, BASS_WMA_ENCODE_STANDARD | BASS_WMA_ENCODE_SOURCE, wma_bitrate, out_file_path.c_str());
		int error = BASS_ErrorGetCode();
		//如果出现了错误，则写入错误日志
		if (error != 0)
		{
			CString log_str;
			log_str.Format(_T("转换 “%s” 到 WMA 出错："), file_path.c_str());
			switch (error)
			{
			case BASS_ERROR_WMA: log_str += L"没有安装 Windows Media Player 9 或更高版本。"; break;
			case BASS_ERROR_NOTAVAIL: log_str += L"无法找到可支持请求的采样格式和比特率的编解码器。"; break;
			default: log_str += L"未知错误。"; break;
			}
			CCommon::WriteLog((theApp.m_module_dir + L"error.log").c_str(), wstring(log_str));
		}
		if (hEncode == 0)
		{
			BASS_StreamFree(hStream);
			::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, -2);
			return false;
		}
		//写入WMA标签
		if (pthis->m_write_tag)
		{
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"Title", song_info.title.c_str());
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"Author", song_info.artist.c_str());
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"WM/AlbumTitle", song_info.album.c_str());
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"WM/Year", song_info.year.c_str());
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"WM/TrackNumber", CAudioCommon::TrackToString(song_info.track).GetString());
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"WM/Genre", song_info.genre.c_str());
			m_bass_wma_lib.BASS_WMA_EncodeSetTagW(hEncode, L"Description", song_info.comment.c_str());
		}

		while (BASS_ChannelIsActive(hStream) != BASS_ACTIVE_STOPPED)
		{
			if (theApp.m_format_convert_dialog_exit)
			{
				m_bass_encode_lib.BASS_Encode_Stop(hEncode);
				BASS_StreamFree(hStream);
				return false;
			}
			//if (m_bass_wma_lib.BASS_WMA_EncodeWrite(hEncode, buff, BASS_ChannelGetData(hStream, buff, BUFF_SIZE)) == FALSE)
			//	break;
			BASS_ChannelGetData(hStream, buff, BUFF_SIZE);
			//获取转换百分比
			QWORD position = BASS_ChannelGetPosition(hStream, 0);
			static int last_percent{ -1 };
			int percent = static_cast<int>(position * 100 / length);
			if (percent < 0 || percent>100)
			{
				::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, -3);
				BASS_StreamFree(hStream);
				return false;
			}
			if (last_percent != percent)
				::PostMessage(pthis->GetSafeHwnd(), WM_CONVERT_PROGRESS, file_index, percent);
			last_percent = percent;
		}
	}

	BASS_StreamFree(hStream);
	return true;
}


UINT CFormatConvertDlg::ThreadFunc(LPVOID lpParam)
{
	CFormatConvertDlg* pThis{ (CFormatConvertDlg*)lpParam };
	for (size_t i{}; i < pThis->m_file_list.size(); i++)
	{
		if (theApp.m_format_convert_dialog_exit)
			return 0;
		//编码文件
		if(EncodeSingleFile(pThis, i))
			::PostMessage(pThis->GetSafeHwnd(), WM_CONVERT_PROGRESS, i, 101);
	}
	::PostMessage(pThis->GetSafeHwnd(), WM_CONVERT_COMPLETE, 0, 0);
	return 0;
}


void CFormatConvertDlg::OnCbnSelchangeOutFormatCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_encode_format = static_cast<EncodeFormat>(m_encode_format_combo.GetCurSel());
	GetDlgItem(IDC_ENCODER_CONFIG_BUTTON)->EnableWindow(m_encode_format != EncodeFormat::WAV);
}


void CFormatConvertDlg::OnBnClickedStartConvertButton()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_encoder_successed)
	{
		CString info;
		info.Format(_T("BASS 编码器加载失败，文件 %s 可能丢失或已经损坏！"), (m_encode_dir + L"bassenc.dll").c_str());
		MessageBox(info, NULL, MB_ICONERROR | MB_OK);
		return;
	}

	if (m_out_dir.empty())
	{
		MessageBox(_T("请设置输出目录！"), NULL, MB_ICONWARNING | MB_OK);
		return;
	}
	else if (!CCommon::FolderExist(m_out_dir))
	{
		CString info;
		info.Format(_T("输出目录 \"%s\" 不存在！"), m_out_dir.c_str());
		MessageBox(info, NULL, MB_ICONWARNING | MB_OK);
	}

	//先清除“状态”一列的内容
	for (size_t i{}; i < m_file_list.size(); i++)
	{
		m_file_list_ctrl.SetItemText(i, 2, _T(""));
	}

	EnableControls(false);
	theApp.m_format_convert_dialog_exit = false;
	//创建格式转换的工作线程
	m_pThread = AfxBeginThread(ThreadFunc, this);
	m_thread_runing = true;
#ifndef COMPILE_IN_WIN_XP
	m_pTaskbar->SetProgressState(this->GetSafeHwnd(), TBPF_INDETERMINATE);
#endif
}


void CFormatConvertDlg::OnBnClickedBrowseButton()
{
	// TODO: 在此添加控件通知处理程序代码
#ifdef COMPILE_IN_WIN_XP
	CFolderBrowserDlg folderPickerDlg(this->GetSafeHwnd());
	folderPickerDlg.SetInfo(_T("请选择输出文件夹。"));
#else
	CFolderPickerDialog folderPickerDlg(m_out_dir.c_str());
	folderPickerDlg.m_ofn.lpstrTitle = _T("选择输出文件夹");		//设置对话框标题
#endif // COMPILE_IN_WIN_XP
	if (folderPickerDlg.DoModal() == IDOK)
	{
		m_out_dir = folderPickerDlg.GetPathName();
		if (m_out_dir.back() != L'\\') m_out_dir.push_back(L'\\');	//确保路径末尾有反斜杠
		SetDlgItemText(IDC_OUT_DIR_EDIT, m_out_dir.c_str());
	}

}


afx_msg LRESULT CFormatConvertDlg::OnConvertProgress(WPARAM wParam, LPARAM lParam)
{
	CString percent_str;
	int percent = (int)lParam;
	if (percent == 0)
		m_file_list_ctrl.EnsureVisible(wParam, FALSE);		//转换开始时，确保当前列表项可见
	if(percent < 0)
		percent_str.Format(_T("出错 (%d)"), percent);
	else if(percent == 101)
		percent_str = _T("完成");
	else if(percent == 102)
		percent_str = _T("跳过");
	else
		percent_str.Format(_T("%d%%"), (int)lParam);
	m_file_list_ctrl.SetItemText(wParam, 2, percent_str);

#ifndef COMPILE_IN_WIN_XP
	int position, length;		//总体的进度
	length = m_file_list.size() * 100;
	position = wParam * 100 + percent;
	m_pTaskbar->SetProgressValue(this->GetSafeHwnd(), position, length);
#endif

	return 0;
}


afx_msg LRESULT CFormatConvertDlg::OnConvertComplete(WPARAM wParam, LPARAM lParam)
{
	EnableControls(true);
	m_thread_runing = false;
	return 0;
}


void CFormatConvertDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	//theApp.m_format_convert_dialog_exit = true;
	//if (m_pThread != nullptr)
	//	WaitForSingleObject(m_pThread->m_hThread, 2000);	//等待线程退出

	DestroyWindow();

	//CDialog::OnCancel();
}


void CFormatConvertDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	//theApp.m_format_convert_dialog_exit = true;
	//if (m_pThread != nullptr)
	//	WaitForSingleObject(m_pThread->m_hThread, 2000);	//等待线程退出

	CDialog::OnOK();
}


void CFormatConvertDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	theApp.m_format_convert_dialog_exit = true;
	if (m_pThread != nullptr)
		WaitForSingleObject(m_pThread->m_hThread, 2000);	//等待线程退出

	SaveConfig();
	SaveEncoderConfig();

	CDialog::OnClose();
}


void CFormatConvertDlg::OnBnClickedEncoderConfigButton()
{
	// TODO: 在此添加控件通知处理程序代码
	switch (m_encode_format)
	{
		case EncodeFormat::WAV:
			break;
		case EncodeFormat::MP3:
		{
			CMP3EncodeCfgDlg dlg;
			dlg.m_encode_para = m_mp3_encode_para;
			if(dlg.DoModal() == IDOK)
				m_mp3_encode_para = dlg.m_encode_para;
		}
			break;
		case EncodeFormat::WMA:
		{
			CWmaEncodeCfgDlg dlg;
			dlg.m_encode_para = m_wma_encode_para;
			if (dlg.DoModal() == IDOK)
				m_wma_encode_para = dlg.m_encode_para;
		}
			break;
		case EncodeFormat::OGG:
		{
			COggEncodeCfgDlg dlg;
			dlg.m_encode_quality = m_ogg_encode_quality;
			if (dlg.DoModal() == IDOK)
				m_ogg_encode_quality = dlg.m_encode_quality;
		}
			break;
		default:
			break;
	}
}

void CFormatConvertDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//限制窗口最小大小
	lpMMI->ptMinTrackSize.x = m_min_size.cx;		//设置最小宽度
	lpMMI->ptMinTrackSize.y = m_min_size.cy;		//设置最小高度

	CDialog::OnGetMinMaxInfo(lpMMI);
}


void CFormatConvertDlg::OnBnClickedCopyTagCheck()
{
	// TODO: 在此添加控件通知处理程序代码
	m_write_tag = (((CButton*)GetDlgItem(IDC_COPY_TAG_CHECK))->GetCheck() != 0);
}


void CFormatConvertDlg::OnBnClickedCopyAlbumCoverCheck()
{
	// TODO: 在此添加控件通知处理程序代码
	m_write_album_cover = (((CButton*)GetDlgItem(IDC_COPY_ALBUM_COVER_CHECK))->GetCheck() != 0);
}


void CFormatConvertDlg::OnCbnSelchangeTargetFileExistCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_file_exist_action = ((CComboBox*)GetDlgItem(IDC_TARGET_FILE_EXIST_COMBO))->GetCurSel();
}


void CFormatConvertDlg::OnBnClickedAddNumberCheck()
{
	// TODO: 在此添加控件通知处理程序代码
	m_add_file_serial_num = (((CButton*)GetDlgItem(IDC_ADD_NUMBER_CHECK))->GetCheck() != 0);
}


void CFormatConvertDlg::OnNMRClickSongList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	m_item_selected = pNMItemActivate->iItem;	//获取鼠标选中的项目
	CMenu* pContextMenu = m_list_popup_menu.GetSubMenu(0); //获取第一个弹出菜单
	CPoint point;			//定义一个用于确定光标位置的位置  
	GetCursorPos(&point);	//获取当前光标的位置，以便使得菜单可以跟随光标

	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this); //在指定位置显示弹出菜单

	*pResult = 0;
}


void CFormatConvertDlg::OnAddFile()
{
	// TODO: 在此添加命令处理程序代码
	if (m_thread_runing)
		return;
	//设置过滤器
	wstring filter(L"所有支持的音频格式|");
	for (const auto& ext : CAudioCommon::m_all_surpported_extensions)
	{
		filter += L"*.";
		filter += ext;
		filter.push_back(L';');
	}
	filter.pop_back();
	filter.push_back(L'|');
	for (const auto& format : CAudioCommon::m_surpported_format)
	{
		filter += format.description;
		filter.push_back(L'|');
		filter += format.extensions_list;
		filter.push_back(L'|');
	}
	filter += L"所有文件|*.*||";
	//构造打开文件对话框
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, filter.c_str(), this);
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
			CTagEditDlg::ItemInfo item;
			item.file_path = fileDlg.GetNextPathName(posFile);
			item.song_info = theApp.m_song_data[item.file_path];
			m_file_list.push_back(item);
		}
		ShowFileList();
	}
	delete[] ch;
}


void CFormatConvertDlg::OnDeleteSelect()
{
	// TODO: 在此添加命令处理程序代码
	if (m_thread_runing)
		return;
	int select;
	select = m_file_list_ctrl.GetCurSel();		//获取当前选中项序号
	if (select >= 0 && select < m_file_list.size())
	{
		m_file_list.erase(m_file_list.begin() + select);	//删除选中项
		ShowFileList();
	}
}


void CFormatConvertDlg::OnClearList()
{
	// TODO: 在此添加命令处理程序代码
	if (m_thread_runing)
		return;
	m_file_list.clear();
	m_file_list_ctrl.DeleteAllItems();
}


void CFormatConvertDlg::OnMoveUp()
{
	// TODO: 在此添加命令处理程序代码
	if (m_thread_runing)
		return;
	int select;
	select = m_file_list_ctrl.GetCurSel();		//获取当前选中项序号
	if (select < 0 || select >= m_file_list.size())	//当前选中项在索引以外
	{
		return;
	}
	else if (select > 0)
	{
		//交换文件列表中的选中项和前一项
		auto temp = m_file_list[select];
		m_file_list[select] = m_file_list[select - 1];
		m_file_list[select - 1] = temp;
		ShowFileList();		//上移操作完毕重新显示列表
		m_file_list_ctrl.SetCurSel(select - 1);	//设置选中项为前一项
	}
}


void CFormatConvertDlg::OnMoveDown()
{
	// TODO: 在此添加命令处理程序代码
	if (m_thread_runing)
		return;
	int select;
	select = m_file_list_ctrl.GetCurSel();		//获取当前选中项序号
	if (select < 0 || select >= m_file_list.size())	//当前选中项在索引以外
	{
		return;
	}
	else if (select < m_file_list.size() - 1)
	{
		//交换文件列表中的选中项和后一项
		auto temp = m_file_list[select];
		m_file_list[select] = m_file_list[select + 1];
		m_file_list[select + 1] = temp;
		ShowFileList();		//下移操作完毕重新显示列表
		m_file_list_ctrl.SetCurSel(select + 1);	//设置选中项为前一项
	}
}


BOOL CFormatConvertDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		//按下Ctrl键时
		if (GetKeyState(VK_CONTROL) & 0x80)
		{
			if (pMsg->wParam == VK_UP)		//按Ctr+↑上移
			{
				OnMoveUp();
				return TRUE;
			}
			if (pMsg->wParam == VK_DOWN)	//按Ctr+↓下移
			{
				OnMoveDown();
				return TRUE;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CFormatConvertDlg::OnEditTagInfo()
{
	// TODO: 在此添加命令处理程序代码
	CTagEditDlg dlg(m_file_list, m_item_selected);
	dlg.DoModal();
}


void CFormatConvertDlg::OnInitMenu(CMenu* pMenu)
{
	CDialog::OnInitMenu(pMenu);

	// TODO: 在此处添加消息处理程序代码
	bool item_selected_valid{ m_item_selected >= 0 && m_item_selected < m_file_list.size() };
	pMenu->EnableMenuItem(ID_ADD_FILE, MF_BYCOMMAND | (!m_thread_runing ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_CLEAR_LIST, MF_BYCOMMAND | (!m_thread_runing ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_EDIT_TAG_INFO, MF_BYCOMMAND | ((!m_thread_runing && item_selected_valid) ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_DELETE_SELECT, MF_BYCOMMAND | ((!m_thread_runing && item_selected_valid) ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_MOVE_UP, MF_BYCOMMAND | ((!m_thread_runing && item_selected_valid) ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_MOVE_DOWN, MF_BYCOMMAND | ((!m_thread_runing && item_selected_valid) ? MF_ENABLED : MF_GRAYED));

}

#include "stdafx.h"
#include "Button.h"
#include "Player.h"

void UiElement::Button::Draw()
{
    CalculateRect();
    switch (key)
    {
    case CPlayerUIBase::BTN_TRANSLATE:
    {
        static const wstring& btn_str = theApp.m_str_table.LoadText(L"UI_TXT_BTN_TRANSLATE");
        m_btn.enable = !CPlayer::GetInstance().m_Lyrics.IsEmpty();
        ui->DrawTextButton(rect, CPlayerUIBase::BTN_TRANSLATE, btn_str.c_str(), theApp.m_lyric_setting_data.show_translate);
    }
        break;
    case CPlayerUIBase::BTN_LRYIC:
    {
        static const wstring& btn_str = theApp.m_str_table.LoadText(L"UI_TXT_BTN_DESKTOP_LYRIC");
        ui->DrawTextButton(rect, m_btn, btn_str.c_str(), theApp.m_lyric_setting_data.show_desktop_lyric);
    }
        break;
    case CPlayerUIBase::BTN_AB_REPEAT:
    {
        CString info;
        CPlayer::ABRepeatMode ab_repeat_mode = CPlayer::GetInstance().GetABRepeatMode();
        if (ab_repeat_mode == CPlayer::AM_A_SELECTED)
            info = _T("A-");
        else
            info = _T("A-B");
        CFont* pOldFont = ui->m_draw.GetFont();
        ui->m_draw.SetFont(&theApp.m_font_set.GetFontBySize(8).GetFont(theApp.m_ui_data.full_screen));      //AB�ظ�ʹ��Сһ�����壬������ʱ�������
        m_btn.enable = (!CPlayer::GetInstance().IsError() && !CPlayer::GetInstance().IsPlaylistEmpty());
        ui->DrawTextButton(rect, m_btn, info, ab_repeat_mode != CPlayer::AM_NONE);
        ui->m_draw.SetFont(pOldFont);
    }
        break;
    case CPlayerUIBase::BTN_KARAOKE:
    {
        m_btn.enable = !CPlayer::GetInstance().m_Lyrics.IsEmpty();
        //����ǿ���OK��ʽ��ʾ��ʣ���ť��ʾΪѡ��״̬
        ui->DrawUIButton(rect, CPlayerUIBase::BTN_KARAOKE, m_btn, false, false, 9, theApp.m_lyric_setting_data.lyric_karaoke_disp);
    }
        break;
    default:
        ui->DrawUIButton(rect, key, m_btn, big_icon, show_text, font_size);
        break;
    }
    Element::Draw();
}

void UiElement::Button::FromString(const std::string& key_type)
{
    if (key_type == "menu")
        key = CPlayerUIBase::BTN_MENU;
    else if (key_type == "miniMode")
        key = CPlayerUIBase::BTN_MINI;
    else if (key_type == "miniModeClose")
        key = CPlayerUIBase::BTN_CLOSE;
    else if (key_type == "fullScreen")
        key = CPlayerUIBase::BTN_FULL_SCREEN;
    else if (key_type == "repeatMode")
        key = CPlayerUIBase::BTN_REPETEMODE;
    else if (key_type == "settings")
        key = CPlayerUIBase::BTN_SETTING;
    else if (key_type == "equalizer")
        key = CPlayerUIBase::BTN_EQ;
    else if (key_type == "skin")
        key = CPlayerUIBase::BTN_SKIN;
    else if (key_type == "info")
        key = CPlayerUIBase::BTN_INFO;
    else if (key_type == "find")
        key = CPlayerUIBase::BTN_FIND;
    else if (key_type == "abRepeat")
        key = CPlayerUIBase::BTN_AB_REPEAT;
    else if (key_type == "desktopLyric")
        key = CPlayerUIBase::BTN_LRYIC;
    else if (key_type == "lyricTranslate")
        key = CPlayerUIBase::BTN_TRANSLATE;
    else if (key_type == "stop")
        key = CPlayerUIBase::BTN_STOP;
    else if (key_type == "previous")
        key = CPlayerUIBase::BTN_PREVIOUS;
    else if (key_type == "next")
        key = CPlayerUIBase::BTN_NEXT;
    else if (key_type == "playPause")
        key = CPlayerUIBase::BTN_PLAY_PAUSE;
    else if (key_type == "favorite")
        key = CPlayerUIBase::BTN_FAVOURITE;
    else if (key_type == "mediaLib")
        key = CPlayerUIBase::BTN_MEDIA_LIB;
    else if (key_type == "showPlaylist")
        key = CPlayerUIBase::BTN_SHOW_PLAYLIST;
    else if (key_type == "addToPlaylist")
        key = CPlayerUIBase::BTN_ADD_TO_PLAYLIST;
    else if (key_type == "switchDisplay")
        key = CPlayerUIBase::BTN_SWITCH_DISPLAY;
    else if (key_type == "darkLightMode")
        key = CPlayerUIBase::BTN_DARK_LIGHT;
    else if (key_type == "locateTrack")
        key = CPlayerUIBase::BTN_LOCATE_TO_CURRENT;
    else if (key_type == "openFolder")
        key = CPlayerUIBase::BTN_OPEN_FOLDER;
    else if (key_type == "newPlaylist")
        key = CPlayerUIBase::BTN_NEW_PLAYLIST;
    else if (key_type == "playMyFavourite")
        key = CPlayerUIBase::BTN_PLAY_MY_FAVOURITE;
    else if (key_type == "medialibFolderSort")
        key = CPlayerUIBase::BTN_MEDIALIB_FOLDER_SORT;
    else if (key_type == "medialibPlaylistSort")
        key = CPlayerUIBase::BTN_MEDIALIB_PLAYLIST_SORT;
    else if (key_type == "karaoke")
        key = CPlayerUIBase::BTN_KARAOKE;
    else
        key = CPlayerUIBase::BTN_INVALID;
}

int UiElement::Button::GetMaxWidth(CRect parent_rect) const
{
    //��ʾ�ı�������û��ָ�����ʱʱ�����ı����
    if (show_text && !IsWidthValid())
    {
        std::wstring text = ui->GetButtonText(key);
        //��һ��ִ�е�����ʱ������rect��û�д�layoutԪ���м�����������������һ���жϣ�����߶�Ϊ0����ֱ�ӻ�ȡheight��ֵ
        int btn_height = rect.Height();
        if (btn_height == 0)
            btn_height = Element::height.GetValue(parent_rect);
        int right_space = (btn_height - ui->DPI(16)) / 2;

        //�����ı����ǰ������һ������
        UiFontGuard set_font(ui, font_size);

        int width_text{ ui->m_draw.GetTextExtent(text.c_str()).cx + right_space + btn_height };

        int width_max{ max_width.IsValid() ? max_width.GetValue(parent_rect) : INT_MAX };
        return min(width_text, width_max);
    }
    else
    {
        return Element::GetMaxWidth(parent_rect);
    }
}

void UiElement::Button::ClearRect()
{
    Element::ClearRect();
    ui->m_buttons[key].rect = CRect();
}

void UiElement::Button::LButtonUp(CPoint point)
{
    bool pressed = m_btn.pressed;
    m_btn.pressed = false;

    if (pressed && m_btn.rect.PtInRect(point) && m_btn.enable)
    {
        ui->ButtonClicked(key);
    }
}

void UiElement::Button::LButtonDown(CPoint point)
{
    if (m_btn.enable && m_btn.rect.PtInRect(point))
    {
        m_btn.pressed = true;
    }
}

void UiElement::Button::MouseMove(CPoint point)
{
    if (m_btn.enable)
        m_btn.hover = (m_btn.rect.PtInRect(point));
}

bool UiElement::Button::RButtunUp(CPoint point)
{
    if (m_btn.enable && m_btn.rect.PtInRect(point))
    {
        ui->ButtonRClicked(key);
    }

    return false;
}

void UiElement::Button::MouseLeave()
{
    m_btn.hover = false;
    m_btn.pressed = false;
}

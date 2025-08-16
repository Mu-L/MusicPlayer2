#pragma once
#include "UIElement/UIElement.h"
namespace UiElement
{
    class StackElement;

    //������
    class NavigationBar : public Element
    {
    public:
        virtual void Draw() override;
        virtual void LButtonUp(CPoint point) override;
        virtual void MouseMove(CPoint point) override;
        virtual bool RButtunUp(CPoint point) override;
        virtual void MouseLeave() override;

        enum IconType
        {
            ICON_AND_TEXT,
            ICON_ONLY,
            TEXT_ONLY
        };

        enum Orientation
        {
            Horizontal,
            Vertical,
        };

        IconType icon_type{};
        Orientation orientation{ Horizontal };
        int item_space{};
        int item_height{ 28 };
        int font_size{ 9 };
        std::string stack_element_id;
        std::vector<std::string> tab_list;
        std::vector<CRect> item_rects;
        std::vector<std::wstring> labels;
        int SelectedIndex();
        int hover_index{ -1 };
    private:
        void FindStackElement();        //����StackElement
        bool find_stack_element{};      //����Ѿ����ҹ�StackElement����Ϊtrue
        StackElement* stack_element{};
        int selected_index{};
        int last_hover_index{ -1 };
    };
}


#pragma once
#include "UIElement/UIElement.h"
namespace UiElement
{
    //�����л����������л�һ��StackElement����ʾ��
    class ElementSwitcher : public Element
    {
    public:
        virtual void Draw() override;
        
        //�����л�������ʽ
        enum class Style
        {
            Empty,          //�հ�
            AlbumCover,     //��ʾΪר������
            DropDownIcon,       //��ʾ������ť
        };

        virtual void LButtonUp(CPoint point) override;
        virtual void MouseMove(CPoint point) override;
        virtual void MouseLeave() override;

        Style style{};
        std::string stack_element_id;
        int stack_element_index{ -1 };
        bool hover{};       //������ָ����Ϊtrue
    };
}


#include "solidbutton.h"
#include "solidbuttontheme.h"
#include "util.h"

#include <KColorUtils>


namespace Breeze
{
    DefaultSolidButtonTheme::DefaultSolidButtonTheme(SolidButton *button)
            : SolidButtonTheme(button), m_hasSymbol(false)
    {
        switch (buttonType())
        {
            case KDecoration2::DecorationButtonType::Close:
                m_color = QColor(224, 56, 62);
                break;
            case KDecoration2::DecorationButtonType::Maximize:
                m_color = QColor(252, 184, 39);
                break;
            case KDecoration2::DecorationButtonType::Minimize:
                m_color = QColor(98, 186, 70);
                break;
            case KDecoration2::DecorationButtonType::KeepAbove:
                m_color = QColor(51, 149, 255);
                break;
            case KDecoration2::DecorationButtonType::KeepBelow:
                m_color = QColor(149, 61, 150);
                break;
            case KDecoration2::DecorationButtonType::ContextHelp:
                m_color = QColor(248, 79, 158);
                break;
            case KDecoration2::DecorationButtonType::Shade:
                m_color = QColor(247, 130, 27);
                break;
            default:
            case KDecoration2::DecorationButtonType::Menu:
            case KDecoration2::DecorationButtonType::ApplicationMenu:
            case KDecoration2::DecorationButtonType::OnAllDesktops:
                m_color = QColor(247, 130, 27);
                m_hasSymbol = true;
                break;
        }
    }

    DefaultSolidButtonTheme::DefaultSolidButtonTheme(SolidButton *button, QColor base, bool hasSymbol)
            : SolidButtonTheme(button), m_color(std::move(base)), m_hasSymbol(hasSymbol)
    {

    }

    QBrush DefaultSolidButtonTheme::backgroundColor(SolidButtonStateInfo state) const
    {
        bool active = state.isActive || state.isHovered || state.isPressed || state.isBeingAnimated;
        auto titleBarLuminance = perceptiveLuminance(state.titleBarColor);
        auto color = m_color;

        if (active)
        {
            if (state.isPressed)
                color = color.darker(105);
            else if (state.isBeingAnimated)
                color = color.lighter(105);
            else if (state.isHovered && !state.isChecked)
                color = color.lighter(110);

            if (state.isChecked)
                color = color.darker(130);

            // Set dark or light color based on title bar luminance
            if (titleBarLuminance <= 0.5)
                return color.lighter(115);
            else
                return color;
        }
        else
        {
            color = inactiveGrayFrom(state.titleBarColor);
            color = KColorUtils::tint(color, state.titleBarColor);

            if (titleBarLuminance <= 0.5)
                color = color.lighter(130);

            if (state.isChecked)
                return color.darker(135);
            else
                return color;
        }
    }

    QBrush DefaultSolidButtonTheme::borderColor(SolidButtonStateInfo state) const
    {
        auto color = backgroundColor(state).color();
        auto titleBarLuminance = perceptiveLuminance(state.titleBarColor);

        return titleBarLuminance > 0.5 ? color.darker(130) : color.lighter(130);
    }

    QBrush DefaultSolidButtonTheme::symbolColor(SolidButtonStateInfo state) const
    {
        if (!state.isActive && !state.isHovered && !state.isPressed && !state.isBeingAnimated)
            return backgroundColor(state).color().lighter(150);
        return state.titleBarColor;
    }

    bool DefaultSolidButtonTheme::showBackground(SolidButtonStateInfo state) const
    {
        Q_UNUSED(state)
        return true;
    }

    bool DefaultSolidButtonTheme::showBorder(SolidButtonStateInfo state) const
    {
        return !state.isPressed;
    }

    bool DefaultSolidButtonTheme::showSymbol(SolidButtonStateInfo state) const
    {
        if (!m_hasSymbol)
            return false;

        if (state.isPressed && !state.isBeingAnimated)
            return false;

        return state.isHovered || (state.isChecked && state.isActive) || state.isBeingAnimated;
    }
}
#ifndef SOLID_BUTTON_THEME_H
#define SOLID_BUTTON_THEME_H

/*
 * Copyright 2020  Alejandro Romero Rivera <k1r0d3v@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "solidbuttonstateinfo.h"


namespace Breeze
{
    class SolidButtonTheme
    {
    public:
        explicit SolidButtonTheme(SolidButton *button) : m_button(button)
        {
            Q_ASSERT(m_button);
        }

        virtual ~SolidButtonTheme() = default;

        virtual QBrush backgroundColor(SolidButtonStateInfo state) const = 0;

        virtual bool showBackground(SolidButtonStateInfo state) const = 0;

        virtual QBrush borderColor(SolidButtonStateInfo state) const = 0;

        virtual bool showBorder(SolidButtonStateInfo state) const = 0;

        virtual QBrush symbolColor(SolidButtonStateInfo state) const = 0;

        virtual bool showSymbol(SolidButtonStateInfo state) const = 0;

        KDecoration2::DecorationButtonType buttonType() const
        {
            return m_button->type();
        }

    private:
        SolidButton *m_button;
    };

    class DefaultSolidButtonTheme : public SolidButtonTheme
    {
    public:
        explicit DefaultSolidButtonTheme(SolidButton *button);

        explicit DefaultSolidButtonTheme(SolidButton *button, QColor base, bool hasSymbol);

        QBrush backgroundColor(SolidButtonStateInfo state) const override;

        bool showBackground(SolidButtonStateInfo state) const override;

        QBrush borderColor(SolidButtonStateInfo state) const override;

        bool showBorder(SolidButtonStateInfo state) const override;

        QBrush symbolColor(SolidButtonStateInfo state) const override;

        bool showSymbol(SolidButtonStateInfo state) const override;

    private:
        QColor m_color;
        bool m_hasSymbol;
    };
}

#endif

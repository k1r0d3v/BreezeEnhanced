#ifndef SOLID_BUTTONS_H
#define SOLID_BUTTONS_H

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

#include "solidbutton.h"

namespace Breeze
{
    class CloseButton : public Breeze::SolidButton
    {
    public:
        CloseButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };

    class MaximizeButton : public Breeze::SolidButton
    {
    public:
        MaximizeButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };

    class MinimizeButton : public Breeze::SolidButton
    {
    protected:
    public:
        MinimizeButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };

    class KeepAboveButton : public Breeze::SolidButton
    {
    public:
        KeepAboveButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };

    class KeepBelowButton : public Breeze::SolidButton
    {
    public:
        KeepBelowButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };

    class ApplicationMenuButton : public Breeze::SolidButton
    {
    public:
        ApplicationMenuButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                              QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };

    class MenuButton : public Breeze::SolidButton
    {
    public:
        MenuButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                              QObject *parent);

    protected:
        void drawSymbol(QPainter *painter) override;
    };
}

#endif

#ifndef BUTTON_BASE_H
#define BUTTON_BASE_H

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

#include <QRectF>
#include <QSize>
#include <QVariantList>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButton>


namespace Breeze
{
    enum class ButtonFlag
    {
        FlagNone,
        FlagStandalone,
        FlagFirstInList,
        FlagLastInList
    };

    class ButtonBase : public KDecoration2::DecorationButton
    {
    protected:
        explicit ButtonBase(KDecoration2::DecorationButtonType type, KDecoration2::Decoration* decoration, QObject* parent)
            : KDecoration2::DecorationButton(type, decoration, parent)
        {

        }

        void hoverEnterEvent(QHoverEvent *event) override { KDecoration2::DecorationButton::hoverEnterEvent(event); }
        void hoverLeaveEvent(QHoverEvent *event) override { KDecoration2::DecorationButton::hoverLeaveEvent(event); }
        void hoverMoveEvent(QHoverEvent *event) override { KDecoration2::DecorationButton::hoverMoveEvent(event); }
        void mouseMoveEvent(QMouseEvent *event) override { KDecoration2::DecorationButton::mouseMoveEvent(event); }
        void mousePressEvent(QMouseEvent *event) override { KDecoration2::DecorationButton::mousePressEvent(event); }
        void mouseReleaseEvent(QMouseEvent *event) override { KDecoration2::DecorationButton::mouseReleaseEvent(event); }
        void wheelEvent(QWheelEvent *event) override { KDecoration2::DecorationButton::wheelEvent(event); }

    public:
        ~ButtonBase() override = default;

        virtual ButtonFlag getFlag() const = 0;
        virtual void setFlag(ButtonFlag value) = 0;
        virtual QPointF getOffset() const = 0;
        virtual void setOffset(const QPointF& value) = 0;
        virtual QSize getIconSize() const = 0;
        virtual void setIconSize(const QSize& value) = 0;
    };
}

#endif

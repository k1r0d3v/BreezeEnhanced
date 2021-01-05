#ifndef SOLID_BUTTON_H
#define SOLID_BUTTON_H

/*
* Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
* Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
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

#include "buttonbase.h"
#include "solidbuttonstateinfo.h"

#include <KDecoration2/DecorationButton>
#include <QBrush>
#include <QPen>
#include <QVariant>
#include <utility>


class QVariantAnimation;

namespace Breeze
{
    class SolidButtonTheme;
    class SolidButton : public Breeze::ButtonBase
    {
        Q_OBJECT
    public:
        /**
         * Constructor with type, decoration and parent arguments.
         */
        explicit SolidButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent = nullptr);

        /**
         * Constructor with type, decoration and parent arguments.
         * The type and decoration arguments are given in args QVariantList at indices 0 and 1 respectively.
         */
        explicit SolidButton(QObject *parent, const QVariantList &args);

        /**
         * Default destructor
         */
        ~SolidButton() override = default;

        /**
         * Set the button flag
         */
        void setFlag(ButtonFlag value) override
        {
            m_flag = value;
        }

        /**
         * Gets the button flag
         */
        ButtonFlag getFlag() const override
        {
            return m_flag;
        }

        /**
         * @return The button offset for rendering
         */
        QPointF getOffset() const override
        {
            return m_offset;
        }

        /**
         * Sets the button offset for rendering.
         * The first component indicates the horizontal offset, the second a vertical offset.
         *
         * @param value(horizontal, vertical)
         */
        void setOffset(const QPointF& value) override
        {
            m_offset = value;
        }

        /**
         * @return The icon size
         */
        QSize getIconSize() const override
        {
            return m_iconSize;
        }

        /**
         * Set the icon size
         */
        void setIconSize(const QSize& value) override
        {
            m_iconSize = value;
        }

        /**
         * Set the button opacity. Active state change animation.
         *
         * @param value Opacity in range [0, 1]
         */
        void setOpacity(qreal value)
        {
            if( m_opacity == value ) return;
            m_opacity = value;
            update();
        }

        /**
         * @return The button opacity
         */
        qreal opacity() const
        {
            return m_opacity;
        }

        /**
         * Invoked for painting this DecorationButtons. Implementing sub-classes need to implement
         * this method. The coordinate system of the QPainter is set to Decoration coordinates.
         *
         * This method will be invoked from the rendering thread.
         *
         * @param painter The QPainter to paint this DecorationButton.
         * @param repaintArea The area which is going to be repainted in Decoration coordinates
         **/
        void paint(QPainter *painter, const QRect &repaintRegion) final;

        /**
         * @return The current button theme
         */
        QWeakPointer<SolidButtonTheme> getTheme() const
        {
            return m_theme.toWeakRef();
        }

        /**
         * Set the button theme
         */
        template<typename T, typename... Args>
        void setTheme(Args&& ...args)
        {
            static_assert(std::is_convertible<T*, SolidButtonTheme*>::value, "Expected an instance of SolidButtonTheme");
            m_theme = QSharedPointer<T>(new T(this, std::forward<Args>(args)...));
        }

    protected:
        /**
         * Invoked for painting the button inside a QRect(0, 0, 18, 18)
         *
         * @param painter  The QPainter to paint this DecorationButton
         */
        virtual void draw(QPainter *painter);

        /**
         * Invoked for painting the symbol inside the button
         *
         * @param painter The QPainter to paint this DecorationButton
         */
        virtual void drawSymbol(QPainter *painter);

    private Q_SLOTS:
        /**
         * Apply configuration changes
         */
        void reconfigure();

        /**
         * Update animation state
         */
        void updateAnimationState(bool);

    protected:
        /**
         * @return The button animation
         */
        QSharedPointer<const QVariantAnimation> animation() const
        {
            return m_animation;
        }

        /**
         * @return The current button state info
         */
        SolidButtonStateInfo getState() const;

    private:
        ButtonFlag m_flag = ButtonFlag::FlagNone;
        QSharedPointer<SolidButtonTheme> m_theme;
        QSharedPointer<QVariantAnimation> m_animation; // Active state change animation
        QPointF m_offset; // Vertical and Horizontal offset (for rendering)
        QSize m_iconSize;
        qreal m_opacity = 0;
    };
}

#endif

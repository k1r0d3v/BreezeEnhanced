#ifndef DECORATION_H
#define DECORATION_H

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

#include "breeze.h"
#include "breezesettings.h"
#include "clientutil.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationButtonGroup>

#include <QWindow>
#include <QVariantAnimation>



namespace Breeze
{
    class Decoration : public KDecoration2::Decoration
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         */
        explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());

        /**
         * Destructor
         */
        ~Decoration() override;

        /**
         * @see KDecoration2::Decoration::paint
         */
        void paint(QPainter *painter, const QRect &repaintRegion) override;

        /**
         * @return The decoration internal settings reference
         */
        InternalSettingsPtr internalSettings() const
        {
            return m_internalSettings;
        }

        /**
         * @return The caption height
         */
        int getCaptionHeight() const;

        /**
         * @return The button height
         */
        int getButtonHeight() const;

        /**
         * Active state change animation
         */
        void setOpacity(qreal);

        /**
         * @return The decoration opacity
         */
        qreal getOpacity() const
        {
            return m_opacity;
        }

        /**
         * @return The title bar color
         */
        QColor getTitleBarColor() const;

        /**
         * @return The font color
         */
        QColor getFontColor() const;

        /**
         * @return True when is maximized else false
         */
        bool isMaximized() const
        {
            return m_client->isMaximized() && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        /**
         * @return True when is maximized horizontally else false
         */
        bool isMaximizedHorizontally() const
        {
            return m_client->isMaximizedHorizontally() && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        /**
         * @return True when is vertically else false
         */
        bool isMaximizedVertically() const
        {
            return m_client->isMaximizedVertically() && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        bool isLeftEdge() const
        {
            return (m_client->isMaximizedHorizontally() || m_client->adjacentScreenEdges().testFlag(Qt::LeftEdge))
                && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        bool isRightEdge() const
        {
            return (m_client->isMaximizedHorizontally() || m_client->adjacentScreenEdges().testFlag(Qt::RightEdge))
                && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        bool isTopEdge() const
        {
            return (m_client->isMaximizedVertically() || m_client->adjacentScreenEdges().testFlag(Qt::TopEdge))
                && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        bool isBottomEdge() const
        {
            return (m_client->isMaximizedVertically() || m_client->adjacentScreenEdges().testFlag(Qt::BottomEdge))
                && !m_internalSettings->drawBorderOnMaximizedWindows();
        }

        bool hideTitleBar() const
        {
            return m_internalSettings->hideTitleBar() && !m_client->isShaded();
        }


    public Q_SLOTS:
        void init() override;

    private Q_SLOTS:
        void reconfigure();
        void recalculateBorders();
        void updateButtonsGeometry();
        void updateButtonsGeometryDelayed();
        void updateTitleBar();
        void updateAnimationState();
        void updateTitleBarColor();

    private:
        QPair<QRect,Qt::Alignment> captionRect() const;

        void createButtons();

        void paintTitleBar(QPainter *painter, const QRect &repaintRegion);

        void createShadow();

        int getBorderSize(bool bottom = false) const;

        bool hasNoBorders() const
        {
            if(m_internalSettings && m_internalSettings->mask() & BorderSize)
                return m_internalSettings->borderSize() == InternalSettings::BorderNone;
            else
                return m_settings->borderSize() == KDecoration2::BorderSize::None;
        }

        bool hasNoSideBorders() const
        {
            if(m_internalSettings && m_internalSettings->mask() & BorderSize)
                return m_internalSettings->borderSize() == InternalSettings::BorderNoSides;
            else
                return m_settings->borderSize() == KDecoration2::BorderSize::NoSides;
        }

        int getTitleBarAlpha() const
        {
            if (m_internalSettings->opaqueTitleBar())
                return 255;

            int a = m_internalSettings->opacityOverride() > -1 ? m_internalSettings->opacityOverride()
                                                               : m_internalSettings->backgroundOpacity();
            a =  qBound(0, a, 100);
            return qRound(static_cast<qreal>(a) * static_cast<qreal>(2.55));
        }

    private:
        QSharedPointer<InternalSettings> m_internalSettings = nullptr;
        QSharedPointer<KDecoration2::DecoratedClient> m_client = nullptr;
        QSharedPointer<KDecoration2::DecorationSettings> m_settings = nullptr;
        std::unique_ptr<KDecoration2::DecorationButtonGroup> m_leftButtons = nullptr;
        std::unique_ptr<KDecoration2::DecorationButtonGroup> m_rightButtons = nullptr;
        std::unique_ptr<QVariantAnimation> m_animation = nullptr; // Active state change animation
        std::unique_ptr<QWindow> m_clientWindow = nullptr;
        std::unique_ptr<ClientUtil> m_clientUtil = nullptr;

        QColor m_titleBarColor = {};
        qreal m_opacity = 0; // Active state change opacity
    };
}

#endif

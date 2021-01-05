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

#include "breezedecoration.h"

#include "breeze.h"
#include "breezesettingsprovider.h"
#include "config-breeze.h"
#include "config/breezeconfigwidget.h"

#include "solidbutton.h"

#include "breezeboxshadowrenderer.h"
#include "util.h"
#include "clientutil.h"
#include "buttonfactory.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>
#include <KDecoration2/DecorationButton>

#include <KColorUtils>
#include <KPluginFactory>

#include <QPainter>
#include <QTimer>
#include <QVariantAnimation>
#include <QWindow>
#include <QScreen>
#include <QThread>
#include <QDebug>
#include <QColor>

#include <memory>


K_PLUGIN_FACTORY_WITH_JSON(
    BreezeDecoFactory,
    "breeze.json",
    registerPlugin<Breeze::Decoration>();
            registerPlugin<KDecoration2::DecorationButton>(QStringLiteral("button"),
                                                           Breeze::ButtonFactory::getPluginInstance);
    registerPlugin<Breeze::ConfigWidget>(QStringLiteral("kcmodule"));
)

namespace
{
    struct ShadowParams {
        ShadowParams()
            : offset(QPoint(0, 0))
            , radius(0)
            , opacity(0) {}

        ShadowParams(const QPoint &offset, int radius, qreal opacity)
            : offset(offset)
            , radius(radius)
            , opacity(opacity) {}

        QPoint offset;
        int radius;
        qreal opacity;
    };

    struct CompositeShadowParams {
        CompositeShadowParams() = default;

        CompositeShadowParams(
                const QPoint &offset,
                const ShadowParams &shadow1,
                const ShadowParams &shadow2)
            : offset(offset)
            , shadow1(shadow1)
            , shadow2(shadow2) {}

        bool isNone() const {
            return qMax(shadow1.radius, shadow2.radius) == 0;
        }

        QPoint offset;
        ShadowParams shadow1;
        ShadowParams shadow2;
    };

    const CompositeShadowParams s_shadowParams[] = {
        // None
        CompositeShadowParams(),
        // Small
        CompositeShadowParams(
            QPoint(0, 4),
            ShadowParams(QPoint(0, 0), 16, 1),
            ShadowParams(QPoint(0, -2), 8, 0.4)),
        // Medium
        CompositeShadowParams(
            QPoint(0, 8),
            ShadowParams(QPoint(0, 0), 32, 0.9),
            ShadowParams(QPoint(0, -4), 16, 0.3)),
        // Large
        CompositeShadowParams(
            QPoint(0, 12),
            ShadowParams(QPoint(0, 0), 48, 0.8),
            ShadowParams(QPoint(0, -6), 24, 0.2)),
        // Very large
        CompositeShadowParams(
            QPoint(0, 16),
            ShadowParams(QPoint(0, 0), 64, 0.7),
            ShadowParams(QPoint(0, -8), 32, 0.1)),
    };

    inline CompositeShadowParams lookupShadowParams(int size)
    {
        switch (size) {
        case Breeze::InternalSettings::ShadowNone:
            return s_shadowParams[0];
        case Breeze::InternalSettings::ShadowSmall:
            return s_shadowParams[1];
        case Breeze::InternalSettings::ShadowMedium:
            return s_shadowParams[2];
        case Breeze::InternalSettings::ShadowLarge:
            return s_shadowParams[3];
        case Breeze::InternalSettings::ShadowVeryLarge:
            return s_shadowParams[4];
        default:
            // Fallback to the Large size.
            return s_shadowParams[3];
        }
    }
}

namespace Breeze
{
    static int g_decorationCount = 0;
    static int g_shadowSizeEnum = InternalSettings::ShadowLarge;
    static int g_shadowStrength = 255;
    static QColor g_shadowColor = Qt::black;
    static QSharedPointer<KDecoration2::DecorationShadow> g_shadowPointer;
    static QTimer g_titleBarColorTimer;


    Decoration::Decoration(QObject *parent, const QVariantList &args)
        : KDecoration2::Decoration(parent, args)
        , m_animation(std::make_unique<QVariantAnimation>(this))
    {
        g_decorationCount++;
    }

    Decoration::~Decoration()
    {
        g_decorationCount--;
        if (g_decorationCount == 0)
        {
            // Last decoration destroyed, clean up shadow
            g_shadowPointer.clear();
        }

        disconnect(&g_titleBarColorTimer, &QTimer::timeout, this, &Decoration::updateTitleBarColor);
    }

    void Decoration::init()
    {
        m_client = client().toStrongRef();
        m_settings = settings();

        // Active state change animation
        // It is important start and end value are of the same type, hence 0.0 and not just 0
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        connect(m_animation.get(), &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });

        reconfigure();
        updateTitleBar();

        connect(m_settings.data(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::recalculateBorders);

        // A change in font might cause the borders to change
        recalculateBorders();
        connect(m_settings.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::recalculateBorders);

        // Buttons
        connect(m_settings.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(m_settings.data(), &KDecoration2::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(m_settings.data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);

        // Full reconfiguration
        connect(m_settings.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::reconfigure);
        connect(m_settings.data(), &KDecoration2::DecorationSettings::reconfigured, SettingsProvider::self(), &SettingsProvider::reconfigure, Qt::UniqueConnection);
        connect(m_settings.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

        connect(m_client.data(), &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::recalculateBorders);
        connect(m_client.data(), &KDecoration2::DecoratedClient::maximizedHorizontallyChanged, this, &Decoration::recalculateBorders);
        connect(m_client.data(), &KDecoration2::DecoratedClient::maximizedVerticallyChanged, this, &Decoration::recalculateBorders);
        connect(m_client.data(), &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::recalculateBorders);
        connect(m_client.data(), &KDecoration2::DecoratedClient::captionChanged, this,
            [this]()
            {
                // Update the caption area
                update(titleBar());
            }
       );

        connect(m_client.data(), &KDecoration2::DecoratedClient::activeChanged, this, &Decoration::updateAnimationState);
        connect(m_client.data(), &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateTitleBar);
        connect(m_client.data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateTitleBar);
        //connect(c, &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::setOpaque);
        connect(m_client.data(), &KDecoration2::DecoratedClient::sizeChanged, this, &Decoration::updateTitleBar);

        connect(m_client.data(), &KDecoration2::DecoratedClient::widthChanged, this, &Decoration::updateButtonsGeometry);
        connect(m_client.data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Decoration::updateButtonsGeometry);
        connect(m_client.data(), &KDecoration2::DecoratedClient::adjacentScreenEdgesChanged, this, &Decoration::updateButtonsGeometry);
        connect(m_client.data(), &KDecoration2::DecoratedClient::shadedChanged, this, &Decoration::updateButtonsGeometry);

        // Get ours client window
        m_clientWindow = std::unique_ptr<QWindow>(QWindow::fromWinId(m_client->windowId()));
        m_clientUtil = std::make_unique<ClientUtil>(*m_clientWindow);

        if (!g_titleBarColorTimer.isActive())
            g_titleBarColorTimer.start(250);
        connect(&g_titleBarColorTimer, &QTimer::timeout, this, &Decoration::updateTitleBarColor);

        createButtons();
        createShadow();

        QTimer::singleShot(20, this, &Decoration::updateTitleBarColor);
    }

    void Decoration::setOpacity(qreal value)
    {
        if (m_opacity == value)
            return;

        m_opacity = value;
        update();
    }

    QColor Decoration::getTitleBarColor() const
    {
        auto active = m_client->color(KDecoration2::ColorGroup::Active, KDecoration2::ColorRole::TitleBar);
        auto inactive = m_client->color(KDecoration2::ColorGroup::Inactive, KDecoration2::ColorRole::TitleBar);

        if (m_titleBarColor.isValid())
        {
            active = m_titleBarColor;
            inactive = m_titleBarColor;
        }

        if (hideTitleBar())
            return inactive;
        else if(m_animation->state() == QAbstractAnimation::Running)
            return KColorUtils::mix(inactive, active, m_opacity);
        else
            return m_client->isActive() ? active : inactive;
    }

    QColor Decoration::getFontColor() const
    {
        auto active = m_client->color(KDecoration2::ColorGroup::Active, KDecoration2::ColorRole::Foreground);
        auto inactive = m_client->color(KDecoration2::ColorGroup::Inactive, KDecoration2::ColorRole::Foreground);

        if (m_titleBarColor.isValid())
        {
            auto luminance = perceptiveLuminance(m_titleBarColor);
            active = luminance > 0.5 ? m_internalSettings->darkTextColor() : m_internalSettings->lightTextColor();
            inactive = inactiveGrayFrom(m_titleBarColor);
        }

        if(m_animation->state() == QAbstractAnimation::Running)
            return KColorUtils::mix(inactive, active, m_opacity);
        else
            return  m_client->isActive() ? active : inactive;
    }

    void Decoration::updateTitleBar()
    {
        const bool maximized = isMaximized();
        const int width =  maximized ? m_client->width() : m_client->width() - 2 * m_settings->largeSpacing() * Metrics::TitleBar_SideMargin;
        const int height = maximized ? borderTop() : borderTop() - m_settings->smallSpacing() * Metrics::TitleBar_TopMargin;
        const int x = maximized ? 0 : m_settings->largeSpacing() * Metrics::TitleBar_SideMargin;
        const int y = maximized ? 0 : m_settings->smallSpacing() * Metrics::TitleBar_TopMargin;
        setTitleBar(QRect(x, y, width, height));
    }

    void Decoration::updateTitleBarColor()
    {
        if (m_clientUtil == nullptr || !m_client->isActive())
            return;

        auto color =  m_clientUtil->topLineColor();
        if (color.isValid())
        {
            if (!m_titleBarColor.isValid() || (m_titleBarColor.isValid() && m_titleBarColor != color))
            {
                m_titleBarColor = color;
                update();
            }
        }
    }

    void Decoration::updateAnimationState()
    {
        if(m_internalSettings->animationsEnabled())
        {
            m_animation->setDirection(m_client->isActive() ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
            if(m_animation->state() != QAbstractAnimation::Running)
                m_animation->start();
        }
        else update();
    }

    int Decoration::getBorderSize(bool bottom) const
    {
        const int baseSize = m_settings->smallSpacing();
        if(m_internalSettings && (m_internalSettings->mask() & BorderSize))
        {
            switch (m_internalSettings->borderSize())
            {
                case InternalSettings::BorderNone: return 0;
                case InternalSettings::BorderNoSides: return bottom ? qMax(4, baseSize) : 0;
                default:
                case InternalSettings::BorderTiny: return bottom ? qMax(4, baseSize) : baseSize;
                case InternalSettings::BorderNormal: return baseSize*2;
                case InternalSettings::BorderLarge: return baseSize*3;
                case InternalSettings::BorderVeryLarge: return baseSize*4;
                case InternalSettings::BorderHuge: return baseSize*5;
                case InternalSettings::BorderVeryHuge: return baseSize*6;
                case InternalSettings::BorderOversized: return baseSize*10;
            }
        }
        else
        {
            switch (m_settings->borderSize())
            {
                case KDecoration2::BorderSize::None: return 0;
                case KDecoration2::BorderSize::NoSides: return bottom ? qMax(4, baseSize) : 0;
                default:
                case KDecoration2::BorderSize::Tiny: return bottom ? qMax(4, baseSize) : baseSize;
                case KDecoration2::BorderSize::Normal: return baseSize*2;
                case KDecoration2::BorderSize::Large: return baseSize*3;
                case KDecoration2::BorderSize::VeryLarge: return baseSize*4;
                case KDecoration2::BorderSize::Huge: return baseSize*5;
                case KDecoration2::BorderSize::VeryHuge: return baseSize*6;
                case KDecoration2::BorderSize::Oversized: return baseSize*10;

            }
        }
    }

    void Decoration::reconfigure()
    {
        m_internalSettings = SettingsProvider::self()->internalSettings(this);

        // Animation
        m_animation->setDuration(m_internalSettings->animationsDuration());

        // Borders
        recalculateBorders();

        // Shadow
        createShadow();
    }

    void Decoration::recalculateBorders()
    {
        // Left, Right and Bottom borders
        const int left   = isLeftEdge() ? 0 : getBorderSize();
        const int right  = isRightEdge() ? 0 : getBorderSize();
        const int bottom = (m_client->isShaded() || isBottomEdge()) ? 0 : getBorderSize(true);

        int top = 0;
        if(hideTitleBar())
        {
            top = bottom;
        }
        else
        {
            QFont f; f.fromString(m_internalSettings->titleBarFont());
            QFontMetrics fm(f);
            top += qMax(fm.height(), getButtonHeight());

            // Padding below
            // Extra pixel is used for the active window outline
            const int baseSize = m_settings->smallSpacing();
            top += baseSize*Metrics::TitleBar_BottomMargin + 1;

            // Padding above
            top += baseSize*TitleBar_TopMargin;
        }

        setBorders(QMargins(left, top, right, bottom));

        // Extended sizes
        const int extSize = m_settings->largeSpacing();
        int extSides = 0;
        int extBottom = 0;
        if(hasNoBorders())
        {
            if(!isMaximizedHorizontally())
                extSides = extSize;

            if(!isMaximizedVertically())
                extBottom = extSize;
        }
        else if(hasNoSideBorders() && !isMaximizedHorizontally())
        {
            extSides = extSize;
        }

        setResizeOnlyBorders(QMargins(extSides, 0, extSides, extBottom));
    }

    void Decoration::createButtons()
    {
        m_leftButtons = std::make_unique<KDecoration2::DecorationButtonGroup>(KDecoration2::DecorationButtonGroup::Position::Left, this, ButtonFactory::getButtonGroupInstance);
        m_rightButtons = std::make_unique<KDecoration2::DecorationButtonGroup>(KDecoration2::DecorationButtonGroup::Position::Right, this, ButtonFactory::getButtonGroupInstance);
        updateButtonsGeometry();
    }

    void Decoration::updateButtonsGeometryDelayed()
    {
        QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
    }

    void Decoration::updateButtonsGeometry()
    {
        // Adjust button position
        const int buttonHeight =
                this->getCaptionHeight() + (isTopEdge() ? m_settings->smallSpacing() * Metrics::TitleBar_TopMargin : 0);
        const int buttonWidth = this->getButtonHeight();
        const int verticalOffset = (isTopEdge() ? m_settings->smallSpacing() * Metrics::TitleBar_TopMargin : 0) + (this->getCaptionHeight() -
                this->getButtonHeight()) / 2;
        foreach(const QPointer<KDecoration2::DecorationButton>& button, m_leftButtons->buttons() + m_rightButtons->buttons())
        {
            button.data()->setGeometry(QRectF(QPoint(0, 0), QSizeF(buttonWidth, buttonHeight)));
            dynamic_cast<ButtonBase*>(button.data())->setOffset(QPointF(0, verticalOffset));
            dynamic_cast<ButtonBase*>(button.data())->setIconSize(QSize(buttonWidth, buttonWidth));
        }

        // Left buttons
        if(!m_leftButtons->buttons().isEmpty())
        {
            m_leftButtons->setSpacing(m_internalSettings->buttonSpacing());

            // Padding
            const int verticalPadding = isTopEdge() ? 0 : m_settings->smallSpacing() * Metrics::TitleBar_TopMargin;
            const int horizontalPadding = m_settings->smallSpacing() * Metrics::TitleBar_SideMargin + m_internalSettings->leftPadding();
            if(isLeftEdge())
            {
                // Add offsets on the side buttons, to preserve padding, but satisfy Fitts law
                auto button = dynamic_cast<ButtonBase*>(m_leftButtons->buttons().front().data());
                button->setGeometry(QRectF(QPoint(0, 0), QSizeF(buttonWidth + horizontalPadding, buttonHeight)));
                button->setFlag(ButtonFlag::FlagFirstInList);
                button->setOffset({ static_cast<qreal>(horizontalPadding), button->getOffset().y() });

                m_leftButtons->setPos(QPointF(0, verticalPadding));

            }
            else
                m_leftButtons->setPos(QPointF(horizontalPadding + borderLeft(), verticalPadding));
        }

        // Right buttons
        if(!m_rightButtons->buttons().isEmpty())
        {
            m_rightButtons->setSpacing(m_internalSettings->buttonSpacing());

            // Padding
            const int vPadding = isTopEdge() ? 0 : m_settings->smallSpacing()*Metrics::TitleBar_TopMargin;
            const int hPadding = m_settings->smallSpacing()*Metrics::TitleBar_SideMargin - m_internalSettings->rightPadding();
            if(isRightEdge())
            {
                auto button = dynamic_cast<ButtonBase*>(m_rightButtons->buttons().back().data());
                button->setGeometry(QRectF(QPoint(0, 0), QSizeF(buttonWidth + hPadding, buttonHeight)));
                button->setFlag(ButtonFlag::FlagLastInList);

                m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width(), vPadding));
            }
            else
                m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width() - hPadding - borderRight(), vPadding));
        }

        update();
    }

    void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
    {
        // TODO: optimize based on repaintRegion
        auto &c = m_client;
        auto &s = m_settings;

        // Paint background of borders
        if(!c->isShaded())
        {
            painter->fillRect(rect(), Qt::transparent);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);

            QColor winCol = this->getTitleBarColor();
            winCol.setAlpha(getTitleBarAlpha());
            painter->setBrush(winCol);

            // clip away the top part
            if(!hideTitleBar())
                painter->setClipRect(0, borderTop(), size().width(), size().height() - borderTop(), Qt::IntersectClip);

            if(s->isAlphaChannelSupported())
                painter->drawRoundedRect(rect(), (internalSettings()->frameRadius()), (internalSettings()->frameRadius()));
            else
                painter->drawRect(rect());

            painter->restore();
        }

        // Paint title bar
        if(!hideTitleBar())
            paintTitleBar(painter, repaintRegion);

        // Paint borders when no aplha is supported
        if(!hasNoBorders() && !s->isAlphaChannelSupported())
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(c->isActive() ?
                c->color(KDecoration2::ColorGroup::Active, KDecoration2::ColorRole::TitleBar):
                c->color(KDecoration2::ColorGroup::Inactive, KDecoration2::ColorRole::Foreground));

            painter->drawRect(rect().adjusted(0, 0, -1, -1));
            painter->restore();
        }
    }

    void Decoration::paintTitleBar(QPainter *painter, const QRect &repaintRegion)
    {
        const auto &c = m_client;
        const QRect titleRect(QPoint(0, 0), QSize(size().width(), borderTop()));

        if (!titleRect.intersects(repaintRegion))
            return;

        painter->save();
        painter->setPen(Qt::NoPen);

        // render a linear gradient on title area and draw a light border at the top
        if(m_internalSettings->drawBackgroundGradient() && !m_internalSettings->flatTitleBar())
        {
            QColor titleBarColor(this->getTitleBarColor());
            titleBarColor.setAlpha(getTitleBarAlpha());

            QLinearGradient gradient(0, 0, 0, titleRect.height());
            QColor lightCol(titleBarColor.lighter(130 + m_internalSettings->backgroundGradientIntensity()));
            gradient.setColorAt(0.0, lightCol);
            gradient.setColorAt(0.99 / static_cast<qreal>(titleRect.height()), lightCol);
            gradient.setColorAt(1.0 / static_cast<qreal>(titleRect.height()), titleBarColor.lighter(100 + m_internalSettings->backgroundGradientIntensity()));
            gradient.setColorAt(1.0, titleBarColor);

            painter->setBrush(gradient);

        } else {

            QColor titleBarColor = this->getTitleBarColor();
            titleBarColor.setAlpha(getTitleBarAlpha());

            QLinearGradient gradient(0, 0, 0, titleRect.height());
            QColor lightCol(titleBarColor.lighter(130));
            gradient.setColorAt(0.0, lightCol);
            gradient.setColorAt(0.99 / static_cast<qreal>(titleRect.height()), lightCol);
            gradient.setColorAt(1.0 / static_cast<qreal>(titleRect.height()), titleBarColor);
            gradient.setColorAt(1.0, titleBarColor);

            painter->setBrush(gradient);

        }

        auto &s = m_settings;
        if(isMaximized() || !s->isAlphaChannelSupported())
        {
            painter->drawRect(titleRect);
        }
        else if(c->isShaded())
        {
            painter->drawRoundedRect(titleRect, (internalSettings()->frameRadius()), (internalSettings()->frameRadius()));
        }
        else
        {
            painter->setClipRect(titleRect, Qt::IntersectClip);

            // the rect is made a little bit larger to be able to clip away the rounded corners at the bottom and sides
            painter->drawRoundedRect(titleRect.adjusted(
                isLeftEdge() ? -(internalSettings()->frameRadius()):0,
                isTopEdge() ? -(internalSettings()->frameRadius()):0,
                isRightEdge() ? (internalSettings()->frameRadius()):0,
                (internalSettings()->frameRadius())),
                (internalSettings()->frameRadius()), (internalSettings()->frameRadius()));
        }

        painter->restore();

        // draw caption
        QFont f; f.fromString(m_internalSettings->titleBarFont());
        // KDE needs this FIXME: Why?
        QFontDatabase fd; f.setStyleName(fd.styleString(f));
        painter->setFont(f);
        painter->setPen(getFontColor());
        const auto cR = captionRect();
        const QString caption = painter->fontMetrics().elidedText(c->caption(), Qt::ElideMiddle, cR.first.width());
        painter->drawText(cR.first, static_cast<int>(cR.second | Qt::TextSingleLine), caption);

        // draw all buttons
        m_leftButtons->paint(painter, repaintRegion);
        m_rightButtons->paint(painter, repaintRegion);
    }

    int Decoration::getButtonHeight() const
    {
        const int baseSize = m_settings->gridUnit();
        switch(m_internalSettings->buttonSize())
        {
            case InternalSettings::ButtonTiny: return baseSize;
            case InternalSettings::ButtonSmall: return static_cast<int>(baseSize*1.5);
            default:
            case InternalSettings::ButtonDefault: return baseSize*2;
            case InternalSettings::ButtonLarge: return static_cast<int>(baseSize*2.5);
            case InternalSettings::ButtonVeryLarge: return static_cast<int>(baseSize*3.5);
        }
    }

    int Decoration::getCaptionHeight() const
    {
        return hideTitleBar() ? borderTop() : borderTop() - m_settings->smallSpacing() * (Metrics::TitleBar_BottomMargin + Metrics::TitleBar_TopMargin) - 1;
    }

    QPair<QRect,Qt::Alignment> Decoration::captionRect() const
    {
        if(hideTitleBar()) return qMakePair(QRect(), Qt::AlignCenter);
        else {

            const int extraTitleMargin = m_internalSettings->extraTitleMargin();
            auto &c = m_client;
            const int leftOffset = m_leftButtons->buttons().isEmpty() ?
                Metrics::TitleBar_SideMargin*m_settings->smallSpacing() + extraTitleMargin :
                m_leftButtons->geometry().x() + m_leftButtons->geometry().width() + Metrics::TitleBar_SideMargin*m_settings->smallSpacing() + extraTitleMargin;

            const int rightOffset = m_rightButtons->buttons().isEmpty() ?
                Metrics::TitleBar_SideMargin*m_settings->smallSpacing() + extraTitleMargin:
                size().width() - m_rightButtons->geometry().x() + Metrics::TitleBar_SideMargin*m_settings->smallSpacing() + extraTitleMargin;

            const int yOffset = m_settings->smallSpacing()*Metrics::TitleBar_TopMargin;
            const QRect maxRect(leftOffset, yOffset, size().width() - leftOffset - rightOffset, getCaptionHeight());

            switch(m_internalSettings->titleAlignment())
            {
                case InternalSettings::AlignLeft:
                return qMakePair(maxRect, Qt::AlignVCenter|Qt::AlignLeft);

                case InternalSettings::AlignRight:
                return qMakePair(maxRect, Qt::AlignVCenter|Qt::AlignRight);

                case InternalSettings::AlignCenter:
                return qMakePair(maxRect, Qt::AlignCenter);

                default:
                case InternalSettings::AlignCenterFullWidth:
                {

                    // full caption rect
                    const QRect fullRect = QRect(0, yOffset, size().width(), getCaptionHeight());
                    QFont f; f.fromString(m_internalSettings->titleBarFont());
                    QFontMetrics fm(f);
                    QRect boundingRect(fm.boundingRect(c->caption()));

                    // text bounding rect
                    boundingRect.setTop(yOffset);
                    boundingRect.setHeight(getCaptionHeight());
                    boundingRect.moveLeft((size().width() - boundingRect.width())/2);

                    if(boundingRect.left() < leftOffset) return qMakePair(maxRect, Qt::AlignVCenter|Qt::AlignLeft);
                    else if(boundingRect.right() > size().width() - rightOffset) return qMakePair(maxRect, Qt::AlignVCenter|Qt::AlignRight);
                    else return qMakePair(fullRect, Qt::AlignCenter);

                }

            }

        }

    }

    void Decoration::createShadow()
    {
        if (!g_shadowPointer
                ||g_shadowSizeEnum != m_internalSettings->shadowSize()
                || g_shadowStrength != m_internalSettings->shadowStrength()
                || g_shadowColor != m_internalSettings->shadowColor())
        {
            g_shadowSizeEnum = m_internalSettings->shadowSize();
            g_shadowStrength = m_internalSettings->shadowStrength();
            g_shadowColor = m_internalSettings->shadowColor();

            const CompositeShadowParams params = lookupShadowParams(g_shadowSizeEnum);
            if (params.isNone()) {
                g_shadowPointer.clear();
                setShadow(g_shadowPointer);
                return;
            }

            auto withOpacity = [](const QColor &color, qreal opacity) -> QColor {
                QColor c(color);
                c.setAlphaF(opacity);
                return c;
            };

            const QSize boxSize = BoxShadowRenderer::calculateMinimumBoxSize(params.shadow1.radius)
                .expandedTo(BoxShadowRenderer::calculateMinimumBoxSize(params.shadow2.radius));

            BoxShadowRenderer shadowRenderer;
            shadowRenderer.setBorderRadius((internalSettings()->frameRadius()) + 0.5);
            shadowRenderer.setBoxSize(boxSize);
            shadowRenderer.setDevicePixelRatio(1.0); // TODO: Create HiDPI shadows?

            const qreal strength = static_cast<qreal>(g_shadowStrength) / 255.0;
            shadowRenderer.addShadow(params.shadow1.offset, params.shadow1.radius,
                withOpacity(g_shadowColor, params.shadow1.opacity * strength));
            shadowRenderer.addShadow(params.shadow2.offset, params.shadow2.radius,
                withOpacity(g_shadowColor, params.shadow2.opacity * strength));

            QImage shadowTexture = shadowRenderer.render();

            QPainter painter(&shadowTexture);
            painter.setRenderHint(QPainter::Antialiasing);

            const QRect outerRect = shadowTexture.rect();

            QRect boxRect(QPoint(0, 0), boxSize);
            boxRect.moveCenter(outerRect.center());

            // Mask out inner rect.
            const QMargins padding = QMargins(
                boxRect.left() - outerRect.left() - Metrics::Shadow_Overlap - params.offset.x(),
                boxRect.top() - outerRect.top() - Metrics::Shadow_Overlap - params.offset.y(),
                outerRect.right() - boxRect.right() - Metrics::Shadow_Overlap + params.offset.x(),
                outerRect.bottom() - boxRect.bottom() - Metrics::Shadow_Overlap + params.offset.y());
            const QRect innerRect = outerRect - padding;

            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::black);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            painter.drawRoundedRect(
                innerRect,
                (internalSettings()->frameRadius()) + 0.5,
                (internalSettings()->frameRadius()) + 0.5);

            // Draw outline.
            painter.setPen(withOpacity(g_shadowColor, 0.2 * strength));
            painter.setBrush(Qt::NoBrush);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawRoundedRect(
                innerRect,
                (internalSettings()->frameRadius()) - 0.5,
                (internalSettings()->frameRadius()) - 0.5);

            painter.end();

            g_shadowPointer = QSharedPointer<KDecoration2::DecorationShadow>::create();
            g_shadowPointer->setPadding(padding);
            g_shadowPointer->setInnerShadowRect(QRect(outerRect.center(), QSize(1, 1)));
            g_shadowPointer->setShadow(shadowTexture);
        }

        setShadow(g_shadowPointer);
    }
} // namespace


#include "breezedecoration.moc"

/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2014  Hugo Pereira Da Costa <hugo.pereira@free.fr>
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
#include "solidbuttontheme.h"
#include "breezedecoration.h"
#include "util.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationSettings>

#include <QPainter>
#include <QVariantAnimation>
#include <QPainterPath>

#define this_decoration (qobject_cast<Breeze::Decoration*>(this->decoration()))

namespace Breeze
{
    SolidButton::SolidButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration* decoration, QObject* parent)
        : ButtonBase(type, decoration, parent)
        , m_animation(new QVariantAnimation(this))
    {
        if (this_decoration == nullptr)
        {
            qDebug("SolidButton: Decoration error: Expected a solid button decoration, exiting");
            exit(-1);
        }

        // It is important start and end value are of the same type,
        // hence 0.0 and not just 0 because the QVariant value
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
        connect(m_animation.data(), &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setOpacity(value.toReal());
        });

        // Setup default geometry
        const int height = this_decoration->getButtonHeight();
        KDecoration2::DecorationButton::setGeometry(QRect(0, 0, height, height));
        m_iconSize = QSize(height, height);


        // Setup default theme
        m_theme = QSharedPointer<SolidButtonTheme>(new DefaultSolidButtonTheme(this));

        // Connections
        connect(decoration->client().toStrongRef().data(), SIGNAL(iconChanged(QIcon)), this, SLOT(update()));
        connect(decoration->settings().data(), &KDecoration2::DecorationSettings::reconfigured, this, &SolidButton::reconfigure);
        connect(this, &KDecoration2::DecorationButton::hoveredChanged, this, [this](auto hover) {
            // Do not animate nothing if the button is pressed
            if (!isPressed())
                updateAnimationState(hover);
        });
        connect(this, &KDecoration2::DecorationButton::pressedChanged, this, [this](auto pressed) {
            if (isHovered() && pressed) // Play animation(backwards) if we press the button
                updateAnimationState(false);
            else if (isChecked() && !pressed) // Play animation if we release the button while checked
                updateAnimationState(true);
        });

        reconfigure();
    }

    SolidButton::SolidButton(QObject *parent, const QVariantList &args)
        : SolidButton(args.at(0).value<KDecoration2::DecorationButtonType>(),
                 args.at(1).value<KDecoration2::Decoration*>(),
                           parent)
    {
        m_flag = ButtonFlag::FlagStandalone;
        //! icon size must return to !valid because it was altered from the default constructor,
        //! in Standalone mode the button is not using the decoration metrics but its geometry
        m_iconSize = QSize(-1, -1);
    }

    void SolidButton::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        if (this->decoration() == nullptr)
            return;

        painter->save();

        // Translate from offset
        if(m_flag == ButtonFlag::FlagFirstInList)
            painter->translate(m_offset);
        else
            painter->translate(0, m_offset.y());

        // Use the button geometry instead
        if(!m_iconSize.isValid())
            m_iconSize = geometry().size().toSize();

        // Prepare for painting
        painter->setRenderHints(QPainter::Antialiasing);

        // Scale painter so that its window matches QRect(-1, -1, 20, 20)
        // this makes all further rendering and scaling simpler
        // all further rendering is expected to be preformed inside QRect(0, 0, 18, 18)
        painter->translate(geometry().topLeft());

        const qreal width = m_iconSize.width();
        painter->scale(width/20, width/20);
        painter->translate(1, 1);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::NoPen);

        // Draw
        this->draw(painter);
        painter->restore();
    }

    void SolidButton::draw(QPainter *painter)
    {
        auto state = getState();
        auto center = QPointF(static_cast<qreal>(9), static_cast<qreal>(9));
        auto radius = static_cast<qreal>(7);
        if (!isPressed())
            radius += (state.isChecked ? 1 :  1.5) * m_animation->currentValue().toReal();

        if (m_theme->showBorder(state))
        {
            auto pen = QPen(m_theme->borderColor(state).color());
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setWidthF(PenWidth::Symbol * qMax(static_cast<qreal>(1.0), static_cast<qreal>(20) / m_iconSize.width()));
            painter->setPen(pen);
        }

        if (m_theme->showBackground(state))
            painter->setBrush(m_theme->backgroundColor(state));

        if (m_theme->showBackground(state) || m_theme->showBorder(state))
            painter->drawEllipse(center, radius, radius);

        if (m_theme->showSymbol(state))
        {
            // Reset brush and pen before symbol drawing
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::NoPen);
            this->drawSymbol(painter);
        }
    }

    void SolidButton::drawSymbol(QPainter *painter)
    {
        auto state = getState();
        auto radius = 1.5 + static_cast<qreal>(1.5) * m_animation->currentValue().toReal();

        painter->setBrush(m_theme->symbolColor(state));
        painter->drawEllipse(QPointF(9, 9), radius, radius);
    }

    void SolidButton::reconfigure()
    {
        auto settings = this_decoration->internalSettings();
        m_animation->setDuration(settings->animationsDuration());
    }

    void SolidButton::updateAnimationState(bool forward)
    {
        if(!this_decoration->internalSettings()->animationsEnabled())
            return;

        QAbstractAnimation::Direction dir = forward ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        if(m_animation->state() == QAbstractAnimation::Running && m_animation->direction() != dir)
            m_animation->stop();
        m_animation->setDirection(dir);

        if(m_animation->state() != QAbstractAnimation::Running)
        {
            if ((forward && m_animation->currentValue().toReal() != 1.0) ||
                (!forward && m_animation->currentValue().toReal() != 0.0))
                m_animation->start();
        }
    }

    SolidButtonStateInfo SolidButton::getState() const
    {
        SolidButtonStateInfo state;
        state.isActive = this_decoration->client().toStrongRef()->isActive();
        state.isChecked = isChecked();
        state.isPressed = isPressed();
        state.isHovered = isHovered();
        state.isBeingAnimated = m_animation->state() == QAbstractAnimation::Running;
        state.titleBarColor = this_decoration->getTitleBarColor();
        state.icon = decoration()->client().toStrongRef()->icon();
        return state;
    }
}

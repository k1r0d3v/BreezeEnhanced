#include <QPainter>
#include <QVariantAnimation>
#include <QPainterPath>
#include <QPalette>
#include <KIconThemes/KIconLoader>
#include <KIconThemes/KIconTheme>

#include "solidbutton.h"
#include "solidbuttons.h"
#include "solidbuttontheme.h"
#include "util.h"


class IconOnlyTheme : public Breeze::DefaultSolidButtonTheme
{
public:
    explicit IconOnlyTheme(Breeze::SolidButton *button)
            : DefaultSolidButtonTheme(button)
    {

    }

    bool showBackground(SolidButtonStateInfo state) const override
    {
        return false;
    }

    bool showBorder(SolidButtonStateInfo state) const override
    {
        return false;
    }

    bool showSymbol(SolidButtonStateInfo state) const override
    {
        return true;
    }

    QBrush symbolColor(SolidButtonStateInfo state) const override
    {
        auto titleBarLuminance = perceptiveLuminance(state.titleBarColor);
        return titleBarLuminance > 0.5 ? QColor(32, 32, 32) : QColor(240, 240, 240);
    }
};


void Breeze::CloseButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto theme = getTheme().toStrongRef();
    auto color = theme->symbolColor(state).color();

    auto symbolPen = QPen(color);
    symbolPen.setJoinStyle(Qt::MiterJoin);
    symbolPen.setWidthF(1.7 * qMax(static_cast<qreal>(1.0), 20 / this->size().width()));

    auto scale = [this](QPointF q, QPointF v) {
        auto animationValue = this->animation()->currentValue().toReal();
        return QPointF((q.x() + v.x()) - v.x() * animationValue, (q.y() + v.y()) - v.y() * animationValue);
    };

    painter->setPen(symbolPen);
    painter->drawLine( scale(QPointF(6, 6), QPointF(3, 3)), scale(QPointF(12, 12), QPointF(-3, -3)));
    painter->drawLine( scale(QPointF(6, 12), QPointF(3, -3)), scale(QPointF(12, 6 ), QPointF(-3, 3)));
}

Breeze::CloseButton::CloseButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                                 QObject *parent) : SolidButton(type, decoration, parent)
{

}

void Breeze::MaximizeButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto theme = getTheme().toStrongRef();
    auto color = theme->symbolColor(state).color();

    // two triangles
    QPainterPath path1, path2;
    if( isChecked() )
    {
        path1.moveTo(8.5, 9.5);
        path1.lineTo(2.5, 9.5);
        path1.lineTo(8.5, 15.5);

        path2.moveTo(9.5, 8.5);
        path2.lineTo(15.5, 8.5);
        path2.lineTo(9.5, 2.5);
    }
    else
    {
        path1.moveTo(5, 13);
        path1.lineTo(11, 13);
        path1.lineTo(5, 7);

        path2.moveTo(13, 5);
        path2.lineTo(7, 5);
        path2.lineTo(13, 11);
    }

    painter->fillPath(path1, QBrush(color));
    painter->fillPath(path2, QBrush(color));
}

Breeze::MaximizeButton::MaximizeButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                                       QObject *parent) : SolidButton(type, decoration, parent)
{

}

void Breeze::MinimizeButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto theme = getTheme().toStrongRef();
    auto color = theme->symbolColor(state).color();

    auto scale = [this](QPointF q, QPointF v) {
        auto animationValue = this->animation()->currentValue().toReal();
        return QPointF((q.x() + v.x()) - v.x() * animationValue, (q.y() + v.y()) - v.y() * animationValue);
    };

    auto symbolPen = QPen(color);
    symbolPen.setJoinStyle(Qt::MiterJoin);
    //symbolPen.setWidthF(1.7 * qMax(static_cast<qreal>(1.0), 20 / this->size().width()));
    symbolPen.setWidth(2);

    painter->setPen(symbolPen);
    painter->drawLine(scale(QPointF(5, 9), {2, 0}), scale(QPointF(13, 9), {-2, 0}));
}

Breeze::MinimizeButton::MinimizeButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                                       QObject *parent) : SolidButton(type, decoration, parent)
{

}

void Breeze::KeepAboveButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto theme = getTheme().toStrongRef();
    auto color = theme->symbolColor(state).color();

    auto scale = [this](QPointF q, QPointF v) {
        auto animationValue = this->animation()->currentValue().toReal();
        return QPointF((q.x() + v.x()) - v.x() * animationValue, (q.y() + v.y()) - v.y() * animationValue);
    };

    auto symbolPen = QPen(color);
    symbolPen.setJoinStyle(Qt::MiterJoin);
    symbolPen.setWidthF(1.7 * qMax(static_cast<qreal>(1.0), 20 / this->size().width()));

    painter->setPen(symbolPen);
    painter->drawPolyline(QPolygonF()
                                  << scale(QPointF(6, 8), {1, 0})
                                  << scale(QPointF(9, 5), {0, 1})
                                  << scale(QPointF(12, 8), {-1, 0}));

    painter->drawPolyline(QPolygonF()
                                  << scale(QPointF(6, 12), {1, 0})
                                  << scale(QPointF(9, 9), {0, 1})
                                  << scale(QPointF(12, 12), {-1, 0}));
}

Breeze::KeepAboveButton::KeepAboveButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                                         QObject *parent) : SolidButton(type, decoration, parent)
{

}

void Breeze::KeepBelowButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto theme = getTheme().toStrongRef();
    auto color = theme->symbolColor(state).color();

    auto symbolPen = QPen(color);
    symbolPen.setJoinStyle(Qt::MiterJoin);
    symbolPen.setWidthF(1.7 * qMax(static_cast<qreal>(1.0), 20 / this->size().width()));

    painter->setPen(symbolPen);
    painter->drawPolyline(QPolygonF()
                                  << QPointF(6, 6)
                                  << QPointF(9, 9)
                                  << QPointF(12, 6));
    painter->drawPolyline(QPolygonF()
                                  << QPointF(6, 10)
                                  << QPointF(9, 13)
                                  << QPointF(12, 10));
}

Breeze::KeepBelowButton::KeepBelowButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                                         QObject *parent) : SolidButton(type, decoration, parent)
{

}

void Breeze::ApplicationMenuButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto theme = getTheme().toStrongRef();
    auto color = theme->symbolColor(state).color();

    auto symbolPen = QPen(color);
    symbolPen.setWidthF(1.7 * qMax(static_cast<qreal>(1.0), 20 / this->size().width()));

    painter->setPen(symbolPen);
    painter->drawLine(QPointF(4.5, 6), QPointF(13.5, 6));
    painter->drawLine(QPointF(4.5, 9), QPointF(13.5, 9));
    painter->drawLine(QPointF(4.5, 12), QPointF(13.5, 12));
}

Breeze::ApplicationMenuButton::ApplicationMenuButton(KDecoration2::DecorationButtonType type,
                                                     KDecoration2::Decoration *decoration, QObject *parent)
        : SolidButton(type, decoration, parent)
{
    this->setTheme<IconOnlyTheme>();
}

Breeze::MenuButton::MenuButton(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration,
                               QObject *parent) : SolidButton(type, decoration, parent)
{
    this->setTheme<IconOnlyTheme>();
}

void Breeze::MenuButton::drawSymbol(QPainter *painter)
{
    auto state = getState();
    auto iconRect = QRectF(QPoint(-1, -1), getIconSize()).toRect();
    auto iconColor = getTheme().toStrongRef()->symbolColor(state).color();

    const QPalette activePalette = KIconLoader::global()->customPalette();

    QPalette palette = QPalette(activePalette);
    palette.setColor(QPalette::Foreground, iconColor);
    KIconLoader::global()->setCustomPalette(palette);

    state.icon.paint(painter, iconRect);
    if (activePalette == QPalette())
        KIconLoader::global()->resetPalette();
    else
        KIconLoader::global()->setCustomPalette(palette);
}

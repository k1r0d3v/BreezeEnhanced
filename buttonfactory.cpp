#include "buttonfactory.h"
#include "solidbutton.h"
#include "solidbuttons.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButton>
#include <KSharedConfig>


using KDecoration2::DecorationButtonType;

namespace Breeze
{
    KDecoration2::DecorationButton *ButtonFactory::getButtonGroupInstance(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
    {
        SolidButton *button;
        switch(type)
        {
            case DecorationButtonType::Close:
                button = new CloseButton(type, decoration, parent);
                break;
            case DecorationButtonType::Maximize:
                button = new MaximizeButton(type, decoration, parent);
                break;
            case DecorationButtonType::Minimize:
                button = new MinimizeButton(type, decoration, parent);
                break;
            case DecorationButtonType::KeepAbove:
                button = new KeepAboveButton(type, decoration, parent);
                break;
            case DecorationButtonType::KeepBelow:
                button = new KeepBelowButton(type, decoration, parent);
                break;
            case DecorationButtonType::ApplicationMenu:
                button = new ApplicationMenuButton(type, decoration, parent);
                break;
            case DecorationButtonType::Menu:
                button = new MenuButton(type, decoration, parent);
                break;
            default:
            case DecorationButtonType::ContextHelp:
            case DecorationButtonType::Shade:
            case DecorationButtonType::OnAllDesktops:
                button = new SolidButton(type, decoration, parent);
                break;
        }

        return button;
    }

    QObject *ButtonFactory::getPluginInstance(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metadata, const QVariantList &args)
    {
        Q_UNUSED(parentWidget);
        Q_UNUSED(metadata);
        return new SolidButton(parent, args);
    }
}
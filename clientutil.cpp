#include "config-breeze.h"
#include "clientutil.h"

#include <queue>

#if BREEZE_HAVE_X11
#include <QX11Info>
#include <QtX11Extras>

#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>

#include "QtX11ImageConversion.h"
#endif


ClientUtil::ClientUtil(const QWindow &window)
    : m_window(window)
{

}

QImage ClientUtil::renderToImage(int resultWidth, int resultHeight)
{
#if BREEZE_HAVE_X11
    bool x11Support = true;
#else
    bool x11Support = false;
#endif

    if (!QX11Info::isCompositingManagerRunning() || !x11Support)
    {
        qDebug() << "ClientUtil: warning: Offscreen buffer not available, falling back to screen capture";

        // This screen pointer is a smart pointer reference, we must not delete it
        auto screen = m_window.screen();
        if (screen == nullptr)
            return {};

        auto pixmap = screen->grabWindow(m_window.winId(), 0, 0, resultWidth, resultHeight);
        if (pixmap.isNull())
            return {};

        auto image = pixmap.toImage();
        if (image.isNull())
            return {};

        return image;
    }

    auto display = QX11Info::display();

    // Make sure we have the RENDER extension
    int render_event_base, render_error_base;
    if(!XRenderQueryExtension(display, &render_event_base, &render_error_base)) {
        qDebug() << "ClientUtil: error: No RENDER extension found";
        return {};
    }

    // Redirect window to an offscreen buffer
    XCompositeRedirectWindow(display, m_window.winId(), CompositeRedirectAutomatic);

    // Get the window attributes and render format of our window
    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, m_window.winId(), &attr))
    {
        // Release offscreen redirection
        XCompositeUnredirectWindow(display, m_window.winId(), CompositeRedirectAutomatic);
        return {};
    }
    XRenderPictFormat *format = XRenderFindVisualFormat(display, attr.visual);
    int width                 = attr.width;
    int height                = attr.height;

    XRenderPictureAttributes pa;
    pa.subwindow_mode = IncludeInferiors; // Don't clip child widgets

    //
    auto windowId = m_window.winId();
    auto windowPicture = XRenderCreatePicture(display, windowId, format, CPSubwindowMode, &pa);
    if (windowPicture == None)
    {
        qDebug() << "ClientUtil: XRenderCreatePicture error: Could not create the window picture";
        return {};
    }

    if (resultWidth >= 0)
        width = std::max(1, std::min(width, resultWidth));

    if (resultHeight >= 0)
        height = std::max(1, std::min(height, resultHeight));

    bool hasAlpha             = format->type == PictTypeDirect && format->direct.alphaMask;
    int depth                 = attr.depth;

    // Create a temporal picture to render the window
    auto windowTmpPixmap = XCreatePixmap(display, windowId, width, height, depth);
    if (windowTmpPixmap == None)
    {
        XRenderFreePicture(display, windowPicture);
        qDebug() << "ClientUtil: XCreatePixmap error: Failed to create pixmap";
        return {};
    }

    auto windowTmpPicture = XRenderCreatePicture(display, windowTmpPixmap, format, None, &pa);
    if (windowTmpPicture == None)
    {
        XRenderFreePicture(display, windowPicture);
        XFreePixmap(display, windowTmpPixmap);
        qDebug() << "ClientUtil: XRenderCreatePicture error: Failed to create picture";
        return {};
    }

    // Render and tell the server to complete the operation
    XRenderComposite(display, hasAlpha ? PictOpOver : PictOpSrc, windowPicture, None, windowTmpPicture, 0, 0, 0, 0, 0, 0, width, height);
    //XFlush(display);

    //
    auto windowResultImage = XGetImage(display, windowTmpPixmap, 0, 0, width, height, AllPlanes, ZPixmap);
    if (windowResultImage == None)
    {
        // Release offscreen redirection
        XCompositeUnredirectWindow(display, m_window.winId(), CompositeRedirectAutomatic);

        XRenderFreePicture(display, windowPicture);
        XFreePixmap(display, windowTmpPixmap);
        XRenderFreePicture(display, windowTmpPicture);
        qDebug() << "ClientUtil: XGetImage error: Invalid render result";
        return {};
    }

    // Release offscreen redirection
    XCompositeUnredirectWindow(display, m_window.winId(), CompositeRedirectAutomatic);

    //
    QImage image = qimageFromXImage(windowResultImage);

    // Free resources
    XRenderFreePicture(display, windowPicture);
    XFreePixmap(display, windowTmpPixmap);
    XRenderFreePicture(display, windowTmpPicture);
    XDestroyImage(windowResultImage);

    return image;
}

QColor ClientUtil::topLineColor()
{
    auto image = renderToImage(-1, 2);
    if (image.isNull())
        return {};

    // Get the color from the mode of the first two rows of the client pixels
    QMap<QRgb, int> colorCount;
    QPair<QRgb, int> modeColor = {0, 0};
    for (int j = 0; j < image.height() ; ++j)
    {
        for (int i = 0; i < image.width(); ++i)
        {
            auto color = image.pixelColor(i, j).rgb();

            if (!colorCount.contains(color))
                colorCount[color] = 1;
            else
                colorCount[color] += 1;

            auto count = colorCount[color];
            if (count > modeColor.second)
                modeColor = {color, count};
        }
    }

    return QColor::fromRgb(modeColor.first);
}

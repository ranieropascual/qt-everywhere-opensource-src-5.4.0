/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQuick.Dialogs module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickplatformfontdialog_p.h"
#include "qquickitem.h"

#include <private/qguiapplication_p.h>
#include <QWindow>
#include <QQuickView>
#include <QQuickWindow>

QT_BEGIN_NAMESPACE

/*!
    \qmltype FontDialog
    \instantiates QQuickPlatformFontDialog
    \inqmlmodule QtQuick.Dialogs 1
    \ingroup qtquick-visual
    \ingroup dialogs
    \brief Dialog component for choosing a font.
    \since 5.2

    FontDialog allows the user to select a font. The dialog is initially
    invisible. You need to set the properties as desired first, then set
    \l visible to true or call \l open().

    Here is a minimal example to open a font dialog and exit after the user
    chooses a font:

    \qml
    import QtQuick 2.2
    import QtQuick.Dialogs 1.1

    FontDialog {
        id: fontDialog
        title: "Please choose a font"
        font: Qt.font({ family: "Arial", pointSize: 24, weight: Font.Normal })
        onAccepted: {
            console.log("You chose: " + fontDialog.font)
            Qt.quit()
        }
        onRejected: {
            console.log("Canceled")
            Qt.quit()
        }
        Component.onCompleted: visible = true
    }
    \endqml

    A FontDialog window is automatically transient for its parent window. So
    whether you declare the dialog inside an \l Item or inside a \l Window, the
    dialog will appear centered over the window containing the item, or over
    the Window that you declared.

    The implementation of FontDialog will be a platform font dialog if
    possible. If that isn't possible, then it will try to instantiate a
    \l QFontDialog. If that also isn't possible, then it will fall back to a QML
    implementation, DefaultFontDialog.qml. In that case you can customize the
    appearance by editing this file. DefaultFontDialog.qml contains a Rectangle
    to hold the dialog's contents, because certain embedded systems do not
    support multiple top-level windows. When the dialog becomes visible, it
    will automatically be wrapped in a Window if possible, or simply reparented
    on top of the main window if there can only be one window.
*/

/*!
    \qmlsignal QtQuick::Dialogs::FontDialog::accepted

    The \a accepted signal is emitted when the user has finished using the
    dialog. You can then inspect the \a font property to get the selection.

    Example:

    \qml
    FontDialog {
        onAccepted: { console.log("Selected font: " + font) }
    }
    \endqml

    The corresponding handler is \c onAccepted.
*/

/*!
    \qmlsignal QtQuick::Dialogs::FontDialog::rejected

    The \a rejected signal is emitted when the user has dismissed the dialog,
    either by closing the dialog window or by pressing the Cancel button.

    The corresponding handler is \c onRejected.
*/

/*!
    \class QQuickPlatformFontDialog
    \inmodule QtQuick.Dialogs
    \internal

    \brief The QQuickPlatformFontDialog class provides a font dialog

    The dialog is implemented via the QQuickPlatformFontDialogHelper when possible;
    otherwise it falls back to a QFontDialog or a QML implementation.

    \since 5.2
*/

/*!
    Constructs a file dialog with parent window \a parent.
*/
QQuickPlatformFontDialog::QQuickPlatformFontDialog(QObject *parent) :
    QQuickAbstractFontDialog(parent)
{
}

/*!
    Destroys the file dialog.
*/
QQuickPlatformFontDialog::~QQuickPlatformFontDialog()
{
    if (m_dlgHelper)
        m_dlgHelper->hide();
    delete m_dlgHelper;
}

QPlatformFontDialogHelper *QQuickPlatformFontDialog::helper()
{
    QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
    if (parentItem)
        m_parentWindow = parentItem->window();

    if ( !m_dlgHelper && QGuiApplicationPrivate::platformTheme()->
            usePlatformNativeDialog(QPlatformTheme::FontDialog) ) {
        m_dlgHelper = static_cast<QPlatformFontDialogHelper *>(QGuiApplicationPrivate::platformTheme()
           ->createPlatformDialogHelper(QPlatformTheme::FontDialog));
        if (!m_dlgHelper)
            return m_dlgHelper;
        connect(m_dlgHelper, SIGNAL(accept()), this, SLOT(accept()));
        connect(m_dlgHelper, SIGNAL(reject()), this, SLOT(reject()));
        connect(m_dlgHelper, SIGNAL(currentFontChanged(QFont)), this, SLOT(setFont(QFont)));
        connect(m_dlgHelper, SIGNAL(fontSelected(QFont)), this, SLOT(setFont(QFont)));
    }

    return m_dlgHelper;
}

/*!
    \qmlproperty bool FontDialog::visible

    This property holds whether the dialog is visible. By default this is
    false.

    \sa modality
*/

/*!
    \qmlproperty Qt::WindowModality FontDialog::modality

    Whether the dialog should be shown modal with respect to the window
    containing the dialog's parent Item, modal with respect to the whole
    application, or non-modal.

    By default it is \c Qt.WindowModal.

    Modality does not mean that there are any blocking calls to wait for the
    dialog to be accepted or rejected; it's only that the user will be
    prevented from interacting with the parent window and/or the application
    windows at the same time.

    You probably need to write an onAccepted handler if you wish to change a
    font after the user has pressed the OK button, or an onCurrentFontChanged
    handler if you wish to react to every change the user makes while the
    dialog is open.
*/

/*!
    \qmlmethod void FontDialog::open()

    Shows the dialog to the user. It is equivalent to setting \l visible to
    true.
*/

/*!
    \qmlmethod void FontDialog::close()

    Closes the dialog.
*/

/*!
    \qmlproperty string FontDialog::title

    The title of the dialog window.
*/

/*!
    \qmlproperty bool FontDialog::scalableFonts

    Whether the dialog will show scalable fonts or not.
*/

/*!
    \qmlproperty bool FontDialog::nonScalableFonts

    Whether the dialog will show non scalable fonts or not.
*/

/*!
    \qmlproperty bool FontDialog::monospacedFonts

    Whether the dialog will show monospaced fonts or not.
*/

/*!
    \qmlproperty bool FontDialog::proportionalFonts

    Whether the dialog will show proportional fonts or not.
*/

/*!
    \qmlproperty font FontDialog::font

    The font which the user selected and accepted.
*/

/*!
    \qmlproperty font FontDialog::currentFont

    The font which the user selected.

    \since 5.3
*/

QT_END_NAMESPACE

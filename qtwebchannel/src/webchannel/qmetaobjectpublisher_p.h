/****************************************************************************
**
** Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
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

#ifndef QMETAOBJECTPUBLISHER_P_H
#define QMETAOBJECTPUBLISHER_P_H

#include "variantargument_p.h"
#include "signalhandler_p.h"

#include <QStringList>
#include <QMetaObject>
#include <QBasicTimer>

#include "qwebchannelglobal.h"

QT_BEGIN_NAMESPACE

// NOTE: keep in sync with corresponding maps in qwebchannel.js and WebChannelTest.qml
enum MessageType {
    TypeInvalid = 0,

    TYPES_FIRST_VALUE = 1,

    TypeSignal = 1,
    TypePropertyUpdate = 2,
    TypeInit = 3,
    TypeIdle = 4,
    TypeDebug = 5,
    TypeInvokeMethod = 6,
    TypeConnectToSignal = 7,
    TypeDisconnectFromSignal = 8,
    TypeSetProperty = 9,
    TypeResponse = 10,

    TYPES_LAST_VALUE = 10
};

class QWebChannel;
class QWebChannelAbstractTransport;
class Q_WEBCHANNEL_EXPORT QMetaObjectPublisher : public QObject
{
    Q_OBJECT

public:
    explicit QMetaObjectPublisher(QWebChannel *webChannel);
    virtual ~QMetaObjectPublisher();

    /**
     * Register @p object nuder the given @p id.
     *
     * The properties, signals and public methods of the QObject are
     * published to the remote client, where an object with the given identifier
     * is constructed.
     *
     * TODO: This must be called, before clients are initialized.
     */
    void registerObject(const QString &id, QObject *object);

    /**
     * Send the given message to all known transports.
     */
    void broadcastMessage(const QJsonObject &message) const;

    /**
     * Serialize the QMetaObject of @p object and return it in JSON form.
     */
    QJsonObject classInfoForObject(const QObject *object) const;

    /**
     * Set the client to idle or busy, based on the value of @p isIdle.
     *
     * When the value changed, start/stop the property update timer accordingly.
     */
    void setClientIsIdle(bool isIdle);

    /**
     * Initialize clients by sending them the class information of the registered objects.
     *
     * Furthermore, if that was not done already, connect to their property notify signals.
     */
    void initializeClients();

    /**
     * Go through all properties of the given object and connect to their notify signal.
     *
     * When receiving a notify signal, it will store the information in pendingPropertyUpdates which
     * gets send via a Qt.propertyUpdate message to the server when the grouping timer timeouts.
     */
    void initializePropertyUpdates(const QObject *const object, const QJsonObject &objectInfo);

    /**
     * Send the clients the new property values since the last time this function was invoked.
     *
     * This is a grouped batch of all properties for which their notify signal was emitted.
     * The list of signals as well as the arguments they contained, are also transmitted to
     * the remote clients.
     *
     * @sa timer, initializePropertyUpdates
     */
    void sendPendingPropertyUpdates();

    /**
     * Invoke the method of index @p methodIndex on @p object with the arguments @p args.
     *
     * The return value of the method invocation is then serialized and a response message
     * is returned.
     */
    QJsonValue invokeMethod(QObject *const object, const int methodIndex, const QJsonArray &args);

    /**
     * Callback of the signalHandler which forwards the signal invocation to the webchannel clients.
     */
    void signalEmitted(const QObject *object, const int signalIndex, const QVariantList &arguments);

    /**
     * Callback for registered or wrapped objects which erases all data related to @p object.
     *
     * @sa signalEmitted
     */
    void objectDestroyed(const QObject *object);

    /**
     * Given a QVariant containing a QObject*, wrap the object and register for property updates
     * return the objects class information.
     *
     * All other input types are returned as-is.
     *
     * TODO: support wrapping of initially-registered objects
     */
    QJsonValue wrapResult(const QVariant &result);

    /**
     * Invoke delete later on @p object.
     */
    void deleteWrappedObject(QObject *object) const;

    /**
     * When updates are blocked, no property updates are transmitted to remote clients.
     */
    void setBlockUpdates(bool block);

Q_SIGNALS:
    void blockUpdatesChanged(bool block);

public Q_SLOTS:
    /**
     * Handle the @p message and if needed send a response to @p transport.
     */
    void handleMessage(const QJsonObject &message, QWebChannelAbstractTransport *transport);

protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

private:
    friend class QQmlWebChannelPrivate;
    friend class QWebChannel;
    friend class TestWebChannel;

    QWebChannel *webChannel;
    SignalHandler<QMetaObjectPublisher> signalHandler;

    // true when the client is idle, false otherwise
    bool clientIsIdle;

    // true when no property updates should be sent, false otherwise
    bool blockUpdates;

    // true when at least one client needs to be initialized,
    // i.e. when a Qt.init came in which was not handled yet.
    bool pendingInit;

    // true when at least one client was initialized and thus
    // the property updates have been initialized and the
    // object info map set.
    bool propertyUpdatesInitialized;

    // Map of registered objects indexed by their id.
    QHash<QString, QObject *> registeredObjects;

    // Map the registered objects to their id.
    QHash<const QObject *, QString> registeredObjectIds;

    // Map of objects to maps of signal indices to a set of all their property indices.
    // The last value is a set as a signal can be the notify signal of multiple properties.
    typedef QHash<int, QSet<int> > SignalToPropertyNameMap;
    QHash<const QObject *, SignalToPropertyNameMap> signalToPropertyMap;

    // Objects that changed their properties and are waiting for idle client.
    // map of object name to map of signal index to arguments
    typedef QHash<int, QVariantList> SignalToArgumentsMap;
    typedef QHash<const QObject *, SignalToArgumentsMap> PendingPropertyUpdates;
    PendingPropertyUpdates pendingPropertyUpdates;

    // Maps wrapped object to class info
    QHash<const QObject *, QJsonObject> wrappedObjects;

    // Aggregate property updates since we get multiple Qt.idle message when we have multiple
    // clients. They all share the same QWebProcess though so we must take special care to
    // prevent message flooding.
    QBasicTimer timer;
};

QT_END_NAMESPACE

#endif // QMETAOBJECTPUBLISHER_P_H

/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QAUDIOOUTPUTALSA_H
#define QAUDIOOUTPUTALSA_H

#include <alsa/asoundlib.h>

#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qdatetime.h>

#include <QtMultimedia/qaudio.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudiosystem.h>

QT_BEGIN_NAMESPACE

class QAlsaAudioOutput : public QAbstractAudioOutput
{
    friend class OutputPrivate;
    Q_OBJECT
public:
    QAlsaAudioOutput(const QByteArray &device);
    ~QAlsaAudioOutput();

    qint64 write( const char *data, qint64 len );

    void start(QIODevice* device);
    QIODevice* start();
    void stop();
    void reset();
    void suspend();
    void resume();
    int bytesFree() const;
    int periodSize() const;
    void setBufferSize(int value);
    int bufferSize() const;
    void setNotifyInterval(int milliSeconds);
    int notifyInterval() const;
    qint64 processedUSecs() const;
    qint64 elapsedUSecs() const;
    QAudio::Error error() const;
    QAudio::State state() const;
    void setFormat(const QAudioFormat& fmt);
    QAudioFormat format() const;
    void setVolume(qreal);
    qreal volume() const;


    QIODevice* audioSource;
    QAudioFormat settings;
    QAudio::Error errorState;
    QAudio::State deviceState;

private slots:
    void userFeed();
    bool deviceReady();

signals:
    void processMore();

private:
    bool opened;
    bool pullMode;
    bool resuming;
    int buffer_size;
    int period_size;
    int intervalTime;
    qint64 totalTimeValue;
    unsigned int buffer_time;
    unsigned int period_time;
    snd_pcm_uframes_t buffer_frames;
    snd_pcm_uframes_t period_frames;
    int xrun_recovery(int err);

    int setFormat();
    bool open();
    void close();

    QTimer* timer;
    QByteArray m_device;
    int bytesAvailable;
    QTime timeStamp;
    QTime clockStamp;
    qint64 elapsedTimeOffset;
    char* audioBuffer;
    snd_pcm_t* handle;
    snd_pcm_access_t access;
    snd_pcm_format_t pcmformat;
    snd_timestamp_t* timestamp;
    snd_pcm_hw_params_t *hwparams;
    qreal m_volume;
};

class OutputPrivate : public QIODevice
{
    friend class QAlsaAudioOutput;
    Q_OBJECT
public:
    OutputPrivate(QAlsaAudioOutput* audio);
    ~OutputPrivate();

    qint64 readData( char* data, qint64 len);
    qint64 writeData(const char* data, qint64 len);

private:
    QAlsaAudioOutput *audioDevice;
};

QT_END_NAMESPACE


#endif

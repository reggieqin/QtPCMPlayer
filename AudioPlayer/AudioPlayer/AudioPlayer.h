#pragma once

#include <QtWidgets/QMainWindow>
#include <qfile.h>
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#include <qiodevice.h>
#include <qsound.h>

#include "ui_AudioPlayer.h"

class AudioPlayer : public QMainWindow
{
    Q_OBJECT

public:
    AudioPlayer(QWidget *parent = Q_NULLPTR);

public slots:
    void play();
    void pauseresume();
    void handleStateChanged(QAudio::State newState);
    void notify();
    void rewind();
    void fastforward();

private:
    Ui::AudioPlayerClass ui;

    QAudioOutput* audio; // class member.
    QIODevice* qAudioDevice;
    QByteArray *bytes;
    QSound *bells;

    qint64 lastByte;
};

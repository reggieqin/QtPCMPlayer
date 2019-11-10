
#include <qdebug.h>
#include "AudioPlayer.h"


AudioPlayer::AudioPlayer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    audio = nullptr;
    lastByte = 0;
}

void AudioPlayer::play()
{
    QFile sourceFile("C:\\Users\\regqin\\source\\qt\\AudioPlayer\\x64\\Debug\\Ringtone.wav");
    sourceFile.open(QIODevice::ReadOnly);

    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }

    if (audio)
    {
        delete audio;
        audio = nullptr;
    }

    audio = new QAudioOutput(format, this);
    audio->setNotifyInterval(10);
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    connect(audio, SIGNAL(notify()), this, SLOT(notify()));
    qAudioDevice = audio->start();

    bytes = sourceFile.readAll();
    lastByte = 0;

    int chunks = audio->bytesFree() / audio->periodSize();
    int periodSize = audio->periodSize();
    while (chunks) {
        qAudioDevice->write(bytes.data() + lastByte, audio->periodSize());
        lastByte = lastByte + periodSize;
        --chunks;
    }

    sourceFile.close();
}

void AudioPlayer::pauseresume()
{
    auto state = audio->state();
    if (state == QAudio::ActiveState)
    {
        audio->suspend();
    }
    else if (state == QAudio::SuspendedState)
    {
        audio->resume();
    }
}

void AudioPlayer::handleStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
        {
            // Finished playing (no more data)
            qInfo("Idle");
            //audio->stop();
            //delete audio;
            break;
        }

        case QAudio::StoppedState:
            qInfo("Stopped");
            // Stopped for other reasons
            if (audio->error() != QAudio::NoError) {
                // Error handling
            }
            break;
        case QAudio::ActiveState:
            qInfo("Playing");
            break;
        case QAudio::SuspendedState:
            qInfo("Suspended");
            break;

        default:
            // ... other cases as appropriate
            break;
    }
}

void AudioPlayer::notify()
{
    if (lastByte < 0)
        lastByte = 0;

    int chunks = audio->bytesFree() / audio->periodSize();
    int periodSize = audio->periodSize();
    while (chunks) {
        if (lastByte + periodSize > bytes.size())
            break;

        qAudioDevice->write(bytes.data() + lastByte, audio->periodSize());
        lastByte = lastByte + periodSize;
        --chunks;
    }

}

void AudioPlayer::rewind()
{
    lastByte -= bytes.size() / 10;
}

void AudioPlayer::fastforward()
{
    lastByte += bytes.size() / 10;
}
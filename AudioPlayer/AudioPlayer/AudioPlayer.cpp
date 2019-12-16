
#include <qdebug.h>
#include <qsound.h>
#include <qsoundeffect.h>
#include "AudioPlayer.h"

short ulaw2linear(char ulawbyte)
{
    static int exp_lut[8] = { 0,132,396,924,1980,4092,8316,16764 };

    int sign, exponent, mantissa, sample;

    ulawbyte = ~ulawbyte;
    sign = (ulawbyte & 0x80);
    exponent = (ulawbyte >> 4) & 0x07;
    mantissa = ulawbyte & 0x0F;
    sample = exp_lut[exponent] + (mantissa << (exponent + 3));
    if (sign != 0) sample = -sample;

    return (short)sample;
}

AudioPlayer::AudioPlayer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    audio = nullptr;
    lastByte = 0;
    bells = new QSound("C:/Users/regqin/source/qt/AudioPlayer/x64/Debug/test.wav");
}

void AudioPlayer::play()
{
    //bells->play();

    QFile sourceFile("C:\\Users\\regqin\\source\\qt\\AudioPlayer\\x64\\Debug\\test.wav");
    sourceFile.open(QIODevice::ReadOnly);

    QAudioFormat format;
    // Set up the format, eg.
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    auto list = QSoundEffect::supportedMimeTypes();
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

    auto b = sourceFile.readAll();
    short *s = new short[b.size()];
    for (int i = 0; i < b.size(); i++)
    {
        s[i] = ulaw2linear(*(b.data() + i));
    }
    bytes = new QByteArray((char*)s, b.size() * 2);
    
    lastByte = 0;

    int chunks = audio->bytesFree() / audio->periodSize();
    int periodSize = audio->periodSize();
    while (chunks) {
        qAudioDevice->write(bytes->data() + lastByte, audio->periodSize());
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
        if (lastByte + periodSize > bytes->size())
            break;

        qAudioDevice->write(bytes->data() + lastByte, audio->periodSize());
        lastByte = lastByte + periodSize;
        --chunks;
    }

}

void AudioPlayer::rewind()
{
    lastByte -= bytes->size() / 10;
}

void AudioPlayer::fastforward()
{
    lastByte += bytes->size() / 10;
}
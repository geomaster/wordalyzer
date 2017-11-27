#include <SFML/Audio.hpp>
#include <thread>
#include <iostream>

#include "record.hpp"

using namespace wordalyzer;
using namespace std;

audio_t wordalyzer::record_audio()
{
    if (!sf::SoundBufferRecorder::isAvailable()) {
        throw recording_exception("Audio recording is not supported.");
    }

    sf::SoundBufferRecorder recorder;
    cout << "[*] About to record audio. Press ENTER when ready.";
    cin.get();

    for (int i = 3; i >= 1; i--) {
        cout << "[|] Recording in " << i << "..." << endl;
        std::this_thread::sleep_for(1s);
    }

    recorder.start();
    cout << "[*] Recording in progress. Press ENTER to stop.";
    cin.get();
    recorder.stop();

    audio_t res;
    const sf::SoundBuffer& buf = recorder.getBuffer();
    const sf::Int16* samples = buf.getSamples();
    size_t sample_count = buf.getSampleCount();

    res.samples.resize(sample_count);
    res.sample_rate = buf.getSampleRate();

    for (size_t i = 0; i < sample_count; i++) {
        res.samples[i] = samples[i] / 32767.0f;
    }

    cout << "[+] Successfully recorded " << res.samples_to_ms(sample_count) << "ms" << endl;

    return res;
}

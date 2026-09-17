typedef struct Note {
    int freq = 440;
    int dur = 1000;
    float vol = 1.0;
} Note;

class sound {
private:
    Note *channels = nullptr;
    int chanCount = 0;

public:
    sound(int count) {
        chanCount = count;
        channels = new Note[chanCount];
    }

    ~sound() {
        delete[] channels;
    }

    int setChannel(int channel, int freq, int dur, float vol) { 
        if (channel >= 0 && channel < chanCount) {
            channels[channel].freq = freq;
            channels[channel].dur = dur;
            channels[channel].vol = vol;
            return 0;
        }
        return 1;
    }

    void flush() {
        for (int i = 0; i < chanCount; i++) {
            int freq = channels[i].freq;
            int dur = channels[i].dur;

            std::this_thread::sleep_for(std::chrono::milliseconds(dur + 100));
            std::thread([freq, dur]() {
                Beep(freq, dur);
            }).detach();
        }
    }
};
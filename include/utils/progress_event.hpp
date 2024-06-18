#pragma once
class ProgressEvent{
private:
    ProgressEvent() {}
    int _current = 0;
    int _target = 0;
    int _max = 60;

public:
    ProgressEvent(const ProgressEvent&) = delete;
    ProgressEvent& operator=(const ProgressEvent &) = delete;
    ProgressEvent(ProgressEvent &&) = delete;
    ProgressEvent & operator=(ProgressEvent &&) = delete;

    static auto& instance(){
        static ProgressEvent event;
        return event;
    }

    int increment() {
        if (_current < _target) {
            ++_current;
        }
        return _current;
    }

    void reset() {
        _current = 0;
        _target = 0;
        _max = 60;
    }

    inline void setTotalSteps(int steps) { _max = steps*5; }
    inline void setStep(int step) { _target = step*5; }
    inline bool finished() { return _current == _max; }
    inline int getMax() { return _max; }
};

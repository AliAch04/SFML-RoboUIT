#pragma once
#include <vector>
#include <deque>
#include <random>
#include <algorithm>
#include <memory>

struct Experience {
    std::vector<double> state;
    int action;
    double reward;
    std::vector<double> nextState;
    bool terminal;

    Experience(const std::vector<double>& s, int a, double r,
        const std::vector<double>& ns, bool term)
        : state(s), action(a), reward(r), nextState(ns), terminal(term) {
    }
};

class ExperienceReplay {
private:
    std::deque<Experience> buffer;
    size_t maxSize;
    size_t batchSize;
    std::mt19937 rng;

public:
    ExperienceReplay(size_t maxSize = 10000, size_t batchSize = 32);

    void addExperience(const Experience& experience);
    void addExperience(const std::vector<double>& state, int action, double reward,
        const std::vector<double>& nextState, bool terminal);

    std::vector<Experience> sampleBatch();

    // Prioritized Experience Replay (optionnel)
    std::vector<Experience> samplePrioritizedBatch();
    void updatePriority(const Experience& experience, double priority);

    // Getters
    size_t size() const { return buffer.size(); }
    size_t capacity() const { return maxSize; }
    bool isFull() const { return buffer.size() >= maxSize; }

    // Clear buffer
    void clear() { buffer.clear(); }

    // Save/load
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
};